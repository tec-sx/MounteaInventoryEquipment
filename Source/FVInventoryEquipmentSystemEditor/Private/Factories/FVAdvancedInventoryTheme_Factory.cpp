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

#include "FVAdvancedInventoryTheme_Factory.h"
#include "Utilities/FVAdvancedInventoryEditorUtilities.h"
#include "Settings/FVAdvancedInventoryThemeConfig.h"

UFVAdvancedInventoryTheme_Factory::UFVAdvancedInventoryTheme_Factory()
{
	bCreateNew = true;
	bEditAfterNew = true;

	SupportedClass = UFVAdvancedInventoryThemeConfig::StaticClass();
}

UObject* UFVAdvancedInventoryTheme_Factory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UFVAdvancedInventoryThemeConfig>(InParent, ParentClass, Name, Flags, Context);
}

bool UFVAdvancedInventoryTheme_Factory::ConfigureProperties()
{
	static const FText TitleText = FText::FromString(TEXT("Pick Parent Class for new FV Inventory Theme Config"));
	
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = FFVAdvancedInventoryEditorUtilities::PickChildrenOfClass(TitleText, ChosenClass, UFVAdvancedInventoryThemeConfig::StaticClass());

	if ( bPressedOk )
		ParentClass = ChosenClass;

	return bPressedOk;
}
