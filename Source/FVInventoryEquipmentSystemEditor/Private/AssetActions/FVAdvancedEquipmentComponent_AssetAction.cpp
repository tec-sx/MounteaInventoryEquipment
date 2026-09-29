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


#include "FVAdvancedEquipmentComponent_AssetAction.h"

#include "Components/FVEquipmentComponent.h"
#include "Definitions/FVAdvancedInventoryEditorBaseTypes.h"

#define LOCTEXT_NAMESPACE "FVAdvancedEquipmentComponent_AssetAction"

FFVAdvancedEquipmentComponent_AssetAction::FFVAdvancedEquipmentComponent_AssetAction()
{}

FText FFVAdvancedEquipmentComponent_AssetAction::GetName() const
{
	return LOCTEXT("FVAdvancedEquipmentComponent_AssetAction_Name", "FV Equipment Component");
}

FColor FFVAdvancedEquipmentComponent_AssetAction::GetTypeColor() const
{
	return FColor::Green;
}

UClass* FFVAdvancedEquipmentComponent_AssetAction::GetSupportedClass() const
{
	return UFVEquipmentComponent::StaticClass();
}

uint32 FFVAdvancedEquipmentComponent_AssetAction::GetCategories()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return FAssetToolsModule::GetModule().Get().FindAdvancedAssetCategory(FName("FVInventoryEquipmentSystem"));
	}
	
	return  EAssetTypeCategories::Misc;
}

const TArray<FText>& FFVAdvancedEquipmentComponent_AssetAction::GetSubMenus() const
{
	static const TArray<FText> AssetTypeActionSubMenu
	{
		FVAdvancedInventoryBaseTypes::EquipmentBaseClassesMenuEntry
	};
	return AssetTypeActionSubMenu;
};

#undef LOCTEXT_NAMESPACE