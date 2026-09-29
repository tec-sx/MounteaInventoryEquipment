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

#include "Components/FVInventoryComponent.h"

#include "Algo/Copy.h"
#include "Definitions/FVAdvancedInventoryNotification.h"
#include "Definitions/FVInventoryBaseEnums.h"
#include "Definitions/FVInventoryItemTemplate.h"
#include "Logs/FVAdvancedInventoryLog.h"
#include "Net/UnrealNetwork.h"
#include "Statics/FVInventoryStatics.h"
#include "Statics/FVInventorySystemStatics.h"

UFVInventoryComponent::UFVInventoryComponent() : InventoryTypeFlag(EInventoryFlags::EIF_Private)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	UFVInventoryComponent::SetComponentTickEnabled(false);
	
	bAutoActivate = true;
	
	SetIsReplicatedByDefault(true);
	SetActiveFlag(true);

	ComponentTags.Append( { TEXT("FV"), TEXT("Inventory") } );
}

void UFVInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UFVInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UFVInventoryComponent, InventoryItems, COND_InitialOrOwner);
}

void UFVInventoryComponent::OnRep_InventoryItems()
{
	// Process replicated Items
	// TODO refresh UI by sending Command
	// Do I need to do anything? Maybe I dont
}

AActor* UFVInventoryComponent::GetOwningActor_Implementation() const
{
	return GetOwner();
}

bool UFVInventoryComponent::AddItem_Implementation(const FFVInventoryItem& Item)
{
	if (!Execute_CanAddItem(this, Item))
	{
		auto notificationData =  UFVInventoryStatics::CreateNotificationData(
			FVInventoryNotificationBaseTypes::ItemNotUpdated,
			this,
			Item.GetGuid(),
			0
		);
		Execute_ProcessInventoryNotification(this, notificationData);
		return false;
	}

	if (!IsAuthority())
	{
		AddItem_Server(Item);
		return true;
	}
	
	auto existingItem = Execute_FindItem(this, FInventoryItemSearchParams(Item.GetGuid()));
	if (!existingItem.IsItemValid())
		existingItem = Execute_FindItem(this, FInventoryItemSearchParams(Item.GetTemplate()));

	if (existingItem.IsItemValid())
	{
		const auto existingTemplate = existingItem.GetTemplate();
		const auto newTemplate = Item.GetTemplate();
		if (UFVInventorySystemStatics::HasFlag(existingTemplate->ItemFlags, EInventoryItemFlags::EIIF_Unique)
			|| UFVInventorySystemStatics::HasFlag(newTemplate->ItemFlags, EInventoryItemFlags::EIIF_Unique)
			/*|| !UFVInventorySystemStatics::HasFlag(newTemplate->ItemFlags, EInventoryItemFlags::EIIF_Stackable)
			|| !UFVInventorySystemStatics::HasFlag(existingTemplate->ItemFlags, EInventoryItemFlags::EIIF_Stackable)*/)
		{
			Execute_ProcessInventoryNotification(this, UFVInventoryStatics::CreateNotificationData(
				FVInventoryNotificationBaseTypes::ItemNotUpdated,
				this,
				Item.GetGuid(),
				0
			));
			return false;
		}
		
		const int32 AvailableSpace = Item.GetTemplate()->MaxQuantity - existingItem.GetQuantity();
		const int32 AmountToAdd = FMath::Min(Item.GetQuantity(), AvailableSpace);

		if (Item.IsItemInInventory())
		{
			if (!Execute_DecreaseItemQuantity(Item.GetOwningInventory().GetObject(), Item.GetGuid(), AmountToAdd))
			{
				Execute_ProcessInventoryNotification(this, UFVInventoryStatics::CreateNotificationData(
					FVInventoryNotificationBaseTypes::ItemNotUpdated,
					this,
					Item.GetGuid(),
					0
				));
				return false;
			}
		}

		if (Execute_IncreaseItemQuantity(this, existingItem.GetGuid(), AmountToAdd))
		{
			InventoryItems.MarkArrayDirty();
			return true;
		}
	}
	else
	{
		const int32 AmountToAdd = FMath::Min(Item.GetQuantity(), Item.GetTemplate()->MaxQuantity);
		
		if (Item.IsItemInInventory())
		{
			if (!Execute_DecreaseItemQuantity(Item.GetOwningInventory().GetObject(), Item.GetGuid(), AmountToAdd))
			{
				Execute_ProcessInventoryNotification(this, UFVInventoryStatics::CreateNotificationData(
					FVInventoryNotificationBaseTypes::ItemNotUpdated,
					this,
					Item.GetGuid(),
					0
				));
				return false;
			}
		}

		FFVInventoryItem newItem = Item;
		if (AmountToAdd != Item.GetQuantity())
			newItem.Quantity += AmountToAdd;

		InventoryItems.Items.Add(newItem);
		InventoryItems.Items.Last().SetOwningInventory(this);
		InventoryItems.MarkArrayDirty();
		
		OnItemAdded.Broadcast(newItem);
		PostItemAdded_Client(newItem);
		return true;
	}

	return false;
}

bool UFVInventoryComponent::AddItemFromTemplate_Implementation(UFVInventoryItemTemplate* Template, const int32 Quantity, const float Durability)
{
	if (!IsValid(Template))
	{
		//Invalid item template
		Execute_ProcessInventoryNotification(this, UFVInventoryStatics::CreateNotificationData(
			FVInventoryNotificationBaseTypes::ItemNotUpdated,
			this,
			FGuid(),
			0
		));
		return false;
	}

	return Execute_AddItem(this, FFVInventoryItem(Template, Quantity, Durability, nullptr));
}

bool UFVInventoryComponent::RemoveItem_Implementation(const FGuid& ItemGuid)
{
	if (!IsActive()) return false;
	
	const int32 ItemIndex = Execute_FindItemIndex(this, FInventoryItemSearchParams(ItemGuid));
	if (ItemIndex == INDEX_NONE)
		return false;

	const FFVInventoryItem RemovedItem = InventoryItems.Items[ItemIndex];
	
	OnItemRemoved.Broadcast(RemovedItem);
	
	if (!IsAuthority())
	{
		RemoveItem_Server(ItemGuid);
		return true;
	}
	
	PostItemRemoved_Client(RemovedItem);
	
	InventoryItems.Items.RemoveAt(ItemIndex);
	InventoryItems.MarkArrayDirty();

	return true;
}

bool UFVInventoryComponent::RemoveItemFromTemplate_Implementation(UFVInventoryItemTemplate* const Template, const int32 Quantity)
{
	if (!IsValid(Template))
		return false;

	auto inventoryItem = Execute_FindItem(this, FInventoryItemSearchParams(Template));
	if (!inventoryItem.IsItemValid())
	{
		// Cannot find item matching template with sufficient quantity
		Execute_ProcessInventoryNotification(this, UFVInventoryStatics::CreateNotificationData(
				FVInventoryNotificationBaseTypes::ItemNotUpdated,
				this,
				FGuid(),
				0
			));
		return false;
	}
	
	if (inventoryItem.Quantity - Quantity <= 0)
	{
		return Execute_RemoveItem(this, inventoryItem.GetGuid());
	}
	else
	{
		if (!IsAuthority())
			ChangeItemQuantity_Server(inventoryItem.GetGuid(), -Quantity);
		else
			Execute_DecreaseItemQuantity(this, inventoryItem.GetGuid(), Quantity);
		return true;
	}	
}

bool UFVInventoryComponent::CanAddItem_Implementation(const FFVInventoryItem& Item) const
{
	if (!IsActive()) return false;
	
	if (!Item.IsItemValid() || !Item.GetTemplate())
		return false;
	
	auto existingItem = Execute_FindItem(this, FInventoryItemSearchParams(Item.GetGuid()));
	if (!existingItem.IsItemValid()) 
		existingItem = Execute_FindItem(this, FInventoryItemSearchParams(Item.GetTemplate()));

	if (existingItem.IsItemValid())
		return existingItem.GetQuantity() < existingItem.GetTemplate()->MaxQuantity;

	return true;
}

bool UFVInventoryComponent::CanAddItemFromTemplate_Implementation(UFVInventoryItemTemplate* const Template, const int32 Quantity) const
{
	if (!IsValid(Template))
		return false;

	return Execute_CanAddItem(this, FFVInventoryItem(Template, Quantity, Template->BaseDurability, nullptr));
}

FFVInventoryItem UFVInventoryComponent::FindItem_Implementation(const FInventoryItemSearchParams& SearchParams) const
{
	const auto foundItem = InventoryItems.Items.FindByPredicate([&SearchParams](const FFVInventoryItem& Item)
	{
		if (SearchParams.bSearchByGuid && Item.GetGuid() != SearchParams.ItemGuid)
			return false;

		if (SearchParams.bSearchByTemplate && Item.GetTemplate() != SearchParams.Template)
			return false;

		if (SearchParams.bSearchByTags)
		{
			if (SearchParams.bRequireAllTags)
				return Item.GetCustomData().HasAll(SearchParams.Tags);
			else
				return Item.GetCustomData().HasAny(SearchParams.Tags);
		}
		
		if (SearchParams.bSearchByCategory && Item.GetTemplate()->ItemCategory != SearchParams.CategoryId)
			return false;
		
		if (SearchParams.bSearchByRarity && Item.GetTemplate()->ItemRarity != SearchParams.RarityId)
			return false;

		return true;
	});

	return foundItem ? *foundItem : FFVInventoryItem();
}

int32 UFVInventoryComponent::FindItemIndex_Implementation(const FInventoryItemSearchParams& SearchParams) const
{
	return InventoryItems.Items.IndexOfByPredicate([&SearchParams](const FFVInventoryItem& Item)
	{
		if (SearchParams.bSearchByGuid && Item.GetGuid() != SearchParams.ItemGuid)
			return false;

		if (SearchParams.bSearchByTemplate && Item.GetTemplate() != SearchParams.Template)
			return false;

		if (SearchParams.bSearchByTags)
		{
			if (SearchParams.bRequireAllTags)
				return Item.GetCustomData().HasAll(SearchParams.Tags);
			else
				return Item.GetCustomData().HasAny(SearchParams.Tags);
		}

		return true;
	});
}

TArray<FFVInventoryItem> UFVInventoryComponent::FindItems_Implementation(const FInventoryItemSearchParams& SearchParams) const
{
	TArray<FFVInventoryItem> returnResult;
	
	if (!SearchParams.bSearchByGuid && !SearchParams.bSearchByTemplate && 
		!SearchParams.bSearchByTags && !SearchParams.bSearchByCategory && 
		!SearchParams.bSearchByRarity)
	{
		return InventoryItems.Items;
	}
    
	Algo::CopyIf(InventoryItems.Items, returnResult, [&SearchParams](const FFVInventoryItem& Item) -> bool
	{
		if (!Item.IsItemValid())
			return false;
		
		if (SearchParams.bSearchByGuid && Item.GetGuid() == SearchParams.ItemGuid)
			return true;

		if (SearchParams.bSearchByTemplate && Item.GetTemplate() == SearchParams.Template)
			return true;

		if (SearchParams.bSearchByTags)
		{
			if (SearchParams.bRequireAllTags)
			{
				if (Item.GetCustomData().HasAll(SearchParams.Tags))
					return true;
			}
			else
			{
				if (Item.GetCustomData().HasAny(SearchParams.Tags))
					return true;
			}
		}

		if (SearchParams.bSearchByCategory && Item.GetTemplate()->ItemCategory == SearchParams.CategoryId)
			return true;
        
		if (SearchParams.bSearchByRarity && Item.GetTemplate()->ItemRarity == SearchParams.RarityId)
			return true;
		
		return false;
	});
    
	return returnResult;
}

TArray<FFVInventoryItem> UFVInventoryComponent::GetAllItems_Implementation() const
{
	return InventoryItems.Items;
}

bool UFVInventoryComponent::IncreaseItemQuantity_Implementation(const FGuid& ItemGuid, const int32 Amount)
{
	if (!IsActive()) return false;
	
	if (!IsAuthority())
	{
		ChangeItemQuantity_Server(ItemGuid, Amount);
		return true;
	}
	
	const int32 index = Execute_FindItemIndex(this, FInventoryItemSearchParams(ItemGuid));
	if (index != INDEX_NONE && InventoryItems.Items.IsValidIndex(index))
	{
		auto& inventoryItem = InventoryItems.Items[index];
		
		const int32 OldQuantity = inventoryItem.GetQuantity();
		if (inventoryItem.SetQuantity(OldQuantity + Amount))
		{
			InventoryItems.MarkItemDirty(inventoryItem);
			OnItemQuantityChanged.Broadcast(inventoryItem, OldQuantity, inventoryItem.GetQuantity());

			// TODO: Don't spam client if same as server
			PostItemQuantityChanged(inventoryItem, OldQuantity, inventoryItem.GetQuantity());
			return true;
		}
	}
	return false;
}

bool UFVInventoryComponent::DecreaseItemQuantity_Implementation(const FGuid& ItemGuid, const int32 Amount)
{
	if (!IsActive()) return false;
	
	if (!IsAuthority())
	{
		ChangeItemQuantity_Server(ItemGuid, -Amount);
		return true;
	}
	
	const int32 index = Execute_FindItemIndex(this, FInventoryItemSearchParams(ItemGuid));
	if (index != INDEX_NONE && InventoryItems.Items.IsValidIndex(index))
	{
		auto& inventoryItem = InventoryItems.Items[index];
		
		const int32 OldQuantity =inventoryItem.GetQuantity();
		const int32 NewQuantity = OldQuantity - Amount;
				 
		if (NewQuantity <= 0)
			return Execute_RemoveItem(this, ItemGuid);
						 
		if (inventoryItem.SetQuantity(NewQuantity))
		{
			InventoryItems.MarkItemDirty(inventoryItem);
			OnItemQuantityChanged.Broadcast(inventoryItem, OldQuantity, NewQuantity);
			PostItemQuantityChanged(inventoryItem, OldQuantity, NewQuantity);
			return true;
		}
	}
	return false;
}

bool UFVInventoryComponent::ModifyItemDurability_Implementation(const FGuid& ItemGuid, const float DeltaDurability)
{
	if (!IsActive()) return false;
	
	if (!IsAuthority())
		return false;
	
	const int32 index = Execute_FindItemIndex(this, FInventoryItemSearchParams(ItemGuid));
	if (index != INDEX_NONE && InventoryItems.Items.IsValidIndex(index))
	{
		auto& inventoryItem = InventoryItems.Items[index];
		const float OldDurability = inventoryItem.GetDurability();
		const float NewDurability = FMath::Clamp(OldDurability + DeltaDurability, 0.f, 1.f);
				 
		if (inventoryItem.SetDurability(NewDurability))
		{
			InventoryItems.MarkItemDirty(inventoryItem);
			OnItemDurabilityChanged.Broadcast(inventoryItem, OldDurability, NewDurability);
			PostItemDurabilityChanged(inventoryItem, OldDurability, NewDurability);
			return true;
		}
	}
	return false;
}

void UFVInventoryComponent::ClearInventory_Server_Implementation()
{
	Execute_ClearInventory(this);
}

void UFVInventoryComponent::ClearInventory_Implementation()
{
	if (!IsAuthority())
	{
		ClearInventory_Server();
		return;
	}
	
	for (const auto& Item : InventoryItems.Items)
	{
		OnItemRemoved.Broadcast(Item);
	}
	InventoryItems.Items.Empty();
}

bool UFVInventoryComponent::HasItem_Implementation(const FInventoryItemSearchParams& SearchParams) const
{
	const FFVInventoryItem foundItem = Execute_FindItem(this, SearchParams);
	return foundItem.IsItemValid();
}

void UFVInventoryComponent::ProcessInventoryNotification_Implementation(const FInventoryNotificationData& Notification)
{
	if (!Notification.NotificationConfig.bIsEnabled)
		return;
	
	if (!IsAuthority() || (IsAuthority() && UFVInventorySystemStatics::CanExecuteCosmeticEvents(GetWorld())))
		OnNotificationProcessed.Broadcast(Notification);
	else if (IsAuthority() && !UFVInventorySystemStatics::CanExecuteCosmeticEvents(GetWorld()))
		ProcessInventoryNotification_Client(Notification.ItemGuid, Notification.Type, Notification.DeltaAmount);
}

bool UFVInventoryComponent::IsAuthority() const
{
	const AActor* Owner = GetOwner();
	if (!Owner || !Owner->GetWorld())
		return false;

	if (const ENetMode NetMode = Owner->GetWorld()->GetNetMode(); NetMode == NM_Standalone)
		return true;
	
	if (Owner->HasAuthority())
		return true;
		
	return false;
}

void UFVInventoryComponent::ChangeItemQuantity_Server_Implementation(const FGuid& ItemGuid, const int32 DeltaAmount)
{
	if (DeltaAmount < 0)
		Execute_DecreaseItemQuantity(this, ItemGuid, FMath::Abs(DeltaAmount));
	else if (DeltaAmount > 0)
		Execute_IncreaseItemQuantity(this, ItemGuid, FMath::Abs(DeltaAmount));
}

void UFVInventoryComponent::ProcessInventoryNotification_Client_Implementation(const FGuid& TargetItem, const FString& NotifType, const int32 QuantityDelta)
{
	// Wait for the next tick before executing the broadcast to avoid timing issues with replication (item might not exist yet)
	GetWorld()->GetTimerManager().SetTimerForNextTick([this, NotifType, TargetItem, QuantityDelta]()
	{
		auto targetItem = Execute_FindItem(this, FInventoryItemSearchParams(TargetItem));
		
		OnNotificationProcessed.Broadcast(UFVInventoryStatics::CreateNotificationData(
			NotifType,
			this,
			targetItem.GetGuid(),
			QuantityDelta
		));
	});
}

void UFVInventoryComponent::AddItem_Server_Implementation(const FFVInventoryItem& Item)
{
	Execute_AddItem(this, Item);
}

void UFVInventoryComponent::RemoveItem_Server_Implementation(const FGuid& ItemGuid)
{
	Execute_RemoveItem(this, ItemGuid);
}

void UFVInventoryComponent::PostItemAdded_Client_Implementation(const FFVInventoryItem& Item)
{
	if (IsAuthority() && UFVInventorySystemStatics::CanExecuteCosmeticEvents(GetWorld()))
	{
		Execute_ProcessInventoryNotification(this, UFVInventoryStatics::CreateNotificationData(
				FVInventoryNotificationBaseTypes::ItemAdded,
				this,
				Item.GetGuid(),
				Item.Quantity
			));
	}
	
	OnItemAdded.Broadcast(Item);
}

void UFVInventoryComponent::PostItemRemoved_Client_Implementation(const FFVInventoryItem& Item)
{
	if (IsAuthority() && UFVInventorySystemStatics::CanExecuteCosmeticEvents(GetWorld()))
	{
		Execute_ProcessInventoryNotification(this, UFVInventoryStatics::CreateNotificationData(
				FVInventoryNotificationBaseTypes::ItemRemoved,
				this,
				Item.GetGuid(),
				Item.GetQuantity()
			));
	}
	
	OnItemRemoved.Broadcast(Item);
}

void UFVInventoryComponent::PostItemQuantityChanged_Implementation(const FFVInventoryItem& Item, const int32 OldQuantity, const int32 NewQuantity)
{
	if (IsAuthority() && UFVInventorySystemStatics::CanExecuteCosmeticEvents(GetWorld()))
	{
		Execute_ProcessInventoryNotification(this, UFVInventoryStatics::CreateNotificationData(
				OldQuantity > NewQuantity ? FVInventoryNotificationBaseTypes::ItemRemoved : FVInventoryNotificationBaseTypes::ItemAdded,
				this,
				Item.GetGuid(),
				OldQuantity > NewQuantity ? FMath::Abs(OldQuantity - NewQuantity) : -FMath::Abs(OldQuantity - NewQuantity)
			));
	}
	
	OnItemQuantityChanged.Broadcast(Item, OldQuantity, NewQuantity);
}

void UFVInventoryComponent::PostItemDurabilityChanged_Implementation(const FFVInventoryItem& Item, const int32 OldDurability, const int32 NewDurability)
{
	OnItemDurabilityChanged.Broadcast(Item, OldDurability, NewDurability);
}