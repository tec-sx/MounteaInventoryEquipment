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

#include "FVAdvancedInventoryThemeConfig_AssetAction.h"

#include "Definitions/FVAdvancedInventoryEditorBaseTypes.h"
#include "Settings/FVAdvancedInventoryThemeConfig.h"

#define LOCTEXT_NAMESPACE "FVAdvancedInventoryThemeConfigAssetAction"

FFVAdvancedInventoryThemeConfig_AssetAction::FFVAdvancedInventoryThemeConfig_AssetAction()
{
}

FText FFVAdvancedInventoryThemeConfig_AssetAction::GetName() const
{
	return LOCTEXT("FVAdvancedInventoryThemeConfigAssetAction", "FV Advanced Inventory Theme Config");
}

FColor FFVAdvancedInventoryThemeConfig_AssetAction::GetTypeColor() const
{
	return FColor::Orange;
}

UClass* FFVAdvancedInventoryThemeConfig_AssetAction::GetSupportedClass() const
{
	return UFVAdvancedInventoryThemeConfig::StaticClass();
}

uint32 FFVAdvancedInventoryThemeConfig_AssetAction::GetCategories()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return FAssetToolsModule::GetModule().Get().FindAdvancedAssetCategory(FName("FVInventoryEquipmentSystem"));
	}

	return EAssetTypeCategories::Misc;
}

const TArray<FText>& FFVAdvancedInventoryThemeConfig_AssetAction::GetSubMenus() const
{
	static const TArray<FText> AssetTypeActionSubMenu
	{
		FVAdvancedInventoryBaseTypes::ConfigurationMenuEntry
	};
	return AssetTypeActionSubMenu;
}


#undef LOCTEXT_NAMESPACE
