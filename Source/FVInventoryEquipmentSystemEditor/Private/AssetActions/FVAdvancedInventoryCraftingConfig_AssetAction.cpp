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

#include "FVAdvancedInventoryCraftingConfig_AssetAction.h"

#include "Definitions/FVAdvancedInventoryEditorBaseTypes.h"
#include "Settings/FVAdvancedCraftingConfig.h"


#define LOCTEXT_NAMESPACE "FVAdvancedInventoryCraftingConfig_AssetAction "

FFVAdvancedInventoryCraftingConfig_AssetAction::FFVAdvancedInventoryCraftingConfig_AssetAction()
{
}

FText FFVAdvancedInventoryCraftingConfig_AssetAction::GetName() const
{
	return LOCTEXT("FVAdvancedInventoryCraftingConfig_AssetAction", "FV Advanced Crafting Config");
}

FColor FFVAdvancedInventoryCraftingConfig_AssetAction::GetTypeColor() const
{
	return FColor::Orange;
}

UClass* FFVAdvancedInventoryCraftingConfig_AssetAction::GetSupportedClass() const
{
	return UFVAdvancedCraftingConfig::StaticClass();
}

uint32 FFVAdvancedInventoryCraftingConfig_AssetAction::GetCategories()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return FAssetToolsModule::GetModule().Get().FindAdvancedAssetCategory(FName("FVInventoryEquipmentSystem"));
	}

	return EAssetTypeCategories::Misc;
}

const TArray<FText>& FFVAdvancedInventoryCraftingConfig_AssetAction::GetSubMenus() const
{
	static const TArray<FText> AssetTypeActionSubMenu
	{
		FVAdvancedInventoryBaseTypes::CraftingClassesMenuEntry
	};
	return AssetTypeActionSubMenu;
}


#undef LOCTEXT_NAMESPACE
