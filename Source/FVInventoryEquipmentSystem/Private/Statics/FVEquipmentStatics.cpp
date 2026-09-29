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


#include "Statics/FVEquipmentStatics.h"

#include "Algo/Count.h"
#include "Algo/Find.h"
#include "Algo/Sort.h"
#include "Algo/Transform.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/FVEquipmentComponent.h"
#include "Definitions/FVAdvancedAttachmentSlot.h"
#include "Definitions/FVEquipmentBaseEnums.h"
#include "Definitions/FVInventoryItem.h"
#include "Definitions/FVInventoryItemTemplate.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Attachments/FVAdvancedAttachmentAttachableInterface.h"
#include "Interfaces/Attachments/FVAdvancedAttachmentContainerInterface.h"
#include "Interfaces/Equipment/FVAdvancedEquipmentInterface.h"
#include "Interfaces/Inventory/FVAdvancedInventoryInterface.h"
#include "Logs/FVAdvancedInventoryLog.h"
#include "Settings/FVAdvancedEquipmentSettingsConfig.h"
#include "Settings/FVAdvancedInventorySettings.h"
#include "Statics/FVAttachmentsStatics.h"

#define FV_BIND_EQUIPMENT_ITEM_DELEGATE(Target, Binding, HandleGetter) \
	if (!IsValid(Target) || !(Binding).IsBound()) \
		return false; \
	IFVAdvancedEquipmentItemInterface* nativeInterface = Cast<IFVAdvancedEquipmentItemInterface>(Target); \
	if (!nativeInterface) \
		return false; \
	nativeInterface->HandleGetter().AddUnique(Binding); \
	return true

#define FV_UNBIND_EQUIPMENT_ITEM_DELEGATE(Target, Binding, HandleGetter) \
	if (!IsValid(Target) || !(Binding).IsBound()) \
		return false; \
	IFVAdvancedEquipmentItemInterface* nativeInterface = Cast<IFVAdvancedEquipmentItemInterface>(Target); \
	if (!nativeInterface) \
		return false; \
	nativeInterface->HandleGetter().Remove(Binding); \
	return true

const FFVEquipmentSlotHeaderData* UFVEquipmentStatics::ResolveSlotHeaderData(
	const UFVAdvancedAttachmentSlotBase* Slot,
	const UFVAdvancedEquipmentSettingsConfig* SettingsConfig)
{
	if (!IsValid(Slot) || !IsValid(SettingsConfig))
		return nullptr;

	return SettingsConfig->AllowedEquipmentSlots.Find(Slot->SlotName);
}

bool UFVEquipmentStatics::BindToOnEquipmentItemStateChanged(UObject* Target, const FFVEquipmentItemStateChangedBinding& Binding)
{
	FV_BIND_EQUIPMENT_ITEM_DELEGATE(Target, Binding, GetOnEquipmentItemStateChangedHandle);
}

bool UFVEquipmentStatics::UnbindFromOnEquipmentItemStateChanged(UObject* Target, const FFVEquipmentItemStateChangedBinding& Binding)
{
	FV_UNBIND_EQUIPMENT_ITEM_DELEGATE(Target, Binding, GetOnEquipmentItemStateChangedHandle);
}

bool UFVEquipmentStatics::TryGetAttachmentTags(UObject* AttachmentObject, FGameplayTagContainer& OutTags)
{
	OutTags.Reset();
	if (!IsValid(AttachmentObject))
		return false;

	const TScriptInterface<IFVAdvancedAttachmentAttachableInterface> attachableInterface =
		UFVAdvancedAttachmentSlot::FindAttachableInterface(AttachmentObject);
	if (!attachableInterface.GetObject())
		return false;

	OutTags = IFVAdvancedAttachmentAttachableInterface::Execute_GetTags(attachableInterface.GetObject());
	return !OutTags.IsEmpty();
}

bool UFVEquipmentStatics::IsAttachmentActiveEquipmentItem(UObject* AttachmentObject)
{
	const TScriptInterface<IFVAdvancedEquipmentItemInterface> equipmentItemInterface =
		FindEquipmentItemInterface(AttachmentObject);
	if (!equipmentItemInterface.GetObject())
		return true;

	return IFVAdvancedEquipmentItemInterface::Execute_GetEquipmentItemState(equipmentItemInterface.GetObject()) ==
		EEquipmentItemState::EES_Active;
}

bool UFVEquipmentStatics::ValidateSlotItemTypeCompatibility(
	const FFVInventoryItem& ItemDefinition,
	const UFVAdvancedAttachmentSlot* TargetSlot,
	const UFVAdvancedEquipmentSettingsConfig* SettingsConfig)
{
	if (!IsValid(TargetSlot) || !IsValid(SettingsConfig))
		return true;

	const FGameplayTagContainer* allowedItemTypesPtr = &TargetSlot->AllowedItemTypes;
	if (allowedItemTypesPtr->IsEmpty())
	{
		if (const FFVEquipmentSlotHeaderData* slotHeaderData = SettingsConfig->AllowedEquipmentSlots.Find(TargetSlot->SlotName))
			allowedItemTypesPtr = &slotHeaderData->AllowedItemTypes;
	}
	const FGameplayTagContainer& allowedItemTypes = *allowedItemTypesPtr;
	const UFVInventoryItemTemplate* itemTemplate = ItemDefinition.GetTemplate();
	const FGameplayTag equipmentItemType = IsValid(itemTemplate) ? itemTemplate->EquipmentItemType : FGameplayTag();

	if (allowedItemTypes.IsEmpty())
	{
		LOG_INFO(TEXT("[Validate Equipment Item Request]: Slot '%s' has empty AllowedItemTypes. Allowing all item types."),
			*TargetSlot->SlotName.ToString());
		return true;
	}

	if (!equipmentItemType.IsValid())
	{
		LOG_WARNING(TEXT("[Validate Equipment Item Request]: Item '%s' has empty EquipmentItemType. Allowing all slot types."),
			*ItemDefinition.GetItemName().ToString());
		return true;
	}

	if (!allowedItemTypes.HasTag(equipmentItemType))
	{
		LOG_WARNING(TEXT("[Validate Equipment Item Request]: Item '%s' (ItemType '%s') is not allowed in slot '%s'."),
			*ItemDefinition.GetItemName().ToString(),
			*equipmentItemType.ToString(),
			*TargetSlot->SlotName.ToString());
		return false;
	}

	return true;
}

AActor* UFVEquipmentStatics::ResolveOwningActor(UObject* Target)
{
	if (AActor* ownerActor = Cast<AActor>(Target))
		return ownerActor;

	if (const UActorComponent* ownerComponent = Cast<UActorComponent>(Target))
		return ownerComponent->GetOwner();

	return nullptr;
}

USceneComponent* UFVEquipmentStatics::ResolveQuickUseAttachmentTarget(UObject* Outer, const FName& VisualSlotId, FName& OutSocketName)
{
	OutSocketName = NAME_None;

	if (IsValid(Outer) && Outer->Implements<UFVAdvancedAttachmentContainerInterface>() && !VisualSlotId.IsNone())
	{
		UFVAdvancedAttachmentSlot* visualSlot =
			IFVAdvancedAttachmentContainerInterface::Execute_GetSlot(Outer, VisualSlotId);
		if (IsValid(visualSlot))
		{
			OutSocketName = visualSlot->GetAttachmentSocketName();

			if (IsValid(visualSlot->AttachmentTargetComponentOverride))
				return visualSlot->AttachmentTargetComponentOverride;

			if (visualSlot->ParentContainer.GetObject())
				return IFVAdvancedAttachmentContainerInterface::Execute_GetAttachmentTargetComponent(
					visualSlot->ParentContainer.GetObject());

			return IFVAdvancedAttachmentContainerInterface::Execute_GetAttachmentTargetComponent(Outer);
		}
	}

	AActor* owningActor = ResolveOwningActor(Outer);
	if (!IsValid(owningActor))
		return nullptr;

	if (USkeletalMeshComponent* ownerSkeletalMesh = owningActor->FindComponentByClass<USkeletalMeshComponent>())
		return ownerSkeletalMesh;

	return owningActor->GetRootComponent();
}

void UFVEquipmentStatics::ApplyQuickUsePlaceholderMesh(AActor* PlaceholderActor, const FFVInventoryItem& ItemDefinition, const UClass* PlaceholderClass)
{
	if (!IsValid(PlaceholderActor) || !ItemDefinition.IsItemValid())
		return;

	const UFVInventoryItemTemplate* itemTemplate = ItemDefinition.GetTemplate();
	if (!IsValid(itemTemplate) || !IsValid(itemTemplate->ItemMesh))
		return;

	if (UStaticMesh* staticMesh = Cast<UStaticMesh>(itemTemplate->ItemMesh))
	{
		if (!SetQuickUseItemStaticMesh(PlaceholderActor, staticMesh))
		{
			const FString className = IsValid(PlaceholderClass) ? PlaceholderClass->GetName() : TEXT("Unknown");
			LOG_WARNING(TEXT("[Spawn Quick Use Placeholder Actor]: Placeholder class '%s' does not expose quick-use mesh interface for static mesh setup."),
				*className)
		}
		return;
	}

	if (USkeletalMesh* skeletalMesh = Cast<USkeletalMesh>(itemTemplate->ItemMesh))
	{
		if (!SetQuickUseItemSkeletalMesh(PlaceholderActor, skeletalMesh))
		{
			const FString className = IsValid(PlaceholderClass) ? PlaceholderClass->GetName() : TEXT("Unknown");
			LOG_WARNING(TEXT("[Spawn Quick Use Placeholder Actor]: Placeholder class '%s' does not expose quick-use mesh interface for skeletal mesh setup."),
				*className)
		}
	}
}

UObject* UFVEquipmentStatics::ResolveFirstInterfaceObject(UObject* Target, TSubclassOf<UInterface> InterfaceClass,
	const bool bResolveOwnerForComponents)
{
	const UClass* interfaceClassPtr = InterfaceClass.Get();
	if (!IsValid(Target) || !IsValid(interfaceClassPtr))
		return nullptr;

	if (Target->GetClass()->ImplementsInterface(interfaceClassPtr))
		return Target;

	AActor* actor = Cast<AActor>(Target);
	if (!IsValid(actor) && bResolveOwnerForComponents)
	{
		if (const UActorComponent* component = Cast<UActorComponent>(Target))
			actor = component->GetOwner();
	}

	return ResolveFirstInterfaceObjectFromActor(actor, InterfaceClass);
}

UObject* UFVEquipmentStatics::ResolveFirstInterfaceObjectFromActor(AActor* Actor, TSubclassOf<UInterface> InterfaceClass)
{
	const UClass* interfaceClassPtr = InterfaceClass.Get();
	if (!IsValid(Actor) || !IsValid(interfaceClassPtr))
		return nullptr;

	const TArray<UActorComponent*> components = Actor->GetComponentsByInterface(InterfaceClass);
	if (!components.IsEmpty())
		return components[0];

	const UBlueprintGeneratedClass* blueprintClass = Cast<UBlueprintGeneratedClass>(Actor->GetClass());
	return ResolveFirstInterfaceObjectFromBlueprintClass(blueprintClass, InterfaceClass);
}

UObject* UFVEquipmentStatics::ResolveFirstInterfaceObjectFromBlueprintClass(
	const UBlueprintGeneratedClass* BlueprintClass,
	TSubclassOf<UInterface> InterfaceClass)
{
	const UClass* interfaceClassPtr = InterfaceClass.Get();
	if (!IsValid(BlueprintClass) || !IsValid(interfaceClassPtr) || !BlueprintClass->SimpleConstructionScript)
		return nullptr;

	for (const USCS_Node* node : BlueprintClass->SimpleConstructionScript->GetAllNodes())
	{
		if (!node)
			continue;

		const UActorComponent* componentTemplate = node->GetActualComponentTemplate(
			const_cast<UBlueprintGeneratedClass*>(BlueprintClass));
		if (IsValid(componentTemplate) && componentTemplate->GetClass()->ImplementsInterface(interfaceClassPtr))
			return const_cast<UActorComponent*>(componentTemplate);
	}

	return nullptr;
}

UFVAdvancedEquipmentSettingsConfig* UFVEquipmentStatics::GetEquipmentSettingsConfig()
{
	const auto* inventorySettings = GetMutableDefault<UFVAdvancedInventorySettings>();
	return inventorySettings ? inventorySettings->AdvancedEquipmentSettingsConfig.LoadSynchronous() : nullptr;
}

TSet<TSoftClassPtr<USceneComponent>> UFVEquipmentStatics::GetAllowedAttachmentTargets()
{
	if (const auto* equipmentConfig = GetEquipmentSettingsConfig())
		return equipmentConfig->AllowedAttachmentTargets;

	TSet<TSoftClassPtr<USceneComponent>> returnValue;

	returnValue.Add(TSoftClassPtr<USceneComponent>(USceneComponent::StaticClass()));
	returnValue.Add(TSoftClassPtr<USceneComponent>(UStaticMeshComponent::StaticClass()));
	returnValue.Add(TSoftClassPtr<USceneComponent>(USkeletalMeshComponent::StaticClass()));

	return returnValue;
}

TSoftClassPtr<AActor> UFVEquipmentStatics::GetDefaultQuickUseItemClass()
{
	if (const auto* equipmentConfig = GetEquipmentSettingsConfig())
		return equipmentConfig->DefaultQuickUseItemClass;

	return TSoftClassPtr<AActor>();
}

TScriptInterface<IFVAdvancedEquipmentItemInterface> UFVEquipmentStatics::FindEquipmentItemInterface(UObject* Target)
{
	TScriptInterface<IFVAdvancedEquipmentItemInterface> result;

	UObject* interfaceObject = ResolveFirstInterfaceObject(
		Target,
		UFVAdvancedEquipmentItemInterface::StaticClass(),
		false);
	if (IsValid(interfaceObject))
	{
		result.SetObject(interfaceObject);
		result.SetInterface(Cast<IFVAdvancedEquipmentItemInterface>(interfaceObject));
	}

	return result;
}

TScriptInterface<IFVAdvancedEquipmentInterface> UFVEquipmentStatics::FindEquipmentInterface(UObject* Target)
{
	TScriptInterface<IFVAdvancedEquipmentInterface> result;

	UObject* interfaceObject = ResolveFirstInterfaceObject(
		Target,
		UFVAdvancedEquipmentInterface::StaticClass(),
		true);
	if (IsValid(interfaceObject))
	{
		result.SetObject(interfaceObject);
		result.SetInterface(Cast<IFVAdvancedEquipmentInterface>(interfaceObject));
	}

	return result;
}

TScriptInterface<IFVAdvancedQuickUseItemInterface> UFVEquipmentStatics::FindQuickUseItemInterface(UObject* Target)
{
	TScriptInterface<IFVAdvancedQuickUseItemInterface> result;

	UObject* interfaceObject = ResolveFirstInterfaceObject(
		Target,
		UFVAdvancedQuickUseItemInterface::StaticClass(),
		true);
	if (IsValid(interfaceObject))
	{
		result.SetObject(interfaceObject);
		result.SetInterface(Cast<IFVAdvancedQuickUseItemInterface>(interfaceObject));
	}

	return result;
}

bool UFVEquipmentStatics::SetQuickUseItemStaticMesh(UObject* Target, UStaticMesh* NewStaticMesh)
{
	const TScriptInterface<IFVAdvancedQuickUseItemInterface> quickUseInterface = FindQuickUseItemInterface(Target);
	if (!quickUseInterface.GetObject())
		return false;

	return IFVAdvancedQuickUseItemInterface::Execute_SetStaticMesh(quickUseInterface.GetObject(), NewStaticMesh);
}

bool UFVEquipmentStatics::SetQuickUseItemSkeletalMesh(UObject* Target, USkeletalMesh* NewSkeletalMesh)
{
	const TScriptInterface<IFVAdvancedQuickUseItemInterface> quickUseInterface = FindQuickUseItemInterface(Target);
	if (!quickUseInterface.GetObject())
		return false;

	return IFVAdvancedQuickUseItemInterface::Execute_SetSkeletalMesh(quickUseInterface.GetObject(), NewSkeletalMesh);
}

FName UFVEquipmentStatics::ResolveFallbackSlotId(const FName& CurrentSlotId)
{
	const UFVAdvancedEquipmentSettingsConfig* settingsConfig = GetEquipmentSettingsConfig();
	if (!settingsConfig)
		return NAME_None;

	const FFVEquipmentSlotHeaderData* slotData = settingsConfig->AllowedEquipmentSlots.Find(CurrentSlotId);
	if (!slotData)
		return NAME_None;

	return slotData->FallbackSlot;
}

FName UFVEquipmentStatics::ResolveActiveSlotId(const FName& StorageSlotId)
{
	const UFVAdvancedEquipmentSettingsConfig* settingsConfig = GetEquipmentSettingsConfig();
	if (!settingsConfig)
		return NAME_None;

	for (const auto& [slotId, slotData] : settingsConfig->AllowedEquipmentSlots)
	{
		if (slotData.FallbackSlot == StorageSlotId)
			return slotId;
	}

	return NAME_None;
}

FName UFVEquipmentStatics::ResolveBestSlotIdFromTags(const FGameplayTagContainer& DesiredTags,
	const FGameplayTag& EquipmentItemType)
{
	const UFVAdvancedEquipmentSettingsConfig* settingsConfig = GetEquipmentSettingsConfig();
	if (!IsValid(settingsConfig))
		return NAME_None;
	if (DesiredTags.IsEmpty())
		return NAME_None;
	if (settingsConfig->AllowedEquipmentSlots.IsEmpty())
		return NAME_None;

	struct FSlotCandidate
	{
		FName SlotName = NAME_None;
		bool bHasAllDesiredTags = false;
		int32 OverlapCount = 0;
		int32 ExtraTags = 0;
	};

	TArray<FGameplayTag> desiredTags;
	DesiredTags.GetGameplayTagArray(desiredTags);

	const auto isSlotCompatible = [&desiredTags, &settingsConfig, &EquipmentItemType](const FName& slotName)
	{
		const FFVEquipmentSlotHeaderData* slotData = settingsConfig->AllowedEquipmentSlots.Find(slotName);
		if (!slotData)
			return false;

		if (!slotData->bIsEnabled)
			return false;

		if (EquipmentItemType.IsValid() &&
			!slotData->AllowedItemTypes.IsEmpty() &&
			!slotData->AllowedItemTypes.HasTag(EquipmentItemType))
		{
			return false;
		}

		const int32 overlapCount = Algo::CountIf(desiredTags, [slotData](const FGameplayTag& desiredTag)
		{
			return slotData->TagContainer.HasTag(desiredTag);
		});

		return overlapCount > 0;
	};

	TArray<FName> slotNames;
	settingsConfig->AllowedEquipmentSlots.GetKeys(slotNames);

	TArray<FSlotCandidate> candidates;
	candidates.Reserve(slotNames.Num());

	Algo::TransformIf(
		slotNames,
		candidates,
		isSlotCompatible,
		[&desiredTags, &settingsConfig, &DesiredTags](const FName& slotName)
		{
			const FFVEquipmentSlotHeaderData* slotData = settingsConfig->AllowedEquipmentSlots.Find(slotName);
			check(slotData);

			const int32 overlapCount = Algo::CountIf(desiredTags, [slotData](const FGameplayTag& desiredTag)
			{
				return slotData->TagContainer.HasTag(desiredTag);
			});

			FSlotCandidate candidate;
			candidate.SlotName = slotName;
			candidate.bHasAllDesiredTags = slotData->TagContainer.HasAll(DesiredTags);
			candidate.OverlapCount = overlapCount;
			candidate.ExtraTags = slotData->TagContainer.Num() - overlapCount;
			return candidate;
		});

	if (candidates.IsEmpty())
		return NAME_None;

	Algo::Sort(candidates, [](const FSlotCandidate& left, const FSlotCandidate& right)
	{
		if (left.bHasAllDesiredTags != right.bHasAllDesiredTags)
			return left.bHasAllDesiredTags;

		if (left.OverlapCount != right.OverlapCount)
			return left.OverlapCount > right.OverlapCount;

		if (left.ExtraTags != right.ExtraTags)
			return left.ExtraTags < right.ExtraTags;

		return left.SlotName.LexicalLess(right.SlotName);
	});

	return candidates[0].SlotName;
}

UFVAdvancedAttachmentSlot* UFVEquipmentStatics::FindSlotWithEquippedItem(UObject* Outer, const FGuid& ItemGuid)
{
	UFVAdvancedAttachmentSlot* foundSlot = nullptr;
	TScriptInterface<IFVAdvancedEquipmentItemInterface> foundInterface;
	return FindEquippedItemSlotAndInterface(Outer, ItemGuid, foundSlot, foundInterface) ? foundSlot : nullptr;
}

bool UFVEquipmentStatics::FindEquippedItemSlotAndInterface(UObject* Outer, const FGuid& ItemGuid,
	UFVAdvancedAttachmentSlot*& OutSlot, TScriptInterface<IFVAdvancedEquipmentItemInterface>& OutInterface)
{
	OutSlot = nullptr;
	OutInterface = TScriptInterface<IFVAdvancedEquipmentItemInterface>();

	if (!IsValid(Outer) || !ItemGuid.IsValid())
		return false;
	if (!Outer->Implements<UFVAdvancedAttachmentContainerInterface>())
		return false;

	const TArray<UFVAdvancedAttachmentSlot*> slots = IFVAdvancedAttachmentContainerInterface::Execute_GetAttachmentSlots(Outer);
	for (UFVAdvancedAttachmentSlot* slot : slots)
	{
		if (!IsValid(slot) || !slot->IsOccupied() || !IsValid(slot->Attachment))
			continue;

		const TScriptInterface<IFVAdvancedEquipmentItemInterface> iface = FindEquipmentItemInterface(slot->Attachment);
		if (!iface.GetObject())
			continue;

		if (IFVAdvancedEquipmentItemInterface::Execute_GetEquippedItemId(iface.GetObject()) == ItemGuid)
		{
			OutSlot = slot;
			OutInterface = iface;
			return true;
		}
	}

	return false;
}

bool UFVEquipmentStatics::TryGetEquippedItemGuidFromSlot(UObject* Outer, const FName& SlotId, FGuid& OutItemGuid)
{
	OutItemGuid = FGuid();

	if (!IsValid(Outer) || SlotId.IsNone())
		return false;

	if (!Outer->Implements<UFVAdvancedAttachmentContainerInterface>())
		return false;

	if (!IFVAdvancedAttachmentContainerInterface::Execute_IsValidSlot(Outer, SlotId))
		return false;

	const UFVAdvancedAttachmentSlot* slot = IFVAdvancedAttachmentContainerInterface::Execute_GetSlot(Outer, SlotId);
	if (!IsValid(slot) || !slot->IsOccupied() || !IsValid(slot->Attachment))
		return false;

	const TScriptInterface<IFVAdvancedEquipmentItemInterface> itemInterface = FindEquipmentItemInterface(slot->Attachment);
	if (!itemInterface.GetObject())
		return false;

	OutItemGuid = IFVAdvancedEquipmentItemInterface::Execute_GetEquippedItemId(itemInterface.GetObject());
	return OutItemGuid.IsValid();
}

bool UFVEquipmentStatics::TryResolveInventoryItemByGuid(UObject* Outer, const FGuid& ItemGuid, FFVInventoryItem& OutItemDefinition)
{
	OutItemDefinition = FFVInventoryItem();

	if (!IsValid(Outer) || !ItemGuid.IsValid())
		return false;

	const auto tryResolveFromInventory = [&](const UObject* Candidate) -> bool
	{
		if (!IsValid(Candidate) || !Candidate->Implements<UFVAdvancedInventoryInterface>())
			return false;

		const FFVInventoryItem foundItem = IFVAdvancedInventoryInterface::Execute_FindItem(
			Candidate,
			FInventoryItemSearchParams(ItemGuid));
		if (!foundItem.IsItemValid())
			return false;

		OutItemDefinition = foundItem;
		return true;
	};

	if (tryResolveFromInventory(Outer))
		return true;

	AActor* owningActor = ResolveOwningActor(Outer);
	if (tryResolveFromInventory(owningActor))
		return true;

	if (!IsValid(owningActor))
		return false;

	const TArray<UActorComponent*> inventoryComponents = owningActor->GetComponentsByInterface(
		UFVAdvancedInventoryInterface::StaticClass());
	for (UActorComponent* inventoryComponent : inventoryComponents)
	{
		if (tryResolveFromInventory(inventoryComponent))
			return true;
	}

	return false;
}

AActor* UFVEquipmentStatics::SpawnQuickUsePlaceholderActor(UObject* Outer, const FFVInventoryItem& ItemDefinition,
	const FName& VisualSlotId)
{
	if (!IsValid(Outer) || !ItemDefinition.IsItemValid())
		return nullptr;

	UWorld* world = Outer->GetWorld();
	if (!IsValid(world) || world->GetNetMode() == NM_DedicatedServer)
		return nullptr;

	const TSoftClassPtr<AActor> quickUseClass = GetDefaultQuickUseItemClass();
	if (quickUseClass.IsNull())
		return nullptr;

	UClass* quickUseActorClass = quickUseClass.LoadSynchronous();
	if (!IsValid(quickUseActorClass))
	{
		LOG_WARNING(TEXT("[Spawn Quick Use Placeholder Actor]: Default quick use item class failed to load."))
		return nullptr;
	}

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = ResolveOwningActor(Outer);
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* placeholderActor = world->SpawnActor<AActor>(quickUseActorClass, FTransform::Identity, spawnParams);
	if (!IsValid(placeholderActor))
		return nullptr;

	placeholderActor->SetReplicates(false);
	placeholderActor->SetActorEnableCollision(false);
	ApplyQuickUsePlaceholderMesh(placeholderActor, ItemDefinition, quickUseActorClass);

	FName attachmentSocket = NAME_None;
	USceneComponent* attachmentTarget = ResolveQuickUseAttachmentTarget(Outer, VisualSlotId, attachmentSocket);
	if (IsValid(attachmentTarget))
	{
		placeholderActor->AttachToComponent(
			attachmentTarget,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			attachmentSocket);
	}

	return placeholderActor;
}

bool UFVEquipmentStatics::IsSlotBlockedByCurrentAttachments(const UObject* Outer,
	const UFVAdvancedAttachmentSlotBase* TargetSlot, const bool bIgnoreTargetSlotOccupant)
{
	if (!IsValid(Outer) || !IsValid(TargetSlot))
		return false;

	if (!Outer->Implements<UFVAdvancedAttachmentContainerInterface>())
		return false;

	const UFVAdvancedEquipmentSettingsConfig* settingsConfig = GetEquipmentSettingsConfig();
	const FFVEquipmentSlotHeaderData* slotHeaderData = ResolveSlotHeaderData(TargetSlot, settingsConfig);
	if (!slotHeaderData || slotHeaderData->BlockedByTags.IsEmpty())
		return false;

	const TArray<UFVAdvancedAttachmentSlot*> slots =
		IFVAdvancedAttachmentContainerInterface::Execute_GetAttachmentSlots(Outer);
	for (const UFVAdvancedAttachmentSlot* slot : slots)
	{
		if (!IsValid(slot) || !slot->IsOccupied() || !IsValid(slot->Attachment))
			continue;

		if (bIgnoreTargetSlotOccupant && slot == TargetSlot)
			continue;

		if (!IsAttachmentActiveEquipmentItem(slot->Attachment))
			continue;

		FGameplayTagContainer attachmentTags;
		if (!TryGetAttachmentTags(slot->Attachment, attachmentTags))
			continue;

		if (slotHeaderData->BlockedByTags.HasAny(attachmentTags))
			return true;
	}

	return false;
}

bool UFVEquipmentStatics::ClearBlockedSlotsForAttachment(UObject* Outer, UObject* BlockingAttachment,
	const FGuid& BlockingItemGuid, const TSet<FName>& IgnoredSlots)
{
	if (!IsValid(Outer))
		return false;

	if (!IsValid(BlockingAttachment))
		return true;

	if (!Outer->Implements<UFVAdvancedAttachmentContainerInterface>())
		return false;

	const UFVAdvancedEquipmentSettingsConfig* settingsConfig = GetEquipmentSettingsConfig();
	if (!IsValid(settingsConfig))
		return true;

	FGameplayTagContainer blockingTags;
	if (!TryGetAttachmentTags(BlockingAttachment, blockingTags) || blockingTags.IsEmpty())
		return true;

	const TArray<UFVAdvancedAttachmentSlot*> slots =
		IFVAdvancedAttachmentContainerInterface::Execute_GetAttachmentSlots(Outer);
	for (const UFVAdvancedAttachmentSlot* slot : slots)
	{
		if (!IsValid(slot) || IgnoredSlots.Contains(slot->SlotName))
			continue;

		const FFVEquipmentSlotHeaderData* slotHeaderData = ResolveSlotHeaderData(slot, settingsConfig);
		if (!slotHeaderData || slotHeaderData->BlockedByTags.IsEmpty())
			continue;

		if (!slotHeaderData->BlockedByTags.HasAny(blockingTags))
			continue;

		if (!slot->IsOccupied() || !IsValid(slot->Attachment))
			continue;

		const TScriptInterface<IFVAdvancedEquipmentItemInterface> equipmentItemInterface =
			FindEquipmentItemInterface(slot->Attachment);
		if (equipmentItemInterface.GetObject() && BlockingItemGuid.IsValid())
		{
			const FGuid occupantGuid = IFVAdvancedEquipmentItemInterface::Execute_GetEquippedItemId(
				equipmentItemInterface.GetObject());
			if (occupantGuid.IsValid() && occupantGuid == BlockingItemGuid)
				continue;
		}

		if (Outer->Implements<UFVAdvancedEquipmentInterface>() && equipmentItemInterface.GetObject())
		{
			if (!IFVAdvancedEquipmentInterface::Execute_UnequipItemFromSlot(Outer, slot->SlotName, true))
				return false;
			continue;
		}

		UObject* previousAttachment = slot->Attachment;
		AActor* previousAttachmentActor = ResolveAttachmentActor(previousAttachment);
		if (!IFVAdvancedAttachmentContainerInterface::Execute_TryDetach(Outer, slot->SlotName))
			return false;

		if (IsValid(previousAttachmentActor))
			previousAttachmentActor->Destroy();
	}

	return true;
}

bool UFVEquipmentStatics::SwitchEquippedItemSlot(UObject* Outer, UFVAdvancedAttachmentSlot* CurrentSlot, UFVAdvancedAttachmentSlot* TargetSlot)
{
	if (!IsValid(Outer) || !IsValid(CurrentSlot) || !IsValid(TargetSlot))
		return false;

	if (!Outer->Implements<UFVAdvancedAttachmentContainerInterface>())
		return false;

	if (CurrentSlot == TargetSlot)
		return false;

	if (!CurrentSlot->IsOccupied() || !IsValid(CurrentSlot->Attachment))
		return false;

	if (TargetSlot->IsOccupied() && !IsValid(TargetSlot->Attachment))
		return false;

	// Clear target slot if occupied
	if (TargetSlot->IsOccupied())
	{
		AActor* occupantActor = ResolveAttachmentActor(TargetSlot->Attachment);
		if (!IFVAdvancedAttachmentContainerInterface::Execute_TryDetach(Outer, TargetSlot->SlotName))
			return false;

		if (IsValid(occupantActor))
			occupantActor->Destroy();
	}

	UObject* attachmentObject = CurrentSlot->Attachment;
	if (!IFVAdvancedAttachmentContainerInterface::Execute_TryDetach(Outer, CurrentSlot->SlotName))
		return false;

	if (!IFVAdvancedAttachmentContainerInterface::Execute_TryAttach(Outer, TargetSlot->SlotName, attachmentObject))
	{
		// Rollback: re-attach to original slot
		IFVAdvancedAttachmentContainerInterface::Execute_TryAttach(Outer, CurrentSlot->SlotName, attachmentObject);
		return false;
	}

	return true;
}

bool UFVEquipmentStatics::EnsureSlotIsReadyForEquip(UObject* Outer, const UFVAdvancedAttachmentSlot* TargetSlot)
{
	if (!IsValid(Outer) || !IsValid(TargetSlot))
		return false;
	if (!Outer->Implements<UFVAdvancedAttachmentContainerInterface>())
		return false;

	if (!TargetSlot->IsOccupied())
		return true;

	if (!IsValid(TargetSlot->Attachment))
		return false;

	AActor* occupantActor = ResolveAttachmentActor(TargetSlot->Attachment);
	if (!IFVAdvancedAttachmentContainerInterface::Execute_TryDetach(Outer, TargetSlot->SlotName))
		return false;

	if (IsValid(occupantActor))
		occupantActor->Destroy();

	return true;
}

AActor* UFVEquipmentStatics::ResolveAttachmentActor(UObject* AttachmentObject)
{
	if (AActor* attachmentActor = Cast<AActor>(AttachmentObject))
		return attachmentActor;

	if (const UActorComponent* attachmentComponent = Cast<UActorComponent>(AttachmentObject))
		return attachmentComponent->GetOwner();

	return nullptr;
}

bool UFVEquipmentStatics::IsTargetClassValid(const UClass* TargetClass)
{
	if (!UFVAttachmentsStatics::IsTargetClassValid(TargetClass))
		return false;
	
	if (!IsValid(TargetClass))
		return false;

	const TSubclassOf<UInterface> equipmentItemInterfaceClass = UFVAdvancedEquipmentItemInterface::StaticClass();
	if (TargetClass->ImplementsInterface(equipmentItemInterfaceClass.Get()))
		return true;

	const AActor* targetActor = Cast<AActor>(TargetClass->GetDefaultObject());
	if (!IsValid(targetActor))
		return false;

	const TArray<UActorComponent*> components = targetActor->GetComponentsByInterface(UFVAdvancedEquipmentItemInterface::StaticClass());
	if (!components.IsEmpty())
		return true;

	const UBlueprintGeneratedClass* blueprintClass = Cast<UBlueprintGeneratedClass>(TargetClass);
	return IsValid(ResolveFirstInterfaceObjectFromBlueprintClass(blueprintClass, equipmentItemInterfaceClass));
}

bool UFVEquipmentStatics::ValidateEquipmentItemRequest(const UObject* Outer, const FFVInventoryItem& ItemDefinition, UFVAdvancedAttachmentSlot*& TargetSlot)
{
	if (!IsValid(Outer))
		return false;

	if (!IsValid(Outer->GetWorld()))
		return false;
	
	if (!Outer->Implements<UFVAdvancedAttachmentContainerInterface>())
		return false;

	if (!ItemDefinition.IsItemValid())
		return false;

	const UFVInventoryItemTemplate* itemTemplate = ItemDefinition.GetTemplate();
	if (!IsValid(itemTemplate))
		return false;

	UClass* spawnClass = itemTemplate->SpawnActor.LoadSynchronous();
	if (!IsTargetClassValid(spawnClass))
		return false;

	const AActor* defaultActor = spawnClass->GetDefaultObject<AActor>();
	if (!IsValid(defaultActor))
		return false;

	if (!IsValid(TargetSlot))
	{
		const auto equipmentInterface = FindEquipmentItemInterface(const_cast<AActor*>(defaultActor));
		const FName preferredSlotName = equipmentInterface.GetObject() ? 
			IFVAdvancedEquipmentItemInterface::Execute_GetEquipmentItemPreferredSlot(equipmentInterface.GetObject()) : 
			NAME_None;
	
		const auto preferredSlotTag = equipmentInterface.GetObject() ?
			IFVAdvancedEquipmentItemInterface::Execute_GetEquipmentPreferredSlotTag(equipmentInterface.GetObject()) :
			FGameplayTag();
	
		if (preferredSlotName == NAME_None && !preferredSlotTag.IsValid())
			return false;

		if (IFVAdvancedAttachmentContainerInterface::Execute_IsValidSlot(Outer, preferredSlotName))
		{
			TargetSlot = IFVAdvancedAttachmentContainerInterface::Execute_GetSlot(Outer, preferredSlotName);
		}
		else if (preferredSlotTag.IsValid())
		{
			const FName freeTaggedSlot = IFVAdvancedAttachmentContainerInterface::Execute_FindFirstFreeSlotWithTags(
				Outer,
				FGameplayTagContainer(preferredSlotTag));
			TargetSlot = IFVAdvancedAttachmentContainerInterface::Execute_GetSlot(Outer, freeTaggedSlot);
		}
	}

	if (!IsValid(TargetSlot) || !TargetSlot->CanAttach())
		return false;

	const UFVAdvancedEquipmentSettingsConfig* settingsConfig = GetEquipmentSettingsConfig();
	if (!ValidateSlotItemTypeCompatibility(ItemDefinition, TargetSlot, settingsConfig))
		return false;

	if (IsSlotBlockedByCurrentAttachments(Outer, TargetSlot, true))
	{
		LOG_WARNING(TEXT("[Validate Equipment Item Request]: Slot '%s' is currently blocked by active equipment tags."),
			*TargetSlot->SlotName.ToString());
		return false;
	}
	
	return true;
}

bool UFVEquipmentStatics::CreateEquipmentItemAndAttach(UObject* Outer, const FFVInventoryItem& ItemDefinition, const UFVAdvancedAttachmentSlot* TargetSlot, 
	AActor*& OutSpawnedActor)
{
	OutSpawnedActor = nullptr;

	const UFVInventoryItemTemplate* itemTemplate = ItemDefinition.GetTemplate();
	if (!IsValid(itemTemplate))
		return false;
	if (!IsValid(Outer))
		return false;
	if (!Outer->Implements<UFVAdvancedAttachmentContainerInterface>())
		return false;
	if (!IsValid(TargetSlot))
		return false;

	UWorld* world = Outer->GetWorld();
	if (!IsValid(world))
		return false;
	
	UClass* classToSpawn = itemTemplate->SpawnActor.LoadSynchronous();
	if (!IsValid(classToSpawn))
		return false;
	
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	OutSpawnedActor = world->SpawnActor<AActor>(classToSpawn, FTransform(), spawnParams);
	if (!IsValid(OutSpawnedActor))
		return false;

	if (ItemDefinition.GetGuid().IsValid())
	{
		const TScriptInterface<IFVAdvancedEquipmentItemInterface> equipmentItemInterface = FindEquipmentItemInterface(OutSpawnedActor);
		if (equipmentItemInterface.GetObject())
		{
			IFVAdvancedEquipmentItemInterface::Execute_SetEquippedItemId(
				equipmentItemInterface.GetObject(),
				ItemDefinition.GetGuid()
			);
		}
	}
	
	if (!IFVAdvancedAttachmentContainerInterface::Execute_TryAttach(Outer, TargetSlot->SlotName, OutSpawnedActor))
	{
		if (IsValid(OutSpawnedActor))
			OutSpawnedActor->Destroy();
		OutSpawnedActor = nullptr;
		return false;
	}

	// Set initial equipment state after successful attach
	{
		const TScriptInterface<IFVAdvancedEquipmentItemInterface> equipItemInterface = FindEquipmentItemInterface(OutSpawnedActor);
		if (equipItemInterface.GetObject())
		{
			const bool bAutoActivate = IFVAdvancedEquipmentItemInterface::Execute_DoesAutoActive(equipItemInterface.GetObject());
			const EEquipmentItemState initialState = bAutoActivate ? EEquipmentItemState::EES_Active : EEquipmentItemState::EES_Equipped;
			IFVAdvancedEquipmentItemInterface::Execute_SetEquipmentItemState(equipItemInterface.GetObject(), initialState);
		}
	}

	return true;
}

bool UFVEquipmentStatics::EquipItemGeneral(UObject* Outer, const FFVInventoryItem& ItemDefinition, AActor*& OutSpawnedActor)
{
	OutSpawnedActor = nullptr;

	UFVAdvancedAttachmentSlot* preferredSlot = nullptr;
	if (!ValidateEquipmentItemRequest(Outer, ItemDefinition, preferredSlot))
		return false;

	return EquipItemToResolvedSlot(Outer, ItemDefinition, preferredSlot, OutSpawnedActor);
}

bool UFVEquipmentStatics::EquipItemToSlot(UObject* Outer, const FFVInventoryItem& ItemDefinition, UFVAdvancedAttachmentSlot* TargetSlot,
	AActor*& OutSpawnedActor)
{
	if (!ValidateEquipmentItemRequest(Outer, ItemDefinition, TargetSlot))
		return false;

	return EquipItemToResolvedSlot(Outer, ItemDefinition, TargetSlot, OutSpawnedActor);
}

bool UFVEquipmentStatics::EquipItemToResolvedSlot(UObject* Outer, const FFVInventoryItem& ItemDefinition, UFVAdvancedAttachmentSlot* ResolvedTargetSlot,
	AActor*& OutSpawnedActor)
{
	OutSpawnedActor = nullptr;
	if (!IsValid(Outer) || !IsValid(ResolvedTargetSlot))
		return false;
	if (!Outer->Implements<UFVAdvancedAttachmentContainerInterface>())
		return false;

	const bool bSupportsEquipmentInterface = Outer->Implements<UFVAdvancedEquipmentInterface>();
	if (bSupportsEquipmentInterface && IFVAdvancedEquipmentInterface::Execute_IsEquipmentItemEquipped(Outer, ItemDefinition))
	{
		UFVAdvancedAttachmentSlot* currentSlot = FindSlotWithEquippedItem(Outer, ItemDefinition.GetGuid());
		if (!IsValid(currentSlot) || currentSlot == ResolvedTargetSlot)
			return false;

		return SwitchEquippedItemSlot(Outer, currentSlot, ResolvedTargetSlot);
	}

	if (!EnsureSlotIsReadyForEquip(Outer, ResolvedTargetSlot))
		return false;

	return CreateEquipmentItemAndAttach(Outer, ItemDefinition, ResolvedTargetSlot, OutSpawnedActor);
}

bool UFVEquipmentStatics::ValidateItemEquipped(const UFVEquipmentComponent* EquipmentComponent, const FFVInventoryItem& ItemDefinition, const FName SlotName)
{
	if (!IsValid(EquipmentComponent) || !ItemDefinition.IsItemValid())
		return false;

	FGuid expectedItemGuid = ItemDefinition.GetGuid();
	if (!expectedItemGuid.IsValid())
		return false;

	const auto resolveExpectedItemGuid = [&]() -> bool
	{
		const auto inventoryManager = ItemDefinition.GetOwningInventory();
		if (inventoryManager.GetObject() == nullptr || !inventoryManager.GetObject()->Implements<UFVAdvancedInventoryInterface>())
			return false;

		const auto itemData = IFVAdvancedInventoryInterface::Execute_FindItem(
			inventoryManager.GetObject(),
			FInventoryItemSearchParams(ItemDefinition.GetGuid())
		);
		if (!itemData.IsItemValid())
			return false;

		expectedItemGuid = itemData.GetGuid();
		return expectedItemGuid.IsValid();
	};

	if (!resolveExpectedItemGuid())
		return false;

	const auto hasMatchingEquippedGuid = [&](const UFVAdvancedAttachmentSlot* Slot) -> bool
	{
		if (!IsValid(Slot) || !Slot->IsOccupied() || !IsValid(Slot->Attachment))
			return false;

		const TScriptInterface<IFVAdvancedEquipmentItemInterface> equipmentItemInterface = FindEquipmentItemInterface(Slot->Attachment);
		if (!equipmentItemInterface.GetObject())
			return false;

		return IFVAdvancedEquipmentItemInterface::Execute_GetEquippedItemId(equipmentItemInterface.GetObject()) == expectedItemGuid;
	};

	if (SlotName != NAME_None)
	{
		const UFVAdvancedAttachmentSlot* foundSlot = IFVAdvancedAttachmentContainerInterface::Execute_GetSlot(EquipmentComponent, SlotName);
		return hasMatchingEquippedGuid(foundSlot);
	}

	const TArray<UFVAdvancedAttachmentSlot*> attachmentSlots = IFVAdvancedAttachmentContainerInterface::Execute_GetAttachmentSlots(EquipmentComponent);
	if (attachmentSlots.IsEmpty())
		return false;

	return Algo::FindByPredicate(attachmentSlots, hasMatchingEquippedGuid) != nullptr;
}

bool UFVEquipmentStatics::EquipItem(const TScriptInterface<IFVAdvancedEquipmentInterface>& Target, const FFVInventoryItem& ItemDefinition)
{
	if (!Target.GetObject())
		return false;
	return IFVAdvancedEquipmentInterface::Execute_EquipItem(Target.GetObject(), ItemDefinition) != nullptr;
}

bool UFVEquipmentStatics::EquipItemToSlot(const TScriptInterface<IFVAdvancedEquipmentInterface>& Target,
	const FFVInventoryItem& ItemDefinition, const FName& SlotName)
{
	if (!Target.GetObject())
		return false;
	return IFVAdvancedEquipmentInterface::Execute_EquipItemToSlot(Target.GetObject(), SlotName, ItemDefinition) != nullptr;
}

bool UFVEquipmentStatics::IsItemEquipped(const TScriptInterface<IFVAdvancedEquipmentInterface>& Target,
                                              const FFVInventoryItem& ItemDefinition)
{
	if (!Target.GetObject())
		return false;
	return IFVAdvancedEquipmentInterface::Execute_IsEquipmentItemEquipped(Target.GetObject(), ItemDefinition);
}

bool UFVEquipmentStatics::IsItemEquippedInSlot(const TScriptInterface<IFVAdvancedEquipmentInterface>& Target, 
	const FFVInventoryItem& ItemDefinition, const FName& SlotName)
{
	if (!Target.GetObject())
		return false;
	return IFVAdvancedEquipmentInterface::Execute_IsEquipmentItemEquippedInSlot(Target.GetObject(), ItemDefinition, SlotName);
}

bool UFVEquipmentStatics::UnequipItem(const TScriptInterface<IFVAdvancedEquipmentInterface>& Target, const FFVInventoryItem& ItemDefinition, bool bUseFallbackSlot)
{
	if (!Target.GetObject())
		return false;
	return IFVAdvancedEquipmentInterface::Execute_UnequipItem(Target.GetObject(), ItemDefinition, bUseFallbackSlot);
}

bool UFVEquipmentStatics::ActivateEquipmentItem(const TScriptInterface<IFVAdvancedEquipmentInterface>& Target, const FFVInventoryItem& ItemDefinition, const FName& TargetSlotId)
{
	if (!Target.GetObject())
		return false;
	return IFVAdvancedEquipmentInterface::Execute_ActivateEquipmentItem(Target.GetObject(), ItemDefinition, TargetSlotId);
}

bool UFVEquipmentStatics::ActivateQuickUseItem(const TScriptInterface<IFVAdvancedEquipmentInterface>& Target,
	const FName& SlotId, const FName& TargetSlotId)
{
	if (!Target.GetObject())
		return false;
	return IFVAdvancedEquipmentInterface::Execute_ActivateQuickUseItem(Target.GetObject(), SlotId, TargetSlotId);
}

bool UFVEquipmentStatics::DeactivateEquipmentItem(const TScriptInterface<IFVAdvancedEquipmentInterface>& Target, const FFVInventoryItem& ItemDefinition, const FName& TargetSlotId)
{
	if (!Target.GetObject())
		return false;
	return IFVAdvancedEquipmentInterface::Execute_DeactivateEquipmentItem(Target.GetObject(), ItemDefinition, TargetSlotId);
}

bool UFVEquipmentStatics::AnimAttachItem(const TScriptInterface<IFVAdvancedEquipmentInterface>& Target)
{
	if (!Target.GetObject())
		return false;
	return IFVAdvancedEquipmentInterface::Execute_AnimAttachItem(Target.GetObject());
}

bool UFVEquipmentStatics::AnimQuickItemUsed(const TScriptInterface<IFVAdvancedEquipmentInterface>& Target)
{
	if (!Target.GetObject())
		return false;
	return IFVAdvancedEquipmentInterface::Execute_AnimQuickItemUsed(Target.GetObject());
}
