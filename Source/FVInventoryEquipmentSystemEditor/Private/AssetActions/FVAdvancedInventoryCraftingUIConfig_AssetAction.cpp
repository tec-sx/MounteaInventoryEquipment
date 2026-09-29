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

#include "FVAdvancedInventoryCraftingUIConfig_AssetAction.h"

#include "Definitions/FVAdvancedInventoryEditorBaseTypes.h"
#include "Settings/FVAdvancedCraftingUIConfig.h"

#define LOCTEXT_NAMESPACE "FVAdvancedInventoryCraftingUIConfig_AssetAction"

FFVAdvancedInventoryCraftingUIConfig_AssetAction::FFVAdvancedInventoryCraftingUIConfig_AssetAction()
{
}

FText FFVAdvancedInventoryCraftingUIConfig_AssetAction::GetName() const
{
	return LOCTEXT("FVAdvancedInventoryCraftingUIConfig_AssetAction", "FV Advanced Crafting UI Config");
}

FColor FFVAdvancedInventoryCraftingUIConfig_AssetAction::GetTypeColor() const
{
	return FColor::Orange;
}

UClass* FFVAdvancedInventoryCraftingUIConfig_AssetAction::GetSupportedClass() const
{
	return UFVAdvancedCraftingUIConfig::StaticClass();
}

uint32 FFVAdvancedInventoryCraftingUIConfig_AssetAction::GetCategories()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return FAssetToolsModule::GetModule().Get().FindAdvancedAssetCategory(FName("FVInventoryEquipmentSystem"));
	}

	return EAssetTypeCategories::Misc;
}

const TArray<FText>& FFVAdvancedInventoryCraftingUIConfig_AssetAction::GetSubMenus() const
{
	static const TArray<FText> AssetTypeActionSubMenu
	{
		FVAdvancedInventoryBaseTypes::CraftingClassesMenuEntry
	};
	return AssetTypeActionSubMenu;
}

#undef LOCTEXT_NAMESPACE
