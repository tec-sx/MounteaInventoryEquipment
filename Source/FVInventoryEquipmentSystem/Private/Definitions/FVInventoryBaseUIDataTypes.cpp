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


#include "Definitions/FVInventoryBaseUIDataTypes.h"

#include "Blueprint/UserWidget.h"
#include "Interfaces/Widgets/Items/FVAdvancedInventoryItemWidgetInterface.h"

bool FInventorySlot::IsEmpty() const
{
	if (!ItemWidget)
		return true;
	
	if (!ItemWidget->Implements<UFVAdvancedInventoryItemWidgetInterface>())
		return true;
	
	return IFVAdvancedInventoryItemWidgetInterface::Execute_GetInventoryData(ItemWidget).IsEmpty();
}

FString FInventorySlot::ToString() const
{
	if (!ItemWidget)
		return FString::Printf(
			TEXT("ItemWidget: EMPTY")
		);
	
	if (!ItemWidget->Implements<UFVAdvancedInventoryItemWidgetInterface>())
		return FString::Printf(
			TEXT("ItemWidget: Does not implement UFVAdvancedInventoryItemWidgetInterface.")
		);
	
	const auto itemData = IFVAdvancedInventoryItemWidgetInterface::Execute_GetInventoryData(ItemWidget);
	
	if (!itemData.IsEmpty())
		return FString::Printf(
			TEXT("ItemWidget: Item Data INVALID.")
		);
	
	return FString::Printf(
		TEXT("%s\nItemWidget: %d"),
		*itemData.ContainingItem.ToString(),
		itemData.Quantity
	);
}
