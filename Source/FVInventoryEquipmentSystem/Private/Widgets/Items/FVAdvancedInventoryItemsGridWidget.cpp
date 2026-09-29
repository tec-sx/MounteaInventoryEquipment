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

#include "Widgets/Items/FVAdvancedInventoryItemsGridWidget.h"

#include "Algo/Accumulate.h"
#include "Algo/AnyOf.h"
#include "Algo/Copy.h"
#include "Algo/Find.h"

#include "Interfaces/Inventory/FVAdvancedInventoryInterface.h"
#include "Interfaces/Widgets/Items/FVAdvancedInventoryItemSlotWidgetInterface.h"

#include "Statics/FVInventoryUIStatics.h"

bool UFVAdvancedInventoryItemsGridWidget::AddItemToEmptySlot_Implementation(const FGuid& ItemId)
{
	const int32 EmptySlotIndex = Execute_FindEmptySlotIndex(this, ItemId);
	if (!GridSlots.Array().IsValidIndex(EmptySlotIndex)) return false;

	return Execute_AddItemToSlot(this, ItemId, EmptySlotIndex);
}

bool UFVAdvancedInventoryItemsGridWidget::AddItemToSlot_Implementation(const FGuid& ItemId, const int32 SlotIndex)
{
	return false; 
	/*
	if (!Execute_IsSlotEmpty(this, SlotIndex)) return false;
	const auto parentInventory = ParentUIComponent->Execute_GetParentInventory(ParentUIComponent.GetObject());
	if (parentInventory.GetObject() == nullptr) return false;
	const auto item = parentInventory->Execute_FindItem(parentInventory.GetObject(), FInventoryItemSearchParams(ItemId));
	if (!item.IsItemValid()) return false;
	
	FFVInventoryGridSlot tempGridSlot = GridSlots.Array()[SlotIndex];

	const TObjectPtr<UUserWidget>& slotWidget = tempGridSlot.SlotWidget;
	if (!IsValid(slotWidget)) return false;
	if (!slotWidget->Implements<UFVAdvancedInventoryItemSlotWidgetInterface>()) return false;
		
	tempGridSlot.OccupiedItemId = ItemId;
	tempGridSlot.SlotQuantity = item.GetQuantity();
	GridSlots.Emplace(tempGridSlot);

	IFVAdvancedInventoryItemSlotWidgetInterface::Execute_AddItemToSlot(slotWidget, ItemId);
	IFVAdvancedInventoryItemSlotWidgetInterface::Execute_StoreGridSlotData(slotWidget, tempGridSlot);

	ParentUIComponent->Execute_AddSlot(ParentUIComponent.GetObject(), tempGridSlot);

	IFVInventoryGenericWidgetInterface::Execute_RefreshWidget(slotWidget);

	return true;
	*/
}

bool UFVAdvancedInventoryItemsGridWidget::RemoveItemFromSlot_Implementation(const int32 SlotIndex)
{
	return false;
	/*
	if (!GridSlots.Array().IsValidIndex(SlotIndex)) return false;

	FFVInventoryGridSlot tempGridSlot = GridSlots.Array()[SlotIndex];

	const TObjectPtr<UUserWidget>& slotWidget = tempGridSlot.SlotWidget;
	if (!IsValid(slotWidget)) return false;
	if (!slotWidget->Implements<UFVAdvancedInventoryItemSlotWidgetInterface>()) return false;

	const FGuid itemId = tempGridSlot.OccupiedItemId;
	IFVAdvancedInventoryItemSlotWidgetInterface::Execute_RemoveItemFromSlot(slotWidget, itemId);

	FFVInventoryGridSlot cleanSlot = tempGridSlot;
	cleanSlot.ResetSlot();
	IFVAdvancedInventoryItemSlotWidgetInterface::Execute_StoreGridSlotData(slotWidget, cleanSlot);
	
	
	ParentUIComponent->Execute_UpdateSlot(ParentUIComponent.GetObject(), cleanSlot);
	
	GridSlots.Emplace(cleanSlot);

	IFVInventoryGenericWidgetInterface::Execute_RefreshWidget(slotWidget);
	
	tempGridSlot.ResetSlot();

	return true;
	*/
}

bool UFVAdvancedInventoryItemsGridWidget::RemoveItemFromGrid_Implementation(const FGuid& ItemId, const int32 Quantity)
{
	return false;
	/*
	if (!ItemId.IsValid()) return false;
	
	auto allSlots = Execute_GetGridSlotsDataForItem(this, ItemId);
	if (allSlots.Num() == 0) return false;
	
	// If Quantity is -1 or 0, remove all
	if (Quantity <= 0)
	{
		bool bSuccess = true;
		for (const auto& slot : allSlots)
		{
			int32 slotIndex = Execute_GetSlotIndexByItem(this, slot.OccupiedItemId);
			if (slotIndex != INDEX_NONE)
			{
				if (!Execute_RemoveItemFromSlot(this, slotIndex))
					bSuccess = false;
			}
		}
		return bSuccess;
	}

	allSlots.Sort([](const FFVInventoryGridSlot& a, const FFVInventoryGridSlot& b) {
		if (a.SlotPosition.Y != b.SlotPosition.Y)
			return a.SlotPosition.Y > b.SlotPosition.Y; // Sort by Y descending first
		return a.SlotPosition.X > b.SlotPosition.X; // Then by X descending
	});
	
	int32 remainingQuantity = Quantity;
	bool bAnyRemoved = false;
	
	for (const auto& slot : allSlots)
	{
		if (remainingQuantity <= 0) break;
		
		int32 slotIndex = Execute_GetSlotIndexByItem(this, slot.OccupiedItemId);
		if (slotIndex == INDEX_NONE) continue;
		
		if (slot.SlotQuantity <= remainingQuantity)
		{
			// Remove entire slot
			if (Execute_RemoveItemFromSlot(this, slotIndex))
			{
				remainingQuantity -= slot.SlotQuantity;
				bAnyRemoved = true;
			}
		}
		else
		{
			// Reduce quantity in slot
			FFVInventoryGridSlot updatedSlot = slot;
			updatedSlot.SlotQuantity -= remainingQuantity;
			GridSlots.Emplace(updatedSlot);
			ParentUIComponent->Execute_UpdateSlot(ParentUIComponent.GetObject(), updatedSlot);
			
			if (IsValid(updatedSlot.SlotWidget))
				IFVInventoryGenericWidgetInterface::Execute_RefreshWidget(updatedSlot.SlotWidget);
				
			remainingQuantity = 0;
			bAnyRemoved = true;
		}
	}
	
	return bAnyRemoved;
	*/
}

FGuid UFVAdvancedInventoryItemsGridWidget::GetItemInSlot_Implementation(const int32 SlotIndex) const
{
	return FGuid();
	/*
	if (!GridSlots.Array().IsValidIndex(SlotIndex)) return FGuid();
	const FFVInventoryGridSlot gridSlot = GridSlots.Array()[SlotIndex];
	return gridSlot.OccupiedItemId;
	*/
}

bool UFVAdvancedInventoryItemsGridWidget::SwapItemsBetweenSlots_Implementation(const int32 SlotIndex1, const int32 SlotIndex2)
{
	return false;
	/*
	if (!GridSlots.Array().IsValidIndex(SlotIndex1) || !GridSlots.Array().IsValidIndex(SlotIndex2))
		return false;

	FFVInventoryGridSlot tempSlot1 = GridSlots.Array()[SlotIndex1];
	FFVInventoryGridSlot tempSlot2 = GridSlots.Array()[SlotIndex2];

	GridSlots.Remove(tempSlot1);
	GridSlots.Remove(tempSlot2);

	Swap(tempSlot1.OccupiedItemId, tempSlot2.OccupiedItemId);

	GridSlots.Add(tempSlot1);
	GridSlots.Add(tempSlot2);

	if (IsValid(tempSlot1.SlotWidget) && tempSlot1.SlotWidget->Implements<UFVInventoryGenericWidgetInterface>())
		IFVInventoryGenericWidgetInterface::Execute_RefreshWidget(tempSlot1.SlotWidget);

	if (IsValid(tempSlot2.SlotWidget) && tempSlot2.SlotWidget->Implements<UFVInventoryGenericWidgetInterface>())
		IFVInventoryGenericWidgetInterface::Execute_RefreshWidget(tempSlot2.SlotWidget);

	return true;
	*/
}

void UFVAdvancedInventoryItemsGridWidget::ClearAllSlots_Implementation()
{
	return;
	/*
	TSet<FFVInventoryGridSlot> clearGridSlots;
	for (const auto& gridSlot : GridSlots)
	{
		FFVInventoryGridSlot tempGridSlot = gridSlot;
		tempGridSlot.OccupiedItemId = FGuid();
		const TObjectPtr<UUserWidget>& slotWidget = tempGridSlot.SlotWidget;
		if (IsValid(slotWidget) && slotWidget->Implements<UFVAdvancedInventoryItemSlotWidgetInterface>())
			IFVAdvancedInventoryItemSlotWidgetInterface::Execute_StoreGridSlotData(slotWidget, tempGridSlot);
		clearGridSlots.Add(tempGridSlot);
	}

	GridSlots.Empty();
	GridSlots.Append(clearGridSlots);
	*/
}

int32 UFVAdvancedInventoryItemsGridWidget::GetTotalSlots_Implementation() const
{
	return GridSlots.Num();
}

bool UFVAdvancedInventoryItemsGridWidget::IsSlotEmpty_Implementation(const int32 SlotIndex) const
{
	if (!GridSlots.Array().IsValidIndex(SlotIndex)) return true;

	return GridSlots.Array()[SlotIndex].IsEmpty();
}

int32 UFVAdvancedInventoryItemsGridWidget::GetSlotIndexByItem_Implementation(const FGuid& ItemId) const
{
	return INDEX_NONE;
	/*
	const TArray<FFVInventoryGridSlot>& Slots = GridSlots.Array();

	const FFVInventoryGridSlot* FoundSlot = Algo::FindByPredicate(Slots,
		[&ItemId](const FFVInventoryGridSlot& checkedSlot)
		{
			return checkedSlot.OccupiedItemId == ItemId;
		});

	return FoundSlot ? Slots.IndexOfByKey(*FoundSlot) : INDEX_NONE;
	*/
}

int32 UFVAdvancedInventoryItemsGridWidget::GetGridSlotIndexByCoords_Implementation(
	const FIntPoint& SlotCoords) const
{
	const FFVInventoryGridSlot* FoundSlot = Algo::FindByPredicate(GridSlots,
		[&SlotCoords](const FFVInventoryGridSlot& checkedSlot)
		{
			return checkedSlot.SlotPosition == SlotCoords;
		});

	return FoundSlot ? GridSlots.Array().IndexOfByKey(*FoundSlot) : -1;
}

bool UFVAdvancedInventoryItemsGridWidget::IsItemInGrid_Implementation(const FGuid& ItemId) const
{
	return false;
	/*
	return Algo::AnyOf(GridSlots,
		[&ItemId](const FFVInventoryGridSlot& checkedSlot)
		{
			return checkedSlot.OccupiedItemId == ItemId;
		});
	*/
}

FFVInventoryGridSlot UFVAdvancedInventoryItemsGridWidget::GetGridSlotData_Implementation(
	const int32 SlotIndex) const
{
	if (GridSlots.Array().IsValidIndex(SlotIndex) == false)
		return FFVInventoryGridSlot();
	return GridSlots.Array()[SlotIndex];
}

TSet<FFVInventoryGridSlot> UFVAdvancedInventoryItemsGridWidget::GetGridSlotsData_Implementation() const
{
	return GridSlots;
}

UUserWidget* UFVAdvancedInventoryItemsGridWidget::FindEmptyWidgetSlot_Implementation() const
{
	return nullptr;
	/*
	const FFVInventoryGridSlot* EmptySlot = Algo::FindByPredicate(GridSlots,
		[](const FFVInventoryGridSlot& checkedSlot)
		{
			return checkedSlot.IsEmpty();
		});

	return EmptySlot ? EmptySlot->SlotWidget : nullptr;
	*/
}

int32 UFVAdvancedInventoryItemsGridWidget::FindEmptySlotIndex_Implementation(const FGuid& ItemId) const
{
	return UFVInventoryUIStatics::Helper_FindEmptyGridSlotIndex(this, ItemId, ParentUIComponent.GetObject() );
}

UUserWidget* UFVAdvancedInventoryItemsGridWidget::GetItemSlotWidget_Implementation(const int32 SlotIndex) const
{
	return nullptr;
	/*
	if (!GridSlots.Array().IsValidIndex(SlotIndex)) return nullptr;

	return GridSlots.Array()[SlotIndex].SlotWidget;
	*/
}

UUserWidget* UFVAdvancedInventoryItemsGridWidget::GetItemWidgetInSlot_Implementation(const int32 SlotIndex) const
{
	return nullptr;
	/*
	if (!GridSlots.Array().IsValidIndex(SlotIndex)) return nullptr;

	const TArray<FFVInventoryGridSlot> slotsArray = GridSlots.Array();
	const FFVInventoryGridSlot& checkedSlot = slotsArray[SlotIndex];

	if (!IsValid(checkedSlot.SlotWidget)) return nullptr;

	if (!checkedSlot.SlotWidget->Implements<UFVAdvancedInventoryItemSlotWidgetInterface>()) return nullptr;

	return IFVAdvancedInventoryItemSlotWidgetInterface::Execute_GetItemWidgetInSlot(checkedSlot.SlotWidget);
	*/
}

void UFVAdvancedInventoryItemsGridWidget::AddSlot_Implementation(const FFVInventoryGridSlot& SlotData)
{
	GridSlots.Add(SlotData);
}

bool UFVAdvancedInventoryItemsGridWidget::UpdateItemInSlot_Implementation(const FGuid& ItemId, const int32 SlotIndex)
{
	return UFVInventoryUIStatics::Helper_ItemsGrid_UpdateItemInSlot(this, ItemId, SlotIndex, ParentUIComponent);
}

int32 UFVAdvancedInventoryItemsGridWidget::GetStacksSizeForItem_Implementation(const FGuid& ItemId)
{
	return INDEX_NONE;
	/*
	if (!ItemId.IsValid()) return INDEX_NONE;
	auto allSlots = Execute_GetGridSlotsDataForItem(this, ItemId);
	
	return Algo::Accumulate(allSlots, 0, [&ItemId](const int32 Sum, const FFVInventoryGridSlot& gridSlot) {
		return Sum + (gridSlot.OccupiedItemId == ItemId ? gridSlot.SlotQuantity : 0);
	});
	*/
}

TSet<FFVInventoryGridSlot> UFVAdvancedInventoryItemsGridWidget::GetGridSlotsDataForItem_Implementation(
	const FGuid& ItemId)
{
	TSet<FFVInventoryGridSlot> returnValue;
	return returnValue;
	/*
	returnValue.Reserve(GridSlots.Num());
	
	Algo::CopyIf(GridSlots, returnValue, [&ItemId](const FFVInventoryGridSlot& gridSlot) {
		return gridSlot.OccupiedItemId == ItemId;
	});
	
	return returnValue;
	*/
}
