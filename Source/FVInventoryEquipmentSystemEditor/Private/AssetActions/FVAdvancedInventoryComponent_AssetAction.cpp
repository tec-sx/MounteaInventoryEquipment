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


#include "FVAdvancedInventoryComponent_AssetAction.h"

#include "Components/FVInventoryComponent.h"
#include "Definitions/FVAdvancedInventoryEditorBaseTypes.h"

#define LOCTEXT_NAMESPACE "FVAdvancedInventoryComponent_AssetAction"

FFVAdvancedInventoryComponent_AssetAction::FFVAdvancedInventoryComponent_AssetAction()
{}

FText FFVAdvancedInventoryComponent_AssetAction::GetName() const
{
	return LOCTEXT("FVAdvancedInventoryComponent_AssetAction_Name", "FV Inventory Component");
}

FColor FFVAdvancedInventoryComponent_AssetAction::GetTypeColor() const
{
	return FColor::Green;
}

UClass* FFVAdvancedInventoryComponent_AssetAction::GetSupportedClass() const
{
	return UFVInventoryComponent::StaticClass();
}

uint32 FFVAdvancedInventoryComponent_AssetAction::GetCategories()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return FAssetToolsModule::GetModule().Get().FindAdvancedAssetCategory(FName("FVInventoryEquipmentSystem"));
	}
	
	return  EAssetTypeCategories::Misc;
}

const TArray<FText>& FFVAdvancedInventoryComponent_AssetAction::GetSubMenus() const
{
	static const TArray<FText> AssetTypeActionSubMenu
	{
		FVAdvancedInventoryBaseTypes::InventoryBaseClassesMenuEntry
	};
	return AssetTypeActionSubMenu;
};

#undef LOCTEXT_NAMESPACE