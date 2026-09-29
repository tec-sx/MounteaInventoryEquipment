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

#include "FVAdvancedInventoryGlobalConfig_AssetAction.h"

#include "Definitions/FVAdvancedInventoryEditorBaseTypes.h"
#include "Settings/FVAdvancedInventoryGlobalConfig.h"

#define LOCTEXT_NAMESPACE "FVAdvancedInventoryGlobalConfig_AssetAction"

FFVAdvancedInventoryGlobalConfig_AssetAction::FFVAdvancedInventoryGlobalConfig_AssetAction()
{
}

FText FFVAdvancedInventoryGlobalConfig_AssetAction::GetName() const
{
	return LOCTEXT("FVAdvancedInventoryGlobalConfig_AssetAction", "FV Advanced Inventory Global Config");
}

FColor FFVAdvancedInventoryGlobalConfig_AssetAction::GetTypeColor() const
{
	return FColor::Orange;
}

UClass* FFVAdvancedInventoryGlobalConfig_AssetAction::GetSupportedClass() const
{
	return UFVAdvancedInventoryGlobalConfig::StaticClass();
}

uint32 FFVAdvancedInventoryGlobalConfig_AssetAction::GetCategories()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return FAssetToolsModule::GetModule().Get().FindAdvancedAssetCategory(FName("FVInventoryEquipmentSystem"));
	}

	return EAssetTypeCategories::Misc;
}

const TArray<FText>& FFVAdvancedInventoryGlobalConfig_AssetAction::GetSubMenus() const
{
	static const TArray<FText> AssetTypeActionSubMenu
	{
		FVAdvancedInventoryBaseTypes::ConfigurationMenuEntry
	};
	return AssetTypeActionSubMenu;
}

#undef LOCTEXT_NAMESPACE
