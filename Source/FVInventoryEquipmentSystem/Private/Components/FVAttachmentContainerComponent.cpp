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


#include "Components/FVAttachmentContainerComponent.h"

#include "Algo/ForEach.h"
#include "Definitions/FVAdvancedAttachmentSlot.h"
#include "Definitions/FVEquipmentBaseEnums.h"
#include "Logs/FVAdvancedInventoryLog.h"
#include "Misc/DataValidation.h"
#include "Net/UnrealNetwork.h"
#include "Statics/FVAttachmentsStatics.h"
#include "Statics/FVEquipmentStatics.h"
#include "Statics/FVInventorySystemStatics.h"
#include "UObject/ObjectSaveContext.h"
#include "Components/FVAttachableComponent.h"

UFVAttachmentContainerComponent::UFVAttachmentContainerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	UFVAttachmentContainerComponent::SetComponentTickEnabled(false);
	
	bAutoActivate = true;
	
	SetIsReplicatedByDefault(true);
	SetActiveFlag(true);

	ComponentTags.Append( { TEXT("FV"), TEXT("Attachment") } );

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	bReplicateUsingRegisteredSubObjectList = true;
#endif
	
	ApplyParentContainer();
}

void UFVAttachmentContainerComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto attachmentTargetComponent = UFVAttachmentsStatics::GetAvailableComponentByName(Execute_GetOwningActor(this), DefaultAttachmentTarget);
	if (!IsValid(attachmentTargetComponent))
		LOG_ERROR(TEXT("Default attachment target component '%s' is not valid!"), *DefaultAttachmentTarget.ToString());

	Execute_SetDefaultAttachmentTargetComponent(this, attachmentTargetComponent);

	Algo::ForEach(AttachmentSlots, [this](const auto& AttachmentSlot)
	{
		if (IsValid(AttachmentSlot))
		{
			AttachmentSlot->InitializeAttachmentSlot(this);
			AttachmentSlot->BeginPlay();
		}
	});
}

bool UFVAttachmentContainerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	bool wroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
    
	for (auto& Slot : AttachmentSlots)
	{
		if (IsValid(Slot))
			AddReplicatedSubObject(Slot);
	}
	return wroteSomething;
#else
	bool wroteSomething = true;

	for (UORReplicatedObject* ReplicatedObject : ReplicatedObjects)
	{
		if (IsValid(ReplicatedObject))
			wroteSomething |= Channel->ReplicateSubobject(ReplicatedObject, *Bunch, *RepFlags);
	}

	return wroteSomething;
#endif
}

AActor* UFVAttachmentContainerComponent::GetOwningActor_Implementation() const
{
	return UFVInventorySystemStatics::GetOwningActor(this);
}

void UFVAttachmentContainerComponent::SetDefaultAttachmentTargetComponent_Implementation(USceneComponent* NewTarget)
{
	if (NewTarget != DefaultAttachmentTargetComponent)
		DefaultAttachmentTargetComponent = NewTarget;
}

bool UFVAttachmentContainerComponent::IsValidSlot_Implementation(const FName& SlotId) const
{
	const auto foundSlot = AttachmentSlots.FindByPredicate([SlotId](const TObjectPtr<UFVAdvancedAttachmentSlot>& Slot)
	{
		return Slot && Slot->SlotName == SlotId;
	});

	return foundSlot && *foundSlot && (*foundSlot)->IsSlotValid();
}

UFVAdvancedAttachmentSlot* UFVAttachmentContainerComponent::GetSlot_Implementation(const FName& SlotId) const
{
	const auto foundSlot = AttachmentSlots.FindByPredicate([SlotId](const TObjectPtr<UFVAdvancedAttachmentSlot>& Slot)
	{
		return Slot && Slot->SlotName == SlotId;
	});

	return foundSlot ? foundSlot->Get() : nullptr;
}

bool UFVAttachmentContainerComponent::IsSlotOccupied_Implementation(const FName& SlotId) const
{
	const auto foundSlotPtr = AttachmentSlots.FindByPredicate([SlotId](const UFVAdvancedAttachmentSlot* Slot) 
	{
		return Slot && Slot->SlotName == SlotId;
	});
	const UFVAdvancedAttachmentSlot* foundSlot = foundSlotPtr ? *foundSlotPtr : nullptr;
	return foundSlot && foundSlot->IsOccupied();
}

bool UFVAttachmentContainerComponent::DisableSlot_Implementation(const FName& SlotId)
{
	auto foundSlotPtr = AttachmentSlots.FindByPredicate([SlotId](const UFVAdvancedAttachmentSlot* Slot) 
	{
		return Slot && Slot->SlotName == SlotId;
	});
	UFVAdvancedAttachmentSlot* foundSlot = foundSlotPtr ? *foundSlotPtr : nullptr;
	if (!foundSlot)
		return false;

	foundSlot->DisableSlot();
	OnSlotStateChanged.Broadcast(SlotId, false);
	return true;
}

bool UFVAttachmentContainerComponent::TryAttach_Implementation(const FName& SlotId, UObject* Attachment)
{
	if (!Attachment) return false;

	if (!GetOwner()->HasAuthority())
	{
		Server_TryAttach(SlotId, Attachment);
		return true;
	}

	const bool bSuccess = TryAttachInternal(SlotId, Attachment);
	if (bSuccess)
		OnAttachmentChanged.Broadcast(SlotId, Attachment, nullptr);
	return bSuccess;
}

void UFVAttachmentContainerComponent::Server_TryAttach_Implementation(const FName& SlotId, UObject* Attachment)
{
	Execute_TryAttach(this, SlotId, Attachment);
}

bool UFVAttachmentContainerComponent::TryAttachInternal(const FName& SlotId, UObject* Attachment)
{
	if (!Attachment) return false;

	auto foundSlot = AttachmentSlots.FindByPredicate([SlotId](const UFVAdvancedAttachmentSlot* Slot)
	{
		return Slot && Slot->SlotName == SlotId;
	});
    
	return foundSlot && (*foundSlot)->Attach(Attachment);
}

bool UFVAttachmentContainerComponent::TryDetach_Implementation(const FName& SlotId)
{
	auto foundSlotPtr = AttachmentSlots.FindByPredicate([SlotId](const UFVAdvancedAttachmentSlot* Slot)
	{
		return Slot && Slot->SlotName == SlotId;
	});
	UFVAdvancedAttachmentSlot* foundSlot = foundSlotPtr ? *foundSlotPtr : nullptr;
	if (!foundSlot)
		return false;

	if (!GetOwner()->HasAuthority())
	{
		Server_TryDetach(SlotId);
		return true;
	}

	return foundSlot->Detach();
}

void UFVAttachmentContainerComponent::Server_TryDetach_Implementation(const FName& SlotId)
{
	Execute_TryDetach(this, SlotId);
}

bool UFVAttachmentContainerComponent::ForceAttach_Implementation(const FName& SlotId, UObject* Attachment)
{
	if (!Attachment)
		return false;

	auto foundSlotPtr = AttachmentSlots.FindByPredicate([SlotId](const UFVAdvancedAttachmentSlot* Slot)
	{
		return Slot && Slot->SlotName == SlotId;
	});
	UFVAdvancedAttachmentSlot* foundSlot = foundSlotPtr ? *foundSlotPtr : nullptr;
	if (!foundSlot)
		return false;

	foundSlot->Attachment = Attachment;
	foundSlot->State = EAttachmentSlotState::EASS_Occupied;
	return true;
}

bool UFVAttachmentContainerComponent::ForceDetach_Implementation(const FName& SlotId)
{
	auto foundSlotPtr = AttachmentSlots.FindByPredicate([SlotId](const UFVAdvancedAttachmentSlot* Slot)
	{
		return Slot && Slot->SlotName == SlotId;
	});
	UFVAdvancedAttachmentSlot* foundSlot = foundSlotPtr ? *foundSlotPtr : nullptr;
	if (!foundSlot)
		return false;

	foundSlot->Attachment = nullptr;
	foundSlot->State = EAttachmentSlotState::EASS_Empty;
	return true;
}

FName UFVAttachmentContainerComponent::FindFirstFreeSlotWithTags_Implementation(const FGameplayTagContainer& RequiredTags) const
{
	const auto* found = Algo::FindByPredicate(AttachmentSlots,
		[&](const UFVAdvancedAttachmentSlot* slot)
		{
			return slot != nullptr
				&& !slot->IsOccupied()
				&& slot->CanAttach()
				&& slot->MatchesTags(RequiredTags, true)
				&& !UFVEquipmentStatics::IsSlotBlockedByCurrentAttachments(this, slot, true);
		});

	return found ? (*found)->SlotName : NAME_None;
}

FName UFVAttachmentContainerComponent::GetSlotIdForAttachable_Implementation(const UFVAttachableComponent* Attachable) const
{
	if (!Attachable)
		return NAME_None;

	const auto* Found = Algo::FindByPredicate(AttachmentSlots,
		[&](const UFVAdvancedAttachmentSlot* Slot)
		{
			return Slot != nullptr && Slot->IsOccupied() && Slot->Attachment == Attachable;
		});

	return Found ? (*Found)->SlotName : NAME_None;	
}

FName UFVAttachmentContainerComponent::GetFirstEmptySlot_Implementation() const
{
	const auto* Found = Algo::FindByPredicate(AttachmentSlots,
		[&](const UFVAdvancedAttachmentSlot* Slot)
		{
			return Slot != nullptr && !Slot->IsOccupied();
		});

	return Found ? (*Found)->SlotName : NAME_None;
}

void UFVAttachmentContainerComponent::ClearAll_Implementation()
{
	for (auto& attachmentSlot : AttachmentSlots)
	{
		attachmentSlot->Detach();
	}
}

void UFVAttachmentContainerComponent::ApplyParentContainer()
{
	for (auto& slot : AttachmentSlots)
	{
		if (!slot || slot->ParentContainer.GetObject()) continue;
		slot->ParentContainer = this;
	}
}

TArray<FName> UFVAttachmentContainerComponent::GetAvailableTargetNames() const
{
	const AActor* ownerActor = UFVInventorySystemStatics::GetOwningActor(this);

	if (!IsValid(ownerActor))
	{
		LOG_ERROR(TEXT("[%s] No valid owner actor"), *GetName());
		return TArray<FName>();
	}
	return UFVAttachmentsStatics::GetAvailableComponentNames(ownerActor);
}

void UFVAttachmentContainerComponent::GetLifetimeReplicatedProps(
	TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UFVAttachmentContainerComponent, AttachmentSlots);
}

#if WITH_EDITOR

void UFVAttachmentContainerComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UFVAttachmentContainerComponent, AttachmentSlots))
	{
		ApplyParentContainer();
	}
}

void UFVAttachmentContainerComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UFVAttachmentContainerComponent, AttachmentSlots))
	{
		if (!(PropertyChangedEvent.ChangeType & (EPropertyChangeType::ArrayAdd | EPropertyChangeType::Duplicate)))
			return;

		const int32 slotIndex = PropertyChangedEvent.GetArrayIndex(GET_MEMBER_NAME_STRING_CHECKED(UFVAttachmentContainerComponent, AttachmentSlots));
		int32 resolvedSlotIndex = slotIndex;
		if (!AttachmentSlots.IsValidIndex(resolvedSlotIndex))
		{
			resolvedSlotIndex = AttachmentSlots.IndexOfByPredicate([](const TObjectPtr<UFVAdvancedAttachmentSlot>& Slot)
			{
				return Slot == nullptr;
			});
		}

		if (!AttachmentSlots.IsValidIndex(resolvedSlotIndex) || AttachmentSlots[resolvedSlotIndex] != nullptr)
			return;

		AttachmentSlots[resolvedSlotIndex] = NewObject<UFVAdvancedAttachmentSlot>(this, UFVAdvancedAttachmentSlot::StaticClass(), NAME_None, RF_Transactional);
		Modify();
	}
}

EDataValidationResult UFVAttachmentContainerComponent::IsDataValid(FDataValidationContext& Context) const
{
	for (const auto& slot : AttachmentSlots)
	{
		slot->IsDataValid(Context);
	}
	Super::IsDataValid(Context);
	return Context.GetNumErrors() > 0 ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}

#endif
