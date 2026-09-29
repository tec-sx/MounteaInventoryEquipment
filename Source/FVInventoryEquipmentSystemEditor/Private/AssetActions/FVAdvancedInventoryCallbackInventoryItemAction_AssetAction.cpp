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


#include "FVAdvancedInventoryCallbackInventoryItemAction_AssetAction.h"
#include "Decorations/FVCallbackInventoryItemAction.h"
#include "Definitions/FVAdvancedInventoryEditorBaseTypes.h"

#define LOCTEXT_NAMESPACE "FVAdvancedInventoryItemCallbackAction"

FFVAdvancedInventoryCallbackInventoryItemAction_AssetAction::FFVAdvancedInventoryCallbackInventoryItemAction_AssetAction()
{}

FText FFVAdvancedInventoryCallbackInventoryItemAction_AssetAction::GetName() const
{
	return LOCTEXT("FVAdvancedInventoryItemCallbackAction_Name", "FV Inventory Callback Item Action");
}

FColor FFVAdvancedInventoryCallbackInventoryItemAction_AssetAction::GetTypeColor() const
{
	return FColor::Purple;
}

UClass* FFVAdvancedInventoryCallbackInventoryItemAction_AssetAction::GetSupportedClass() const
{
	return UFVCallbackInventoryItemAction::StaticClass();
}

uint32 FFVAdvancedInventoryCallbackInventoryItemAction_AssetAction::GetCategories()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return FAssetToolsModule::GetModule().Get().FindAdvancedAssetCategory(FName("FVInventoryEquipmentSystem"));
	}
	
	return  EAssetTypeCategories::Misc;
}

const TArray<FText>& FFVAdvancedInventoryCallbackInventoryItemAction_AssetAction::GetSubMenus() const
{
	static const TArray<FText> AssetTypeActionSubMenu
	{
		FVAdvancedInventoryBaseTypes::AssetActionsMenuEntry
	};
	return AssetTypeActionSubMenu;
};

#undef LOCTEXT_NAMESPACE