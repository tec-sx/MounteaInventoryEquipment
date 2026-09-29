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


#include "Definitions/FVInventoryItemTemplate.h"

#include "Components/ActorComponent.h"
#include "Decorations/FVSelectableInventoryItemAction.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Attachments/FVAdvancedAttachmentAttachableInterface.h"
#include "Logs/FVAdvancedInventoryLog.h"
#include "Settings/FVAdvancedInventorySettings.h"
#include "Settings/FVAdvancedInventorySettingsConfig.h"
#include "Statics/FVInventoryStatics.h"

UFVInventoryItemTemplate::UFVInventoryItemTemplate():
	Guid(FGuid::NewGuid()),
	ItemCategory(TEXT("Miscellaneous")),
	ItemRarity(TEXT("Common")),
	MaxQuantity(1),
	MaxStackSize(99),
	ItemFlags(0),
	SpawnActor(nullptr),
	ItemMesh(nullptr),
	bHasDurability(false),
	MaxDurability(0),
	BaseDurability(0),
	DurabilityPenalization(0),
	DurabilityToPriceCoefficient(0),
	bHasPrice(false),
	BasePrice(0),
	SellPriceCoefficient(1.f),
	bHasWeight(false),
	Weight(0)
{
}

FString UFVInventoryItemTemplate::GetJson() const
{
	return JsonManifest;
}

void UFVInventoryItemTemplate::SetJson(const FString& Json)
{
	if (Json.Equals(JsonManifest))
		return;
	
	JsonManifest = Json;
}

bool UFVInventoryItemTemplate::CalculateJson()
{
    return UFVInventoryStatics::ItemTemplate_CalculateItemTemplateJson(this);
}

TArray<FString> UFVInventoryItemTemplate::GetAllowedCategories()
{
	auto inventorySettings = GetMutableDefault<UFVAdvancedInventorySettings>();
	TArray<FString> returnValues;
	if (!IsValid(inventorySettings))
	{		
		returnValues.Add(TEXT("Miscellaneous"));
		return returnValues;
	}
	inventorySettings->GetAllowedCategories().GetKeys(returnValues);
	return returnValues;
}

TArray<FString> UFVInventoryItemTemplate::GetAllowedSubCategories() const
{
	auto inventorySettings = GetMutableDefault<UFVAdvancedInventorySettings>();
	if (!IsValid(inventorySettings))
		return TArray<FString>();
	
	auto advancedInventorySettingsConfig = inventorySettings->AdvancedInventorySettingsConfig.LoadSynchronous();
	if (!IsValid(advancedInventorySettingsConfig))
		return TArray<FString>();

	if (advancedInventorySettingsConfig->AllowedCategories.Num() <= 0)
		return TArray<FString>();

	TArray<FString> returnValues;
	if (const auto categoryConfiguration = advancedInventorySettingsConfig->AllowedCategories.Find(ItemCategory))
		categoryConfiguration->SubCategories.GetKeys(returnValues);
	
	return returnValues;
}

TArray<FString> UFVInventoryItemTemplate::GetAllowedRarities()
{	
	auto inventorySettings = GetMutableDefault<UFVAdvancedInventorySettings>();
	TArray<FString> returnValues;
	if (!IsValid(inventorySettings))
	{
		returnValues.Add(TEXT("Common"));
		return returnValues;
	}
	inventorySettings->GetAllowedRarities().GetKeys(returnValues);
	return returnValues;
}

#if WITH_EDITOR

namespace
{
	bool DoesClassProvideAttachableInterface(const UClass* TargetClass)
	{
		if (!IsValid(TargetClass))
			return false;

		if (TargetClass->ImplementsInterface(UFVAdvancedAttachmentAttachableInterface::StaticClass()))
			return true;

		const AActor* targetActor = Cast<AActor>(TargetClass->GetDefaultObject());
		if (!IsValid(targetActor))
			return false;

		TArray<UActorComponent*> components = targetActor->GetComponentsByInterface(UFVAdvancedAttachmentAttachableInterface::StaticClass());
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
			if (IsValid(componentTemplate) && componentTemplate->GetClass()->ImplementsInterface(UFVAdvancedAttachmentAttachableInterface::StaticClass()))
				return true;
		}

		return false;
	}
}

void UFVInventoryItemTemplate::ReloadItemActions()
{
	const auto inventorySettings = GetMutableDefault<UFVAdvancedInventorySettings>();
	if (!inventorySettings)
		return;

	if (const auto advancedInventorySettingsConfig = inventorySettings->AdvancedInventorySettingsConfig.LoadSynchronous())
	{
		auto categoryConfiguration = advancedInventorySettingsConfig->AllowedCategories.Find(ItemCategory);
		if (categoryConfiguration && categoryConfiguration->CategoryData.AllowedActions.Num() > 0)
		{
			const auto categoryActionClasses = categoryConfiguration->CategoryData.AllowedActions;
			const auto currentItemActions = ItemActions;
			for (const auto& currentItemAction : currentItemActions)
			{
				if (currentItemAction)
					currentItemAction->MarkAsGarbage();
			}
			ItemActions.Empty();
			
			for (const auto& categoryActionClass : categoryActionClasses)
			{
				UFVSelectableInventoryItemAction* newAction = NewObject<UFVSelectableInventoryItemAction>(this, categoryActionClass.LoadSynchronous());
				ItemActions.Add(newAction);
			}
		}
	}
	
	MarkPackageDirty();
	TemplateChangedDelegate.Broadcast(this);
}

void UFVInventoryItemTemplate::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	auto inventorySettings = GetMutableDefault<UFVAdvancedInventorySettings>();
	if (!inventorySettings)
		return;
	auto advancedInventorySettingsConfig = inventorySettings->AdvancedInventorySettingsConfig.LoadSynchronous();
	
	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFVInventoryItemTemplate, ItemCategory))
	{
		if (advancedInventorySettingsConfig)
		{
			auto categoryConfiguration = advancedInventorySettingsConfig->AllowedCategories.Find(ItemCategory);
			ItemFlags = categoryConfiguration->CategoryData.CategoryFlags;
			
			ReloadItemActions();
		}

		ItemSubCategory = TEXT("");
	}
	
	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFVInventoryItemTemplate, SpawnActor))
	{
		if (!SpawnActor.IsNull())
		{
			UClass* spawnClass = SpawnActor.LoadSynchronous();
			if (!IsValid(spawnClass))
				SpawnActor = nullptr;
			else if (!DoesClassProvideAttachableInterface(spawnClass))
			{
				LOG_WARNING(TEXT("SpawnActor '%s' does not implement IFVAdvancedAttachmentAttachableInterface and has no component that does. Clearing selection."),
					*spawnClass->GetName());
				SpawnActor = nullptr;
			}
		}
	}
	
	CalculateJson();
}

#endif
