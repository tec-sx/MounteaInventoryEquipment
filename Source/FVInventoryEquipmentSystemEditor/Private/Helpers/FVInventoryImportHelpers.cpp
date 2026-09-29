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

#include "Helpers/FVInventoryImportHelpers.h"

#include "Statics/FVAdvancedInventoryItemTemplateEditorStatics.h"
#include "Definitions/FVInventoryItemTemplate.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Dom/JsonObject.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/SavePackage.h"

// ---------------------------------------------------------------------------
// JSON helpers
// ---------------------------------------------------------------------------

FString FFVInventoryImportHelpers::SanitizeAssetName(const FString& RawName)
{
	FString name = RawName;
	name.RemoveSpacesInline();

	const FRegexPattern invalidChars(TEXT("[\\\\:\\*\\?\"<>\\| ,.&!~@#']"));
	FRegexMatcher matcher(invalidChars, name);

	FString sanitized;
	sanitized.Reserve(name.Len());
	int32 last = 0;
	while (matcher.FindNext())
	{
		sanitized += name.Mid(last, matcher.GetMatchBeginning() - last);
		last = matcher.GetMatchEnding();
	}
	sanitized += name.Mid(last);
	return sanitized;
}

FGuid FFVInventoryImportHelpers::ExtractGuidFromJsonField(const FString& JsonString, const FString& FieldName)
{
	TSharedPtr<FJsonObject> obj;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(reader, obj) || !obj.IsValid())
		return FGuid();

	FString guidStr;
	FGuid guid;
	if (obj->TryGetStringField(FieldName, guidStr) && FGuid::Parse(guidStr, guid))
		return guid;

	return FGuid();
}

bool FFVInventoryImportHelpers::CheckGuidInKnownSet(const FString& GuidString, const FString& Context,
	const TSet<FGuid>& KnownGuids, FString& OutError)
{
	FGuid guid;
	if (!FGuid::Parse(GuidString, guid) || !guid.IsValid())
	{
		OutError = FString::Printf(TEXT("Invalid GUID '%s' in %s"), *GuidString, *Context);
		return false;
	}
	if (!KnownGuids.Contains(guid))
	{
		OutError = FString::Printf(
			TEXT("Item GUID %s referenced in %s not found in bundled items or project"),
			*GuidString, *Context);
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
// Asset system helpers
// ---------------------------------------------------------------------------

TMap<FGuid, UFVInventoryItemTemplate*> FFVInventoryImportHelpers::BuildProjectItemGuidMap()
{
	TMap<FGuid, UFVInventoryItemTemplate*> map;
	for (UFVInventoryItemTemplate* t : UFVAdvancedInventoryItemTemplateEditorStatics::LoadAllExistingTemplates())
	{
		if (IsValid(t))
			map.Add(t->Guid, t);
	}
	return map;
}

TSet<FGuid> FFVInventoryImportHelpers::ParseBundledGuids(const TArray<FString>& BundledItemJsons)
{
	TSet<FGuid> guids;
	for (const FString& json : BundledItemJsons)
	{
		const FGuid guid = UFVAdvancedInventoryItemTemplateEditorStatics::ExtractGuidFromJson(json);
		if (guid.IsValid())
			guids.Add(guid);
	}
	return guids;
}

void FFVInventoryImportHelpers::ImportBundledItems(const TArray<FString>& BundledItemJsons,
	const FString& TargetFolder,
	TMap<FGuid, UFVInventoryItemTemplate*>& InOutItemsByGuid)
{
	const TArray<UFVInventoryItemTemplate*> existingTemplates =
		UFVAdvancedInventoryItemTemplateEditorStatics::LoadAllExistingTemplates();

	for (const FString& itemJson : BundledItemJsons)
	{
		const FGuid g = UFVAdvancedInventoryItemTemplateEditorStatics::ExtractGuidFromJson(itemJson);
		if (!g.IsValid() || InOutItemsByGuid.Contains(g))
			continue;

		if (UFVInventoryItemTemplate* found =
			UFVAdvancedInventoryItemTemplateEditorStatics::FindTemplateByGuid(existingTemplates, g))
		{
			InOutItemsByGuid.Add(g, found);
			continue;
		}

		const FString nameStr = UFVAdvancedInventoryItemTemplateEditorStatics::ExtractNameFromJson(itemJson);
		const FString assetName = FString::Printf(TEXT("ImportedTemplate_%s"),
			nameStr.IsEmpty() ? *g.ToString(EGuidFormats::Short) : *nameStr);

		FString createError;
		UFVInventoryItemTemplate* newTemplate =
			UFVAdvancedInventoryItemTemplateEditorStatics::CreateTemplateAsset(
				TargetFolder, assetName, createError);

		if (newTemplate)
		{
			FString parseErr;
			UFVAdvancedInventoryItemTemplateEditorStatics::ParseSingleTemplateJson(
				itemJson, newTemplate, parseErr);
			newTemplate->ReloadItemActions();
			newTemplate->CalculateJson();
			UFVAdvancedInventoryItemTemplateEditorStatics::SaveTemplateAsset(
				newTemplate, FPaths::Combine(TargetFolder, assetName));
			InOutItemsByGuid.Add(g, newTemplate);
		}
	}
}

// ---------------------------------------------------------------------------
// Generic save operations
// ---------------------------------------------------------------------------

bool FFVInventoryImportHelpers::SaveNewAssetPackage(UObject* Asset, const FString& PackagePath)
{
	if (!IsValid(Asset))
		return false;

	UPackage* pkg = Asset->GetOutermost();
	pkg->FullyLoad();
	pkg->MarkPackageDirty();

	FSavePackageArgs saveArgs;
	saveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	saveArgs.SaveFlags     = SAVE_NoError;

	const FString fileName = FPackageName::LongPackageNameToFilename(
		PackagePath, FPackageName::GetAssetPackageExtension());

	if (!UPackage::SavePackage(pkg, Asset, *fileName, saveArgs))
		return false;

	FAssetRegistryModule::AssetCreated(Asset);
	return true;
}

bool FFVInventoryImportHelpers::SaveExistingAssetPackage(UObject* Asset, FString& OutError)
{
	if (!IsValid(Asset))
	{
		OutError = TEXT("Invalid asset object");
		return false;
	}

	UPackage* pkg = Asset->GetOutermost();
	if (!pkg)
	{
		OutError = TEXT("Asset has no valid package");
		return false;
	}

	pkg->MarkPackageDirty();

	FSavePackageArgs saveArgs;
	saveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	saveArgs.SaveFlags     = SAVE_NoError;

	const FString fileName = FPackageName::LongPackageNameToFilename(
		pkg->GetName(), FPackageName::GetAssetPackageExtension());

	if (!UPackage::SavePackage(pkg, Asset, *fileName, saveArgs))
	{
		OutError = TEXT("Failed to save updated asset");
		return false;
	}

	return true;
}
