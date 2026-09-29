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


#include "Statics/FVAttachmentsStatics.h"

#include "Definitions/FVEquipmentBaseEnums.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Interfaces/Attachments/FVAdvancedAttachmentAttachableInterface.h"
#include "Interfaces/Attachments/FVAdvancedAttachmentContainerInterface.h"
#include "Statics/FVEquipmentStatics.h"

#define FV_BIND_ATTACHMENT_CONTAINER_DELEGATE(Target, Binding, HandleGetter) \
	if (!IsValid(Target) || !(Binding).IsBound()) \
		return false; \
	IFVAdvancedAttachmentContainerInterface* nativeInterface = Cast<IFVAdvancedAttachmentContainerInterface>(Target); \
	if (!nativeInterface) \
		return false; \
	nativeInterface->HandleGetter().AddUnique(Binding); \
	return true

#define FV_UNBIND_ATTACHMENT_CONTAINER_DELEGATE(Target, Binding, HandleGetter) \
	if (!IsValid(Target) || !(Binding).IsBound()) \
		return false; \
	IFVAdvancedAttachmentContainerInterface* nativeInterface = Cast<IFVAdvancedAttachmentContainerInterface>(Target); \
	if (!nativeInterface) \
		return false; \
	nativeInterface->HandleGetter().Remove(Binding); \
	return true

#define FV_BIND_ATTACHABLE_DELEGATE(Target, Binding, HandleGetter) \
	if (!IsValid(Target) || !(Binding).IsBound()) \
		return false; \
	IFVAdvancedAttachmentAttachableInterface* nativeInterface = Cast<IFVAdvancedAttachmentAttachableInterface>(Target); \
	if (!nativeInterface) \
		return false; \
	nativeInterface->HandleGetter().AddUnique(Binding); \
	return true

#define FV_UNBIND_ATTACHABLE_DELEGATE(Target, Binding, HandleGetter) \
	if (!IsValid(Target) || !(Binding).IsBound()) \
		return false; \
	IFVAdvancedAttachmentAttachableInterface* nativeInterface = Cast<IFVAdvancedAttachmentAttachableInterface>(Target); \
	if (!nativeInterface) \
		return false; \
	nativeInterface->HandleGetter().Remove(Binding); \
	return true

static void GatherSCSSceneComponentTemplates(const AActor* CDO, TArray<USceneComponent*>& Out)
{
	UBlueprintGeneratedClass* blueprintClass = Cast<UBlueprintGeneratedClass>(CDO->GetClass());
	if (!blueprintClass || !blueprintClass->SimpleConstructionScript)
		return;

	const TArray<USCS_Node*>& blueprintNodes = blueprintClass->SimpleConstructionScript->GetAllNodes();
	for (USCS_Node* blueprintNode : blueprintNodes)
	{
		if (!blueprintNode) continue;

		// Template object for this component as defined in the BP
		UActorComponent* componentTemplate = blueprintNode->GetActualComponentTemplate(blueprintClass);
		if (USceneComponent* sceneComponent = Cast<USceneComponent>(componentTemplate))
			Out.Add(sceneComponent);
	}
}

TArray<USceneComponent*> UFVAttachmentsStatics::GetAvailableComponents(const AActor* Target)
{
	TArray<USceneComponent*> components;
	if (!IsValid(Target))
		return components;

	Target->GetComponents<USceneComponent>(components);

#if WITH_EDITOR
	GatherSCSSceneComponentTemplates(Target, components);
#endif
	
	return components;
}

bool UFVAttachmentsStatics::BindToOnAttachmentChanged(UObject* Target, const FFVAttachmentChangedBinding& Binding)
{
	FV_BIND_ATTACHMENT_CONTAINER_DELEGATE(Target, Binding, GetOnAttachmentChangedEventHandle);
}

bool UFVAttachmentsStatics::UnbindFromOnAttachmentChanged(UObject* Target, const FFVAttachmentChangedBinding& Binding)
{
	FV_UNBIND_ATTACHMENT_CONTAINER_DELEGATE(Target, Binding, GetOnAttachmentChangedEventHandle);
}

bool UFVAttachmentsStatics::BindToOnSlotStateChanged(UObject* Target, const FFVSlotStateChangedBinding& Binding)
{
	FV_BIND_ATTACHMENT_CONTAINER_DELEGATE(Target, Binding, GetOnSlotStateChangedEventHandle);
}

bool UFVAttachmentsStatics::UnbindFromOnSlotStateChanged(UObject* Target, const FFVSlotStateChangedBinding& Binding)
{
	FV_UNBIND_ATTACHMENT_CONTAINER_DELEGATE(Target, Binding, GetOnSlotStateChangedEventHandle);
}

bool UFVAttachmentsStatics::BindToOnContainerCleared(UObject* Target, const FFVContainerClearedBinding& Binding)
{
	FV_BIND_ATTACHMENT_CONTAINER_DELEGATE(Target, Binding, GetOnContainerClearedEventHandle);
}

bool UFVAttachmentsStatics::UnbindFromOnContainerCleared(UObject* Target, const FFVContainerClearedBinding& Binding)
{
	FV_UNBIND_ATTACHMENT_CONTAINER_DELEGATE(Target, Binding, GetOnContainerClearedEventHandle);
}

bool UFVAttachmentsStatics::BindToOnAttachableAttached(UObject* Target, const FFVAttachableAttachedBinding& Binding)
{
	FV_BIND_ATTACHABLE_DELEGATE(Target, Binding, GetOnAttachableAttachedEventHandle);
}

bool UFVAttachmentsStatics::UnbindFromOnAttachableAttached(UObject* Target, const FFVAttachableAttachedBinding& Binding)
{
	FV_UNBIND_ATTACHABLE_DELEGATE(Target, Binding, GetOnAttachableAttachedEventHandle);
}

bool UFVAttachmentsStatics::BindToOnAttachableDetached(UObject* Target, const FFVAttachableDetachedBinding& Binding)
{
	FV_BIND_ATTACHABLE_DELEGATE(Target, Binding, GetOnAttachableDetachedEventHandle);
}

bool UFVAttachmentsStatics::UnbindFromOnAttachableDetached(UObject* Target, const FFVAttachableDetachedBinding& Binding)
{
	FV_UNBIND_ATTACHABLE_DELEGATE(Target, Binding, GetOnAttachableDetachedEventHandle);
}

TArray<FName> UFVAttachmentsStatics::GetAvailableComponentNames(const AActor* Target)
{
	TArray<USceneComponent*> components = GetAvailableComponents(Target);
	const auto allowedComponentTarget = UFVEquipmentStatics::GetAllowedAttachmentTargets();

	TArray<FName> names;
	names.Reserve(components.Num());

	Algo::TransformIf(
		components,
		names,
		[&allowedComponentTarget](const USceneComponent* Comp) { return IsValid(Comp) && allowedComponentTarget.Contains(Comp->GetClass()); },
		[](const USceneComponent* Comp)
		{
			FString componentName = Comp->GetFName().ToString();
			componentName.RemoveFromEnd(TEXT("_GEN_VARIABLE"));
			return FName(*componentName);
		}
	);

	return names;
}

USceneComponent* UFVAttachmentsStatics::GetAvailableComponentByName(const AActor* Target,
	const FName& ComponentName)
{
	if (!IsValid(Target) || ComponentName.IsNone())
		return nullptr;

	const TArray<USceneComponent*> components = GetAvailableComponents(Target);

	USceneComponent* const* match = Algo::FindByPredicate(
		components,
		[&](const USceneComponent* Comp)
		{
			return IsValid(Comp) && Comp->GetFName() == ComponentName;
		}
	);

	return match ? *match : nullptr;
}

TArray<FName> UFVAttachmentsStatics::GetAvailableSocketNames(const AActor* Target, const FName& ComponentName)
{
	if (!IsValid(Target) || ComponentName.IsNone())
		return {};

	const TArray<USceneComponent*> components = GetAvailableComponents(Target);

	const USceneComponent* const* match = Algo::FindByPredicate(
		components,
		[&](const USceneComponent* Comp)
		{
			return IsValid(Comp) && Comp->GetFName() == ComponentName;
		}
	);

	return match ? (*match)->GetAllSocketNames() : TArray<FName>();
}

bool UFVAttachmentsStatics::IsTargetClassValid(const UClass* TargetClass)
{
	if (!IsValid(TargetClass))
		return false;

	if (TargetClass->ImplementsInterface(UFVAdvancedAttachmentAttachableInterface::StaticClass()))
		return true;

	const AActor* targetActor = Cast<AActor>(TargetClass->GetDefaultObject());
	if (!IsValid(targetActor))
		return false;

	const TArray<UActorComponent*> components = targetActor->GetComponentsByInterface(
		UFVAdvancedAttachmentAttachableInterface::StaticClass());
	if (components.Num() > 0)
		return true;

	const UBlueprintGeneratedClass* blueprintClass = Cast<UBlueprintGeneratedClass>(TargetClass);
	if (!blueprintClass || !blueprintClass->SimpleConstructionScript)
		return false;

	for (const USCS_Node* node : blueprintClass->SimpleConstructionScript->GetAllNodes())
	{
		if (!node)
			continue;

		const UActorComponent* componentTemplate = node->GetActualComponentTemplate(
			const_cast<UBlueprintGeneratedClass*>(blueprintClass));
		if (IsValid(componentTemplate) && componentTemplate->GetClass()->ImplementsInterface(
			UFVAdvancedAttachmentAttachableInterface::StaticClass()))
			return true;
	}

	return false;
}

AActor* UFVAttachmentsStatics::GetOwningActor(
	const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_GetOwningActor(Target.GetObject()) : nullptr;
}

FName UFVAttachmentsStatics::GetDefaultAttachmentTarget(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_GetDefaultAttachmentTarget(Target.GetObject()) : NAME_None;
}

bool UFVAttachmentsStatics::IsValidSlot(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const FName& SlotId)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_IsValidSlot(Target.GetObject(), SlotId) : false;
}

UFVAdvancedAttachmentSlot* UFVAttachmentsStatics::GetSlot(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const FName& SlotId)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_GetSlot(Target.GetObject(), SlotId) : nullptr;
}

bool UFVAttachmentsStatics::IsSlotOccupied(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const FName& SlotId)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_IsSlotOccupied(Target.GetObject(), SlotId) : false;
}

bool UFVAttachmentsStatics::DisableSlot(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const FName& SlotId)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_DisableSlot(Target.GetObject(), SlotId) : false;
}

bool UFVAttachmentsStatics::TryAttach(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const FName& SlotId, UObject* Attachment)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_TryAttach(Target.GetObject(), SlotId, Attachment) : false;
}

bool UFVAttachmentsStatics::TryDetach(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const FName& SlotId)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_TryDetach(Target.GetObject(), SlotId) : false;
}

bool UFVAttachmentsStatics::ForceAttach(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const FName& SlotId, UObject* Attachment)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_ForceAttach(Target.GetObject(), SlotId, Attachment) : false;
}

bool UFVAttachmentsStatics::ForceDetach(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const FName& SlotId)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_ForceDetach(Target.GetObject(), SlotId) : false;
}

FName UFVAttachmentsStatics::FindFirstFreeSlotWithTags(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const FGameplayTagContainer& RequiredTags)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_FindFirstFreeSlotWithTags(Target.GetObject(), RequiredTags) : NAME_None;
}

FName UFVAttachmentsStatics::GetSlotIdForAttachable(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target, const UFVAttachableComponent* Attachable)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_GetSlotIdForAttachable(Target.GetObject(), Attachable) : NAME_None;
}

void UFVAttachmentsStatics::ClearAll(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target)
{
	if (Target.GetObject() != nullptr)
		Target->Execute_ClearAll(Target.GetObject());
}

FName UFVAttachmentsStatics::GetFirstEmptySlot(
	const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target)
{
		return Target.GetObject() != nullptr ? Target->Execute_GetFirstEmptySlot(Target.GetObject()) : NAME_None;
}

TArray<UFVAdvancedAttachmentSlot*> UFVAttachmentsStatics::GetAttachmentSlots(
	const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Target)
{
	return Target.GetObject() != nullptr ? Target->Execute_GetAttachmentSlots(Target.GetObject()) : TArray<UFVAdvancedAttachmentSlot*>();
}

TScriptInterface<IFVAdvancedAttachmentContainerInterface> UFVAttachmentsStatics::GetAttachedTo(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_GetAttachedTo(Target.GetObject()) : TScriptInterface<IFVAdvancedAttachmentContainerInterface>();
}

FName UFVAttachmentsStatics::GetId(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_GetId(Target.GetObject()) : NAME_None;
}

void UFVAttachmentsStatics::SetId(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target, const FName& NewId)
{
	if (Target.GetObject() != nullptr)
		Target->Execute_SetId(Target.GetObject(), NewId);
}

FText UFVAttachmentsStatics::GetDisplayName(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_GetDisplayName(Target.GetObject()) : FText::GetEmpty();
}

void UFVAttachmentsStatics::SetDisplayName(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target, const FText& NewDisplayName)
{
	if (Target.GetObject() != nullptr)
		Target->Execute_SetDisplayName(Target.GetObject(), NewDisplayName);
}

FGameplayTagContainer UFVAttachmentsStatics::GetTags(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_GetTags(Target.GetObject()) : FGameplayTagContainer();
}

void UFVAttachmentsStatics::SetTags(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target, const FGameplayTagContainer& NewTags)
{
	if (Target.GetObject() != nullptr)
		Target->Execute_SetTags(Target.GetObject(), NewTags);
}

EAttachmentState UFVAttachmentsStatics::GetState(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_GetState(Target.GetObject()) : EAttachmentState::Default;
}

void UFVAttachmentsStatics::SetState(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target, const EAttachmentState NewState)
{
	if (Target.GetObject() != nullptr)
		Target->Execute_SetState(Target.GetObject(), NewState);
}

bool UFVAttachmentsStatics::IsValidAttachable(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_IsValidAttachable(Target.GetObject()) : false;
}

bool UFVAttachmentsStatics::CanAttach(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_CanAttach(Target.GetObject()) : false;
}

bool UFVAttachmentsStatics::AttachToSlot(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target, const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Container, const FName& SlotId)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_AttachToSlot(Target.GetObject(), Container, SlotId) : false;
}

bool UFVAttachmentsStatics::AttachToContainer(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target, const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Container)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_AttachToContainer(Target.GetObject(), Container) : false;
}

bool UFVAttachmentsStatics::Detach(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_Detach(Target.GetObject()) : false;
}

bool UFVAttachmentsStatics::HasTag(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target, const FGameplayTag& Tag)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_HasTag(Target.GetObject(), Tag) : false;
}

bool UFVAttachmentsStatics::MatchesTags(const TScriptInterface<IFVAdvancedAttachmentAttachableInterface>& Target, const FGameplayTagContainer& OtherTags, bool bRequireAll)
{
	return (Target.GetObject() != nullptr) ? Target->Execute_MatchesTags(Target.GetObject(), OtherTags, bRequireAll) : false;
}
