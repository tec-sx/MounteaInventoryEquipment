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


#include "Components/FVCraftingStationComponent.h"

#include "Net/UnrealNetwork.h"
#include "Definitions/FVCraftingBaseEnums.h"

UFVCraftingStationComponent::UFVCraftingStationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	UFVCraftingStationComponent::SetComponentTickEnabled(false);

	bAutoActivate = true;

	SetIsReplicatedByDefault(true);
	SetActiveFlag(true);

	ComponentTags.Append( { TEXT("FV"), TEXT("Crafting") } );
}

void UFVCraftingStationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UFVCraftingStationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFVCraftingStationComponent, CraftingStationState);
	DOREPLIFETIME(UFVCraftingStationComponent, CraftingParticipants);
}

void UFVCraftingStationComponent::OnRep_CraftingStationState()
{
}

bool UFVCraftingStationComponent::IsCraftingPlaceOccupied_Implementation() const
{
	return CraftingStationState == ECraftingStationState::EASS_Occupied;
}

bool UFVCraftingStationComponent::SetCraftingStationState_Implementation(const ECraftingStationState NewCraftingStationState)
{
	if (!GetOwner()->HasAuthority())
	{
		Server_SetCraftingStationState(NewCraftingStationState);
		return true;
	}

	switch (NewCraftingStationState)
	{
		case ECraftingStationState::EASS_Empty:
			if (CraftingParticipants.Num() > 0)
				return false;
			if (CraftingStationState == ECraftingStationState::EASS_Empty)
				return false;
		case ECraftingStationState::EASS_Inactive:
			if (CraftingStationState == ECraftingStationState::EASS_Inactive)
				return false;
			break;
		case ECraftingStationState::EASS_Occupied:
			if (CraftingStationState == ECraftingStationState::EASS_Occupied)
				return false;
			break;
	}

	CraftingStationState = NewCraftingStationState;
	return true;
}

bool UFVCraftingStationComponent::CanBeUsed_Implementation() const
{
	if (CraftingStationState == ECraftingStationState::EASS_Inactive)
		return false;

	if (CraftingStationState == ECraftingStationState::EASS_Occupied && CraftingParticipants.Num() >= CraftingPlaceCapacity)
		return false;

	return true;
}

bool UFVCraftingStationComponent::StartUsing_Implementation(const TScriptInterface<IFVAdvancedCraftingParticipantInterface>& Participant)
{
	if (!GetOwner()->HasAuthority())
	{
		Server_StartUsing(Participant);
		return true;
	}

	if (CraftingParticipants.Contains(Participant))
		return false;

	if (!Execute_CanBeUsed(this))
		return false;

	CraftingParticipants.Add(Participant);
	Execute_SetCraftingStationState(this, ECraftingStationState::EASS_Occupied);

	return true;
}

bool UFVCraftingStationComponent::StopUsing_Implementation(const TScriptInterface<IFVAdvancedCraftingParticipantInterface>& Participant)
{
	if (!GetOwner()->HasAuthority())
	{
		Server_StopUsing(Participant);
		return true;
	}

	if (Participant.GetObject() == nullptr)
		return false;
	if (!CraftingParticipants.Contains(Participant))
		return false;
	CraftingParticipants.Remove(Participant);
	Execute_SetCraftingStationState(this, ECraftingStationState::EASS_Empty);

	return true;
}

void UFVCraftingStationComponent::Server_StartUsing_Implementation(const TScriptInterface<IFVAdvancedCraftingParticipantInterface>& Participant)
{
	Execute_StartUsing(this, Participant);
}

void UFVCraftingStationComponent::Server_StopUsing_Implementation(const TScriptInterface<IFVAdvancedCraftingParticipantInterface>& Participant)
{
	Execute_StopUsing(this, Participant);
}

void UFVCraftingStationComponent::Server_SetCraftingStationState_Implementation(ECraftingStationState NewState)
{
	Execute_SetCraftingStationState(this, NewState);
}
