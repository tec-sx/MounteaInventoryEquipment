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

#include "Components/FVAttachableComponent.h"

#include "Definitions/FVEquipmentBaseEnums.h"
#include "Interfaces/Attachments/FVAdvancedAttachmentContainerInterface.h"

UFVAttachableComponent::UFVAttachableComponent() : 
	DisplayName(FText::GetEmpty()),
	Id(NAME_None),
	State(EAttachmentState::EAS_Detached)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	UFVAttachableComponent::SetComponentTickEnabled(false);

	bAutoActivate = true;

	SetIsReplicatedByDefault(true);
	SetActiveFlag(true);

	ComponentTags.Append({ TEXT("FV"), TEXT("Attachable"), TEXT("EquipmentItem") });	
}

void UFVAttachableComponent::SetId_Implementation(const FName& NewId)
{
	if (Id != NewId)
		Id = NewId;
}

void UFVAttachableComponent::SetDisplayName_Implementation(const FText& NewDisplayName)
{
	if (!NewDisplayName.EqualTo(DisplayName))
		DisplayName = NewDisplayName;
}

void UFVAttachableComponent::SetTags_Implementation(const FGameplayTagContainer& NewTags)
{
	Tags.Reset();
	Tags.AppendTags(NewTags);
}

void UFVAttachableComponent::SetState_Implementation(const EAttachmentState NewState)
{
	if (NewState != State)
		State = NewState;
}

bool UFVAttachableComponent::IsValidAttachable_Implementation() const
{
	return !DisplayName.IsEmpty() && !Id.IsNone();
}

bool UFVAttachableComponent::CanAttach_Implementation() const
{
	return IsValidAttachable() && State == EAttachmentState::EAS_Detached;
}

bool UFVAttachableComponent::AttachToSlot_Implementation(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, 
	const FName& SlotId)
{
	if (!Execute_CanAttach(this) || !IsValid(Target.GetObject()))
		return false;

	if (!Target->Execute_TryAttach(Target.GetObject(), SlotId, this))
		return false;

	AttachedTo = Target;
	Execute_SetState(this, EAttachmentState::EAS_Attached);

	OnAttachableAttached.Broadcast(Target, SlotId);
	return true;
}

bool UFVAttachableComponent::AttachToContainer_Implementation(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target)
{
	if (!CanAttach() || !Target.GetObject())
		return false;

	const FName foundSlotId = Target->Execute_FindFirstFreeSlotWithTags(Target.GetObject(), Tags);
	if (foundSlotId.IsNone())
		return false;

	return Execute_AttachToSlot(this, Target, foundSlotId);
}

bool UFVAttachableComponent::Detach_Implementation()
{
	if (State != EAttachmentState::EAS_Attached || !AttachedTo.GetObject())
		return false;

	const FName slotId = AttachedTo->Execute_GetSlotIdForAttachable(AttachedTo.GetObject(), this);
	if (!slotId.IsNone())
		AttachedTo->Execute_TryDetach(AttachedTo.GetObject(), slotId);

	AttachedTo = nullptr;
	State = EAttachmentState::EAS_Detached;

	OnAttachableDetached.Broadcast(AttachedTo);
	return true;
}

bool UFVAttachableComponent::HasTag_Implementation(const FGameplayTag& Tag) const
{
	return Tags.HasTag(Tag);
}

bool UFVAttachableComponent::MatchesTags_Implementation(const FGameplayTagContainer& OtherTags, const bool bRequireAll) const
{
	return bRequireAll ? Tags.HasAll(OtherTags) : Tags.HasAny(OtherTags);
}
