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
#include "Definitions/FVInventoryItem.h"
#include "Interfaces/Inventory/FVAdvancedInventoryInterface.h"
#include "FVInventoryComponent.generated.h"

enum class EInventoryNotificationType : uint8;

enum class EInventoryFlags : uint8;
enum class EInventoryType : uint8;

/**
 * UFVInventoryComponent manages inventory item collections with network replication support.
 * Inventory components provide item storage, modification, searching, and notification systems
 * with efficient delta replication for multiplayer inventory management.
 *
 * @see Inventory System
 * @see IFVAdvancedInventoryInterface
 * @see FInventoryItem
 */
UCLASS(ClassGroup=(FV), Blueprintable,
    AutoExpandCategories=("FV","Inventory","FV|Inventory"),
    HideCategories=("Cooking","Collision"),
    meta=(BlueprintSpawnableComponent, DisplayName="FV Inventory Component"))
class FVINVENTORYEQUIPMENTSYSTEM_API UFVInventoryComponent : public UActorComponent, public IFVAdvancedInventoryInterface
{
	GENERATED_BODY()

public:

	UFVInventoryComponent();

protected:

	virtual void BeginPlay() override;

protected:

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_InventoryItems();

	// --- Interface Functions ------------------------------
protected:
	virtual AActor* GetOwningActor_Implementation() const override;
	virtual bool AddItem_Implementation(const FFVInventoryItem& Item) override;
	virtual bool AddItemFromTemplate_Implementation(UFVInventoryItemTemplate* Template, const int32 Quantity = 1, const float Durability = 1.f) override;
	virtual bool RemoveItem_Implementation(const FGuid& ItemGuid) override;
	virtual bool RemoveItemFromTemplate_Implementation(UFVInventoryItemTemplate* Template, const int32 Quantity = 1) override;
	virtual bool CanAddItem_Implementation(const FFVInventoryItem& Item) const override;
	virtual bool CanAddItemFromTemplate_Implementation(UFVInventoryItemTemplate* Template, const int32 Quantity = 1) const override;
	virtual FFVInventoryItem FindItem_Implementation(const FInventoryItemSearchParams& SearchParams) const override;
	virtual int32 FindItemIndex_Implementation(const FInventoryItemSearchParams& SearchParams) const override;
	virtual TArray<FFVInventoryItem> FindItems_Implementation(const FInventoryItemSearchParams& SearchParams) const override;
	virtual TArray<FFVInventoryItem> GetAllItems_Implementation() const override;
	virtual bool IncreaseItemQuantity_Implementation(const FGuid& ItemGuid, const int32 Amount = 1) override;
	virtual bool DecreaseItemQuantity_Implementation(const FGuid& ItemGuid, const int32 Amount = 1) override;
	virtual bool ModifyItemDurability_Implementation(const FGuid& ItemGuid, const float DeltaDurability) override;
	virtual void ClearInventory_Implementation() override;
	virtual bool HasItem_Implementation(const FInventoryItemSearchParams& SearchParams) const override;
	virtual void ProcessInventoryNotification_Implementation(const FInventoryNotificationData& Notification) override;

	// --- Class Functions ------------------------------
protected:
	bool IsAuthority() const;
	
	UFUNCTION(Server, Reliable)
	void AddItem_Server(const FFVInventoryItem& Item);
	UFUNCTION(Server, Reliable)
	void RemoveItem_Server(const FGuid& ItemGuid);
	UFUNCTION(Server, Reliable)
	void ChangeItemQuantity_Server(const FGuid& ItemGuid, const int32 DeltaAmount);
	UFUNCTION(Server, Reliable)
	void ClearInventory_Server();

	UFUNCTION(Client, Unreliable)
	void ProcessInventoryNotification_Client(const FGuid& TargetItem, const FString& NotifType, const int32 QuantityDelta);
	UFUNCTION(Client, Unreliable)
	void PostItemAdded_Client(const FFVInventoryItem& Item);
	UFUNCTION(Client, Unreliable)
	void PostItemRemoved_Client(const FFVInventoryItem& Item);
	UFUNCTION(Client, Unreliable)
	void PostItemQuantityChanged(const FFVInventoryItem& Item, const int32 OldQuantity, const int32 NewQuantity);
	UFUNCTION(Client, Unreliable)
	void PostItemDurabilityChanged(const FFVInventoryItem& Item, const int32 OldDurability, const int32 NewDurability);

	// --- Events ------------------------------
protected:
	virtual FOnItemAdded& GetOnItemAddedEventHandle() override
	{ return OnItemAdded; };
	virtual FOnItemRemoved& GetOnItemRemovedEventHandle() override
	{ return OnItemRemoved; };
	virtual FOnItemQuantityChanged& GetOnItemQuantityChangedEventHandle() override
	{ return OnItemQuantityChanged; };
	virtual FOnItemDurabilityChanged& GetOnItemDurabilityChangedEventHandle() override
	{ return OnItemDurabilityChanged; };
	virtual FOnNotificationProcessed& GetOnNotificationProcessedEventHandle() override
	{ return OnNotificationProcessed; };

private:

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly,
		ReplicatedUsing=OnRep_InventoryItems, Category="Inventory",
		meta=(AllowPrivateAccess),
		meta=(DisplayPriority=0))
	FInventoryItemArray InventoryItems;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category="Inventory",
		meta=(AllowPrivateAccess),
		meta=(DisplayPriority=1))
	EInventoryType InventoryType;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category="Inventory",
		meta=(AllowPrivateAccess),
		meta=(DisplayPriority=2))
	EInventoryFlags InventoryTypeFlag;
	
protected:
	
	/**
	* Called when item is added to inventory
	*/
	UPROPERTY(BlueprintAssignable, Category="FV|Inventory & Equipment|Inventory|Events")
	FOnItemAdded OnItemAdded;

	/**
	* Called when item is removed from inventory
	*/
	UPROPERTY(BlueprintAssignable, Category="FV|Inventory & Equipment|Inventory|Events")
	FOnItemRemoved OnItemRemoved;

	/**
	* Called when item quantity changes
	*/
	UPROPERTY(BlueprintAssignable, Category="FV|Inventory & Equipment|Inventory|Events")
	FOnItemQuantityChanged OnItemQuantityChanged;

	/**
	* Called when item durability changes
	*/
	UPROPERTY(BlueprintAssignable, Category="FV|Inventory & Equipment|Inventory|Events")
	FOnItemDurabilityChanged OnItemDurabilityChanged;

	/**
	* Called when inventory Notification is processed
	*/
	UPROPERTY(BlueprintAssignable, Category="FV|Inventory & Equipment|Inventory|Events")
	FOnNotificationProcessed OnNotificationProcessed;
};