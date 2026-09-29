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


#include "Statics/FVCraftingUIStatics.h"

#include "Blueprint/UserWidget.h"
#include "Interfaces/Crafting/FVAdvancedCraftingParticipantUIInterface.h"
#include "Settings/FVAdvancedCraftingUIConfig.h"
#include "Settings/FVAdvancedInventorySettings.h"
#include "Statics/FVInventoryUIStatics.h"
#include "Subsystems/FVAdvancedCraftingUISubsystem.h"

bool UFVCraftingUIStatics::IsValidCraftingUIHandler(const UObject* Target)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedCraftingParticipantUIInterface>();
}

UFVAdvancedCraftingUISubsystem* UFVCraftingUIStatics::GetCraftingUISubsystem(UObject* Context)
{
	if (!IsValid(Context))
		return nullptr;

	APlayerController* playerController = Cast<APlayerController>(Context);

	if (!IsValid(playerController))
	{
		if (AActor* actor = Cast<AActor>(Context))
			playerController = UFVInventoryUIStatics::FindPlayerController(actor, 3);
		else if (const UActorComponent* actorComp = Cast<UActorComponent>(Context))
		{
			if (AActor* ownerActor = actorComp->GetOwner())
				playerController = UFVInventoryUIStatics::FindPlayerController(ownerActor, 3);
		}
		else if (const UUserWidget* userWidget = Cast<UUserWidget>(Context))
			playerController = UFVInventoryUIStatics::FindPlayerController(userWidget->GetOwningPlayer(), 2);
	}

	if (!IsValid(playerController))
		return nullptr;

	const ULocalPlayer* localPlayer = playerController->GetLocalPlayer();
	return IsValid(localPlayer) ? localPlayer->GetSubsystem<UFVAdvancedCraftingUISubsystem>() : nullptr;
}

UFVAdvancedCraftingUIConfig* UFVCraftingUIStatics::GetCraftingUISettingsConfig()
{
	const auto settings = GetDefault<UFVAdvancedInventorySettings>();
	if (!settings) return nullptr;
	return settings->AdvancedCraftingUISettingsConfig.LoadSynchronous();
}

TScriptInterface<IFVAdvancedCraftingParticipantInterface> UFVCraftingUIStatics::GetParentCraftingParticipant(const UObject* Target)
{
	if (!IsValid(Target) || !Target->Implements<UFVAdvancedCraftingParticipantUIInterface>())
		return nullptr;
		
	return IFVAdvancedCraftingParticipantUIInterface::Execute_GetParentCraftingParticipant(Target);
}

UObject* UFVCraftingUIStatics::GetParentCraftingParticipantObject(const UObject* Target)
{
	return GetParentCraftingParticipant(Target).GetObject();
}

bool UFVCraftingUIStatics::SetParentCraftingParticipant(UObject* Target, const TScriptInterface<IFVAdvancedCraftingParticipantInterface>& Participant)
{
	if (!IsValid(Target) || !Target->Implements<UFVAdvancedCraftingParticipantUIInterface>())
		return false;

	return IFVAdvancedCraftingParticipantUIInterface::Execute_SetCraftingParticipant(Target, Participant);
}

bool UFVCraftingUIStatics::CreateCraftingWidget(const TScriptInterface<IFVAdvancedCraftingParticipantUIInterface>& Target)
{
	return IsValidCraftingUIHandler(Target.GetObject()) ? IFVAdvancedCraftingParticipantUIInterface::Execute_CreateCraftingWidget(Target.GetObject()) : false;
}

UUserWidget* UFVCraftingUIStatics::GetCraftingWidget(const TScriptInterface<IFVAdvancedCraftingParticipantUIInterface>& Target)
{
	return IsValidCraftingUIHandler(Target.GetObject()) ? IFVAdvancedCraftingParticipantUIInterface::Execute_GetCraftingWidget(Target.GetObject()) : nullptr;
}

bool UFVCraftingUIStatics::SetCraftingWidget(const TScriptInterface<IFVAdvancedCraftingParticipantUIInterface>& Target, UUserWidget* NewCraftingWidget)
{
	return IsValidCraftingUIHandler(Target.GetObject()) ? IFVAdvancedCraftingParticipantUIInterface::Execute_SetCraftingWidget(Target.GetObject(), NewCraftingWidget) : false;
}

void UFVCraftingUIStatics::RemoveCraftingWidget(const TScriptInterface<IFVAdvancedCraftingParticipantUIInterface>& Target)
{
	if (IsValidCraftingUIHandler(Target.GetObject()))
		IFVAdvancedCraftingParticipantUIInterface::Execute_RemoveCraftingWidget(Target.GetObject());
}

FString UFVCraftingUIStatics::GetCraftingSelectedCategoryId(UObject* Target)
{
	return IsValidCraftingUIHandler(Target) ? IFVAdvancedCraftingParticipantUIInterface::Execute_GetCraftingSelectedCategoryId(Target) : TEXT("none");
}

bool UFVCraftingUIStatics::CraftingCategorySelected(UObject* Target,const FString& SelectedCategoryId)
{
	return IsValidCraftingUIHandler(Target) ? IFVAdvancedCraftingParticipantUIInterface::Execute_CraftingCategorySelected(Target, SelectedCategoryId) : false;
}

FGuid UFVCraftingUIStatics::GetActiveCraftableItemGuid_Implementation(UObject* Target)
{
	return IsValidCraftingUIHandler(Target) ? IFVAdvancedCraftingParticipantUIInterface::Execute_GetActiveCraftableItemGuid(Target) : FGuid();
}

bool UFVCraftingUIStatics::CraftableItemSelected(UObject* Target, const FGuid& SelectedItem)
{
	return IsValidCraftingUIHandler(Target) ? IFVAdvancedCraftingParticipantUIInterface::Execute_CraftableItemSelected(Target, SelectedItem) : false;
}
