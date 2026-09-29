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


#include "Components/FVAdvancedInventoryLoadoutComponent.h"

#include "Algo/ForEach.h"
#include "Algo/Transform.h"
#include "Definitions/FVAdvancedInventoryLoadoutConfig.h"
#include "Definitions/FVAdvancedInventoryLoadoutItem.h"
#include "Definitions/FVInventoryItemTemplate.h"
#include "Interfaces/Equipment/FVAdvancedEquipmentInterface.h"
#include "Interfaces/Inventory/FVAdvancedInventoryInterface.h"
#include "Logs/FVAdvancedInventoryLog.h"
#include "Statics/FVEquipmentStatics.h"
#include "Statics/FVInventoryStatics.h"

using FResolvedLoadoutItem = TPair<UFVAdvancedInventoryLoadoutItem*, FFVInventoryItem>;

static FString GetLoadoutItemDebugName(const UFVAdvancedInventoryLoadoutItem* Item)
{
	if (!IsValid(Item))
		return TEXT("<InvalidItem>");

#if WITH_EDITORONLY_DATA
	if (Item->DisplayName != NAME_None)
		return Item->DisplayName.ToString();
#endif

	return IsValid(Item->ItemTemplate) ? Item->ItemTemplate->DisplayName.ToString() : TEXT("<MissingTemplate>");
}

static void LogFailedLoadoutItems(const TArray<UFVAdvancedInventoryLoadoutItem*>& FailedItems, const TCHAR* InvalidItemMessage, const TCHAR* FailedItemMessage)
{
	Algo::ForEach(FailedItems, [InvalidItemMessage, FailedItemMessage](const UFVAdvancedInventoryLoadoutItem* Failure)
	{
		if (!IsValid(Failure))
		{
			LOG_WARNING(TEXT("%s"), InvalidItemMessage)
			return;
		}

		LOG_WARNING(TEXT("%s\n%s"), FailedItemMessage, *GetLoadoutItemDebugName(Failure))
	});
}

static void LoadItemsToInventory(const TScriptInterface<IFVAdvancedInventoryInterface>& Inventory, const TArray<UFVAdvancedInventoryLoadoutItem*>& SourceItems,
	TArray<UFVAdvancedInventoryLoadoutItem*>& OutLoadedItems, TArray<UFVAdvancedInventoryLoadoutItem*>& OutFailedItems)
{
	OutLoadedItems.Reserve(SourceItems.Num());
	OutFailedItems.Reserve(SourceItems.Num());

	Algo::ForEach(SourceItems, [&Inventory, &OutLoadedItems, &OutFailedItems](UFVAdvancedInventoryLoadoutItem* Item)
	{
		const bool bAdded = IsValid(Item) && UFVInventoryStatics::AddItemFromTemplate(Inventory, Item->ItemTemplate, Item->BaseQuantity, Item->BaseDurability);
		if (bAdded)
		{
			OutLoadedItems.Add(Item);
			return;
		}

		OutFailedItems.Add(Item);
	});
}

static bool IsLoadoutItemEquippable(const UFVAdvancedInventoryLoadoutItem* Item)
{
	if (!IsValid(Item) || !Item->bAutomaticallyEquip)
		return false;

	const UFVInventoryItemTemplate* itemTemplate = Item->ItemTemplate;
	return IsValid(itemTemplate) && itemTemplate->EquipmentItemType.IsValid() && !itemTemplate->AttachmentSlots.IsEmpty();
}

static TArray<UFVAdvancedInventoryLoadoutItem*> CollectItemsToEquip(const TArray<UFVAdvancedInventoryLoadoutItem*>& LoadedItems)
{
	TArray<UFVAdvancedInventoryLoadoutItem*> itemsToEquip;
	itemsToEquip.Reserve(LoadedItems.Num());

	Algo::TransformIf(LoadedItems, itemsToEquip,
		[](const UFVAdvancedInventoryLoadoutItem* Item) { return IsLoadoutItemEquippable(Item); },
		[](UFVAdvancedInventoryLoadoutItem* Item) { return Item; });

	return itemsToEquip;
}

static TArray<FResolvedLoadoutItem> ResolveInventoryItemsForEquip(const TScriptInterface<IFVAdvancedInventoryInterface>& Inventory,
	const TArray<UFVAdvancedInventoryLoadoutItem*>& ItemsToEquip)
{
	TArray<FResolvedLoadoutItem> inventoryItems;
	inventoryItems.Reserve(ItemsToEquip.Num());

	Algo::Transform(ItemsToEquip, inventoryItems, [&Inventory](UFVAdvancedInventoryLoadoutItem* Item)
	{
		return FResolvedLoadoutItem(
			Item,
			UFVInventoryStatics::FindItem(Inventory, FInventoryItemSearchParams(Item->ItemTemplate))
		);
	});

	return inventoryItems;
}

static void EquipResolvedInventoryItems(const TScriptInterface<IFVAdvancedEquipmentInterface>& Equipment, const TArray<FResolvedLoadoutItem>& InventoryItems,
	TArray<UFVAdvancedInventoryLoadoutItem*>& OutFailedEquipments)
{
	OutFailedEquipments.Reserve(InventoryItems.Num());

	Algo::ForEach(InventoryItems, [&Equipment, &OutFailedEquipments](const FResolvedLoadoutItem& ItemPair)
	{
		if (!ItemPair.Value.IsItemValid())
		{
			OutFailedEquipments.Add(ItemPair.Key);
			return;
		}

		bool bEquipped = false;
		if (IsValid(ItemPair.Key) && ItemPair.Key->EquipmentSlot != NAME_None)
			bEquipped = UFVEquipmentStatics::EquipItemToSlot(Equipment, ItemPair.Value, ItemPair.Key->EquipmentSlot);

		if (!bEquipped)
			bEquipped = UFVEquipmentStatics::EquipItem(Equipment, ItemPair.Value);

		if (!bEquipped)
			OutFailedEquipments.Add(ItemPair.Key);
	});
}

UFVAdvancedInventoryLoadoutComponent::UFVAdvancedInventoryLoadoutComponent() :
	bAutoLoad(true)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	UFVAdvancedInventoryLoadoutComponent::SetComponentTickEnabled(false);

	bAutoActivate = true;

	SetIsReplicatedByDefault(true);
	SetActiveFlag(true);

	ComponentTags.Append({ TEXT("FV"), TEXT("Loadout") });
}

bool UFVAdvancedInventoryLoadoutComponent::LoadLoadout_Implementation()
{
	if (!GetOwner()->HasAuthority())
	{
		Server_LoadLoadout();
		return true;
	}
	
	if (!IsValid(LoadoutConfiguration.LoadSynchronous()))
	{
		LOG_WARNING(TEXT("[LoadLoadout] Loadout configuration is invalid!"))
		return false;
	}
	if (!IsValid(RelatedInventory.GetObject()))
	{
		LOG_WARNING(TEXT("[LoadLoadout] Inventory is invalid!"))
		return false;
	}
	const TArray<UFVAdvancedInventoryLoadoutItem*> items = Execute_GetLoadoutItems(this);
	if (items.IsEmpty())
	{
		LOG_WARNING(TEXT("[LoadLoadout] No items to load!"))
		return false;
	}

	TArray<UFVAdvancedInventoryLoadoutItem*> loadedItems;
	TArray<UFVAdvancedInventoryLoadoutItem*> failedItems;

	LoadItemsToInventory(RelatedInventory, items, loadedItems, failedItems);
	LogFailedLoadoutItems(
		failedItems,
		TEXT("[LoadLoadout] Failed to load invalid/null item from Loadout!"),
		TEXT("[LoadLoadout] Failed to load item from Loadout:")
	);

	if (!IsValid(RelatedEquipment.GetObject()))
		return failedItems.IsEmpty();

	const TArray<UFVAdvancedInventoryLoadoutItem*> itemsToEquip = CollectItemsToEquip(loadedItems);
	if (itemsToEquip.IsEmpty())
		return true;

	const TArray<FResolvedLoadoutItem> inventoryItems = ResolveInventoryItemsForEquip(RelatedInventory, itemsToEquip);

	TArray<UFVAdvancedInventoryLoadoutItem*> failedEquipments;
	EquipResolvedInventoryItems(RelatedEquipment, inventoryItems, failedEquipments);
	LogFailedLoadoutItems(
		failedEquipments,
		TEXT("[LoadLoadout] Failed to equip invalid/null item from Loadout!"),
		TEXT("[LoadLoadout] Failed to equip item from Loadout:")
	);

	return true;
}

TArray<UFVAdvancedInventoryLoadoutItem*> UFVAdvancedInventoryLoadoutComponent::GetLoadoutItems_Implementation() const
{
	if (!IsValid(LoadoutConfiguration.LoadSynchronous()))
		return {};
	return LoadoutConfiguration->Items;
}

void UFVAdvancedInventoryLoadoutComponent::Server_LoadLoadout_Implementation()
{
	Execute_LoadLoadout(this);
}

void UFVAdvancedInventoryLoadoutComponent::InitializeInventoryAndEquipment()
{
	auto inventoryComponent = GetOwner()->FindComponentByInterface(UFVAdvancedInventoryInterface::StaticClass());
	if (!IsValid(inventoryComponent))
		LOG_ERROR(TEXT("[FVInventoryUIComponent] Cannot find 'Inventory' component in Parent! Loadouts will NOT work!"))
	else
	{
		RelatedInventory = inventoryComponent;
		ensureMsgf(RelatedInventory.GetObject() != nullptr, TEXT("[FVAdvancedInventoryLoadoutComponent] Failed to update 'RelatedInventory'"));
	}
		
	auto equipmentComponent = GetOwner()->FindComponentByInterface(UFVAdvancedEquipmentInterface::StaticClass());
	if (!IsValid(equipmentComponent))
		LOG_ERROR(TEXT("[FVInventoryUIComponent] Cannot find 'Equipment' component in Parent! Loadouts will NOT work!"))
	else
	{
		RelatedEquipment = equipmentComponent;
		ensureMsgf(RelatedEquipment.GetObject() != nullptr, TEXT("[FVAdvancedInventoryLoadoutComponent] Failed to update 'RelatedEquipment'"));
	}
}

void UFVAdvancedInventoryLoadoutComponent::BeginPlay()
{
	Super::BeginPlay();	
	
	InitializeInventoryAndEquipment();
	
	if (bAutoLoad && GetOwner() && GetOwner()->HasAuthority())
		Execute_LoadLoadout(this);
}

