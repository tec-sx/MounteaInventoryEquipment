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


#include "Components/FVEquipmentComponent.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Definitions/FVAdvancedAttachmentSlot.h"
#include "Definitions/FVEquipmentBaseEnums.h"
#include "Definitions/FVInventoryItemTemplate.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Equipment/FVAdvancedEquipmentItemInterface.h"
#include "Logs/FVAdvancedInventoryLog.h"
#include "Statics/FVEquipmentStatics.h"

UFVEquipmentComponent::UFVEquipmentComponent()
{
	ComponentTags.Append( { TEXT("Equipment") } );
}

void UFVEquipmentComponent::Server_EquipItem_Implementation(const FFVInventoryItem& ItemDefinition)
{
	Execute_EquipItem(this, ItemDefinition);
}

void UFVEquipmentComponent::Server_EquipItemToSlot_Implementation(const FFVInventoryItem& ItemDefinition, const FName& SlotId)
{
	Execute_EquipItemToSlot(this, SlotId, ItemDefinition);
}

void UFVEquipmentComponent::Server_UnequipItem_Implementation(const FFVInventoryItem& ItemDefinition, const bool bUseFallbackSlot)
{
	Execute_UnequipItem(this, ItemDefinition, bUseFallbackSlot);
}

bool UFVEquipmentComponent::ValidateEquipRequest(const FFVInventoryItem& ItemDefinition) const
{
	if (!ItemDefinition.IsItemValid())
	{
		LOG_WARNING(TEXT("[Equip Item]: Item Data not valid. No equipment will happen."))
		return false;
	}

	const UFVInventoryItemTemplate* templateAsset = ItemDefinition.GetTemplate();
	if (!IsValid(templateAsset))
	{
		LOG_WARNING(TEXT("[Equip Item]: Item Template not valid. No equipment will happen."))
		return false;
	}
	
	const auto spawnActorClass = templateAsset->SpawnActor;
	if (spawnActorClass.IsNull())
	{
		LOG_WARNING(TEXT("[Equip Item]: Spawn Actor not valid. No equipment will happen."))
		return false;
	}

	UClass* spawnClass = spawnActorClass.LoadSynchronous();
	if (!IsValid(spawnClass))
	{
		LOG_WARNING(TEXT("[Equip Item]: Spawn Actor class failed to load. No equipment will happen."))
		return false;
	}

	if (!UFVEquipmentStatics::IsTargetClassValid(spawnClass))
	{
		LOG_WARNING(TEXT("[Equip Item]: Spawn Actor '%s' is not a valid equipment item class."), *spawnClass->GetName());
		return false;
	}

	return true;
}

bool UFVEquipmentComponent::TryExecuteEquipOverride(const FFVInventoryItem& ItemDefinition, AActor*& OutSpawnedActor) const
{
	OutSpawnedActor = nullptr;
	if (OverrideEquipItemFunction.GetMemberName() == NAME_None)
		return false;

	UClass* scopeClass = OverrideEquipItemFunction.GetMemberParentClass();
	if (!IsValid(scopeClass))
		return false;

	UFunction* targetFunction = scopeClass->FindFunctionByName(OverrideEquipItemFunction.GetMemberName());
	if (!IsValid(targetFunction))
		return false;

	struct FParams
	{
		FFVInventoryItem ItemDefinition;
		AActor* OutSpawnedActor;
		bool ReturnValue;
	};

	FParams params;
	params.ItemDefinition = ItemDefinition;
	params.OutSpawnedActor = nullptr;
	params.ReturnValue = false;

	UObject* targetObject = nullptr;
	if (targetFunction->HasAnyFunctionFlags(FUNC_Static))
	{
		targetObject = scopeClass->GetDefaultObject();
	}
	else
	{
		UFVEquipmentComponent* mutableThis = const_cast<UFVEquipmentComponent*>(this);
		if (mutableThis->IsA(scopeClass))
		{
			targetObject = mutableThis;
		}
		else if (AActor* owner = GetOwner(); IsValid(owner) && owner->IsA(scopeClass))
		{
			targetObject = owner;
		}
	}

	if (!IsValid(targetObject))
		return false;

	targetObject->ProcessEvent(targetFunction, &params);
	OutSpawnedActor = params.OutSpawnedActor;
	return params.ReturnValue;
}

AActor* UFVEquipmentComponent::EquipItem_Implementation(const FFVInventoryItem& ItemDefinition)
{
	if (!ValidateEquipRequest(ItemDefinition))
		return nullptr;
	
	if (!IsAuthority())
	{
		Server_EquipItem(ItemDefinition);
		return nullptr;
	}
	
	AActor* spawnedActor = nullptr;
	if (TryExecuteEquipOverride(ItemDefinition, spawnedActor))
		return spawnedActor;

	// Fallback
	const bool bResult = UFVEquipmentStatics::EquipItemGeneral(this, ItemDefinition, spawnedActor);
	return bResult ? spawnedActor : nullptr;
}

AActor* UFVEquipmentComponent::EquipItemToSlot_Implementation(const FName& SlotId, const FFVInventoryItem& ItemDefinition)
{
	if (SlotId.IsNone())
		return nullptr;
	
	if (!Execute_IsValidSlot(this, SlotId))
		return nullptr;

	if (!IsAuthority())
	{
		Server_EquipItemToSlot(ItemDefinition, SlotId);
		return nullptr;
	}

	UFVAdvancedAttachmentSlot* targetSlot = Execute_GetSlot(this, SlotId);
	if (!IsValid(targetSlot))
		return nullptr;

	AActor* spawnedActor = nullptr;
	const bool bResult = UFVEquipmentStatics::EquipItemToSlot(this, ItemDefinition, targetSlot, spawnedActor);
	return bResult ? spawnedActor : nullptr;
}

bool UFVEquipmentComponent::UnequipItem_Implementation(const FFVInventoryItem& ItemDefinition, const bool bUseFallbackSlot)
{
	if (!ItemDefinition.IsItemValid())
		return false;

	if (!IsAuthority())
	{
		Server_UnequipItem(ItemDefinition, bUseFallbackSlot);
		return true;
	}

	const TArray<UFVAdvancedAttachmentSlot*> attachmentSlots = Execute_GetAttachmentSlots(this);
	for (const UFVAdvancedAttachmentSlot* slot : attachmentSlots)
	{
		if (!IsValid(slot) || !slot->IsOccupied() || !IsValid(slot->Attachment))
			continue;

		const TScriptInterface<IFVAdvancedEquipmentItemInterface> equipmentItemInterface = UFVEquipmentStatics::FindEquipmentItemInterface(slot->Attachment);
		if (!equipmentItemInterface.GetObject())
			continue;

		if (IFVAdvancedEquipmentItemInterface::Execute_GetEquippedItemId(equipmentItemInterface.GetObject()) != ItemDefinition.GetGuid())
			continue;

		return Execute_UnequipItemFromSlot(this, slot->SlotName, bUseFallbackSlot);
	}

	return false;
}

bool UFVEquipmentComponent::UnequipItemFromSlot_Implementation(const FName& SlotId, const bool bUseFallbackSlot)
{
	if (SlotId.IsNone())
		return false;
	if (!Execute_IsValidSlot(this, SlotId))
		return false;

	UFVAdvancedAttachmentSlot* targetSlot = Execute_GetSlot(this, SlotId);
	if (!IsValid(targetSlot) || !targetSlot->IsOccupied() || !IsValid(targetSlot->Attachment))
		return false;

	UObject* attachmentObject = targetSlot->Attachment;

	if (bUseFallbackSlot)
	{
		const FName fallbackSlotId = UFVEquipmentStatics::ResolveFallbackSlotId(SlotId);
		const bool bHasValidFallback = !fallbackSlotId.IsNone() &&
			fallbackSlotId != SlotId &&
			Execute_IsValidSlot(this, fallbackSlotId);

		if (bHasValidFallback)
		{
			UFVAdvancedAttachmentSlot* fallbackSlot = Execute_GetSlot(this, fallbackSlotId);
			if (!IsValid(fallbackSlot))
				return false;

			if (UFVEquipmentStatics::SwitchEquippedItemSlot(this, targetSlot, fallbackSlot))
			{
				const TScriptInterface<IFVAdvancedEquipmentItemInterface> equipItemInterface = UFVEquipmentStatics::FindEquipmentItemInterface(fallbackSlot->Attachment);
				if (equipItemInterface.GetObject())
					IFVAdvancedEquipmentItemInterface::Execute_SetEquipmentItemState(equipItemInterface.GetObject(), EEquipmentItemState::EES_Equipped);

				return true;
			}

			LOG_WARNING(TEXT("[Unequip Item From Slot]: Failed to switch slots. Falling back to detaching from current slot."));
		}
	}

	if (!Execute_TryDetach(this, SlotId))
		return false;

	AActor* detachedAttachmentActor = UFVEquipmentStatics::ResolveAttachmentActor(attachmentObject);
	if (IsValid(detachedAttachmentActor) && detachedAttachmentActor != GetOwner())
		detachedAttachmentActor->Destroy();

	return true;
}

bool UFVEquipmentComponent::IsEquipmentItemEquipped_Implementation(const FFVInventoryItem& ItemDefinition) const
{
	return UFVEquipmentStatics::ValidateItemEquipped(this, ItemDefinition);
}

bool UFVEquipmentComponent::IsEquipmentItemEquippedInSlot_Implementation(const FFVInventoryItem& ItemDefinition, const FName& SlotName) const
{
	return UFVEquipmentStatics::ValidateItemEquipped(this, ItemDefinition, SlotName);
}

bool UFVEquipmentComponent::ActivateQuickUseItem_Implementation(const FName& SlotId, const FName& TargetSlotId)
{
	if (SlotId.IsNone())
		return false;

	FGuid itemGuid;
	if (!UFVEquipmentStatics::TryGetEquippedItemGuidFromSlot(this, SlotId, itemGuid))
		return false;

	FFVInventoryItem quickUseItemDefinition;
	if (!UFVEquipmentStatics::TryResolveInventoryItemByGuid(this, itemGuid, quickUseItemDefinition))
	{
		LOG_WARNING(TEXT("[Activate Quick Use Item]: Failed to resolve inventory item definition for equipped item guid '%s'."),
			*itemGuid.ToString());
		return false;
	}

	const FName resolvedActiveSlotId = UFVEquipmentStatics::ResolveActiveSlotId(SlotId);
	const FName resolvedVisualSlotId = !TargetSlotId.IsNone()
		? TargetSlotId
		: (!resolvedActiveSlotId.IsNone() ? resolvedActiveSlotId : SlotId);

	AActor* quickUsePlaceholderActor = UFVEquipmentStatics::SpawnQuickUsePlaceholderActor(
		this,
		quickUseItemDefinition,
		resolvedVisualSlotId);
	const bool bRegisteredQuickUsePlaceholder = IsValid(quickUsePlaceholderActor)
		&& RegisterQuickUsePlaceholderActor(itemGuid, quickUsePlaceholderActor);

	if (IsValid(quickUsePlaceholderActor) && !bRegisteredQuickUsePlaceholder)
	{
		LOG_WARNING(TEXT("[Activate Quick Use Item]: Failed to register quick-use placeholder actor for '%s'. Actor will be kept alive temporarily."),
			*itemGuid.ToString());
		quickUsePlaceholderActor->SetLifeSpan(3.0f);
	}

	const bool bActivated = Execute_ActivateEquipmentItem(this, quickUseItemDefinition, resolvedVisualSlotId);
	if (!bActivated && bRegisteredQuickUsePlaceholder)
		ConsumeQuickUsePlaceholderActor(itemGuid, true);

	return bActivated;
}

bool UFVEquipmentComponent::BuildEquipmentTransitionContext(const FGuid& ItemGuid, const FName& TargetSlotId,
	const EEquipmentItemState ExpectedState, const EEquipmentItemState NewState, const bool bResolveAsActivation,
	FEquipmentTransitionContext& OutContext)
{
	OutContext = FEquipmentTransitionContext();
	OutContext.ItemGuid = ItemGuid;
	OutContext.ResolvedTargetSlotId = TargetSlotId;
	OutContext.ExpectedState = ExpectedState;
	OutContext.NewState = NewState;
	OutContext.bResolveAsActivation = bResolveAsActivation;

	UFVAdvancedAttachmentSlot* currentSlotRaw = nullptr;
	TScriptInterface<IFVAdvancedEquipmentItemInterface> equipItemInterface;
	if (!UFVEquipmentStatics::FindEquippedItemSlotAndInterface(this, ItemGuid, currentSlotRaw, equipItemInterface))
		return false;

	OutContext.CurrentSlot = currentSlotRaw;
	OutContext.EquipItemInterface = equipItemInterface;

	const EEquipmentItemState currentState = IFVAdvancedEquipmentItemInterface::Execute_GetEquipmentItemState(equipItemInterface.GetObject());
	if (currentState != ExpectedState)
		return false;

	if (OutContext.ResolvedTargetSlotId.IsNone())
	{
		OutContext.ResolvedTargetSlotId = bResolveAsActivation
			? UFVEquipmentStatics::ResolveActiveSlotId(currentSlotRaw->SlotName)
			: UFVEquipmentStatics::ResolveFallbackSlotId(currentSlotRaw->SlotName);
	}

	OutContext.bNeedsSlotSwitch = !OutContext.ResolvedTargetSlotId.IsNone() &&
		OutContext.ResolvedTargetSlotId != currentSlotRaw->SlotName;
	if (OutContext.bNeedsSlotSwitch)
	{
		OutContext.TargetSlot = Execute_GetSlot(this, OutContext.ResolvedTargetSlotId);
		if (!IsValid(OutContext.TargetSlot))
			return false;
	}

	return true;
}

bool UFVEquipmentComponent::BuildTransitionContextForType(const FGuid& ItemGuid, const FName& TargetSlotId,
	const EEquipmentTransitionType TransitionType, FEquipmentTransitionContext& OutContext)
{
	EEquipmentItemState expectedState = EEquipmentItemState::EES_Idle;
	EEquipmentItemState newState = EEquipmentItemState::EES_Idle;
	bool bResolveAsActivation = false;
	if (!ResolveTransitionDefinition(TransitionType, expectedState, newState, bResolveAsActivation))
		return false;

	return BuildEquipmentTransitionContext(
		ItemGuid,
		TargetSlotId,
		expectedState,
		newState,
		bResolveAsActivation,
		OutContext);
}

bool UFVEquipmentComponent::ResolveTransitionDefinition(const EEquipmentTransitionType TransitionType,
	EEquipmentItemState& OutExpectedState, EEquipmentItemState& OutNewState, bool& OutResolveAsActivation) const
{
	switch (TransitionType)
	{
		case EEquipmentTransitionType::EET_Activate:
			OutExpectedState = EEquipmentItemState::EES_Equipped;
			OutNewState = EEquipmentItemState::EES_Active;
			OutResolveAsActivation = true;
			return true;
		case EEquipmentTransitionType::EET_Deactivate:
			OutExpectedState = EEquipmentItemState::EES_Active;
			OutNewState = EEquipmentItemState::EES_Equipped;
			OutResolveAsActivation = false;
			return true;
		default:
			return false;
	}
}

bool UFVEquipmentComponent::ExecuteEquipmentStateTransition(const FEquipmentTransitionContext& Context, const bool bLocalOnly)
{
	if (!IsValid(Context.CurrentSlot) || !Context.EquipItemInterface.GetObject())
		return false;

	const EEquipmentItemState currentState = IFVAdvancedEquipmentItemInterface::Execute_GetEquipmentItemState(Context.EquipItemInterface.GetObject());
	if (currentState != Context.ExpectedState)
		return false;

	TScriptInterface<IFVAdvancedEquipmentItemInterface> equipItemInterface = Context.EquipItemInterface;
	if (!bLocalOnly && Context.NewState == EEquipmentItemState::EES_Active)
	{
		TSet<FName> ignoredSlots;
		if (IsValid(Context.CurrentSlot))
			ignoredSlots.Add(Context.CurrentSlot->SlotName);
		if (IsValid(Context.TargetSlot))
			ignoredSlots.Add(Context.TargetSlot->SlotName);

		if (!UFVEquipmentStatics::ClearBlockedSlotsForAttachment(
			this,
			Context.CurrentSlot->Attachment,
			Context.ItemGuid,
			ignoredSlots))
		{
			return false;
		}
	}

	if (Context.bNeedsSlotSwitch)
	{
		if (!IsValid(Context.TargetSlot))
			return false;

		bool bSwitchSuccess = false;
		if (bLocalOnly)
		{
			if (Context.TargetSlot->IsOccupied())
				return false;

			UObject* attachmentObject = Context.CurrentSlot->Attachment;
			if (!Context.CurrentSlot->Detach())
				return false;

			if (!Context.TargetSlot->Attach(attachmentObject))
			{
				Context.CurrentSlot->Attach(attachmentObject);
				return false;
			}

			bSwitchSuccess = true;
		}
		else
			bSwitchSuccess = UFVEquipmentStatics::SwitchEquippedItemSlot(this, Context.CurrentSlot, Context.TargetSlot);

		if (!bSwitchSuccess)
			return false;

		equipItemInterface = UFVEquipmentStatics::FindEquipmentItemInterface(Context.TargetSlot->Attachment);
	}

	if (!equipItemInterface.GetObject())
		return false;

	IFVAdvancedEquipmentItemInterface::Execute_SetEquipmentItemState(equipItemInterface.GetObject(), Context.NewState);
	return true;
}

bool UFVEquipmentComponent::ShouldUseDeferredTransition(const FEquipmentTransitionContext& Context,
	const EEquipmentTransitionType TransitionType) const
{
	if (TransitionType != EEquipmentTransitionType::EET_Activate && TransitionType != EEquipmentTransitionType::EET_Deactivate)
		return false;

	if (!Context.EquipItemInterface.GetObject())
		return false;

	const bool bItemAutoActivates = IFVAdvancedEquipmentItemInterface::Execute_DoesAutoActive(Context.EquipItemInterface.GetObject());
	const bool bRequiresEvent = IFVAdvancedEquipmentItemInterface::Execute_DoesRequireActivationEvent(Context.EquipItemInterface.GetObject());
	return !bItemAutoActivates && bRequiresEvent;
}

void UFVEquipmentComponent::ArmPendingActivation(const FFVInventoryItem& ItemDefinition, const FEquipmentTransitionContext& Context,
	const EEquipmentTransitionType TransitionType, UAnimMontage* Montage, const float MontageDuration)
{
	PendingActivation.ItemGuid = ItemDefinition.GetGuid();
	PendingActivation.SourceSlotId = IsValid(Context.CurrentSlot) ? Context.CurrentSlot->SlotName : NAME_None;
	PendingActivation.TargetSlotId = Context.ResolvedTargetSlotId;
	PendingActivation.Montage = Montage;
	PendingActivation.TransitionType = TransitionType;
	PendingActivation.TimeoutSeconds = ResolvePendingTransitionTimeout(Montage, MontageDuration);
	PendingActivation.RequestedAtTimeSeconds = GetOwner() && GetOwner()->GetWorld()
		? GetOwner()->GetWorld()->GetTimeSeconds()
		: -1.0;
	SetCurrentTransitionType(TransitionType);
}

double UFVEquipmentComponent::ResolvePendingTransitionTimeout(UAnimMontage* Montage, const float MontageDuration) const
{
	double timeoutSeconds = PendingActivationTimeoutSeconds;
	if (MontageDuration > 0.f)
		timeoutSeconds = MontageDuration;
	else if (IsValid(Montage))
		timeoutSeconds = Montage->GetPlayLength();

	if (timeoutSeconds <= 0.0)
		return timeoutSeconds;

	const double marginMultiplier = 1.0 + FMath::Max(0.0, static_cast<double>(PendingActivationTimeoutMarginPercent));
	return timeoutSeconds * marginMultiplier;
}

bool UFVEquipmentComponent::IsPendingActivationExpired() const
{
	// TODO: Investigate how to protect properly animation based workflow.
	// Timeout expiration is intentionally disabled for now.
	// Pending transitions are consumed by the first AnimAttachItem call.
	return false;
}

void UFVEquipmentComponent::ResetPendingActivationIfExpired()
{
	if (IsPendingActivationExpired())
	{
		PendingActivation.Reset();
		ResetCurrentTransitionType();
	}
}

bool UFVEquipmentComponent::IsTransitionInProgress(const EEquipmentTransitionType RequestedTransitionType) const
{
	if (PendingActivation.IsValid())
	{
		if (RequestedTransitionType == EEquipmentTransitionType::EET_None)
			return true;

		return PendingActivation.TransitionType == RequestedTransitionType;
	}

	if (CurrentTransitionType == EEquipmentTransitionType::EET_None)
		return false;

	if (RequestedTransitionType == EEquipmentTransitionType::EET_None)
		return true;

	return CurrentTransitionType == RequestedTransitionType;
}

void UFVEquipmentComponent::SetCurrentTransitionType(const EEquipmentTransitionType NewTransitionType)
{
	CurrentTransitionType = NewTransitionType;
}

void UFVEquipmentComponent::ResetCurrentTransitionType()
{
	CurrentTransitionType = EEquipmentTransitionType::EET_None;
}

UAnimInstance* UFVEquipmentComponent::ResolveOwnerAnimInstance() const
{
	const AActor* owner = GetOwner();
	if (!IsValid(owner))
		return nullptr;

	const USkeletalMeshComponent* mesh = owner->FindComponentByClass<USkeletalMeshComponent>();
	if (!IsValid(mesh))
		return nullptr;

	return mesh->GetAnimInstance();
}

UAnimMontage* UFVEquipmentComponent::ResolveTransitionMontage(const FEquipmentTransitionContext& Context,
	const EEquipmentTransitionType TransitionType) const
{
	if (!Context.EquipItemInterface.GetObject())
		return nullptr;

	UObject* itemObject = Context.EquipItemInterface.GetObject();
	switch (TransitionType)
	{
	case EEquipmentTransitionType::EET_Activate:
		return IFVAdvancedEquipmentItemInterface::Execute_GetActivationAnimation(itemObject);
	case EEquipmentTransitionType::EET_Deactivate:
		if (IFVAdvancedEquipmentItemInterface::Execute_DoesUseDeactivationAnimation(itemObject))
		{
			if (UAnimMontage* deactivationMontage = IFVAdvancedEquipmentItemInterface::Execute_GetDeactivationAnimation(itemObject))
				return deactivationMontage;
		}
		return IFVAdvancedEquipmentItemInterface::Execute_GetActivationAnimation(itemObject);
	default:
		return nullptr;
	}
}

bool UFVEquipmentComponent::TryStartTransitionMontage(const FFVInventoryItem& ItemDefinition,
	const FEquipmentTransitionContext& Context, const EEquipmentTransitionType TransitionType)
{
	if (!ShouldUseDeferredTransition(Context, TransitionType))
		return false;

	UAnimMontage* montage = ResolveTransitionMontage(Context, TransitionType);
	if (!IsValid(montage))
		return false;

	UAnimInstance* animInstance = ResolveOwnerAnimInstance();
	if (!IsValid(animInstance))
		return false;

	const float duration = animInstance->Montage_Play(montage);
	if (duration <= 0.f)
		return false;

	ArmPendingActivation(ItemDefinition, Context, TransitionType, montage, duration);

	FOnMontageEnded endDelegate;
	endDelegate.BindUObject(this, &UFVEquipmentComponent::OnTransitionMontageEnded);
	animInstance->Montage_SetEndDelegate(endDelegate, montage);
	return true;
}

bool UFVEquipmentComponent::ExecuteItemTransitionRequest(const FFVInventoryItem& ItemDefinition,
	const FName& TargetSlotId, const EEquipmentTransitionType TransitionType)
{
	ResetPendingActivationIfExpired();

	if (!ItemDefinition.IsItemValid())
		return false;

	if (IsTransitionInProgress())
		return false;

	FEquipmentTransitionContext transitionContext;
	if (!BuildTransitionContextForType(ItemDefinition.GetGuid(), TargetSlotId, TransitionType, transitionContext))
		return false;

	if (ShouldUseDeferredTransition(transitionContext, TransitionType))
	{
		if (TryStartTransitionMontage(ItemDefinition, transitionContext, TransitionType))
			return true;

		UAnimMontage* transitionMontage = ResolveTransitionMontage(transitionContext, TransitionType);
		if (!IsValid(transitionMontage))
		{
			ArmPendingActivation(ItemDefinition, transitionContext, TransitionType);
			return true;
		}

		const TCHAR* transitionLabel = TransitionType == EEquipmentTransitionType::EET_Activate
			? TEXT("Activate Equipment Item")
			: TEXT("Deactivate Equipment Item");
		LOG_WARNING(TEXT("[%s]: Failed to play transition montage. Falling back to immediate transition."), transitionLabel)
	}

	if (!IsAuthority())
	{
		if (TransitionType == EEquipmentTransitionType::EET_Activate)
			Server_ActivateEquipmentItem(ItemDefinition, transitionContext.ResolvedTargetSlotId);
		else if (TransitionType == EEquipmentTransitionType::EET_Deactivate)
			Server_DeactivateEquipmentItem(ItemDefinition, transitionContext.ResolvedTargetSlotId);
		else
			return false;

		return true;
	}

	return ExecuteEquipmentStateTransition(transitionContext);
}

bool UFVEquipmentComponent::ActivateEquipmentItem_Implementation(const FFVInventoryItem& ItemDefinition, const FName& TargetSlotId)
{
	return ExecuteItemTransitionRequest(ItemDefinition, TargetSlotId, EEquipmentTransitionType::EET_Activate);
}

bool UFVEquipmentComponent::DeactivateEquipmentItem_Implementation(const FFVInventoryItem& ItemDefinition, const FName& TargetSlotId)
{
	return ExecuteItemTransitionRequest(ItemDefinition, TargetSlotId, EEquipmentTransitionType::EET_Deactivate);
}

void UFVEquipmentComponent::Server_ActivateEquipmentItem_Implementation(const FFVInventoryItem& ItemDefinition, const FName& TargetSlotId)
{
	Execute_ActivateEquipmentItem(this, ItemDefinition, TargetSlotId);
}

void UFVEquipmentComponent::Server_DeactivateEquipmentItem_Implementation(const FFVInventoryItem& ItemDefinition, const FName& TargetSlotId)
{
	Execute_DeactivateEquipmentItem(this, ItemDefinition, TargetSlotId);
}

void UFVEquipmentComponent::Server_AnimAttachItem_Implementation(const FGuid& ItemGuid, const FName& TargetSlotId,
	const EEquipmentTransitionType TransitionType)
{
	FEquipmentTransitionContext transitionContext;
	if (!BuildTransitionContextForType(ItemGuid, TargetSlotId, TransitionType, transitionContext))
		return;

	ExecuteEquipmentStateTransition(transitionContext);
}

void UFVEquipmentComponent::Server_AnimQuickItemUsed_Implementation(const FGuid& ItemGuid)
{
	ConsumeQuickUsePlaceholderActor(ItemGuid, false);
}

bool UFVEquipmentComponent::AnimAttachItem_Implementation()
{
	ResetPendingActivationIfExpired();

	if (!PendingActivation.IsValid())
		return false;

	const FPendingEquipmentActivation pendingData = PendingActivation;
	PendingActivation.Reset();
	ResetCurrentTransitionType();

	if (!IsAuthority())
	{
		// Client-side prediction prevents temporary world-space floating before server replication arrives.
		FEquipmentTransitionContext transitionContext;
		if (BuildTransitionContextForType(
			pendingData.ItemGuid,
			pendingData.TargetSlotId,
			pendingData.TransitionType,
			transitionContext))
		{
			ExecuteEquipmentStateTransition(transitionContext, true);
		}

		Server_AnimAttachItem(pendingData.ItemGuid, pendingData.TargetSlotId, pendingData.TransitionType);
		return true;
	}

	FEquipmentTransitionContext transitionContext;
	if (!BuildTransitionContextForType(
		pendingData.ItemGuid,
		pendingData.TargetSlotId,
		pendingData.TransitionType,
		transitionContext))
	{
		return false;
	}

	return ExecuteEquipmentStateTransition(transitionContext);
}

bool UFVEquipmentComponent::AnimQuickItemUsed_Implementation()
{
	const FGuid quickUseItemGuid = QuickUsePlaceholderItemGuid;
	const bool bConsumed = ConsumeQuickUsePlaceholderActor(quickUseItemGuid, true);
	if (!bConsumed)
		return false;

	if (!IsAuthority())
		Server_AnimQuickItemUsed(quickUseItemGuid);

	return true;
}

bool UFVEquipmentComponent::RegisterQuickUsePlaceholderActor(const FGuid& ItemGuid, AActor* PlaceholderActor)
{
	if (!ItemGuid.IsValid() || !IsValid(PlaceholderActor))
		return false;

	if (PlaceholderActor == GetOwner())
		return false;

	if (QuickUsePlaceholderActor.IsValid() && QuickUsePlaceholderActor.Get() != PlaceholderActor)
	{
		LOG_WARNING(TEXT("[Register Quick Use Placeholder Actor]: Replacing existing quick use placeholder actor."))
		if (AActor* existingPlaceholder = QuickUsePlaceholderActor.Get(); IsValid(existingPlaceholder))
			existingPlaceholder->Destroy();
	}

	QuickUsePlaceholderActor = PlaceholderActor;
	QuickUsePlaceholderItemGuid = ItemGuid;
	return true;
}

bool UFVEquipmentComponent::ConsumeQuickUsePlaceholderActor(const FGuid& ItemGuid, const bool bIgnoreItemGuidMismatch)
{
	AActor* placeholderActor = QuickUsePlaceholderActor.Get();
	if (!IsValid(placeholderActor))
		return false;

	if (!bIgnoreItemGuidMismatch && ItemGuid.IsValid() && QuickUsePlaceholderItemGuid.IsValid() && ItemGuid != QuickUsePlaceholderItemGuid)
		return false;

	QuickUsePlaceholderActor.Reset();
	QuickUsePlaceholderItemGuid.Invalidate();

	if (placeholderActor != GetOwner())
		placeholderActor->Destroy();

	return true;
}

bool UFVEquipmentComponent::TryGetPendingEquipmentActivation(FPendingEquipmentActivation& OutPendingActivation) const
{
	if (IsPendingActivationExpired())
	{
		OutPendingActivation = FPendingEquipmentActivation();
		return false;
	}

	OutPendingActivation = PendingActivation;
	return OutPendingActivation.IsValid();
}

void UFVEquipmentComponent::OnTransitionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	PendingActivation.Reset();
	ResetCurrentTransitionType();
}

bool UFVEquipmentComponent::IsAuthority() const
{
	const AActor* owningActor = GetOwner();
	if (!owningActor || !owningActor->GetWorld())
		return false;

	if (const ENetMode netMode = owningActor->GetWorld()->GetNetMode(); netMode == NM_Standalone)
		return true;
	
	if (owningActor->HasAuthority())
		return true;
		
	return false;
}
