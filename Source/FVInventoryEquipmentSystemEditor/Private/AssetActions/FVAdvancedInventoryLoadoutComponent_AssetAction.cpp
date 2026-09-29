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


#include "FVAdvancedInventoryLoadoutComponent_AssetAction.h"

#include "Definitions/FVAdvancedInventoryLoadoutConfig.h"
#include "Components/FVAdvancedInventoryLoadoutComponent.h"
#include "Definitions/FVAdvancedInventoryEditorBaseTypes.h"

#define LOCTEXT_NAMESPACE "FVAdvancedInventoryLoadoutComponent_AssetAction"

FFVAdvancedInventoryLoadoutComponent_AssetAction::FFVAdvancedInventoryLoadoutComponent_AssetAction()
{}

FText FFVAdvancedInventoryLoadoutComponent_AssetAction::GetName() const
{
	return LOCTEXT("FVAdvancedInventoryLoadoutComponent_AssetAction_Name", "FV Inventory Loadout Component");
}

FColor FFVAdvancedInventoryLoadoutComponent_AssetAction::GetTypeColor() const
{
	return FColor::White;
}

UClass* FFVAdvancedInventoryLoadoutComponent_AssetAction::GetSupportedClass() const
{
	return UFVAdvancedInventoryLoadoutComponent::StaticClass();
}

uint32 FFVAdvancedInventoryLoadoutComponent_AssetAction::GetCategories()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return FAssetToolsModule::GetModule().Get().FindAdvancedAssetCategory(FName("FVInventoryEquipmentSystem"));
	}
	
	return  EAssetTypeCategories::Misc;
}

const TArray<FText>& FFVAdvancedInventoryLoadoutComponent_AssetAction::GetSubMenus() const
{
	static const TArray<FText> AssetTypeActionSubMenu
	{
		FVAdvancedInventoryBaseTypes::LoadoutClassesMenuEntry
	};
	return AssetTypeActionSubMenu;
};

#undef LOCTEXT_NAMESPACE
