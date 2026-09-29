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

#include "FVAdvancedInventoryInteractiveWidgetEnvConfig_AssetAction.h"

#include "Definitions/FVAdvancedInventoryEditorBaseTypes.h"
#include "Settings/TemplatesConfig/FVAdvancedInventoryPreviewEnvironmentSettings.h"

#define LOCTEXT_NAMESPACE "FVAdvancedInventoryInteractiveWidgetEnvironmentConfigAssetAction"

FFVAdvancedInventoryInteractiveWidgetEnvironmentConfig_AssetAction::FFVAdvancedInventoryInteractiveWidgetEnvironmentConfig_AssetAction()
{
}

FText FFVAdvancedInventoryInteractiveWidgetEnvironmentConfig_AssetAction::GetName() const
{
	return LOCTEXT("FVAdvancedInventoryInteractiveWidgetEnvironmentConfigAssetAction", "FV Advanced Inventory Interactive Widget Environment Config");
}

FColor FFVAdvancedInventoryInteractiveWidgetEnvironmentConfig_AssetAction::GetTypeColor() const
{
	return FColor::Orange;
}

UClass* FFVAdvancedInventoryInteractiveWidgetEnvironmentConfig_AssetAction::GetSupportedClass() const
{
	return UFVAdvancedInventoryPreviewEnvironmentSettings::StaticClass();
}

uint32 FFVAdvancedInventoryInteractiveWidgetEnvironmentConfig_AssetAction::GetCategories()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return FAssetToolsModule::GetModule().Get().FindAdvancedAssetCategory(FName("FVInventoryEquipmentSystem"));
	}

	return EAssetTypeCategories::Misc;
}

const TArray<FText>& FFVAdvancedInventoryInteractiveWidgetEnvironmentConfig_AssetAction::GetSubMenus() const
{
	static const TArray<FText> AssetTypeActionSubMenu
	{
		FVAdvancedInventoryBaseTypes::ConfigurationMenuEntry
	};
	return AssetTypeActionSubMenu;
}


#undef LOCTEXT_NAMESPACE
