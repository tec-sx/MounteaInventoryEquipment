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

#include "FVAdvancedInventoryLoadoutConfigs_AssetAction.h"

#include "FVAdvancedInventoryThemeConfig_AssetAction.h"
#include "Definitions/FVAdvancedInventoryEditorBaseTypes.h"
#include "Definitions/FVAdvancedInventoryLoadoutConfig.h"


#define LOCTEXT_NAMESPACE "FVAdvancedInventoryLoadoutConfigs_AssetAction"

FFVAdvancedInventoryLoadoutConfigs_AssetAction::FFVAdvancedInventoryLoadoutConfigs_AssetAction()
{
}

FText FFVAdvancedInventoryLoadoutConfigs_AssetAction::GetName() const
{
	return LOCTEXT("FVAdvancedInventoryLoadoutConfigs_AssetAction", "FV Advanced Inventory Loadout Config");
}

FColor FFVAdvancedInventoryLoadoutConfigs_AssetAction::GetTypeColor() const
{
	return FColor::White;
}

UClass* FFVAdvancedInventoryLoadoutConfigs_AssetAction::GetSupportedClass() const
{
	return UFVAdvancedInventoryLoadoutConfig::StaticClass();
}

uint32 FFVAdvancedInventoryLoadoutConfigs_AssetAction::GetCategories()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return FAssetToolsModule::GetModule().Get().FindAdvancedAssetCategory(FName("FVInventoryEquipmentSystem"));
	}

	return EAssetTypeCategories::Misc;
}

const TArray<FText>& FFVAdvancedInventoryLoadoutConfigs_AssetAction::GetSubMenus() const
{
	static const TArray<FText> AssetTypeActionSubMenu
	{
		FVAdvancedInventoryBaseTypes::LoadoutClassesMenuEntry
	};
	return AssetTypeActionSubMenu;
}


#undef LOCTEXT_NAMESPACE
