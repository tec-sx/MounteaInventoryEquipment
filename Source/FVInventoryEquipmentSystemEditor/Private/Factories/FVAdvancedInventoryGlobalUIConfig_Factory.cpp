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

#include "FVAdvancedInventoryGlobalUIConfig_Factory.h"

#include "AssetToolsModule.h"
#include "Definitions/FVAdvancedInventoryEditorBaseTypes.h"
#include "Settings/FVAdvancedInventoryGlobalUIConfig.h"
#include "Utilities/FVAdvancedInventoryEditorUtilities.h"

#define LOCTEXT_NAMESPACE "FVAdvancedInventoryGlobalUIConfig_Factory"

UFVAdvancedInventoryGlobalUIConfig_Factory::UFVAdvancedInventoryGlobalUIConfig_Factory()
{
	bCreateNew = true;
	bEditAfterNew = true;

	SupportedClass = UFVAdvancedInventoryGlobalUIConfig::StaticClass();
	ParentClass = UFVAdvancedInventoryGlobalUIConfig::StaticClass();
}

UObject* UFVAdvancedInventoryGlobalUIConfig_Factory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
	EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UFVAdvancedInventoryGlobalUIConfig>(InParent, ParentClass, Name, Flags, Context);
}

bool UFVAdvancedInventoryGlobalUIConfig_Factory::ConfigureProperties()
{
	static const FText TitleText = FText::FromString(TEXT("Pick Parent Class for new FV Global UI Config"));

	UClass* ChosenClass = nullptr;
	const bool bPressedOk = FFVAdvancedInventoryEditorUtilities::PickChildrenOfClass(TitleText, ChosenClass, UFVAdvancedInventoryGlobalUIConfig::StaticClass());

	if (bPressedOk)
		ParentClass = ChosenClass;

	return bPressedOk;
}

FText UFVAdvancedInventoryGlobalUIConfig_Factory::GetDisplayName() const
{
	return LOCTEXT("FVAdvancedInventoryGlobalUIConfig_Factory_DisplayName", "FV Advanced Inventory Global UI Config");
}

FString UFVAdvancedInventoryGlobalUIConfig_Factory::GetDefaultNewAssetName() const
{
	return TEXT("NewFVAdvancedInventoryGlobalUIConfig");
}

uint32 UFVAdvancedInventoryGlobalUIConfig_Factory::GetMenuCategories() const
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return FAssetToolsModule::GetModule().Get().FindAdvancedAssetCategory(FName("FVInventoryEquipmentSystem"));
	}

	return EAssetTypeCategories::Misc;
}

const TArray<FText>& UFVAdvancedInventoryGlobalUIConfig_Factory::GetMenuCategorySubMenus() const
{
	static const TArray<FText> AssetTypeActionSubMenu
	{
		FVAdvancedInventoryBaseTypes::ConfigurationMenuEntry
	};
	return AssetTypeActionSubMenu;
}

#undef LOCTEXT_NAMESPACE
