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


#include "Components/FVInventoryUIComponent.h"

#include "Algo/AnyOf.h"
#include "Blueprint/UserWidget.h"
#include "Decorations/FVSelectableInventoryItemAction.h"
#include "Engine/LocalPlayer.h"

#include "Definitions/FVInventoryBaseCommands.h"

#include "Helpers/FVAdvancedInventoryWidgetPayload.h"

#include "Interfaces/Inventory/FVAdvancedInventoryInterface.h"
#include "Interfaces/UserInterface/FVAdvancedInventorySharedHUDInterface.h"
#include "Interfaces/Widgets/FVInventorySystemWrapperWidgetInterface.h"
#include "Interfaces/Widgets/FVInventoryGenericWidgetInterface.h"
#include "Interfaces/Widgets/Inventory/FVAdvancedInventoryWidgetInterface.h"
#include "Interfaces/Widgets/Notification/FVInventoryNotificationContainerWidgetInterface.h"
#include "Interfaces/Widgets/Notification/FVInventoryNotificationWidgetInterface.h"

#include "Logs/FVAdvancedInventoryLog.h"

#include "Settings/FVAdvancedInventorySettings.h"
#include "Settings/FVAdvancedInventoryUIConfig.h"

#include "Statics/FVInventorySystemStatics.h"
#include "Statics/FVInventoryUIStatics.h"
#include "Subsystems/FVAdvancedInventoryUISubsystem.h"
#include "Subsystems/FVAdvancedInventorySharedHUDSubsystem.h"

class UFVAdvancedInventorySettings;

UFVInventoryUIComponent::UFVInventoryUIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	UFVInventoryUIComponent::SetComponentTickEnabled(false);
	
	bAutoActivate = true;
	
	SetIsReplicatedByDefault(false);
	SetActiveFlag(true);

	ComponentTags.Append( { TEXT("FV"), TEXT("Inventory"), TEXT("UI") } );
}

void UFVInventoryUIComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && 
		(GetOwnerRole() == ROLE_Authority || GetOwnerRole() == ROLE_AutonomousProxy) && 
		UFVInventorySystemStatics::CanExecuteCosmeticEvents(GetWorld()))
	{
		UIConfig = GetDefault<UFVAdvancedInventorySettings>()->AdvancedInventoryUISettingsConfig.LoadSynchronous();
		Execute_EmptyItemActionsQueue(this); // Do NOT receive any queue items
		{
			auto inventoryComponent = GetOwner()->FindComponentByInterface(UFVAdvancedInventoryInterface::StaticClass());
			if (!IsValid(inventoryComponent))
				LOG_ERROR(TEXT("[FVInventoryUIComponent] Cannot find 'Inventory' component in Parent! UI will NOT work!"))
			else
			{
				Execute_SetParentInventory(this, inventoryComponent);
				ensureMsgf(ParentInventory.GetObject() != nullptr, TEXT("[FVInventoryUIComponent] Failed to update 'ParentInventory'"));

				ParentInventory->GetOnNotificationProcessedEventHandle().AddUniqueDynamic(this, &UFVInventoryUIComponent::ForwardInventoryNotificationToSubsystem);
			
				ParentInventory->GetOnItemAddedEventHandle().AddUniqueDynamic(this, &UFVInventoryUIComponent::ProcessItemAdded);
				ParentInventory->GetOnItemRemovedEventHandle().AddUniqueDynamic(this, &UFVInventoryUIComponent::ProcessItemRemoved);

				ParentInventory->GetOnItemDurabilityChangedEventHandle().AddUniqueDynamic(this, &UFVInventoryUIComponent::ProcessItemDurabilityChanged);
				ParentInventory->GetOnItemQuantityChangedEventHandle().AddUniqueDynamic(this, &UFVInventoryUIComponent::ProcessItemQuantityChanged);
			}
		
			const auto inventoryUISubsystem = UFVInventoryUIStatics::GetInventoryUISubsystem(
				UFVInventoryUIStatics::FindPlayerController(GetOwner(), 3));
			if (!inventoryUISubsystem)
				LOG_ERROR(TEXT("[FVInventoryUIComponent] Cannot find 'Inventory UI Subsystem'. UI will NOT work!"))
			else
				inventoryUISubsystem->RegisterInventoryUIManager(this);
		}
	}
}

void UFVInventoryUIComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Execute_EmptyItemActionsQueue(this); // we don't care if this runs on server I guess?
	
	switch (EndPlayReason)
	{		
		case EEndPlayReason::EndPlayInEditor:
			break;
		case EEndPlayReason::RemovedFromWorld:
		case EEndPlayReason::Destroyed:
		case EEndPlayReason::LevelTransition:
		case EEndPlayReason::Quit:
			{
				if (GetOwner() && 
					(GetOwnerRole() == ROLE_Authority || GetOwnerRole() == ROLE_AutonomousProxy) && 
					UFVInventorySystemStatics::CanExecuteCosmeticEvents(GetWorld()))
				{
					const auto inventoryUISubsystem = UFVInventoryUIStatics::GetInventoryUISubsystem(
						UFVInventoryUIStatics::FindPlayerController(GetOwner(), 3));
					if (!inventoryUISubsystem)
						LOG_ERROR(TEXT("[FVInventoryUIComponent] Cannot find 'Inventory UI Subsystem'. UI will UNREGISTER properly!"))
					else
						inventoryUISubsystem->UnregisterInventoryUIManager(this);
				}
			}
			break;
	}
	
	Super::EndPlay(EndPlayReason);
}

TScriptInterface<IFVAdvancedInventoryInterface> UFVInventoryUIComponent::GetParentInventory_Implementation() const
{
	return ParentInventory;
}

void UFVInventoryUIComponent::SetParentInventory_Implementation(const TScriptInterface<IFVAdvancedInventoryInterface>& NewParentInventory)
{
	if (ParentInventory != NewParentInventory) 
		ParentInventory = NewParentInventory;
}

UFVAdvancedInventorySharedHUDSubsystem* UFVInventoryUIComponent::GetSharedHUDSubsystem() const
{
	AActor* ownerActor = GetOwner();
	if (!IsValid(ownerActor))
		return nullptr;

	const APlayerController* playerController = UFVInventoryUIStatics::FindPlayerController(ownerActor, 3);
	if (!IsValid(playerController))
		return nullptr;

	const ULocalPlayer* localPlayer = playerController->GetLocalPlayer();
	return IsValid(localPlayer) ? localPlayer->GetSubsystem<UFVAdvancedInventorySharedHUDSubsystem>() : nullptr;
}

bool UFVInventoryUIComponent::CreateInventoryWidget_Implementation()
{
	const UFVAdvancedInventoryUIConfig* inventoryUIConfig = UFVInventoryUIStatics::GetInventoryUISettingsConfig();
	if (!inventoryUIConfig)
	{
		LOG_ERROR(TEXT("[CreateInventoryWidget] Unable to load Inventory Config!"))
		return false;
	}

	auto widgetClass = inventoryUIConfig->InventoryWidgetClass.LoadSynchronous();
	if (!IsValid(widgetClass))
	{
		LOG_ERROR(TEXT("[CreateInventoryWidget] Unable to load Inventory UI Class from Config!"))
		return false;
	}
	if (!widgetClass->ImplementsInterface(UFVAdvancedInventoryWidgetInterface::StaticClass()))
	{
		LOG_ERROR(TEXT("[CreateInventoryWidget] Inventory UI Class must implement `FVAdvancedInventoryWidgetInterface`!"))
		return false;
	}

	FString Message = TEXT("");
	bool bSuccess = true;
	
	int seachDepth = 0;
	APlayerController* playerController = UFVInventoryUIStatics::FindPlayerController(GetOwner(), seachDepth);
	if (!playerController || !playerController->IsLocalController())
	{
		Message = !playerController ? TEXT("Invalid Player Controller!") : TEXT("UI can be shown only to Local Players!");
		bSuccess = false;
	}
	if (!bSuccess)
	{
		LOG_ERROR(TEXT("[CreateInventoryWidget] Failed to find Player Controller. Message:\n%s"), *Message)
		return false;
	}

	if (!playerController->IsLocalController())
	{
		LOG_WARNING(TEXT("[CreateInventoryWidget] UI Can be created for Local Players only!"))
		return false;
	}

	auto newWidget = CreateWidget<UUserWidget>(playerController, widgetClass);
	if (!newWidget)
	{
		LOG_ERROR(TEXT("[CreateInventoryWidget] Failed to create Inventory Widget!"))
		return false;
	}
	if (!newWidget->Implements<UFVAdvancedInventoryWidgetInterface>())
	{
		LOG_ERROR(TEXT("[CreateInventoryWidget] Inventory UI must implement `FVAdvancedInventoryWidgetInterface`!"))
		return false;
	}

	InventoryWidget = newWidget;
	
	TScriptInterface<IFVAdvancedInventoryWidgetInterface> inventoryWidget = InventoryWidget;
	ensure(inventoryWidget.GetObject() != nullptr);

	const auto sharedHUDSubsystem = GetSharedHUDSubsystem();
	if (!IsValid(sharedHUDSubsystem) || !IsValid(IFVAdvancedInventorySharedHUDInterface::Execute_GetWrapperWidget(sharedHUDSubsystem)))
	{
		LOG_WARNING(TEXT("[FVInventoryUIComponent] Cannot find 'Shared HUD Subsystem'. UI might not work properly"))
		if (InventoryWidget->Implements<UFVInventoryGenericWidgetInterface>())
			IFVInventoryGenericWidgetInterface::Execute_ProcessInventoryWidgetCommand(InventoryWidget, InventoryUICommands::General::Create, nullptr);
	}
	else
		IFVAdvancedInventorySharedHUDInterface::Execute_ExecuteWidgetCommand(sharedHUDSubsystem, InventoryUICommands::General::Create, InventoryWidget);

	return true;
}

void UFVInventoryUIComponent::RemoveInventoryWidget_Implementation()
{
	if (!IsValid(InventoryWidget))
		return;
	
	const auto sharedHUDSubsystem = GetSharedHUDSubsystem();
	if (!IsValid(sharedHUDSubsystem) || !IsValid(IFVAdvancedInventorySharedHUDInterface::Execute_GetWrapperWidget(sharedHUDSubsystem)))
	{
		LOG_WARNING(TEXT("[FVInventoryUIComponent] Cannot find 'Shared HUD Subsystem'. UI might not work properly"))
		InventoryWidget->RemoveFromParent();
		InventoryWidget = nullptr;
	}	
	else
		IFVAdvancedInventorySharedHUDInterface::Execute_ExecuteWidgetCommand(sharedHUDSubsystem, InventoryUICommands::General::Remove, InventoryWidget);
}

bool UFVInventoryUIComponent::SetInventoryWidget_Implementation(UUserWidget* NewInventoryWidget)
{
	if (InventoryWidget == NewInventoryWidget)
		return false;
	
	InventoryWidget = NewInventoryWidget;
	return true;
}

void UFVInventoryUIComponent::SetActiveItemWidget_Implementation(UWidget* NewActiveItemWidget)
{
	if (ActiveItemWidget != NewActiveItemWidget)
		ActiveItemWidget = NewActiveItemWidget;
	
	if (UIConfig && UIConfig->bAllowAutoFocus && ActiveItemWidget)
		ActiveItemWidget->SetFocus();	
}

void UFVInventoryUIComponent::ProcessItemAdded_Implementation(const FFVInventoryItem& AddedItem)
{
	if (!IsValid(InventoryWidget))
	{
		LOG_WARNING(TEXT("[ProcessItemAdded] Invalid Inventory UI!")) return;
	}

	if (InventoryWidget->Implements<UFVInventoryGenericWidgetInterface>())
	{
		UFVAdvancedInventoryWidgetPayload* newPayload = NewObject<UFVAdvancedInventoryWidgetPayload>();
		newPayload->PayloadData.Add(AddedItem.Guid);
		IFVInventoryGenericWidgetInterface::Execute_ProcessInventoryWidgetCommand(InventoryWidget, InventoryUICommands::Items::Added, newPayload);
	}
}

void UFVInventoryUIComponent::ProcessItemModified_Implementation(const FFVInventoryItem& ModifiedItem)
{
	if (!IsValid(InventoryWidget))
	{
		LOG_WARNING(TEXT("[ProcessItemAdded] Invalid Inventory UI!"))
		return;
	}

	if (InventoryWidget->Implements<UFVInventoryGenericWidgetInterface>())
	{
		UFVAdvancedInventoryWidgetPayload* newPayload = NewObject<UFVAdvancedInventoryWidgetPayload>();
		newPayload->PayloadData.Add(ModifiedItem.Guid);
		newPayload->PayloadQuantities.Add(ModifiedItem.Quantity);
		IFVInventoryGenericWidgetInterface::Execute_ProcessInventoryWidgetCommand(InventoryWidget, InventoryUICommands::Items::Modified, newPayload);
	}
}

void UFVInventoryUIComponent::ProcessItemRemoved_Implementation(const FFVInventoryItem& RemovedItem)
{
	if (!IsValid(InventoryWidget))
	{
		LOG_WARNING(TEXT("[ProcessItemAdded] Invalid Inventory UI!")) 
		return;
	}

	if (InventoryWidget->Implements<UFVInventoryGenericWidgetInterface>())
	{
		UFVAdvancedInventoryWidgetPayload* newPayload = NewObject<UFVAdvancedInventoryWidgetPayload>();
		newPayload->PayloadData.Add(RemovedItem.Guid);
		IFVInventoryGenericWidgetInterface::Execute_ProcessInventoryWidgetCommand(InventoryWidget, InventoryUICommands::Items::Removed, newPayload);
	}
}

void UFVInventoryUIComponent::CategorySelected_Implementation(const FString& SelectedCategoryId)
{
	if (ActiveCategoryId.Equals(SelectedCategoryId, ESearchCase::IgnoreCase)) return;
	ActiveCategoryId = SelectedCategoryId;
	ActiveItemGuid = FGuid();	
	ActiveItemWidget = nullptr;
	
	if (IsValid(InventoryWidget) && InventoryWidget->Implements<UFVInventoryGenericWidgetInterface>())
	{
		TScriptInterface<IFVInventoryGenericWidgetInterface> genericWidget = InventoryWidget;
		genericWidget->Execute_ProcessInventoryWidgetCommand(InventoryWidget, InventoryUICommands::Categories::Selected, nullptr);
	}

	OnCategorySelected.Broadcast(SelectedCategoryId);
}

void UFVInventoryUIComponent::ItemSelected_Implementation(const FGuid& SelectedItem)
{
	if (ActiveItemGuid == SelectedItem) return;
	ActiveItemGuid = SelectedItem;
	
	if (IsValid(InventoryWidget) && InventoryWidget->Implements<UFVInventoryGenericWidgetInterface>())
	{
		TScriptInterface<IFVInventoryGenericWidgetInterface> genericWidget = InventoryWidget;
		genericWidget->Execute_ProcessInventoryWidgetCommand(InventoryWidget, InventoryUICommands::Items::Selected, nullptr);
	}

	OnItemSelected.Broadcast(SelectedItem);
}

void UFVInventoryUIComponent::AddCustomItemToMap_Implementation(const FGameplayTag& ItemTag, const FGuid& ItemId)
{
	if (FInventoryUICustomData* customData = CustomItemsMap.Find(ItemTag))
	{
		if (!customData->StoredIds.Contains(ItemId))
			customData->StoredIds.Add(ItemId);
	}
	else
	{
		FInventoryUICustomData newData;
		newData.StoredIds.Add(ItemId);

		CustomItemsMap.Add(ItemTag, MoveTemp(newData));
	}	
}

void UFVInventoryUIComponent::ProcessItemDurabilityChanged(const FFVInventoryItem& Item, const float OldDurability, const float NewDurability)
{
	Execute_ProcessItemModified(this, Item);
}

void UFVInventoryUIComponent::ForwardInventoryNotificationToSubsystem(const FInventoryNotificationData& NotificationData)
{
	const auto sharedHUDSubsystem = GetSharedHUDSubsystem();
	if (!IsValid(sharedHUDSubsystem) || !IsValid(IFVAdvancedInventorySharedHUDInterface::Execute_GetWrapperWidget(sharedHUDSubsystem)))
	{
		LOG_WARNING(TEXT("[FVInventoryUIComponent] Cannot find 'Shared HUD Subsystem'. Notification cannot be displayed"))
		return;
	}

	IFVAdvancedInventorySharedHUDInterface::Execute_CreateInventoryNotification(sharedHUDSubsystem, NotificationData);
}

void UFVInventoryUIComponent::ProcessItemQuantityChanged(const FFVInventoryItem& Item, const int32 OldQuantity, const int32 NewQuantity)
{
	Execute_ProcessItemModified(this, Item);
}

bool UFVInventoryUIComponent::RemoveCustomItemFromMap_Implementation(const FGameplayTag& ItemTag, const FGuid& ItemId)
{
	FInventoryUICustomData* foundData = CustomItemsMap.Find(ItemTag);
	if (!foundData || foundData->StoredIds.IsEmpty())
		return false;

	const int32 removedCount = foundData->StoredIds.RemoveAll([&ItemId](const FGuid& storedId)
	{
		return storedId == ItemId;
	});

	if (removedCount > 0 && foundData->StoredIds.Num() == 0)
		CustomItemsMap.Remove(ItemTag);

	return removedCount > 0;
}

bool UFVInventoryUIComponent::IsItemStoredInCustomMap_Implementation(const FGameplayTag& ItemTag, const FGuid& ItemId)
{
	const FInventoryUICustomData* foundData = CustomItemsMap.Find(ItemTag);
	if (!foundData)
		return false;
	
	return Algo::AnyOf(foundData->StoredIds, [&ItemId](const FGuid& storedId)
	{
		return storedId == ItemId;
	});
}

bool UFVInventoryUIComponent::EnqueueItemAction_Implementation(UFVSelectableInventoryItemAction* ItemAction, UObject* Payload)
{
	if (!IsValid(ItemAction))
		return false;
    
	FActionQueueEntry entry;
	entry.Action = ItemAction;
	entry.Payload = Payload;
	entry.CreationTime = FDateTime::Now();
    
	ActionsQueue.Enqueue(MoveTemp(entry));
    
	return true;
}

void UFVInventoryUIComponent::CompleteQueuedAction_Implementation(UFVSelectableInventoryItemAction* ItemAction, UObject* Payload)
{
	if (!IsValid(ItemAction))
		return;
    
	ItemAction->ExecuteQueuedAction(Payload);
	ActionsQueue.Remove(ItemAction);
}

void UFVInventoryUIComponent::CancelQueuedAction_Implementation(UFVSelectableInventoryItemAction* ItemAction)
{
	if (!IsValid(ItemAction))
		return;
	
	ItemAction->CancelInventoryAction();
	ActionsQueue.Remove(ItemAction);
}

void UFVInventoryUIComponent::EmptyItemActionsQueue_Implementation()
{
	ActionsQueue.Clear();
}

TArray<UFVSelectableInventoryItemAction*> UFVInventoryUIComponent::GetItemActionsQueue_Implementation() const
{
	TArray<UFVSelectableInventoryItemAction*> Result;
	Result.Reserve(ActionsQueue.Pending.Num());

	Algo::TransformIf(
		ActionsQueue.Pending,
		Result,
		[](const FActionQueueEntry& entry) { return IsValid(entry.Action); },
		[](const FActionQueueEntry& entry) { return entry.Action.Get(); }
	);

	return Result;
}

