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


#include "Statics/FVInventoryStatics.h"

#include "Decorations/FVCallbackInventoryItemAction.h"
#include "Decorations/FVInventoryItemAction.h"
#include "Decorations/FVSelectableInventoryItemAction.h"
#include "Definitions/FVAdvancedInventoryNotification.h"
#include "Definitions/FVInventoryBaseUIEnums.h"
#include "Definitions/FVInventoryItemTemplate.h"
#include "Definitions/FVEquipmentBaseDataTypes.h"
#include "Settings/FVAdvancedInventorySettings.h"
#include "Settings/FVAdvancedInventorySettingsConfig.h"
#include "Statics/FVInventorySystemStatics.h"

#define FV_BIND_INVENTORY_DELEGATE(Target, Binding, HandleGetter) \
	if (!IsValid(Target) || !(Binding).IsBound()) \
		return false; \
	IFVAdvancedInventoryInterface* nativeInterface = Cast<IFVAdvancedInventoryInterface>(Target); \
	if (!nativeInterface) \
		return false; \
	nativeInterface->HandleGetter().AddUnique(Binding); \
	return true

#define FV_UNBIND_INVENTORY_DELEGATE(Target, Binding, HandleGetter) \
	if (!IsValid(Target) || !(Binding).IsBound()) \
		return false; \
	IFVAdvancedInventoryInterface* nativeInterface = Cast<IFVAdvancedInventoryInterface>(Target); \
	if (!nativeInterface) \
		return false; \
	nativeInterface->HandleGetter().Remove(Binding); \
	return true

#pragma region ItemActions

bool UFVInventoryStatics::InitializeItemAction(UFVInventoryItemAction* Target,
	const FFVInventoryItem& TargetItem, const TScriptInterface<IFVAdvancedInventoryInterface>& OwningInventory,
	UObject* ContextPayload)
{
	return IsValid(Target) ? Target->InitializeItemAction(TargetItem, OwningInventory, ContextPayload) : false;
}

bool UFVInventoryStatics::ExecuteInventoryAction(UFVInventoryItemAction* Target, const FFVInventoryItem& TargetItem)
{
	return IsValid(Target) ? Target->ExecuteInventoryAction(TargetItem) : false;
}

FGameplayTagContainer UFVInventoryStatics::GetItemActionTags(UFVInventoryItemAction* Target)
{
	return IsValid(Target) ? Target->GetItemActionTags() : FGameplayTagContainer();
}

#pragma endregion 

#pragma region Bindings

bool UFVInventoryStatics::BindToOnItemAdded(UObject* Target, const FFVInventoryItemAddedBinding& Binding)
{
	FV_BIND_INVENTORY_DELEGATE(Target, Binding, GetOnItemAddedEventHandle);
}

bool UFVInventoryStatics::UnbindFromOnItemAdded(UObject* Target, const FFVInventoryItemAddedBinding& Binding)
{
	FV_UNBIND_INVENTORY_DELEGATE(Target, Binding, GetOnItemAddedEventHandle);
}

bool UFVInventoryStatics::BindToOnItemRemoved(UObject* Target, const FFVInventoryItemRemovedBinding& Binding)
{
	FV_BIND_INVENTORY_DELEGATE(Target, Binding, GetOnItemRemovedEventHandle);
}

bool UFVInventoryStatics::UnbindFromOnItemRemoved(UObject* Target, const FFVInventoryItemRemovedBinding& Binding)
{
	FV_UNBIND_INVENTORY_DELEGATE(Target, Binding, GetOnItemRemovedEventHandle);
}

bool UFVInventoryStatics::BindToOnItemQuantityChanged(UObject* Target, const FFVInventoryItemQuantityChangedBinding& Binding)
{
	FV_BIND_INVENTORY_DELEGATE(Target, Binding, GetOnItemQuantityChangedEventHandle);
}

bool UFVInventoryStatics::UnbindFromOnItemQuantityChanged(UObject* Target, const FFVInventoryItemQuantityChangedBinding& Binding)
{
	FV_UNBIND_INVENTORY_DELEGATE(Target, Binding, GetOnItemQuantityChangedEventHandle);
}

bool UFVInventoryStatics::BindToOnItemDurabilityChanged(UObject* Target, const FFVInventoryItemDurabilityChangedBinding& Binding)
{
	FV_BIND_INVENTORY_DELEGATE(Target, Binding, GetOnItemDurabilityChangedEventHandle);
}

bool UFVInventoryStatics::UnbindFromOnItemDurabilityChanged(UObject* Target, const FFVInventoryItemDurabilityChangedBinding& Binding)
{
	FV_UNBIND_INVENTORY_DELEGATE(Target, Binding, GetOnItemDurabilityChangedEventHandle);
}

bool UFVInventoryStatics::BindToOnNotificationProcessed(UObject* Target, const FFVInventoryNotificationProcessedBinding& Binding)
{
	FV_BIND_INVENTORY_DELEGATE(Target, Binding, GetOnNotificationProcessedEventHandle);
}

bool UFVInventoryStatics::UnbindFromOnNotificationProcessed(UObject* Target, const FFVInventoryNotificationProcessedBinding& Binding)
{
	FV_UNBIND_INVENTORY_DELEGATE(Target, Binding, GetOnNotificationProcessedEventHandle);
}

#pragma endregion

FInventoryCategoryData UFVInventoryStatics::GetInventoryCategoryData(const FString& CategoryName, const FString ParentCategory, bool& bResult)
{
	bResult = false;
	if (CategoryName.IsEmpty()) return FInventoryCategoryData();
	
	const auto inventoryConfig = GetInventorySettingsConfig();
	if (!inventoryConfig) return FInventoryCategoryData();
	
	if (ParentCategory.IsEmpty())
	{
		bResult = inventoryConfig->AllowedCategories.Contains(CategoryName);
		if (bResult)
		{
			auto inventoryCategory = *inventoryConfig->AllowedCategories.Find(CategoryName);
			return inventoryCategory.CategoryData;
		}
		return FInventoryCategoryData();
	}
	
	if (!inventoryConfig->AllowedCategories.Contains(ParentCategory)) return FInventoryCategoryData();

	const FInventoryCategory parentCategory = *inventoryConfig->AllowedCategories.Find(ParentCategory);
		
	bResult = parentCategory.SubCategories.Contains(CategoryName);
	if (bResult)
	{
		auto inventoryCategory = *parentCategory.SubCategories.Find(CategoryName);
		return inventoryCategory;
	}
	return FInventoryCategoryData();
}

UFVAdvancedInventorySettings* UFVInventoryStatics::GetInventorySettings()
{
	return GetMutableDefault<UFVAdvancedInventorySettings>();
}

UFVAdvancedInventorySettingsConfig* UFVInventoryStatics::GetInventorySettingsConfig()
{
	const auto settings = GetDefault<UFVAdvancedInventorySettings>();
	if (!settings) return nullptr;
	return settings->AdvancedInventorySettingsConfig.LoadSynchronous();
}

UPrimaryDataAsset* UFVInventoryStatics::GetTemplateConfig(const FString& Key)
{
	const auto config = GetInventorySettingsConfig();
	if (!config) return nullptr;
	const auto templateConfig = config->TemplatesConfig.Find(Key);
	return templateConfig ? templateConfig->LoadSynchronous() : nullptr;
}

FFVInventoryItem UFVInventoryStatics::NewInventoryItem(const FGuid& ItemGuid)
{
	return FFVInventoryItem(ItemGuid);
}

AActor* UFVInventoryStatics::GetOwningActor(const TScriptInterface<IFVAdvancedInventoryInterface>& Target)
{
	return Target.GetObject() ? Target->Execute_GetOwningActor(Target.GetObject()) : nullptr;
}

bool UFVInventoryStatics::AddItem(const TScriptInterface<IFVAdvancedInventoryInterface>& Target, const FFVInventoryItem& Item)
{
	return Target.GetObject() ? Target->Execute_AddItem(Target.GetObject(), Item) : false;
}

bool UFVInventoryStatics::RemoveItem(const TScriptInterface<IFVAdvancedInventoryInterface>& Target, const FGuid& ItemGuid)
{
	return Target.GetObject() ? Target->Execute_RemoveItem(Target.GetObject(), ItemGuid) : false;
}

bool UFVInventoryStatics::CanAddItem(const TScriptInterface<IFVAdvancedInventoryInterface>& Target, const FFVInventoryItem& Item)
{
	return Target.GetObject() ? Target->Execute_CanAddItem(Target.GetObject(), Item) : false;
}

bool UFVInventoryStatics::AddItemFromTemplate(const TScriptInterface<IFVAdvancedInventoryInterface>& Target, UFVInventoryItemTemplate* Template, const int32 Quantity, const float Durability)
{
	return Target.GetObject() ? Target->Execute_AddItemFromTemplate(Target.GetObject(), Template, Quantity, Durability) : false;
}

bool UFVInventoryStatics::RemoveItemFromTemplate(const TScriptInterface<IFVAdvancedInventoryInterface>& Target, UFVInventoryItemTemplate* Template, const int32 Quantity)
{
	return Target.GetObject() ? Target->Execute_RemoveItemFromTemplate(Target.GetObject(), Template, Quantity) : false;
}

bool UFVInventoryStatics::CanAddItemFromTemplate(const TScriptInterface<IFVAdvancedInventoryInterface>& Target, UFVInventoryItemTemplate* Template, const int32 Quantity)
{
	return Target.GetObject() ? Target->Execute_CanAddItemFromTemplate(Target.GetObject(), Template, Quantity) : false;
}

FFVInventoryItem UFVInventoryStatics::FindItem(const TScriptInterface<IFVAdvancedInventoryInterface>& Target, const FInventoryItemSearchParams& SearchParams)
{
	return Target.GetObject() ? Target->Execute_FindItem(Target.GetObject(), SearchParams) : FFVInventoryItem();
}

int32 UFVInventoryStatics::FindItemIndex(const TScriptInterface<IFVAdvancedInventoryInterface>& Target, const FInventoryItemSearchParams& SearchParams)
{
	return Target.GetObject() ? Target->Execute_FindItemIndex(Target.GetObject(), SearchParams) : INDEX_NONE;
}

TArray<FFVInventoryItem> UFVInventoryStatics::FindItems(const TScriptInterface<IFVAdvancedInventoryInterface>& Target, const FInventoryItemSearchParams& SearchParams)
{
	return Target.GetObject() ? Target->Execute_FindItems(Target.GetObject(), SearchParams) : TArray<FFVInventoryItem>();
}

TArray<FFVInventoryItem> UFVInventoryStatics::GetAllItems(const TScriptInterface<IFVAdvancedInventoryInterface>& Target)
{
	return Target.GetObject() ? Target->Execute_GetAllItems(Target.GetObject()) : TArray<FFVInventoryItem>();
}

void UFVInventoryStatics::ClearInventory(const TScriptInterface<IFVAdvancedInventoryInterface>& Target)
{
	if (Target.GetObject())
		Target->Execute_ClearInventory(Target.GetObject());
}

bool UFVInventoryStatics::HasItem(const TScriptInterface<IFVAdvancedInventoryInterface>& Target,
	const FInventoryItemSearchParams& SearchParams)
{
	return IsValid(Target.GetObject()) ? Target->Execute_HasItem(Target.GetObject(), SearchParams) : false;
}

bool UFVInventoryStatics::IncreaseItemQuantity(const TScriptInterface<IFVAdvancedInventoryInterface>& Target, const FGuid& ItemGuid, const int32 Amount)
{
	return Target.GetObject() ? Target->Execute_IncreaseItemQuantity(Target.GetObject(), ItemGuid, Amount) : false;
}

bool UFVInventoryStatics::DecreaseItemQuantity(const TScriptInterface<IFVAdvancedInventoryInterface>& Target, const FGuid& ItemGuid, const int32 Amount)
{
	return Target.GetObject() ? Target->Execute_DecreaseItemQuantity(Target.GetObject(), ItemGuid, Amount) : false;
}

bool UFVInventoryStatics::ModifyItemDurability(const TScriptInterface<IFVAdvancedInventoryInterface>& Target, const FGuid& ItemGuid, const float DeltaDurability)
{
	return Target.GetObject() ? Target->Execute_ModifyItemDurability(Target.GetObject(), ItemGuid, DeltaDurability) : false;
}

void UFVInventoryStatics::ProcessInventoryNotification(const TScriptInterface<IFVAdvancedInventoryInterface>& Target, const FInventoryNotificationData& Notification)
{
	if (Target.GetObject())
		Target->Execute_ProcessInventoryNotification(Target.GetObject(), Notification);
}

FString UFVInventoryStatics::InventoryToString(const TScriptInterface<IFVAdvancedInventoryInterface>& Target)
{
	if (!Target.GetObject() || !Target.GetObject()->Implements<UFVAdvancedInventoryInterface>())
		return FString();
	
	TArray<FFVInventoryItem> allItems = GetAllItems(Target);
	AActor* inventoryOwner = GetOwningActor(Target);
	
	FString returnValue = FString::Printf(TEXT("Inventory [Owner: %s, Items: %d]\n"),
		inventoryOwner ? *inventoryOwner->GetName() : TEXT("None"),
		allItems.Num()
	);
	
	for (int32 i = 0; i < allItems.Num(); ++i)
	{
		returnValue += FString::Printf(TEXT("[%d] %s\n"), i, *InventoryItemToString(allItems[i]));
	}
	
	return returnValue;
}

FString UFVInventoryStatics::InventoryItemToString(const FFVInventoryItem& Item)
{
	return Item.ToString();
}

FInventoryCategory UFVInventoryStatics::GetInventoryCategory(const FFVInventoryItem& Item)
{
	if (!Item.Template) return FInventoryCategory();
	const auto settings = GetDefault<UFVAdvancedInventorySettings>();
	if (!settings) return FInventoryCategory();
	const auto advancedInventorySettingsConfig = settings->AdvancedInventorySettingsConfig.LoadSynchronous();
	if (!advancedInventorySettingsConfig) return FInventoryCategory();
	const auto categoryKey = Item.Template->ItemCategory;
	return advancedInventorySettingsConfig->AllowedCategories.Contains(categoryKey) ? advancedInventorySettingsConfig->AllowedCategories.FindChecked(categoryKey) : FInventoryCategory();
}

FString UFVInventoryStatics::GetInventoryCategoryKey(const FFVInventoryItem& Item)
{
	if (!Item.Template) return TEXT("");
	return Item.Template->ItemCategory;
}

FInventoryRarity UFVInventoryStatics::GetInventoryRarity(const FFVInventoryItem& Item)
{
	if (!Item.Template) return FInventoryRarity();
	const auto settings = GetDefault<UFVAdvancedInventorySettings>();
	if (!settings) return FInventoryRarity();
	const auto advancedInventorySettingsConfig = settings->AdvancedInventorySettingsConfig.LoadSynchronous();
	if (!advancedInventorySettingsConfig) return FInventoryRarity();
	const auto rarityKey = Item.Template->ItemRarity;
	return advancedInventorySettingsConfig->AllowedRarities.Contains(rarityKey) ? advancedInventorySettingsConfig->AllowedRarities.FindChecked(rarityKey) : FInventoryRarity();
}

FString UFVInventoryStatics::GetInventoryRarityKey(const FFVInventoryItem& Item)
{
	if (!Item.Template) return TEXT("");
	return Item.Template->ItemCategory;
}

FText UFVInventoryStatics::GetInventoryItemName(const FFVInventoryItem& Item)
{
	return Item.GetItemName();
}

FText UFVInventoryStatics::GetInventoryItemShortInfo(const FFVInventoryItem& Item)
{
	return Item.GetItemShortInfo();
}

FText UFVInventoryStatics::GetInventoryItemLongInfo(const FFVInventoryItem& Item)
{
	return Item.GetItemLongInfo();
}

UTexture2D* UFVInventoryStatics::GetInventoryItemCover(const FFVInventoryItem& Item)
{
	return Item.GetCover();
}

bool UFVInventoryStatics::IsInventoryItemValid(const FFVInventoryItem& Item)
{
	return Item.IsItemValid();
}

TArray<UFVCallbackInventoryItemAction*> UFVInventoryStatics::GetItemActions(const FFVInventoryItem& Item)
{	
	const auto itemTemplate = Item.GetTemplate();
	if (!itemTemplate) return {};
    
	TArray<UFVCallbackInventoryItemAction*> validActions;
	const TArray<UFVSelectableInventoryItemAction*> uniqueActions = GetDisplayableItemActions(Item);

	Algo::TransformIf(
		uniqueActions,
		validActions,
		[](const UFVSelectableInventoryItemAction* A)
		{
			return A && A->GetCallbackItemAction() != nullptr;
		},
		[](const UFVSelectableInventoryItemAction* A)
		{
			return A->GetCallbackItemAction();
		}
	);

	return validActions;
}

TArray<UFVSelectableInventoryItemAction*> UFVInventoryStatics::GetDisplayableItemActions(const FFVInventoryItem& Item)
{	
	const auto itemTemplate = Item.GetTemplate();
	if (!itemTemplate) return {};
	
	const TArray<UFVSelectableInventoryItemAction*> uniqueActions = itemTemplate->ItemActions;
	TArray<UFVSelectableInventoryItemAction*> validActions = uniqueActions;
	
	validActions.RemoveAll([Item](const UFVSelectableInventoryItemAction* actionData)
	{
		return actionData && actionData->IsAllowed(Item) != true && actionData->IsActionVisible(Item) != true;
	});
	
	validActions.Sort([](const UFVSelectableInventoryItemAction& A, const UFVSelectableInventoryItemAction& B)
	{
		if (!IsValid(&A)) return false;
		if (!IsValid(&B)) return true;
		return A.GetActionPriority() > B.GetActionPriority();
	});

	return validActions;
}

FGameplayTagContainer UFVInventoryStatics::GetItemCustomData(const FFVInventoryItem& Item)
{
	if (!Item.IsItemValid()) return {};
	return Item.GetCustomData();
}

TArray<FFVInventoryItem> UFVInventoryStatics::SortInventoryItems(const TArray<FFVInventoryItem>& Items, const TArray<FInventorySortCriteria>& SortingCriteria)
{
	if (Items.Num() < 1 || SortingCriteria.Num() == 0)
		return TArray<FFVInventoryItem>();
	
	if (Items.Num() == 1)
		return Items;

	TArray<FFVInventoryItem> returnValue = Items;
	enum class ESortKey : uint8 { Name, Value, Weight, Rarity, Unknown };
	
	auto getSortKey = [](const FString& Key) -> ESortKey
	{
		if (Key.Equals(TEXT("Name"), ESearchCase::IgnoreCase)) return ESortKey::Name;
		if (Key.Equals(TEXT("Value"), ESearchCase::IgnoreCase)) return ESortKey::Value;
		if (Key.Equals(TEXT("Weight"), ESearchCase::IgnoreCase)) return ESortKey::Weight;
		if (Key.Equals(TEXT("Rarity"), ESearchCase::IgnoreCase)) return ESortKey::Rarity;
		return ESortKey::Unknown;
	};
	
	TArray<FInventorySortCriteria> sortedCriteria = SortingCriteria;
	sortedCriteria.Sort([](const FInventorySortCriteria& A, const FInventorySortCriteria& B)
	{
		return A.SortPriority > B.SortPriority;
	});
	
	for (const FInventorySortCriteria& Criteria : sortedCriteria)
	{
		const ESortKey sortKey = getSortKey(Criteria.SortingKey);
		
		Algo::StableSort(returnValue, [sortKey](const FFVInventoryItem& A, const FFVInventoryItem& B)
		{
			switch (sortKey)
			{
				case ESortKey::Name:
				return GetInventoryItemName(A).ToString().Compare(GetInventoryItemName(B).ToString()) < 0;
				
				case ESortKey::Value:
					if (!A.Template || !B.Template) return false;
					return A.Template->BasePrice * A.GetQuantity() < B.Template->BasePrice * B.GetQuantity();
					
				case ESortKey::Weight:
					if (!A.Template || !B.Template) return false;
					return A.Template->Weight * A.GetQuantity() < B.Template->Weight * B.GetQuantity();
					
				case ESortKey::Rarity:
					{
						FInventoryRarity RarityA = GetInventoryRarity(A);
						FInventoryRarity RarityB = GetInventoryRarity(B);
						return RarityA.RarityPriority < RarityB.RarityPriority;
					}
				
				default:
					return false;
			}
		});
	}
	
	return returnValue;
}

FString UFVInventoryStatics::ItemTemplate_GetItemTemplateJson(UFVInventoryItemTemplate* ItemTemplate)
{
	if (!ItemTemplate)
		return TEXT("");
	
	ItemTemplate_CalculateItemTemplateJson(ItemTemplate);
	
	return ItemTemplate->GetJson();
}

bool UFVInventoryStatics::ItemTemplate_CalculateItemTemplateJson(UFVInventoryItemTemplate* ItemTemplate)
{
	auto CreateAssetLookup = [](const FSoftObjectPath& SoftPath) -> TSharedPtr<FJsonObject>
    {
        TSharedPtr<FJsonObject> lookupObject = MakeShared<FJsonObject>();
        if (SoftPath.IsValid())
        {
            lookupObject->SetStringField(TEXT("name"), SoftPath.GetAssetName());
            lookupObject->SetStringField(TEXT("path"), SoftPath.ToString());
        }
        else
        {
            lookupObject->SetStringField(TEXT("name"), TEXT(""));
            lookupObject->SetStringField(TEXT("path"), TEXT(""));
        }
        return lookupObject;
    };
    
    TSharedPtr<FJsonObject> rootObject = MakeShared<FJsonObject>();
    
    rootObject->SetStringField(TEXT("guid"), ItemTemplate->Guid.ToString());
    rootObject->SetStringField(TEXT("displayName"), ItemTemplate->DisplayName.ToString());
    rootObject->SetStringField(TEXT("category"), ItemTemplate->ItemCategory);
    rootObject->SetStringField(TEXT("subCategory"), ItemTemplate->ItemSubCategory);
    rootObject->SetStringField(TEXT("rarity"), ItemTemplate->ItemRarity);
    rootObject->SetNumberField(TEXT("flags"), ItemTemplate->ItemFlags);
    rootObject->SetNumberField(TEXT("maxQuantity"), ItemTemplate->MaxQuantity);
    rootObject->SetNumberField(TEXT("maxStackSize"),ItemTemplate->MaxStackSize);
    
    TArray<TSharedPtr<FJsonValue>> tagsArray;
    for (const FGameplayTag& Tag : ItemTemplate->Tags)
    {
        tagsArray.Add(MakeShared<FJsonValueString>(Tag.ToString()));
    }
    rootObject->SetArrayField(TEXT("tags"), tagsArray);
    
    rootObject->SetObjectField(TEXT("spawnActor"), CreateAssetLookup(ItemTemplate->SpawnActor.ToSoftObjectPath()));
    
    TSharedPtr<FJsonObject> descriptionObject = MakeShared<FJsonObject>();
    descriptionObject->SetStringField(TEXT("short"), ItemTemplate->ItemShortInfo.ToString());
    descriptionObject->SetStringField(TEXT("long"), ItemTemplate->ItemLongInfo.ToString());
    rootObject->SetObjectField(TEXT("description"), descriptionObject);
    
    TSharedPtr<FJsonObject> visualsObject = MakeShared<FJsonObject>();
    visualsObject->SetObjectField(TEXT("thumbnail"), CreateAssetLookup(ItemTemplate->ItemThumbnail.ToSoftObjectPath()));
    visualsObject->SetObjectField(TEXT("cover"), CreateAssetLookup(ItemTemplate->ItemCover.ToSoftObjectPath()));
    visualsObject->SetObjectField(TEXT("mesh"), CreateAssetLookup(ItemTemplate->ItemMesh ? FSoftObjectPath(ItemTemplate->ItemMesh->GetPathName()) : FSoftObjectPath()));
    rootObject->SetObjectField(TEXT("visuals"), visualsObject);
    
    TSharedPtr<FJsonObject> durabilityObject = MakeShared<FJsonObject>();
    durabilityObject->SetBoolField(TEXT("enabled"), ItemTemplate->bHasDurability);
    durabilityObject->SetNumberField(TEXT("max"), ItemTemplate->MaxDurability);
    durabilityObject->SetNumberField(TEXT("base"), ItemTemplate->BaseDurability);
    durabilityObject->SetNumberField(TEXT("penalization"), ItemTemplate->DurabilityPenalization);
    durabilityObject->SetNumberField(TEXT("priceCoefficient"), ItemTemplate->DurabilityToPriceCoefficient);
    rootObject->SetObjectField(TEXT("durability"), durabilityObject);
    
    TSharedPtr<FJsonObject> economyObject = MakeShared<FJsonObject>();
    economyObject->SetBoolField(TEXT("enabled"), ItemTemplate->bHasPrice);
    economyObject->SetNumberField(TEXT("basePrice"), ItemTemplate->BasePrice);
    economyObject->SetNumberField(TEXT("sellCoefficient"), ItemTemplate->SellPriceCoefficient);
    rootObject->SetObjectField(TEXT("economy"), economyObject);
    
    TSharedPtr<FJsonObject> weightObject = MakeShared<FJsonObject>();
    weightObject->SetBoolField(TEXT("enabled"), ItemTemplate->bHasWeight);
    weightObject->SetNumberField(TEXT("value"), ItemTemplate->Weight);
    rootObject->SetObjectField(TEXT("weight"), weightObject);
    
    TArray<TSharedPtr<FJsonValue>> attachmentSlotsArray;
    for (const FGameplayTag& Slot : ItemTemplate->AttachmentSlots)
    {
        attachmentSlotsArray.Add(MakeShared<FJsonValueString>(Slot.ToString()));
    }
    rootObject->SetArrayField(TEXT("attachmentSlots"), attachmentSlotsArray);
    
    TArray<TSharedPtr<FJsonValue>> specialAffectsArray;
    for (const TSoftClassPtr<UObject>& Affect : ItemTemplate->ItemSpecialAffects)
    {
        specialAffectsArray.Add(MakeShared<FJsonValueObject>(CreateAssetLookup(Affect.ToSoftObjectPath())));
    }
    rootObject->SetArrayField(TEXT("specialAffects"), specialAffectsArray);
    
    FString outputString;
    TSharedRef<TJsonWriter<>> jsonWriter = TJsonWriterFactory<>::Create(&outputString);
    if (FJsonSerializer::Serialize(rootObject.ToSharedRef(), jsonWriter))
    {
    	ItemTemplate->SetJson(outputString);
        return true;
    }
    
    return false;
}

void UFVInventoryStatics::CleanupInventoryAction(UFVInventoryItemAction* Target)
{
	if (IsValid(Target))
		Target->CleanupInventoryAction();
}

FInventoryNotificationData UFVInventoryStatics::CreateNotificationData(
	const FString& Type,
	const TScriptInterface<IFVAdvancedInventoryInterface>& SourceInventory,
	const FGuid& ItemGuid,
	const int32 QuantityDelta
)
{
	const UFVAdvancedInventorySettingsConfig* Config = GetDefault<UFVAdvancedInventorySettings>()->AdvancedInventorySettingsConfig.LoadSynchronous();
	if (!Config) return FInventoryNotificationData();

	const FInventoryNotificationConfig* NotifConfig = Config->NotificationConfigs.Find(Type);
	if (!NotifConfig) return FInventoryNotificationData();

	auto inventoryItem = SourceInventory->Execute_FindItem(SourceInventory.GetObject(), FInventoryItemSearchParams(ItemGuid));
	/* if (!inventoryItem.IsItemValid()) return FInventoryNotificationData();*/ // TODO: How to process failed Item?

	FText notificationText = NotifConfig->MessageTemplate;
	notificationText = UFVInventorySystemStatics::ReplaceRegexInText(TEXT("${quantity}"), FText::AsNumber(FMath::Abs(QuantityDelta)), notificationText);
	notificationText = UFVInventorySystemStatics::ReplaceRegexInText(TEXT("${itemName}"), inventoryItem.GetItemName(), notificationText);

	
	return FInventoryNotificationData(
		Type,
		NotifConfig->NotificationCategory,
		NotifConfig->MessageTitle,
		notificationText,
		ItemGuid,
		SourceInventory,
		FMath::Abs(QuantityDelta),
		NotifConfig->DefaultDuration,
		*NotifConfig,
		nullptr
	);
}
