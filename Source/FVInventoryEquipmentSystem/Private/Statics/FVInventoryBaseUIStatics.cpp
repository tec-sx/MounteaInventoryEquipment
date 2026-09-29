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


#include "Statics/FVInventoryBaseUIStatics.h"

#include "Blueprint/UserWidget.h"
#include "Components/ActorComponent.h"
#include "Components/Widget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/UserInterface/FVAdvancedInventorySharedHUDInterface.h"
#include "Interfaces/Widgets/FVInventorySystemWrapperWidgetInterface.h"
#include "Settings/FVAdvancedInventoryGlobalUIConfig.h"
#include "Settings/FVAdvancedInventorySettings.h"
#include "Subsystems/FVAdvancedInventorySharedHUDSubsystem.h"

APlayerController* UFVInventoryBaseUIStatics::FindPlayerController(AActor* Actor, int SearchDepth)
{
	SearchDepth++;
	if (SearchDepth >= 8)
		return nullptr;

	if (!Actor)
		return nullptr;

	if (APlayerController* playerController = Cast<APlayerController>(Actor))
		return playerController;

	if (APawn* actorPawn = Cast<APawn>(Actor))
		return Cast<APlayerController>(actorPawn->GetController());

	if (AActor* ownerActor = Actor->GetOwner())
		return FindPlayerController(ownerActor, SearchDepth);

	return nullptr;
}

bool UFVInventoryBaseUIStatics::IsValidBaseUIHandler(const UObject* Target)
{
	return IsValid(Target) && Target->Implements<UFVInventorySystemWrapperWidgetInterface>();
}

UFVAdvancedInventorySharedHUDSubsystem* UFVInventoryBaseUIStatics::GetSharedHUDSubsystem(UObject* Context)
{
	if (!IsValid(Context))
		return nullptr;

	APlayerController* playerController = Cast<APlayerController>(Context);
	if (!IsValid(playerController))
	{
		if (AActor* actor = Cast<AActor>(Context))
			playerController = FindPlayerController(actor, 3);
		else if (const UActorComponent* actorComp = Cast<UActorComponent>(Context))
		{
			if (AActor* ownerActor = actorComp->GetOwner())
				playerController = FindPlayerController(ownerActor, 3);
		}
		else if (const UUserWidget* userWidget = Cast<UUserWidget>(Context))
			playerController = FindPlayerController(userWidget->GetOwningPlayer(), 2);
	}

	if (!IsValid(playerController))
		return nullptr;

	const ULocalPlayer* localPlayer = playerController->GetLocalPlayer();
	return IsValid(localPlayer) ? localPlayer->GetSubsystem<UFVAdvancedInventorySharedHUDSubsystem>() : nullptr;
}

UFVAdvancedInventoryGlobalUIConfig* UFVInventoryBaseUIStatics::GetGlobalUIConfig()
{
	const UFVAdvancedInventorySettings* settings = GetDefault<UFVAdvancedInventorySettings>();
	return IsValid(settings) ? settings->GlobalUIConfig.LoadSynchronous() : nullptr;
}

bool UFVInventoryBaseUIStatics::CreateWrapperWidget(const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Target)
{
	return Target.GetObject() ? Target->Execute_CreateWrapperWidget(Target.GetObject()) : false;
}

UUserWidget* UFVInventoryBaseUIStatics::GetWrapperWidget(const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Target)
{
	return Target.GetObject() ? Target->Execute_GetWrapperWidget(Target.GetObject()) : nullptr;
}

void UFVInventoryBaseUIStatics::RemoveWrapperWidget(const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Target)
{
	if (Target.GetObject())
		Target->Execute_RemoveWrapperWidget(Target.GetObject());
}

void UFVInventoryBaseUIStatics::ExecuteWidgetCommandFromManager(
	const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Target,
	const FString& Command,
	UObject* OptionalPayload)
{
	if (Target.GetObject())
		return IFVAdvancedInventorySharedHUDInterface::Execute_ExecuteWidgetCommand(Target.GetObject(), Command, OptionalPayload);
}

UWidget* UFVInventoryBaseUIStatics::GetNotificationContainer(const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Target)
{
	return Target.GetObject() ? Target->Execute_GetNotificationContainer(Target.GetObject()) : nullptr;
}

void UFVInventoryBaseUIStatics::SetNotificationContainer(const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Target,
	UWidget* NewNotificationContainer)
{
	if (Target.GetObject())
		Target->Execute_SetNotificationContainer(Target.GetObject(), NewNotificationContainer);
}

void UFVInventoryBaseUIStatics::CreateInventoryNotification(
	const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Target,
	const FInventoryNotificationData& NotificationData)
{
	if (Target.GetObject())
		Target->Execute_CreateInventoryNotification(Target.GetObject(), NotificationData);
}

void UFVInventoryBaseUIStatics::RemoveInventoryNotifications(const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Target)
{
	if (Target.GetObject())
		Target->Execute_RemoveInventoryNotifications(Target.GetObject());
}

FGameplayTagContainer UFVInventoryBaseUIStatics::GetManagerWidgetStates(const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Target)
{
	return Target.GetObject() ?
		IFVAdvancedInventorySharedHUDInterface::Execute_GetWidgetStates(Target.GetObject()) : FGameplayTagContainer();
}

void UFVInventoryBaseUIStatics::SetManagerWidgetStates(const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Target,
	const FGameplayTagContainer& NewStates)
{
	if (Target.GetObject())
		return IFVAdvancedInventorySharedHUDInterface::Execute_SetWidgetStates(Target.GetObject(), NewStates);
}

bool UFVInventoryBaseUIStatics::AddWidgetStateTag(const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Target,
	const FGameplayTag& Tag)
{
	return Target.GetObject() ?
		IFVAdvancedInventorySharedHUDInterface::Execute_AddWidgetStateTag(Target.GetObject(), Tag) : false;
}

bool UFVInventoryBaseUIStatics::RemoveWidgetStateTag(const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Target,
	const FGameplayTag& Tag)
{
	return Target.GetObject() ?
		IFVAdvancedInventorySharedHUDInterface::Execute_RemoveWidgetStateTag(Target.GetObject(), Tag) : false;
}

bool UFVInventoryBaseUIStatics::HasWidgetStateTag(const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Target,
	const FGameplayTag& Tag, const bool bExactMatch)
{
	return Target.GetObject() ?
		IFVAdvancedInventorySharedHUDInterface::Execute_HasWidgetStateTag(Target.GetObject(), Tag, bExactMatch) : false;
}

void UFVInventoryBaseUIStatics::ClearWidgetStateTags(const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Target)
{
	if (Target.GetObject())
		return IFVAdvancedInventorySharedHUDInterface::Execute_ClearWidgetStateTags(Target.GetObject());
}

bool UFVInventoryBaseUIStatics::AppendWidgetStateTags(
	const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Target,
	const FGameplayTagContainer& TagsToAppend)
{
	return Target.GetObject() ?
		IFVAdvancedInventorySharedHUDInterface::Execute_AppendWidgetStateTags(Target.GetObject(), TagsToAppend) : false;
}

bool UFVInventoryBaseUIStatics::HasAnyWidgetStates(const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Target)
{
	return Target.GetObject() ? !GetManagerWidgetStates(Target).IsEmpty() : false;
}

void UFVInventoryBaseUIStatics::InitializeWrapperWidget(UObject* Target, const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Parent)
{
	if (IsValidBaseUIHandler(Target))
		IFVInventorySystemWrapperWidgetInterface::Execute_InitializeWrapperWidget(Target, Parent);
}

void UFVInventoryBaseUIStatics::RemoveBaseWrapperWidget(UObject* Target)
{
	if (IsValidBaseUIHandler(Target))
		IFVInventorySystemWrapperWidgetInterface::Execute_RemoveWrapperWidget(Target);
}

bool UFVInventoryBaseUIStatics::AddModalWindow(UObject* Target, UUserWidget* ModalWindow)
{
	return IsValidBaseUIHandler(Target) ? IFVInventorySystemWrapperWidgetInterface::Execute_AddModalWindow(Target, ModalWindow) : false;
}

bool UFVInventoryBaseUIStatics::CleanupModalWindow(UObject* Target)
{
	return IsValidBaseUIHandler(Target) ? IFVInventorySystemWrapperWidgetInterface::Execute_CleanupModalWindow(Target) : false;
}
