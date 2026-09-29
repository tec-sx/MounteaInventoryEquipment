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


#include "FVAdvancedInventoryUIComponent_AssetAction.h"

#include "Components/FVInventoryUIComponent.h"
#include "Definitions/FVAdvancedInventoryEditorBaseTypes.h"

#define LOCTEXT_NAMESPACE "FVAdvancedInventoryUIComponent_AssetAction"

FFVAdvancedInventoryUIComponent_AssetAction::FFVAdvancedInventoryUIComponent_AssetAction()
{}

FText FFVAdvancedInventoryUIComponent_AssetAction::GetName() const
{
	return LOCTEXT("FVAdvancedInventoryUIComponent_AssetAction_Name", "FV Inventory UI Component");
}

FColor FFVAdvancedInventoryUIComponent_AssetAction::GetTypeColor() const
{
	return FColor::Green;
}

UClass* FFVAdvancedInventoryUIComponent_AssetAction::GetSupportedClass() const
{
	return UFVInventoryUIComponent::StaticClass();
}

uint32 FFVAdvancedInventoryUIComponent_AssetAction::GetCategories()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return FAssetToolsModule::GetModule().Get().FindAdvancedAssetCategory(FName("FVInventoryEquipmentSystem"));
	}
	
	return  EAssetTypeCategories::Misc;
}

const TArray<FText>& FFVAdvancedInventoryUIComponent_AssetAction::GetSubMenus() const
{
	static const TArray<FText> AssetTypeActionSubMenu
	{
		FVAdvancedInventoryBaseTypes::InventoryBaseClassesMenuEntry
	};
	return AssetTypeActionSubMenu;
};

#undef LOCTEXT_NAMESPACE