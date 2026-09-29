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


#include "FVAdvancedInventoryItemTemplate_AssetAction.h"

#include "Definitions/FVAdvancedInventoryEditorBaseTypes.h"
#include "Definitions/FVInventoryItemTemplate.h"

#define LOCTEXT_NAMESPACE "FVAdvancedInventoryInteractiveWidgetConfig_AssetAction "

FFVAdvancedInventoryItemTemplate_AssetAction::FFVAdvancedInventoryItemTemplate_AssetAction()
{
}

FText FFVAdvancedInventoryItemTemplate_AssetAction::GetName() const
{
	return LOCTEXT("FVAdvancedInventoryItemTemplate_AssetAction", "Inventory Item Template");
}

FColor FFVAdvancedInventoryItemTemplate_AssetAction::GetTypeColor() const
{
	return FColor::Blue;
}

UClass* FFVAdvancedInventoryItemTemplate_AssetAction::GetSupportedClass() const
{
	return UFVInventoryItemTemplate::StaticClass();
}

uint32 FFVAdvancedInventoryItemTemplate_AssetAction::GetCategories()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return FAssetToolsModule::GetModule().Get().FindAdvancedAssetCategory(FName("FVInventoryEquipmentSystem"));
	}

	return EAssetTypeCategories::Misc;
}

const TArray<FText>& FFVAdvancedInventoryItemTemplate_AssetAction::GetSubMenus() const
{
	static const TArray<FText> AssetTypeActionSubMenu
	{
		FVAdvancedInventoryBaseTypes::TemplatesMenuEntry
	};
	return AssetTypeActionSubMenu;
}


#undef LOCTEXT_NAMESPACE
