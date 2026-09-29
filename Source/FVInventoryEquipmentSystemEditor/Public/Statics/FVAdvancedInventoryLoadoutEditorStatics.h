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

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVAdvancedInventoryLoadoutEditorStatics.generated.h"

class UFVAdvancedInventoryLoadoutConfig;
class UFVInventoryItemTemplate;

/**
 * Editor-only static helpers for importing .mntealoadout / .mntealoadouts files.
 *
 * Import is atomic: all ZIP reading and validation finishes before any UAsset is created.
 * If validation fails, nothing is written.
 */
UCLASS()
class FVINVENTORYEQUIPMENTSYSTEMEDITOR_API UFVAdvancedInventoryLoadoutEditorStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Main entry point. Accepts both a single .mntealoadout ZIP and a .mntealoadouts bundle.
	 * On success, OutLoadouts contains every loadout that was created or updated.
	 */
	static bool ImportLoadoutsFromFilePath(
		const FString& FilePath,
		const FString& TargetFolder,
		TArray<UFVAdvancedInventoryLoadoutConfig*>& OutLoadouts,
		FString& OutErrorMessage);

public:
	// ── ZIP parsing ──────────────────────────────────────────────────────────

	/** Extract loadout.json and all items/*.json from a single .mntealoadout ZIP blob. */
	static bool ParseLoadoutFromZip(
		const TArray<uint8>& ZipBytes,
		FString& OutLoadoutJson,
		TArray<FString>& OutBundledItemJsons,
		FString& OutErrorMessage);

	// ── Validation (read-only, no writes) ────────────────────────────────────

	/**
	 * Validate the loadout JSON before any assets are created.
	 * BundledGuids is the set of item GUIDs that arrived bundled in the same ZIP.
	 * Any referenced GUID must be either bundled or already present in the project.
	 */
	static bool ValidateLoadout(
		const TSharedPtr<FJsonObject>& LoadoutJson,
		const TSet<FGuid>& BundledGuids,
		FString& OutErrorMessage);

	// ── Asset creation / serialization ───────────────────────────────────────

	/** Create a new UFVAdvancedInventoryLoadoutConfig asset in TargetFolder. */
	static UFVAdvancedInventoryLoadoutConfig* CreateLoadoutAsset(
		const FString& TargetFolder,
		const FString& AssetName,
		FString& OutErrorMessage);

	/**
	 * Populate a loadout config from its JSON object.
	 * ItemsByGuid maps item GUIDs → already-imported UFVInventoryItemTemplate objects
	 * (covers both bundled items and pre-existing project assets).
	 */
	static bool DeserializeLoadoutFromJson(
		const TSharedPtr<FJsonObject>& JsonObject,
		UFVAdvancedInventoryLoadoutConfig* Loadout,
		const TMap<FGuid, UFVInventoryItemTemplate*>& ItemsByGuid,
		FString& OutErrorMessage);

	/** Save a loadout config package to disk and register it with the asset registry. */
	static bool SaveLoadoutAsset(
		UFVAdvancedInventoryLoadoutConfig* Loadout,
		const FString& PackagePath);

	// ── Helpers ───────────────────────────────────────────────────────────────

	static FGuid ExtractGuidFromLoadoutJson(const FString& JsonString);
	static FString ExtractNameFromLoadoutJson(const FString& JsonString);

	static TArray<UFVAdvancedInventoryLoadoutConfig*> LoadAllExistingLoadouts();
	static UFVAdvancedInventoryLoadoutConfig* FindLoadoutByGuid(
		const TArray<UFVAdvancedInventoryLoadoutConfig*>& Loadouts,
		const FGuid& Guid);

	static bool UpdateExistingLoadout(
		UFVAdvancedInventoryLoadoutConfig* Loadout,
		const TSharedPtr<FJsonObject>& JsonObject,
		const TMap<FGuid, UFVInventoryItemTemplate*>& ItemsByGuid,
		FString& OutErrorMessage);
};
