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

#include "FVAdvancedInventoryModalsDataTable_Factory.h"

#include "Definitions/FVAdvancedInventoryGlobalUIDataTypes.h"
#include "Definitions/FVAdvancedInventoryModalsDataTable.h"
#include "Engine/DataTable.h"

#define LOCTEXT_NAMESPACE "FVAdvancedInventoryModalsDataTable_Factory"

UFVAdvancedInventoryModalsDataTable_Factory::UFVAdvancedInventoryModalsDataTable_Factory()
{
	bCreateNew = true;
	bEditAfterNew = true;

	SupportedClass = UFVAdvancedInventoryModalsDataTable::StaticClass();
}

UObject* UFVAdvancedInventoryModalsDataTable_Factory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UDataTable* newDataTable = NewObject<UDataTable>(InParent, UDataTable::StaticClass(), Name, Flags, Context);
	newDataTable->RowStruct = FFVModalsConfig::StaticStruct();

	return newDataTable;
}

FText UFVAdvancedInventoryModalsDataTable_Factory::GetDisplayName() const
{
	return LOCTEXT("FVAdvancedInventoryModalsDataTable_Factory_DisplayName", "FV Advanced Inventory Modals Data Table");
}

#undef LOCTEXT_NAMESPACE
