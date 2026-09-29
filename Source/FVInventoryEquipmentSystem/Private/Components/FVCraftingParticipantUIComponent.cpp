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


#include "Components/FVCraftingParticipantUIComponent.h"

#include "Blueprint/UserWidget.h"
#include "Definitions/FVInventoryBaseCommands.h"
#include "Engine/LocalPlayer.h"
#include "Interfaces/Crafting/FVAdvancedCraftingParticipantInterface.h"
#include "Interfaces/UserInterface/FVAdvancedInventorySharedHUDInterface.h"
#include "Interfaces/Widgets/Crafting/FVAdvancedInventoryCraftingWidgetInterface.h"
#include "Interfaces/Widgets/FVInventoryGenericWidgetInterface.h"
#include "Settings/FVAdvancedCraftingUIConfig.h"
#include "Statics/FVInventorySystemStatics.h"
#include "Statics/FVInventoryUIStatics.h"
#include "Subsystems/FVAdvancedCraftingUISubsystem.h"
#include "Subsystems/FVAdvancedInventorySharedHUDSubsystem.h"
#include "Logs/FVAdvancedInventoryLog.h"
#include "Statics/FVCraftingUIStatics.h"


UFVCraftingParticipantUIComponent::UFVCraftingParticipantUIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	UFVCraftingParticipantUIComponent::SetComponentTickEnabled(false);
	
	bAutoActivate = true;
	
	SetIsReplicatedByDefault(false);
	SetActiveFlag(true);

	ComponentTags.Append( { TEXT("FV"), TEXT("Crafting"), TEXT("UI") } );
}

void UFVCraftingParticipantUIComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() &&
		(GetOwnerRole() == ROLE_Authority || GetOwnerRole() == ROLE_AutonomousProxy) &&
		UFVInventorySystemStatics::CanExecuteCosmeticEvents(GetWorld()))
	{
		const auto craftingParticipant = GetOwner()->FindComponentByInterface(UFVAdvancedCraftingParticipantInterface::StaticClass());
		if (!IsValid(craftingParticipant))
		{
			LOG_ERROR(TEXT("[Crafting UI] Unable to find Crafting Participant for Actor %s, component disabled"), *(GetOwner() ? GetOwner()->GetName() : TEXT("'no owner'")))
			return;
		}
		Execute_SetCraftingParticipant(this, craftingParticipant);
		
		APlayerController* playerController = UFVInventoryUIStatics::FindPlayerController(GetOwner(), 3);
		if (!IsValid(playerController))
		{
			LOG_ERROR(TEXT("[FVCraftingParticipantUIComponent] Cannot find valid Player Controller. Crafting UI Subsystem registration failed!"))
			return;
		}

		const ULocalPlayer* localPlayer = playerController->GetLocalPlayer();
		UFVAdvancedCraftingUISubsystem* craftingUISubsystem = IsValid(localPlayer)
			? localPlayer->GetSubsystem<UFVAdvancedCraftingUISubsystem>()
			: nullptr;
		if (!IsValid(craftingUISubsystem))
		{
			LOG_ERROR(TEXT("[FVCraftingParticipantUIComponent] Cannot find 'Crafting UI Subsystem'. UI might not work properly!"))
			return;
		}	

		craftingUISubsystem->RegisterCraftingUIManager(this);
	}
}

void UFVCraftingParticipantUIComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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
				APlayerController* playerController = UFVInventoryUIStatics::FindPlayerController(GetOwner(), 3);
				if (!IsValid(playerController))
				{
					LOG_ERROR(TEXT("[FVCraftingParticipantUIComponent] Cannot find valid Player Controller. Crafting UI Subsystem unregistration failed!"))
					break;
				}

				const ULocalPlayer* localPlayer = playerController->GetLocalPlayer();
				UFVAdvancedCraftingUISubsystem* craftingUISubsystem = IsValid(localPlayer)
					? localPlayer->GetSubsystem<UFVAdvancedCraftingUISubsystem>()
					: nullptr;
				if (!IsValid(craftingUISubsystem))
				{
					LOG_ERROR(TEXT("[FVCraftingParticipantUIComponent] Cannot find 'Crafting UI Subsystem'. UI will UNREGISTER improperly!"))
					break;
				}

				craftingUISubsystem->UnregisterCraftingUIManager(this);
			}
			break;
		}
	}

	Super::EndPlay(EndPlayReason);
}

bool UFVCraftingParticipantUIComponent::CreateCraftingWidget_Implementation()
{
	if (IsValid(CraftingWidget))
	{
		LOG_INFO(TEXT("[CreateCraftingWidget] Crafting Widget already exists. Early return."))
		return true;
	}
	
	const UFVAdvancedCraftingUIConfig* craftingUIConfig = UFVCraftingUIStatics::GetCraftingUISettingsConfig();
	if (!craftingUIConfig)
	{
		LOG_ERROR(TEXT("[CreateCraftingWidget] Unable to load Crafting Config!"))
		return false;
	}

	auto widgetClass = craftingUIConfig->CraftingUserInterfaceClass.LoadSynchronous();
	if (!IsValid(widgetClass))
	{
		LOG_ERROR(TEXT("[CreateCraftingWidget] Unable to load Crafting UI Class from Config!"))
		return false;
	}
	if (!widgetClass->ImplementsInterface(UFVAdvancedInventoryCraftingWidgetInterface::StaticClass()))
	{
		LOG_ERROR(TEXT("[CreateCraftingWidget] Crafting UI Class must implement `FVAdvancedInventoryCraftingWidgetInterface`!"))
		return false;
	}

	FString message = TEXT("");
	bool bSuccess = true;

	int seachDepth = 0;
	APlayerController* playerController = UFVInventoryUIStatics::FindPlayerController(GetOwner(), seachDepth);
	if (!playerController || !playerController->IsLocalController())
	{
		message = !playerController ? TEXT("Invalid Player Controller!") : TEXT("UI can be shown only to Local Players!");
		bSuccess = false;
	}
	if (!bSuccess)
	{
		LOG_ERROR(TEXT("[CreateCraftingWidget] Failed to find Player Controller. Message:\n%s"), *message)
		return false;
	}

	if (!playerController->IsLocalController())
	{
		LOG_WARNING(TEXT("[CreateCraftingWidget] UI can be created for Local Players only!"))
		return false;
	}

	auto newWidget = CreateWidget<UUserWidget>(playerController, widgetClass);
	if (!newWidget)
	{
		LOG_ERROR(TEXT("[CreateCraftingWidget] Failed to create Crafting Widget!"))
		return false;
	}
	if (!newWidget->Implements<UFVAdvancedInventoryCraftingWidgetInterface>())
	{
		LOG_ERROR(TEXT("[CreateCraftingWidget] Crafting UI must implement `FVAdvancedInventoryCraftingWidgetInterface`!"))
		return false;
	}

	CraftingWidget = newWidget;

	TScriptInterface<IFVAdvancedInventoryCraftingWidgetInterface> craftingWidget = CraftingWidget;
	ensure(craftingWidget.GetObject() != nullptr);

	const auto sharedHUDSubsystem = GetSharedHUDSubsystem();
	if (!IsValid(sharedHUDSubsystem) || !IsValid(IFVAdvancedInventorySharedHUDInterface::Execute_GetWrapperWidget(sharedHUDSubsystem)))
	{
		LOG_WARNING(TEXT("[FVCraftingParticipantUIComponent] Cannot find 'Shared HUD Subsystem'. UI might not work properly"))
		if (CraftingWidget->Implements<UFVInventoryGenericWidgetInterface>())
			IFVInventoryGenericWidgetInterface::Execute_ProcessInventoryWidgetCommand(CraftingWidget, InventoryUICommands::General::Create, nullptr);
	}
	else
		IFVAdvancedInventorySharedHUDInterface::Execute_ExecuteWidgetCommand(sharedHUDSubsystem, InventoryUICommands::Crafting::Open, CraftingWidget);

	return true;
}

void UFVCraftingParticipantUIComponent::RemoveCraftingWidget_Implementation()
{
	if (IsValid(CraftingWidget))
	{
		// TODO: execute event using generic widget etc.
		CraftingWidget = nullptr;
	}
}

bool UFVCraftingParticipantUIComponent::SetCraftingWidget_Implementation(UUserWidget* NewCraftingWidget)
{
	if (CraftingWidget == NewCraftingWidget)
		return false;
	CraftingWidget = NewCraftingWidget;
	return true;
}

bool UFVCraftingParticipantUIComponent::SetCraftingParticipant_Implementation(const TScriptInterface<IFVAdvancedCraftingParticipantInterface>& Participant)
{
	if (Participant == CraftingParticipant)
		return false;
	CraftingParticipant = Participant;
	return true;
}

bool UFVCraftingParticipantUIComponent::CraftingCategorySelected_Implementation(const FString& SelectedCategoryId)
{
	const bool bSuccess = SelectedCategoryId.Equals(SelectedCategory, ESearchCase::IgnoreCase);
	if (bSuccess)
		SelectedCategory = SelectedCategoryId;
	return bSuccess;
}

bool UFVCraftingParticipantUIComponent::CraftableItemSelected_Implementation(const FGuid& NewSelectedItem)
{
	if (SelectedItem == NewSelectedItem)
		return false;

	SelectedItem = NewSelectedItem;
	return true;
}

UFVAdvancedInventorySharedHUDSubsystem* UFVCraftingParticipantUIComponent::GetSharedHUDSubsystem() const
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

