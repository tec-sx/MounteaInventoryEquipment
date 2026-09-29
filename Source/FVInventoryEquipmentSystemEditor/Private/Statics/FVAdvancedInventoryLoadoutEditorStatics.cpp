// Copyright (C) 2025 Dominik (Pavlicek) Morse. All rights reserved.
//
// Developed for the Mountea Framework as a free tool. This solution is provided
// for use and sharing without charge. Redistribution is allowed under the following conditions:
//
// - You may use this solution in commercial products, provided the product is not
//   this solution itself (or unless significant modifications have been made to the solution).
// - You may not resell or redistribute the original, unmodified solution.
//
// For more information, visit: https://mountea.tools

#include "Statics/FVAdvancedInventoryLoadoutEditorStatics.h"

#include "Helpers/FVInventoryImportHelpers.h"
#include "Helpers/FVInventoryZipHelper.h"
#include "Statics/FVAdvancedInventoryItemTemplateEditorStatics.h"

#include "Definitions/FVAdvancedInventoryLoadoutConfig.h"
#include "Definitions/FVAdvancedInventoryLoadoutItem.h"
#include "Definitions/FVInventoryItemTemplate.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Dom/JsonObject.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/SavePackage.h"

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool UFVAdvancedInventoryLoadoutEditorStatics::ImportLoadoutsFromFilePath(
	const FString& FilePath,
	const FString& TargetFolder,
	TArray<UFVAdvancedInventoryLoadoutConfig*>& OutLoadouts,
	FString& OutErrorMessage)
{
	TArray<uint8> fileBytes;
	if (!FFVInventoryZipHelper::LoadFileToBytes(FilePath, fileBytes))
	{
		OutErrorMessage = FString::Printf(TEXT("Failed to read file: %s"), *FilePath);
		return false;
	}

	if (!FFVInventoryZipHelper::IsZipFile(fileBytes))
	{
		OutErrorMessage = TEXT("File is not a valid ZIP archive");
		return false;
	}

	// Expand bundle or treat as single
	TArray<TArray<uint8>> singleZips;
	if (FilePath.EndsWith(TEXT(".mntealoadouts")))
	{
		if (!FFVInventoryZipHelper::ExpandBundle(fileBytes, TEXT("mntealoadout"), singleZips))
		{
			OutErrorMessage = TEXT("Failed to expand .mntealoadouts bundle");
			return false;
		}
	}
	else
		singleZips.Add(MoveTemp(fileBytes));

	// ── Phase 1 & 2: Read and validate all ZIPs before writing anything ──────

	struct FParsedLoadout
	{
		FString LoadoutJson;
		TArray<FString> BundledItemJsons;
		TSharedPtr<FJsonObject> ParsedJson;
		TSet<FGuid> BundledGuids;
	};

	TArray<FParsedLoadout> parsed;
	parsed.Reserve(singleZips.Num());

	const TMap<FGuid, UFVInventoryItemTemplate*> projectItems =
		FFVInventoryImportHelpers::BuildProjectItemGuidMap();

	for (const TArray<uint8>& zipBlob : singleZips)
	{
		FParsedLoadout entry;

		FString parseError;
		if (!ParseLoadoutFromZip(zipBlob, entry.LoadoutJson, entry.BundledItemJsons, parseError))
		{
			OutErrorMessage += parseError + TEXT("\n");
			return false;
		}

		TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(entry.LoadoutJson);
		if (!FJsonSerializer::Deserialize(reader, entry.ParsedJson) || !entry.ParsedJson.IsValid())
		{
			OutErrorMessage += TEXT("Failed to parse loadout JSON\n");
			return false;
		}

		entry.BundledGuids = FFVInventoryImportHelpers::ParseBundledGuids(entry.BundledItemJsons);

		// Build combined set: bundled + project
		TSet<FGuid> knownGuids = entry.BundledGuids;
		for (const auto& kv : projectItems)
			knownGuids.Add(kv.Key);

		FString validationError;
		if (!ValidateLoadout(entry.ParsedJson, knownGuids, validationError))
		{
			OutErrorMessage += validationError + TEXT("\n");
			return false;
		}

		parsed.Add(MoveTemp(entry));
	}

	// ── Phase 3: Import bundled items, then persist loadouts ─────────────────

	TArray<UFVAdvancedInventoryLoadoutConfig*> existingLoadouts = LoadAllExistingLoadouts();

	for (FParsedLoadout& entry : parsed)
	{
		const FString folderToUse = TargetFolder.IsEmpty() ? TEXT("/Game/") : TargetFolder;

		TMap<FGuid, UFVInventoryItemTemplate*> itemsByGuid = projectItems;
		FFVInventoryImportHelpers::ImportBundledItems(entry.BundledItemJsons, folderToUse, itemsByGuid);

		// Find or create loadout asset
		const FGuid loadoutGuid = ExtractGuidFromLoadoutJson(entry.LoadoutJson);
		UFVAdvancedInventoryLoadoutConfig* loadout = FindLoadoutByGuid(existingLoadouts, loadoutGuid);

		if (loadout)
		{
			FString updateError;
			if (UpdateExistingLoadout(loadout, entry.ParsedJson, itemsByGuid, updateError))
				OutLoadouts.Add(loadout);
			else
				OutErrorMessage += updateError + TEXT("\n");
		}
		else
		{
			const FString name = ExtractNameFromLoadoutJson(entry.LoadoutJson);
			const FString assetName = FString::Printf(TEXT("ImportedLoadout_%s"),
				name.IsEmpty() ? *loadoutGuid.ToString(EGuidFormats::Short) : *name);

			FString createError;
			loadout = CreateLoadoutAsset(folderToUse, assetName, createError);
			if (!loadout)
			{
				OutErrorMessage += createError + TEXT("\n");
				continue;
			}

			FString deserError;
			if (DeserializeLoadoutFromJson(entry.ParsedJson, loadout, itemsByGuid, deserError))
			{
				const FString pkgPath = FPaths::Combine(folderToUse, assetName);
				if (SaveLoadoutAsset(loadout, pkgPath))
					OutLoadouts.Add(loadout);
			}
			else
				OutErrorMessage += deserError + TEXT("\n");
		}
	}

	if (OutLoadouts.Num() == 0)
	{
		OutErrorMessage = TEXT("No loadouts were successfully imported");
		return false;
	}

	OutErrorMessage = FString::Printf(TEXT("Loadout import complete: %d imported"), OutLoadouts.Num());
	return true;
}

// ---------------------------------------------------------------------------
// ZIP parsing
// ---------------------------------------------------------------------------

bool UFVAdvancedInventoryLoadoutEditorStatics::ParseLoadoutFromZip(
	const TArray<uint8>& ZipBytes,
	FString& OutLoadoutJson,
	TArray<FString>& OutBundledItemJsons,
	FString& OutErrorMessage)
{
	TMap<FString, FString> textFiles;
	TMap<FString, TArray<uint8>> binaryFiles;

	if (!FFVInventoryZipHelper::ExtractEntries(ZipBytes, textFiles, binaryFiles))
	{
		OutErrorMessage = TEXT("Failed to extract ZIP entries");
		return false;
	}

	const FString* loadoutJson = textFiles.Find(TEXT("loadout.json"));
	if (!loadoutJson)
	{
		OutErrorMessage = TEXT("loadout.json not found in ZIP");
		return false;
	}

	OutLoadoutJson = *loadoutJson;

	for (const auto& kv : textFiles)
	{
		if (kv.Key.StartsWith(TEXT("items/")) && kv.Key.EndsWith(TEXT(".json")))
			OutBundledItemJsons.Add(kv.Value);
	}

	return true;
}

// ---------------------------------------------------------------------------
// Validation
// ---------------------------------------------------------------------------

bool UFVAdvancedInventoryLoadoutEditorStatics::ValidateLoadout(
	const TSharedPtr<FJsonObject>& LoadoutJson,
	const TSet<FGuid>& KnownGuids,
	FString& OutErrorMessage)
{
	FString guidStr;
	if (!LoadoutJson->TryGetStringField(TEXT("guid"), guidStr) || guidStr.IsEmpty())
	{
		OutErrorMessage = TEXT("Loadout JSON missing 'guid' field");
		return false;
	}

	// Validate items[] array
	const TArray<TSharedPtr<FJsonValue>>* itemsArray;
	if (LoadoutJson->TryGetArrayField(TEXT("items"), itemsArray))
	{
		for (const TSharedPtr<FJsonValue>& itemVal : *itemsArray)
		{
			const TSharedPtr<FJsonObject>* itemObj;
			if (!itemVal->TryGetObject(itemObj))
				continue;

			const TSharedPtr<FJsonObject>* refObj;
			if ((*itemObj)->TryGetObjectField(TEXT("ref"), refObj))
			{
				FString refGuid;
				if ((*refObj)->TryGetStringField(TEXT("guid"), refGuid))
				{
					FString checkError;
					if (!FFVInventoryImportHelpers::CheckGuidInKnownSet(
						refGuid, TEXT("items[]"), KnownGuids, checkError))
					{
						OutErrorMessage = checkError;
						return false;
					}
				}
			}
		}
	}

	// Validate slots{} map
	const TSharedPtr<FJsonObject>* slotsObj;
	if (LoadoutJson->TryGetObjectField(TEXT("slots"), slotsObj))
	{
		for (const auto& slotKv : (*slotsObj)->Values)
		{
			const TSharedPtr<FJsonObject>* slotVal;
			if (!slotKv.Value->TryGetObject(slotVal))
				continue;

			FString slotGuid;
			if ((*slotVal)->TryGetStringField(TEXT("guid"), slotGuid))
			{
				FString checkError;
				if (!FFVInventoryImportHelpers::CheckGuidInKnownSet(
					slotGuid, FString::Printf(TEXT("slots[%s]"), *slotKv.Key), KnownGuids, checkError))
				{
					OutErrorMessage = checkError;
					return false;
				}
			}
		}
	}

	return true;
}

// ---------------------------------------------------------------------------
// Asset creation & serialization
// ---------------------------------------------------------------------------

UFVAdvancedInventoryLoadoutConfig* UFVAdvancedInventoryLoadoutEditorStatics::CreateLoadoutAsset(
	const FString& TargetFolder,
	const FString& AssetName,
	FString& OutErrorMessage)
{
	const FString packageName = FPaths::Combine(TargetFolder, AssetName);
	UPackage* newPackage = CreatePackage(*packageName);

	if (!newPackage)
	{
		OutErrorMessage = FString::Printf(TEXT("Failed to create package: %s"), *packageName);
		return nullptr;
	}

	UFVAdvancedInventoryLoadoutConfig* loadout = NewObject<UFVAdvancedInventoryLoadoutConfig>(
		newPackage,
		UFVAdvancedInventoryLoadoutConfig::StaticClass(),
		*AssetName,
		RF_Public | RF_Standalone
	);

	if (!loadout)
		OutErrorMessage = TEXT("Failed to create loadout config object");

	return loadout;
}

bool UFVAdvancedInventoryLoadoutEditorStatics::DeserializeLoadoutFromJson(
	const TSharedPtr<FJsonObject>& JsonObject,
	UFVAdvancedInventoryLoadoutConfig* Loadout,
	const TMap<FGuid, UFVInventoryItemTemplate*>& ItemsByGuid,
	FString& OutErrorMessage)
{
	if (!JsonObject.IsValid() || !IsValid(Loadout))
	{
		OutErrorMessage = TEXT("Invalid JSON or loadout object");
		return false;
	}

	Loadout->Items.Empty();

	// Build a map from item GUID → created LoadoutItem so slot assignments can find them
	TMap<FGuid, UFVAdvancedInventoryLoadoutItem*> loadoutItemsByItemGuid;

	// Process items[]
	const TArray<TSharedPtr<FJsonValue>>* itemsArray;
	if (JsonObject->TryGetArrayField(TEXT("items"), itemsArray))
	{
		for (const TSharedPtr<FJsonValue>& itemVal : *itemsArray)
		{
			const TSharedPtr<FJsonObject>* itemObj;
			if (!itemVal->TryGetObject(itemObj))
				continue;

			const TSharedPtr<FJsonObject>* refObj;
			FString refGuidStr;
			if (!(*itemObj)->TryGetObjectField(TEXT("ref"), refObj) ||
				!(*refObj)->TryGetStringField(TEXT("guid"), refGuidStr))
				continue;

			FGuid itemGuid;
			if (!FGuid::Parse(refGuidStr, itemGuid) || !itemGuid.IsValid())
				continue;

			UFVInventoryItemTemplate* const* templatePtr = ItemsByGuid.Find(itemGuid);
			if (!templatePtr || !IsValid(*templatePtr))
				continue;

			UFVAdvancedInventoryLoadoutItem* loadoutItem =
				NewObject<UFVAdvancedInventoryLoadoutItem>(Loadout,
					UFVAdvancedInventoryLoadoutItem::StaticClass(),
					NAME_None,
					RF_NoFlags);

			loadoutItem->Guid         = itemGuid;
			loadoutItem->ItemTemplate = *templatePtr;

			int32 qty = 1;
			if ((*itemObj)->TryGetNumberField(TEXT("quantity"), qty))
				loadoutItem->BaseQuantity = FMath::Max(1, qty);

#if WITH_EDITORONLY_DATA
			FString displayName;
			if ((*refObj)->TryGetStringField(TEXT("displayName"), displayName))
				loadoutItem->DisplayName = FName(*displayName);
#endif

			Loadout->Items.Add(loadoutItem);
			loadoutItemsByItemGuid.Add(itemGuid, loadoutItem);
		}
	}

	// Process slots{} — find matching LoadoutItem and set equipment slot
	const TSharedPtr<FJsonObject>* slotsObj;
	if (JsonObject->TryGetObjectField(TEXT("slots"), slotsObj))
	{
		for (const auto& slotKv : (*slotsObj)->Values)
		{
			const FString slotName(slotKv.Key);

			const TSharedPtr<FJsonObject>* slotVal;
			if (!slotKv.Value->TryGetObject(slotVal))
				continue;

			FString slotGuidStr;
			if (!(*slotVal)->TryGetStringField(TEXT("guid"), slotGuidStr))
				continue;

			FGuid slotItemGuid;
			if (!FGuid::Parse(slotGuidStr, slotItemGuid) || !slotItemGuid.IsValid())
				continue;

			if (UFVAdvancedInventoryLoadoutItem** existingItem = loadoutItemsByItemGuid.Find(slotItemGuid))
			{
				(*existingItem)->EquipmentSlot       = FName(*slotName);
				(*existingItem)->bAutomaticallyEquip = true;
			}
			else
			{
				// Item only mentioned in slots, not in items[] — create an entry for it
				UFVInventoryItemTemplate* const* templatePtr = ItemsByGuid.Find(slotItemGuid);
				if (!templatePtr || !IsValid(*templatePtr))
					continue;

				UFVAdvancedInventoryLoadoutItem* loadoutItem =
					NewObject<UFVAdvancedInventoryLoadoutItem>(Loadout,
						UFVAdvancedInventoryLoadoutItem::StaticClass(),
						NAME_None,
						RF_NoFlags);

				loadoutItem->Guid                = slotItemGuid;
				loadoutItem->ItemTemplate        = *templatePtr;
				loadoutItem->BaseQuantity         = 1;
				loadoutItem->EquipmentSlot        = FName(*slotName);
				loadoutItem->bAutomaticallyEquip  = true;

				Loadout->Items.Add(loadoutItem);
				loadoutItemsByItemGuid.Add(slotItemGuid, loadoutItem);
			}
		}
	}

	return true;
}

bool UFVAdvancedInventoryLoadoutEditorStatics::SaveLoadoutAsset(
	UFVAdvancedInventoryLoadoutConfig* Loadout,
	const FString& PackagePath)
{
	return FFVInventoryImportHelpers::SaveNewAssetPackage(Loadout, PackagePath);
}

bool UFVAdvancedInventoryLoadoutEditorStatics::UpdateExistingLoadout(
	UFVAdvancedInventoryLoadoutConfig* Loadout,
	const TSharedPtr<FJsonObject>& JsonObject,
	const TMap<FGuid, UFVInventoryItemTemplate*>& ItemsByGuid,
	FString& OutErrorMessage)
{
	if (!IsValid(Loadout))
	{
		OutErrorMessage = TEXT("Invalid loadout object");
		return false;
	}

	if (!DeserializeLoadoutFromJson(JsonObject, Loadout, ItemsByGuid, OutErrorMessage))
		return false;

	return FFVInventoryImportHelpers::SaveExistingAssetPackage(Loadout, OutErrorMessage);
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

FGuid UFVAdvancedInventoryLoadoutEditorStatics::ExtractGuidFromLoadoutJson(const FString& JsonString)
{
	return FFVInventoryImportHelpers::ExtractGuidFromJsonField(JsonString, TEXT("guid"));
}

FString UFVAdvancedInventoryLoadoutEditorStatics::ExtractNameFromLoadoutJson(const FString& JsonString)
{
	TSharedPtr<FJsonObject> obj;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(reader, obj) || !obj.IsValid())
		return TEXT("");

	FString name;
	if (obj->TryGetStringField(TEXT("displayName"), name))
		return FFVInventoryImportHelpers::SanitizeAssetName(name);

	return TEXT("");
}

TArray<UFVAdvancedInventoryLoadoutConfig*> UFVAdvancedInventoryLoadoutEditorStatics::LoadAllExistingLoadouts()
{
	TArray<UFVAdvancedInventoryLoadoutConfig*> loadouts;

	FAssetRegistryModule& reg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> assets;
	reg.Get().GetAssetsByClass(UFVAdvancedInventoryLoadoutConfig::StaticClass()->GetClassPathName(), assets, true);

	for (const FAssetData& a : assets)
	{
		if (UFVAdvancedInventoryLoadoutConfig* l = Cast<UFVAdvancedInventoryLoadoutConfig>(a.GetAsset()))
			loadouts.Add(l);
	}
	return loadouts;
}

UFVAdvancedInventoryLoadoutConfig* UFVAdvancedInventoryLoadoutEditorStatics::FindLoadoutByGuid(
	const TArray<UFVAdvancedInventoryLoadoutConfig*>& Loadouts,
	const FGuid& Guid)
{
	if (!Guid.IsValid())
		return nullptr;

	// UFVAdvancedInventoryLoadoutConfig has no stored Guid — we cannot match by GUID.
	// Return nullptr so imports always create a new asset (idempotency requires a Guid field on the asset).
	return nullptr;
}
