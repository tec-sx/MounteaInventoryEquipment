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

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/Widgets/Items/FVAdvancedInventoryItemsGridWidgetInterface.h"
#include "Widgets/FVAdvancedInventoryBaseWidget.h"
#include "FVAdvancedInventoryItemsGridWidget.generated.h"

class IFVAdvancedInventoryUIManagerInterface;

/**
 * UFVAdvancedInventoryItemsGridWidget implements grid-based inventory layout and item management.
 * Grid widgets provide 2D spatial organization for inventory items with drag-drop functionality,
 * slot-based placement, item swapping, and coordinate-based positioning for structured inventory systems.
 *
 * @see Inventory System
 * @see UFVAdvancedInventoryBaseWidget
 * @see FFVInventoryGridSlot
 */
UCLASS()
class FVINVENTORYEQUIPMENTSYSTEM_API UFVAdvancedInventoryItemsGridWidget 
	: public UFVAdvancedInventoryBaseWidget, public IFVAdvancedInventoryItemsGridWidgetInterface
{
	GENERATED_BODY()

public:

	// Interface Implementations

	virtual bool AddItemToEmptySlot_Implementation(const FGuid& ItemId) override;
	virtual bool AddItemToSlot_Implementation(const FGuid& ItemId, const int32 SlotIndex) override;
	virtual bool RemoveItemFromSlot_Implementation(const int32 SlotIndex) override;
	virtual bool RemoveItemFromGrid_Implementation(const FGuid& ItemId, const int32 Quantity = -1) override;
	virtual FGuid GetItemInSlot_Implementation(const int32 SlotIndex) const override;
	virtual bool SwapItemsBetweenSlots_Implementation(const int32 SlotIndex1, const int32 SlotIndex2) override;
	virtual void ClearAllSlots_Implementation() override;
	virtual int32 GetTotalSlots_Implementation() const override;
	virtual bool IsSlotEmpty_Implementation(const int32 SlotIndex) const override;
	virtual int32 GetSlotIndexByItem_Implementation(const FGuid& ItemId) const override;
	virtual int32 GetGridSlotIndexByCoords_Implementation(const FIntPoint& SlotCoords) const override;
	virtual bool IsItemInGrid_Implementation(const FGuid& ItemId) const override;
	virtual FFVInventoryGridSlot GetGridSlotData_Implementation(const int32 SlotIndex) const override;
	virtual TSet<FFVInventoryGridSlot> GetGridSlotsData_Implementation() const override;
	virtual UUserWidget* FindEmptyWidgetSlot_Implementation() const override;
	virtual int32 FindEmptySlotIndex_Implementation(const FGuid& ItemId) const override;
	virtual UUserWidget* GetItemSlotWidget_Implementation(const int32 SlotIndex) const override;
	virtual UUserWidget* GetItemWidgetInSlot_Implementation(const int32 SlotIndex) const override;
	virtual void AddSlot_Implementation(const FFVInventoryGridSlot& SlotData) override;
	virtual bool UpdateItemInSlot_Implementation(const FGuid& ItemId, const int32 SlotIndex = 0) override;
	virtual int32 GetStacksSizeForItem_Implementation(const FGuid& ItemId)  override;
	virtual TSet<FFVInventoryGridSlot> GetGridSlotsDataForItem_Implementation(const FGuid& ItemId)   override;
	
protected:

	UPROPERTY(BlueprintReadOnly, Category="Widget|Data")
	TSet<FFVInventoryGridSlot> GridSlots;
};
