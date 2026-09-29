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

#include "Definitions/FVAdvancedAttachmentSlot.h"

#include "Definitions/FVEquipmentBaseEnums.h"
#include "Interfaces/Attachments/FVAdvancedAttachmentAttachableInterface.h"
#include "Interfaces/Attachments/FVAdvancedAttachmentContainerInterface.h"
#include "Logs/FVAdvancedInventoryLog.h"
#include "Misc/DataValidation.h"
#include "Statics/FVAttachmentsStatics.h"

UFVAdvancedAttachmentSlot::UFVAdvancedAttachmentSlot()
{
	SlotName = "NewEmptySlot";
}

void UFVAdvancedAttachmentSlot::BeginPlay_Implementation()
{
	Super::BeginPlay_Implementation();
	TryResolveAttachmentTarget();
}

void UFVAdvancedAttachmentSlot::TryResolveAttachmentTarget()
{
	if (AttachmentTargetOverride.IsNone() || !ParentContainer.GetObject())
		return;

	AActor* parentActor = ParentContainer->Execute_GetOwningActor(ParentContainer.GetObject());
	if (IsValid(parentActor))
		AttachmentTargetComponentOverride = UFVAttachmentsStatics::GetAvailableComponentByName(parentActor, AttachmentTargetOverride);
}

TArray<FName> UFVAdvancedAttachmentSlot::GetAvailableSocketNames() const
{
	if (SlotType != EAttachmentSlotType::EAST_Socket || !IsValid(ParentContainer.GetObject()))
		return TArray<FName>();

	AActor* parentActor = ParentContainer->Execute_GetOwningActor(ParentContainer.GetObject());
	if (!IsValid(parentActor))
		return TArray<FName>();

	FName targetName = AttachmentTargetOverride.IsNone() ? 
		ParentContainer->Execute_GetDefaultAttachmentTarget(ParentContainer.GetObject()) : 
		AttachmentTargetOverride;
		
	TArray<FName> returnValue = { NAME_None };
	returnValue.Append(UFVAttachmentsStatics::GetAvailableSocketNames(parentActor, targetName));
	return returnValue;
}

TArray<FName> UFVAdvancedAttachmentSlot::GetAvailableTargetNames() const
{
	if (!IsValid(ParentContainer.GetObject()))
		return TArray<FName>();

	AActor* parentActor = ParentContainer->Execute_GetOwningActor(ParentContainer.GetObject());
	TArray<FName> returnValue = { NAME_None };
	if (IsValid(parentActor))
		returnValue.Append(UFVAttachmentsStatics::GetAvailableComponentNames(parentActor));
	return returnValue;
}

USceneComponent* UFVAdvancedAttachmentSlot::GetAttachmentTargetComponent() const
{
	return AttachmentTargetComponentOverride ? 
		AttachmentTargetComponentOverride : 
		TObjectPtr<USceneComponent>(ParentContainer->Execute_GetAttachmentTargetComponent(ParentContainer.GetObject()));
}

bool UFVAdvancedAttachmentSlot::PerformAttachmentLogic(UObject* NewAttachment)
{
	USceneComponent* attachmentTarget = GetAttachmentTargetComponent();
	if (!IsValid(attachmentTarget))
	{
		LOG_WARNING(TEXT("Attachment target component is invalid!"));
		return false;
	}

	if (!ValidateAttachmentSlot(attachmentTarget))
		return false;

	if (IsValid(Attachment) && Attachment != NewAttachment)
		PerformDetachment();

	if (!PerformPhysicalAttachment(NewAttachment, attachmentTarget))
		return false;
	
	LastAttachment = Attachment;
	Attachment = NewAttachment;	
	State = EAttachmentSlotState::EASS_Occupied;
		
	HandleAttachableInterface(NewAttachment);
	return true;
}

void UFVAdvancedAttachmentSlot::HandleAttachableInterface(UObject* NewAttachment) const
{
	TScriptInterface<IFVAdvancedAttachmentAttachableInterface> attachableInterface = FindAttachableInterface(NewAttachment);
	if (attachableInterface.GetObject())
		IFVAdvancedAttachmentAttachableInterface::Execute_AttachToSlot(attachableInterface.GetObject(), ParentContainer, SlotName);
	else
		LOG_WARNING(TEXT("Attachment does not implement the attachable interface! Attachment will be performed, however, it may not behave as expected."));
}

bool UFVAdvancedAttachmentSlot::Attach(UObject* NewAttachment)
{
	return Super::Attach(NewAttachment) ? PerformAttachmentLogic(NewAttachment) : false;
}

bool UFVAdvancedAttachmentSlot::ForceAttach(UObject* NewAttachment)
{
	if (!IsValid(NewAttachment))
		return false;

	return PerformAttachmentLogic(NewAttachment);
}

TScriptInterface<IFVAdvancedAttachmentAttachableInterface> UFVAdvancedAttachmentSlot::FindAttachableInterface(UObject* Object)
{
	TScriptInterface<IFVAdvancedAttachmentAttachableInterface> returnAttachable;

	if (!IsValid(Object))
		return returnAttachable;

	if (Object->GetClass()->ImplementsInterface(UFVAdvancedAttachmentAttachableInterface::StaticClass()))
	{
		returnAttachable.SetObject(Object);
		returnAttachable.SetInterface(Cast<IFVAdvancedAttachmentAttachableInterface>(Object));
		return returnAttachable;
	}

	const AActor* targetActor = Cast<AActor>(Object);
	if (!IsValid(targetActor))
		return returnAttachable;

	const auto actorComponents = UFVAttachmentsStatics::GetAvailableComponents(targetActor);
	auto foundComponent = Algo::FindByPredicate(actorComponents, [](const UActorComponent* Comp)
	{
		return IsValid(Comp) && Comp->GetClass()->ImplementsInterface(UFVAdvancedAttachmentAttachableInterface::StaticClass());
	});

	if (foundComponent)
		returnAttachable = *foundComponent;

	return returnAttachable;
}

bool UFVAdvancedAttachmentSlot::ValidateAttachmentSlot(const USceneComponent* Target) const
{
	if (SlotType == EAttachmentSlotType::EAST_Socket && !Target->DoesSocketExist(SocketName))
	{
		LOG_WARNING(TEXT("Socket '%s' does not exist on target component '%s'!"), *SocketName.ToString(), *Target->GetName());
		return false;
	}
	return true;
}

bool UFVAdvancedAttachmentSlot::PerformPhysicalAttachment(UObject* Object, USceneComponent* Target) const
{
	const FAttachmentTransformRules attachmentRules = FAttachmentTransformRules::SnapToTargetIncludingScale;
	const FName attachmentName = GetAttachmentSocketName();
	
	if (USceneComponent* sceneComp = Cast<USceneComponent>(Object))
		return sceneComp->AttachToComponent(Target, attachmentRules, attachmentName);
	
	if (AActor* actor = Cast<AActor>(Object))
		return actor->AttachToComponent(Target, attachmentRules, attachmentName);
	
	LOG_WARNING(TEXT("Unsupported attachment object type: %s"), *Object->GetName());
	return false;
}

FName UFVAdvancedAttachmentSlot::GetAttachmentSocketName() const
{
	return SlotType == EAttachmentSlotType::EAST_Socket ? SocketName : NAME_None;
}

bool UFVAdvancedAttachmentSlot::Detach()
{
	if (!CanDetach())
		return false;

	PerformDetachment();
	return Super::Detach();
}

bool UFVAdvancedAttachmentSlot::ForceDetach()
{
	PerformDetachment();
	return Super::ForceDetach();
}

void UFVAdvancedAttachmentSlot::PerformDetachment() const
{
	UObject* targetAttachment = Attachment ? Attachment : LastAttachment;
	
	if (USceneComponent* sceneComp = Cast<USceneComponent>(targetAttachment))
		sceneComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	else if (AActor* actor = Cast<AActor>(targetAttachment))
		actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (IsValid(targetAttachment) && targetAttachment->Implements<UFVAdvancedAttachmentAttachableInterface>())
		IFVAdvancedAttachmentAttachableInterface::Execute_SetState(targetAttachment, EAttachmentState::EAS_Detached);
}

void UFVAdvancedAttachmentSlot::OnRep_State()
{
	switch (State)
	{
		case EAttachmentSlotState::EASS_Occupied:
			if (Attachment)
			{
				TryResolveAttachmentTarget();
				if (USceneComponent* attachmentTarget = GetAttachmentTargetComponent())
				{
					if (ValidateAttachmentSlot(attachmentTarget))
						PerformPhysicalAttachment(Attachment, attachmentTarget);
				}
			}
			break;
		case EAttachmentSlotState::EASS_Empty:
			PerformDetachment();
			break;
	}
	
	Super::OnRep_State();
}

#if WITH_EDITOR
EDataValidationResult UFVAdvancedAttachmentSlot::IsDataValid(class FDataValidationContext& Context) const
{
	if (!IsSlotValid())
		Context.AddError(NSLOCTEXT("AttachmentSlot", "AttachmentSlot_SlotInvalid", "Slot is invalid! It must have a valid name and tags."));
	
	UObject::IsDataValid(Context);
	return Context.GetNumErrors() > 0 ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}
#endif
