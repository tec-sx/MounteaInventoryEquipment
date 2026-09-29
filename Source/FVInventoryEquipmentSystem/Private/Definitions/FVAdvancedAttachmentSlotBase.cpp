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


#include "Definitions/FVAdvancedAttachmentSlotBase.h"

#include "Definitions/FVEquipmentBaseEnums.h"
#include "Interfaces/Attachments/FVAdvancedAttachmentAttachableInterface.h"
#include "Interfaces/Attachments/FVAdvancedAttachmentContainerInterface.h"
#include "Logs/FVAdvancedInventoryLog.h"
#include "Net/UnrealNetwork.h"
#include "Settings/FVAdvancedEquipmentSettingsConfig.h"
#include "Settings/FVAdvancedInventorySettings.h"
#include "Statics/FVEquipmentStatics.h"

UFVAdvancedAttachmentSlotBase::UFVAdvancedAttachmentSlotBase() : 
	SlotName(NAME_None),
	State(EAttachmentSlotState::EASS_Empty),
	SlotType(EAttachmentSlotType::EAST_Socket)
{
}

void UFVAdvancedAttachmentSlotBase::BeginPlay_Implementation()
{
	SlotBeginPlay.Broadcast();
}

AActor* UFVAdvancedAttachmentSlotBase::GetOwningActor() const
{
	if (!IsValid(ParentContainer.GetObject()))
		return nullptr;

	const auto ownerObject = ParentContainer.GetObject();
	return ParentContainer->Execute_GetOwningActor(ownerObject);
}

void UFVAdvancedAttachmentSlotBase::InitializeAttachmentSlot(const TScriptInterface<IFVAdvancedAttachmentContainerInterface>& Parent)
{
	ParentContainer = Parent;

	if (SlotName.IsNone())
		return;

	UFVAdvancedEquipmentSettingsConfig* equipmentConfig = nullptr;
	if (const auto advancedSettings = GetDefault<UFVAdvancedInventorySettings>())
		equipmentConfig = advancedSettings->AdvancedEquipmentSettingsConfig.LoadSynchronous();

	if (!IsValid(equipmentConfig))
		return;

	if (const FFVEquipmentSlotHeaderData* headerData = equipmentConfig->AllowedEquipmentSlots.Find(SlotName))
	{
		SlotTags = headerData->TagContainer;
		DisplayName = headerData->DisplayName;
		AllowedItemTypes = headerData->AllowedItemTypes;
	}
}

UWorld* UFVAdvancedAttachmentSlotBase::GetWorld() const
{
	if (GetOuter() == nullptr)
		return nullptr;
		
	if (Cast<UPackage>(GetOuter()) != nullptr)
		return Cast<UWorld>(GetOuter()->GetOuter());

	if (!IsValid(GetOwningActor()))
		return nullptr;
		
	return GetOwningActor()->GetWorld();
}

void UFVAdvancedAttachmentSlotBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UFVAdvancedAttachmentSlotBase, Attachment);
	DOREPLIFETIME(UFVAdvancedAttachmentSlotBase, State);

	if (UBlueprintGeneratedClass* blueprintClass = Cast<UBlueprintGeneratedClass>(GetClass()))
		blueprintClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
}
int32 UFVAdvancedAttachmentSlotBase::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	if (HasAnyFlags(RF_ClassDefaultObject) || !IsSupportedForNetworking())
		return GEngine->GetGlobalFunctionCallspace(Function, this, Stack);
	
	return GetOuter()->GetFunctionCallspace(Function, Stack);
}

bool UFVAdvancedAttachmentSlotBase::CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack)
{
	AActor* owningActor = GetOwningActor();
	if (UNetDriver* netDriver = owningActor->GetNetDriver())
	{
		netDriver->ProcessRemoteFunction(owningActor, Function, Parms, OutParms, Stack, this);
		return true;
	}
	return false;
}

bool UFVAdvancedAttachmentSlotBase::Attach(UObject* NewAttachment)
{
	return (CanAttach() && IsValid(NewAttachment) && CanAttachAttachable(NewAttachment));
}

bool UFVAdvancedAttachmentSlotBase::ForceAttach(UObject* NewAttachment)
{
	LastAttachment = Attachment;
	Attachment = NewAttachment;	
	State = EAttachmentSlotState::EASS_Occupied;
	return true;
}

bool UFVAdvancedAttachmentSlotBase::Detach()
{
	if (!IsOccupied())
		return false;
	
	Attachment = nullptr;
	State = EAttachmentSlotState::EASS_Empty;
	return true;
}

bool UFVAdvancedAttachmentSlotBase::ForceDetach()
{
	Attachment = nullptr;
	State = EAttachmentSlotState::EASS_Empty;
	return true;
}

void UFVAdvancedAttachmentSlotBase::DisableSlot()
{
	if (!IsEmpty())
		Detach();
	State = EAttachmentSlotState::EASS_Locked;
}

bool UFVAdvancedAttachmentSlotBase::CanAttachAttachable(const UObject* NewAttachment) const
{
	if (!IsValid(NewAttachment))
	{
		LOG_ERROR(TEXT("Attachment is not valid!"));
		return false;
	}
	
	if (!NewAttachment->Implements<UFVAdvancedAttachmentAttachableInterface>())
	{
		LOG_WARNING(TEXT("Attachable %s does not implement the required interface! Attachment will be performed, however, it may not behave as expected."), *NewAttachment->GetName())
		return true;
	}

	if (!IFVAdvancedAttachmentAttachableInterface::Execute_CanAttach(NewAttachment))
	{
		LOG_WARNING(TEXT("Attachable object is not compatible with selected Slot!."));
		return false;
	}
	
	if (!MatchesTags(IFVAdvancedAttachmentAttachableInterface::Execute_GetTags(NewAttachment), false))
	{
		LOG_WARNING(TEXT("Attachable object does not match selected Slot tags!"));
		return false;
	}

	if (ParentContainer.GetObject() &&
		UFVEquipmentStatics::IsSlotBlockedByCurrentAttachments(ParentContainer.GetObject(), this, true))
	{
		LOG_WARNING(TEXT("Slot '%s' is currently blocked by active equipment tags."), *SlotName.ToString());
		return false;
	}
	
	return true;
}

bool UFVAdvancedAttachmentSlotBase::IsEmpty() const
{
	return State == EAttachmentSlotState::EASS_Empty && Attachment == nullptr;
}

bool UFVAdvancedAttachmentSlotBase::IsOccupied() const
{
	return State == EAttachmentSlotState::EASS_Occupied && Attachment != nullptr;
}

bool UFVAdvancedAttachmentSlotBase::IsLocked() const
{
	return State == EAttachmentSlotState::EASS_Locked;
}

void UFVAdvancedAttachmentSlotBase::OnRep_State()
{
	UObject* oldAttachment = LastAttachment;
    
	switch (State)
	{
		case EAttachmentSlotState::EASS_Occupied:
			LastAttachment = Attachment;
			break;
		case EAttachmentSlotState::EASS_Empty:
			break;
	}
    
	if (ParentContainer.GetObject())
		ParentContainer->GetOnAttachmentChangedEventHandle().Broadcast(SlotName, Attachment, oldAttachment);
}

TArray<FName> UFVAdvancedAttachmentSlotBase::GetAvailableSlotNames() const
{
	const auto advanedSettings = GetDefault<UFVAdvancedInventorySettings>();
	if (!advanedSettings)
		return {};

	const auto equipmentConfig = advanedSettings->AdvancedEquipmentSettingsConfig.LoadSynchronous();
	if (!IsValid(equipmentConfig))
		return {};

	const auto& allowedSlots = equipmentConfig->AllowedEquipmentSlots;
	TArray<FName> slotNames;
	slotNames.Reserve(allowedSlots.Num());

	Algo::Transform(allowedSlots, slotNames, [](const TPair<FName, FFVEquipmentSlotHeaderData>& Pair)
	{
		return Pair.Key;
	});

	return slotNames;
}

#if WITH_EDITOR
void UFVAdvancedAttachmentSlotBase::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);

	UFVAdvancedEquipmentSettingsConfig* equipmentConfig = nullptr;

	if (const auto advanedSettings = GetDefault<UFVAdvancedInventorySettings>())
		equipmentConfig = advanedSettings->AdvancedEquipmentSettingsConfig.LoadSynchronous();
	
	
	const FName propertName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (propertName == GET_MEMBER_NAME_CHECKED(UFVAdvancedAttachmentSlotBase, SlotName))
	{
		if (IsValid(equipmentConfig))
		{
			if (const FFVEquipmentSlotHeaderData* headerData = equipmentConfig->AllowedEquipmentSlots.Find(SlotName))
			{
				SlotTags.Reset();
				SlotTags.AppendTags(headerData->TagContainer);

				DisplayName = headerData->DisplayName;
				AllowedItemTypes.Reset();
				AllowedItemTypes.AppendTags(headerData->AllowedItemTypes);
			}
		}
	}
}
#endif
