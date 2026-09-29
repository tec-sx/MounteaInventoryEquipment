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
#include "Components/ActorComponent.h"
#include "Definitions/FVAdvancedInventoryNotification.h"
#include "Definitions/FVInventoryBaseUIDataTypes.h"
#include "Definitions/FVInventoryItem.h"
#include "Interfaces/Inventory/FVAdvancedInventoryUIManagerInterface.h"
#include "FVInventoryUIComponent.generated.h"

class UFVAdvancedInventoryUIConfig;
class UFVAdvancedInventorySharedHUDSubsystem;

/**
 * UFVInventoryUIComponent manages user interface aspects of inventory systems.
 * UI components handle widget creation, visibility management, item selection, category filtering,
 * notification display, and grid slot persistence for comprehensive inventory interface control.
 *
 * @see Inventory UI System
 * @see IFVAdvancedInventoryUIManagerInterface
 * @see UFVInventoryComponent
 */
UCLASS(ClassGroup=(FV), Blueprintable, AutoExpandCategories=("FV","Inventory","FV|Inventory"), HideCategories=("Cooking","Collision"), 
    meta=(BlueprintSpawnableComponent),
    meta=(DisplayName="FV Inventory UI Component"))
class FVINVENTORYEQUIPMENTSYSTEM_API UFVInventoryUIComponent : public UActorComponent, public IFVAdvancedInventoryUIManagerInterface
{
	GENERATED_BODY()

public:

	UFVInventoryUIComponent();

public:
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	virtual TScriptInterface<IFVAdvancedInventoryInterface> GetParentInventory_Implementation() const override;
	virtual void SetParentInventory_Implementation(const TScriptInterface<IFVAdvancedInventoryInterface>& NewParentInventory) override;
	
	virtual bool CreateInventoryWidget_Implementation() override;
	virtual UUserWidget* GetInventoryWidget_Implementation() const override
	{
		return InventoryWidget;
	};
	virtual void RemoveInventoryWidget_Implementation() override;
	virtual bool SetInventoryWidget_Implementation(UUserWidget* NewInventoryWidget) override;

	virtual UWidget* GetActiveItemWidget_Implementation() const override
	{
		return ActiveItemWidget;
	};
	virtual void SetActiveItemWidget_Implementation(UWidget* NewActiveItemWidget) override;	

	virtual void ProcessItemAdded_Implementation(const FFVInventoryItem& AddedItem) override;
	virtual void ProcessItemModified_Implementation(const FFVInventoryItem& ModifiedItem) override;
	virtual void ProcessItemRemoved_Implementation(const FFVInventoryItem& RemovedItem) override;

	virtual void CategorySelected_Implementation(const FString& SelectedCategoryId) override;
	virtual FString GetSelectedCategoryId_Implementation() const override
	{
		return ActiveCategoryId;
	};
	virtual void ItemSelected_Implementation(const FGuid& SelectedItem) override;
	virtual FGuid GetActiveItemGuid_Implementation() const override
	{
		return ActiveItemGuid;
	};
	
	virtual TMap<FGameplayTag,FInventoryUICustomData> GetCustomItemsMap_Implementation() const override
	{
		return CustomItemsMap;
	};
	virtual void AddCustomItemToMap_Implementation(const FGameplayTag& ItemTag, const FGuid& ItemId) override;
	virtual void AppendCustomItemsMap_Implementation(const TMap<FGameplayTag, FInventoryUICustomData>& OtherItems) override
	{
		CustomItemsMap.Append(OtherItems);
	};
	virtual void ClearCustomItemsMap_Implementation() override
	{
		CustomItemsMap.Reset();
	};
	virtual bool RemoveCustomItemFromMap_Implementation(const FGameplayTag& ItemTag, const FGuid& ItemId) override;
	virtual bool IsItemStoredInCustomMap_Implementation(const FGameplayTag& ItemTag, const FGuid& ItemId) override;
	
	virtual TArray<UFVSelectableInventoryItemAction*> GetItemActionsQueue_Implementation() const override;
	virtual bool EnqueueItemAction_Implementation(UFVSelectableInventoryItemAction* ItemAction, UObject* Payload) override;
	virtual void EmptyItemActionsQueue_Implementation() override;
	virtual void CompleteQueuedAction_Implementation(UFVSelectableInventoryItemAction* ItemAction, UObject* Payload) override;
	virtual void CancelQueuedAction_Implementation(UFVSelectableInventoryItemAction* ItemAction) override;
		
	virtual FInventoryCategorySelected& GetOnCategorySelectedHandle() override
	{ return OnCategorySelected; };
	virtual FInventoryItemSelected& GetOnItemSelectedHandle() override
	{ return OnItemSelected; };

protected:
	
	
	UFVAdvancedInventorySharedHUDSubsystem* GetSharedHUDSubsystem() const;

	UFUNCTION()
	void ForwardInventoryNotificationToSubsystem(const FInventoryNotificationData& NotificationData);
	
	UFUNCTION()
	void ProcessItemDurabilityChanged(const FFVInventoryItem& Item, const float OldDurability, const float NewDurability);
	UFUNCTION()
	void ProcessItemQuantityChanged(const FFVInventoryItem& Item, const int32 OldQuantity, const int32 NewQuantity);

protected:
	
	// Custom stored map, can be used to store unique Items, like Coins, Favourites etc.
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category="Inventory", 
		meta=(NoResetToDefault))
	TMap<FGameplayTag, FInventoryUICustomData> CustomItemsMap;
	
	FActionsQueue ActionsQueue;
	
	// Currently active category in UI.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="Inventory", 
		meta=(NoResetToDefault))
	FString ActiveCategoryId;

	// Currently active item in UI.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient,  Category="Inventory", 
		meta=(NoResetToDefault))
	FGuid ActiveItemGuid;

	// Currently active item Widget.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="Inventory", 
		meta=(NoResetToDefault),
		meta=(DisplayThumbnail=false))
	TObjectPtr<UWidget> ActiveItemWidget;

private:
	
	/**
	 * Represents the parent inventory interface.
	 *
	 * This variable serves as a reference to the parent inventory system, allowing interaction with the advanced inventory interface.
	 * It enables communication between the inventory UI component and the parent inventory for functions such as handling item modifications,
	 * notifications, and other inventory-related operations.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory", 
		meta=(AllowPrivateAccess), 
		meta=(ExposeOnSpawn),
		meta=(DisplayThumbnail=false))
	TScriptInterface<IFVAdvancedInventoryInterface> ParentInventory;
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory", 
		meta=(AllowPrivateAccess),
		meta=(DisplayThumbnail=false))
	TObjectPtr<UUserWidget> InventoryWidget;
	
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category="Inventory",
		meta=(AllowPrivateAccess),
		meta=(DisplayThumbnail=false))
	TObjectPtr<UFVAdvancedInventoryUIConfig> UIConfig;
	
protected:
	
	/**
	 * Delegate that is broadcast when an inventory category is selected.
	 *
	 * This BlueprintAssignable delegate allows other components to handle the logic
	 * for Category selection in the inventory.
	 * It broadcasts CategoryId.
	 */
	UPROPERTY(BlueprintAssignable, Category="FV|Inventory")
	FInventoryCategorySelected OnCategorySelected;

	/**
	 * Delegate triggered when an inventory item is selected.
	 *
	 * This BlueprintAssignable delegate allows other components to handle the logic
	 * for Item selection in the inventory.
	 * It broadcasts ItemId.
	 */
	UPROPERTY(BlueprintAssignable, Category="FV|Inventory")
	FInventoryItemSelected OnItemSelected;
};
