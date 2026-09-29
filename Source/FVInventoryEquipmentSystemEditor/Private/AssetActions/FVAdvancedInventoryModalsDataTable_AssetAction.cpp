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

#include "FVAdvancedInventoryModalsDataTable_AssetAction.h"

#include "Definitions/FVAdvancedInventoryEditorBaseTypes.h"
#include "Definitions/FVAdvancedInventoryModalsDataTable.h"

#define LOCTEXT_NAMESPACE "FVAdvancedInventoryModalsDataTable_AssetAction"

FFVAdvancedInventoryModalsDataTable_AssetAction::FFVAdvancedInventoryModalsDataTable_AssetAction()
{
}

FText FFVAdvancedInventoryModalsDataTable_AssetAction::GetName() const
{
	return LOCTEXT("FVAdvancedInventoryModalsDataTable_AssetAction", "FV Advanced Inventory Modals Data Table");
}

FColor FFVAdvancedInventoryModalsDataTable_AssetAction::GetTypeColor() const
{
	return FColor::Orange;
}

UClass* FFVAdvancedInventoryModalsDataTable_AssetAction::GetSupportedClass() const
{
	return UFVAdvancedInventoryModalsDataTable::StaticClass();
}

uint32 FFVAdvancedInventoryModalsDataTable_AssetAction::GetCategories()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return FAssetToolsModule::GetModule().Get().FindAdvancedAssetCategory(FName("FVInventoryEquipmentSystem"));
	}

	return EAssetTypeCategories::Misc;
}

const TArray<FText>& FFVAdvancedInventoryModalsDataTable_AssetAction::GetSubMenus() const
{
	static const TArray<FText> AssetTypeActionSubMenu
	{
		FVAdvancedInventoryBaseTypes::ConfigurationMenuEntry
	};
	return AssetTypeActionSubMenu;
}

#undef LOCTEXT_NAMESPACE
