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


#include "Subsystems/FVAdvancedInventorySharedHUDSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Definitions/FVInventoryBaseCommands.h"
#include "Interfaces/Widgets/FVInventoryGenericWidgetInterface.h"
#include "Interfaces/Widgets/FVInventorySystemWrapperWidgetInterface.h"
#include "Interfaces/Widgets/Notification/FVInventoryNotificationContainerWidgetInterface.h"
#include "Interfaces/Widgets/Notification/FVInventoryNotificationWidgetInterface.h"
#include "Logs/FVAdvancedInventoryLog.h"
#include "Settings/FVAdvancedInventoryGlobalUIConfig.h"
#include "Statics/FVInventoryBaseUIStatics.h"
#include "Statics/FVInventoryUIStatics.h"

void UFVAdvancedInventorySharedHUDSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UFVAdvancedInventorySharedHUDSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UFVAdvancedInventorySharedHUDSubsystem::CreateWrapperWidget_Implementation()
{
	if (!IsValid(GetWorld()))
	{
		LOG_ERROR(TEXT("[CreateWrapperWidget] Invalid World!"))
		return false;
	}	
	if (!IsValid(GetLocalPlayer()))
	{
		LOG_ERROR(TEXT("[CreateWrapperWidget] Invalid Local Player!"))
		return false;
	}
	
	const auto localPlayerController = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!IsValid(localPlayerController))
	{
		LOG_ERROR(TEXT("[CreateWrapperWidget] Invalid Player Controller!"))
		return false;
	}
	UFVAdvancedInventoryGlobalUIConfig* globalUIConfig = UFVInventoryBaseUIStatics::GetGlobalUIConfig();
	if (!globalUIConfig)
	{
		LOG_ERROR(TEXT("[CreateWrapperWidget] Unable to load Global UI Config!"))
		return false;
	}
	UIConfig = globalUIConfig;
	
	auto widgetClass = UIConfig->UserInterfaceWrapperClass.LoadSynchronous();
	if (!IsValid(widgetClass))
	{
		LOG_ERROR(TEXT("[CreateWrapperWidget] Unable to load UI Wrapper Class from Config!"))
		return false;
	}
	if (!widgetClass->ImplementsInterface(UFVInventorySystemWrapperWidgetInterface::StaticClass()))
	{
		LOG_ERROR(TEXT("[CreateWrapperWidget] Base UI Wrapper Class must implement `FVInventorySystemBaseWidgetInterface`!"))
		return false;
	}

	FString returnMessage = TEXT("");
	bool bSuccess = true;
	
	int seachDepth = 0;
	const auto playerController = UFVInventoryUIStatics::FindPlayerController(localPlayerController, seachDepth);
	if (!playerController || !playerController->IsLocalController())
	{
		returnMessage = !playerController ? TEXT("Invalid Player Controller!") : TEXT("UI can be shown only to Local Players!");
		bSuccess = false;
	}
	if (!bSuccess)
	{
		LOG_ERROR(TEXT("[CreateWrapperWidget] Failed to find Player Controller. Message:\n%s"), *returnMessage)
		return false;
	}

	if (!playerController->IsLocalController())
	{
		LOG_WARNING(TEXT("[CreateWrapperWidget] UI Wrapper Can be created for Local Players only!"))
		return false;
	}

	auto newWidget = CreateWidget<UUserWidget>(localPlayerController, widgetClass);
	if (!newWidget)
	{
		LOG_ERROR(TEXT("[CreateWrapperWidget] Failed to create Wrapper Widget!"))
		return false;
	}
	if (!newWidget->Implements<UFVInventorySystemWrapperWidgetInterface>())
	{
		LOG_ERROR(TEXT("[CreateWrapperWidget] Base UI wrapper must implement `FVInventorySystemBaseWidgetInterface`!"))
		return false;
	}

	SharedUserInterface = newWidget;
	ensure(SharedUserInterface != nullptr);
	
	IFVInventorySystemWrapperWidgetInterface::Execute_InitializeWrapperWidget(SharedUserInterface, this);
	
	if (SharedUserInterface->Implements<UFVInventoryGenericWidgetInterface>())
		IFVInventoryGenericWidgetInterface::Execute_ProcessInventoryWidgetCommand(SharedUserInterface, InventoryUICommands::Wrapper::Create, nullptr);

	return true;
}

void UFVAdvancedInventorySharedHUDSubsystem::RemoveWrapperWidget_Implementation()
{
	TScriptInterface<IFVInventorySystemWrapperWidgetInterface> wrapperInterface = SharedUserInterface;
	ensure(wrapperInterface.GetObject() != nullptr);
	
	wrapperInterface->Execute_RemoveWrapperWidget(SharedUserInterface);
	
	if (SharedUserInterface->Implements<UFVInventoryGenericWidgetInterface>())
		IFVInventoryGenericWidgetInterface::Execute_ProcessInventoryWidgetCommand(SharedUserInterface, InventoryUICommands::Wrapper::Remove, nullptr);
	else
	{
		SharedUserInterface->RemoveFromParent();
		SharedUserInterface = nullptr;
	}
}

UWidget* UFVAdvancedInventorySharedHUDSubsystem::GetNotificationContainer_Implementation() const
{
	return InventoryNotificationContainerWidget;
}

void UFVAdvancedInventorySharedHUDSubsystem::SetNotificationContainer_Implementation(UWidget* NewNotificationContainer)
{
	if (InventoryNotificationContainerWidget != NewNotificationContainer)
		InventoryNotificationContainerWidget = NewNotificationContainer;
}

void UFVAdvancedInventorySharedHUDSubsystem::CreateInventoryNotification_Implementation(const FInventoryNotificationData& NotificationData)
{
	if (!IsValid(InventoryNotificationContainerWidget))
	{
		LOG_WARNING(TEXT("[CreateInventoryNotification] Invalid Notification Container!"))
		return;
	}

	TScriptInterface<IFVInventoryNotificationContainerWidgetInterface> notificationContainerInterface = InventoryNotificationContainerWidget;
	ensure(notificationContainerInterface.GetObject() != nullptr);
	
	if (!UIConfig)
	{
		LOG_ERROR(TEXT("[CreateInventoryNotification] Unable to load Global UI Config!"))
		return;
	}

	if (UIConfig->NotificationWidgetClass.IsNull())
	{
		LOG_ERROR(TEXT("[CreateInventoryNotification] Unable to load `NotificationWidgetClass` from Config!"))
		return;
	}
	
	auto notificationClass = UIConfig->NotificationWidgetClass.LoadSynchronous();
	auto notificationWidget = CreateWidget(InventoryNotificationContainerWidget, notificationClass);
	if (!IsValid(notificationWidget))
	{
		LOG_ERROR(TEXT("[CreateInventoryNotification] Failed to Create Inventory Notification!"))
		return;
	}

	IFVInventoryNotificationWidgetInterface::Execute_CreateNotification(notificationWidget, NotificationData, InventoryNotificationContainerWidget);

	notificationContainerInterface->Execute_AddNotification(InventoryNotificationContainerWidget, notificationWidget);
}

void UFVAdvancedInventorySharedHUDSubsystem::RemoveInventoryNotifications_Implementation()
{
	if (!IsValid(InventoryNotificationContainerWidget))
	{
		LOG_WARNING(TEXT("[CreateInventoryNotification] Invalid Notification Container!"))
		return;
	}

	TScriptInterface<IFVInventoryNotificationContainerWidgetInterface> notificationContainerInterface = InventoryNotificationContainerWidget;
	ensure(notificationContainerInterface.GetObject() != nullptr);

	IFVInventoryNotificationContainerWidgetInterface::Execute_ClearNotifications(InventoryNotificationContainerWidget);
}

void UFVAdvancedInventorySharedHUDSubsystem::ExecuteWidgetCommand_Implementation(const FString& Command, UObject* OptionalPayload)
{
	if (SharedUserInterface && SharedUserInterface->Implements<UFVInventoryGenericWidgetInterface>())
		IFVInventoryGenericWidgetInterface::Execute_ProcessInventoryWidgetCommand(SharedUserInterface, Command, OptionalPayload);
}

void UFVAdvancedInventorySharedHUDSubsystem::SetWidgetStates_Implementation(const FGameplayTagContainer& NewStates)
{
	if (WidgetStatesContainer != NewStates)
		WidgetStatesContainer = NewStates;
}

bool UFVAdvancedInventorySharedHUDSubsystem::AddWidgetStateTag_Implementation(const FGameplayTag& Tag)
{
	if (WidgetStatesContainer.HasTag(Tag))
		return false;
	WidgetStatesContainer.AddTag(Tag);
	return true;
}

bool UFVAdvancedInventorySharedHUDSubsystem::RemoveWidgetStateTag_Implementation(const FGameplayTag& Tag)
{
	return WidgetStatesContainer.RemoveTag(Tag);
}

bool UFVAdvancedInventorySharedHUDSubsystem::HasWidgetStateTag_Implementation(const FGameplayTag& Tag, bool bExactMatch) const
{
	return bExactMatch ? WidgetStatesContainer.HasTagExact(Tag) : WidgetStatesContainer.HasTag(Tag);
}

bool UFVAdvancedInventorySharedHUDSubsystem::AppendWidgetStateTags_Implementation(const FGameplayTagContainer& TagsToAppend)
{
	WidgetStatesContainer.AppendTags(TagsToAppend);
	return true;
}
