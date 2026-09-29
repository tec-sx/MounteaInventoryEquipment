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


#include "Definitions/FVInventoryItem.h"

#include "Definitions/FVInventoryBaseEnums.h"
#include "Definitions/FVInventoryItemTemplate.h"
#include "Interfaces/Inventory/FVAdvancedInventoryInterface.h"
#include "Statics/FVInventoryStatics.h"

FInventoryItemSnapshot::FInventoryItemSnapshot(const struct FFVInventoryItem& Item)
	: Quantity(Item.GetQuantity())
	, Durability(Item.GetDurability())
	, CustomData(Item.GetCustomData())
{}

bool FInventoryItemSnapshot::HasQuantityChanged(const FFVInventoryItem& Current) const
{
	return Quantity != Current.GetQuantity();
}

int32 FInventoryItemSnapshot::GetQuantityDelta(const FFVInventoryItem& Current) const
{
	return Current.GetQuantity() - Quantity;
}

bool FInventoryItemSnapshot::HasDurabilityChanged(const FFVInventoryItem& Current) const
{
	return !FMath::IsNearlyEqual(Durability, Current.GetDurability());
}

float FInventoryItemSnapshot::GetDurabilityDelta(const FFVInventoryItem& Current) const
{
	return Current.GetDurability() - Durability;
}

bool FInventoryItemSnapshot::HasCustomDataChanged(const FFVInventoryItem& Current) const
{
	return CustomData != Current.GetCustomData();
}

FFVInventoryItem::FFVInventoryItem() : Guid(FGuid()),  Template(nullptr), OwningInventory(nullptr), Quantity(-1), Durability(-1.f)
{
}

FFVInventoryItem::FFVInventoryItem(UFVInventoryItemTemplate* InTemplate, const int32 InQuantity, const float InDurability, TScriptInterface<IFVAdvancedInventoryInterface> InOwningInventory)
	: Guid(FGuid())
	, Template(nullptr)
	, OwningInventory(InOwningInventory)
{
	if (!IsValid(InTemplate))
		return;
			
	Template = InTemplate;
	Guid = FGuid::NewGuid();
		
	Template = InTemplate;
	Quantity = FMath::Clamp(InQuantity, 0, Template->MaxQuantity);
			
	if (Template->bHasDurability)
		Durability = FMath::Clamp(InDurability, 0.f, Template->MaxDurability);
}

bool FFVInventoryItem::IsItemValid() const
{
	return Guid.IsValid() && IsValid(Template);
}

bool FFVInventoryItem::IsItemInInventory() const
{
	return OwningInventory != nullptr;
}

FString FFVInventoryItem::ToString() const
{
	//FString Builder;
	TStringBuilder<512> Builder;

	if (!IsItemValid())
		return TEXT("Invalid Item!");
	
	Builder.Appendf(TEXT("Item [%s] (GUID: %s)\n"), 
		*GetTemplate()->DisplayName.ToString(), 
		*GetGuid().ToString());
	
	Builder.Appendf(TEXT("Quantity: %d/%d\n"), 
		GetQuantity(), 
		GetTemplate()->MaxQuantity);
	
	if (static_cast<uint8>(GetTemplate()->ItemFlags) & static_cast<uint8>(EInventoryItemFlags::EIIF_Durable))
	{
		Builder.Appendf(TEXT("Durability: %.1f/%.1f\n"), 
			GetDurability(), 
			GetTemplate()->MaxDurability);
	}
	
	if (!CustomData.IsEmpty())
	{
		Builder.Append(TEXT("Custom Data: "));
		Builder.Append(CustomData.ToString());
		Builder.Append(TEXT("\n"));
	}
	
	if (AffectorSlots.Num() > 0)
	{
		Builder.Append(TEXT("Affectors:\n"));
		for (const auto& Pair : AffectorSlots)
		{
			Builder.Appendf(TEXT("  - %s: %s\n"), 
				*Pair.Key.ToString(), 
				*Pair.Value.ToString());
		}
	}
	
	return Builder.ToString();
}

FText FFVInventoryItem::GetItemName() const
{
	return IsValid(Template) ? Template->DisplayName : NSLOCTEXT("InventoryItem", "InvalidName", "Invalid");
}

FText FFVInventoryItem::GetItemShortInfo() const
{
	return IsValid(Template) ? Template->ItemShortInfo : NSLOCTEXT("InventoryItem", "InvalidShortInfo", "Invalid");
}

FText FFVInventoryItem::GetItemLongInfo() const
{
	return IsValid(Template) ? Template->ItemLongInfo : NSLOCTEXT("InventoryItem", "ItemLongInfo", "Invalid");
}

UTexture2D* FFVInventoryItem::GetCover() const
{
	return IsValid(Template) ? Template->ItemCover.LoadSynchronous() : nullptr;
}

bool FFVInventoryItem::SetTemplate(UFVInventoryItemTemplate* InTemplate)
{
	if (InTemplate != Template && IsValid(InTemplate))
	{
		Template = InTemplate;
		return true;
	}
	return false;
}

bool FFVInventoryItem::SetQuantity(const int32 InQuantity)
{
	if (InQuantity == Quantity || !IsValid(Template))
		return false;

	if (InQuantity >= 0 && InQuantity <= Template->MaxQuantity)
	{
		Quantity = InQuantity;
		return true;
	}
	
	return false;
}

bool FFVInventoryItem::SetDurability(const float InDurability)
{
	if (InDurability == Durability || !IsValid(Template))
		return false;

	if (!Template->bHasDurability)
		return false;

	if ((FMath::IsNearlyZero(InDurability) || InDurability > 0.0f) && InDurability <= Template->MaxDurability)
	{
		Durability = InDurability;
		return true;
	}
	
	return false;
}

bool FFVInventoryItem::SetAffectorSlots(const TMap<FGameplayTag, FGuid>& InAffectorSlots)
{
	if (InAffectorSlots.Num() != AffectorSlots.Num())
	{
		for (const auto& Pair : InAffectorSlots)
		{
			if (!Pair.Value.IsValid())
				return false;
		}
		
		AffectorSlots = InAffectorSlots;
		return true;
	}
	
	bool bHasDifference = false;
	
	for (const auto& Pair : InAffectorSlots)
	{
		const FGuid* ExistingGuid = AffectorSlots.Find(Pair.Key);
		
		if (!ExistingGuid || *ExistingGuid != Pair.Value)
		{
			if (!Pair.Value.IsValid())
				return false;
			
			bHasDifference = true;
			break;
		}
	}

	if (bHasDifference)
	{
		AffectorSlots = InAffectorSlots;
		return true;
	}

	return false;
}

bool FFVInventoryItem::SetOwningInventory(const TScriptInterface<IFVAdvancedInventoryInterface>& InOwningInventory)
{
	if (InOwningInventory == OwningInventory)
		return false;

	OwningInventory = InOwningInventory;
	return true;
}

bool FFVInventoryItem::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	Ar << Guid;
	Ar << Template;
	Ar << Quantity;
	Ar << Durability;
	CustomData.NetSerialize(Ar, Map, bOutSuccess);

	// Handle TScriptInterface serialization
	UObject* OwningInventoryObj = nullptr;
	if (Ar.IsSaving())
		OwningInventoryObj = OwningInventory.GetObject();
	Ar << OwningInventoryObj;
	if (Ar.IsLoading())
		OwningInventory = TScriptInterface<IFVAdvancedInventoryInterface>(OwningInventoryObj);

	if (Ar.IsLoading())
		AffectorSlots.Empty();
		
	int32 NumPairs = AffectorSlots.Num();
	Ar << NumPairs;

	if (Ar.IsLoading())
	{
		for (int32 i = 0; i < NumPairs; ++i)
		{
			FGameplayTag Key;
			FGuid Value;
			Ar << Key;
			Ar << Value;
			AffectorSlots.Add(Key, Value);
		}
	}
	else
	{
		for (const auto& Pair : AffectorSlots)
		{
			FGameplayTag Key = Pair.Key;
			FGuid Value = Pair.Value;
			Ar << Key;
			Ar << Value;
		}
	}
	
	bOutSuccess = true;
	return true;
}

void FFVInventoryItem::PostReplicatedAdd(const struct FInventoryItemArray& InArraySerializer)
{
	if (IsValid(OwningInventory.GetObject()))
	{
		CapturePreReplicationSnapshot();
		
		OwningInventory->Execute_ProcessInventoryNotification
		(
			OwningInventory.GetObject(),
			UFVInventoryStatics::CreateNotificationData(
				FVInventoryNotificationBaseTypes::ItemAdded,
				OwningInventory,
				Guid,
				Quantity
			)
		);
	}
}

void FFVInventoryItem::PostReplicatedChange(const FInventoryItemArray& InArraySerializer)
{
	if (!IsValid(OwningInventory.GetObject()))
	{
		return;
	}

	if (!IsValid(Template))
	{
		LOG_ERROR(TEXT("[Inventory Item Client Update] Invalid Template! Aborting!"))
		return;
	}

	// Notify if quantity changed
	if (PreReplicationSnapshot.HasQuantityChanged(*this))
	{
		const int32 QuantityDelta = PreReplicationSnapshot.GetQuantityDelta(*this);
		const FString NotificationType = QuantityDelta > 0 
			? FVInventoryNotificationBaseTypes::ItemAdded 
			: FVInventoryNotificationBaseTypes::ItemRemoved;

		OwningInventory->Execute_ProcessInventoryNotification(
			OwningInventory.GetObject(),
			UFVInventoryStatics::CreateNotificationData(
				NotificationType,
				OwningInventory,
				Guid,
				QuantityDelta > 0 ? QuantityDelta : -QuantityDelta
			)
		);
	}

	if (Template->bHasDurability)
	{
		// Notify if durability changed
		if (PreReplicationSnapshot.HasDurabilityChanged(*this))
		{
			const float DurabilityDelta = PreReplicationSnapshot.GetDurabilityDelta(*this);
			if (DurabilityDelta > 0)
			{
				// If the increased durability reaches the maximum, notify accordingly.
				if (FMath::IsNearlyEqual(GetDurability(), GetTemplate()->MaxDurability))
				{
					OwningInventory->Execute_ProcessInventoryNotification(
						OwningInventory.GetObject(),
						UFVInventoryStatics::CreateNotificationData(
							FVInventoryNotificationBaseTypes::DurabilityMax,
							OwningInventory,
							Guid,
							DurabilityDelta
						)
					);
				}
				else
				{
					OwningInventory->Execute_ProcessInventoryNotification(
						OwningInventory.GetObject(),
						UFVInventoryStatics::CreateNotificationData(
							FVInventoryNotificationBaseTypes::DurabilityIncreased,
							OwningInventory,
							Guid,
							-DurabilityDelta
						)
					);
				}
			}
			else if (DurabilityDelta < 0)
			{
				// If the decreased durability drops to zero, notify accordingly.
				if (FMath::IsNearlyZero(GetDurability()))
				{
					OwningInventory->Execute_ProcessInventoryNotification(
						OwningInventory.GetObject(),
						UFVInventoryStatics::CreateNotificationData(
							FVInventoryNotificationBaseTypes::DurabilityZero,
							OwningInventory,
							Guid,
							0
						)
					);
				}
				else
				{
					OwningInventory->Execute_ProcessInventoryNotification(
						OwningInventory.GetObject(),
						UFVInventoryStatics::CreateNotificationData(
							FVInventoryNotificationBaseTypes::DurabilityDecreased,
							OwningInventory,
							Guid,
							-DurabilityDelta
						)
					);
				}
			}
		}
	}
	
	// Update snapshot for next replication cycle
	CapturePreReplicationSnapshot();
}

void FFVInventoryItem::PreReplicatedRemove(const struct FInventoryItemArray& InArraySerializer)
{
	if (IsValid(OwningInventory.GetObject()))
	{
		OwningInventory->Execute_ProcessInventoryNotification
		(
			OwningInventory.GetObject(),
			UFVInventoryStatics::CreateNotificationData(
				FVInventoryNotificationBaseTypes::ItemRemoved,
				OwningInventory,
				Guid,
				-Quantity
			)
		);
	}
}
