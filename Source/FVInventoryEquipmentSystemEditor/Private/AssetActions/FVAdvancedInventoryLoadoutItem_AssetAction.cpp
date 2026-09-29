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


#include "FVAdvancedInventoryLoadoutItem_AssetAction.h"
#include "Definitions/FVAdvancedInventoryLoadoutItem.h"
#include "Definitions/FVAdvancedInventoryEditorBaseTypes.h"

#define LOCTEXT_NAMESPACE "FVAdvancedInventoryItemSelectableAction"

FFVAdvancedInventoryLoadoutItem_AssetAction::FFVAdvancedInventoryLoadoutItem_AssetAction()
{}

FText FFVAdvancedInventoryLoadoutItem_AssetAction::GetName() const
{
	return LOCTEXT("FVAdvancedInventoryItemSelectableAction_Name", "FV Inventory Loadout Item");
}

FColor FFVAdvancedInventoryLoadoutItem_AssetAction::GetTypeColor() const
{
	return FColor::White;
}

UClass* FFVAdvancedInventoryLoadoutItem_AssetAction::GetSupportedClass() const
{
	return UFVAdvancedInventoryLoadoutItem::StaticClass();
}

uint32 FFVAdvancedInventoryLoadoutItem_AssetAction::GetCategories()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return FAssetToolsModule::GetModule().Get().FindAdvancedAssetCategory(FName("FVInventoryEquipmentSystem"));
	}
	
	return  EAssetTypeCategories::Misc;
}

const TArray<FText>& FFVAdvancedInventoryLoadoutItem_AssetAction::GetSubMenus() const
{
	static const TArray<FText> AssetTypeActionSubMenu
	{
		FVAdvancedInventoryBaseTypes::LoadoutClassesMenuEntry
	};
	return AssetTypeActionSubMenu;
};

#undef LOCTEXT_NAMESPACE