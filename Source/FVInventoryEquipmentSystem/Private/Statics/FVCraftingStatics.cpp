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


#include "Statics/FVCraftingStatics.h"

#include "Algo/Copy.h"
#include "Algo/ForEach.h"
#include "Algo/Transform.h"
#include "Definitions/FVCraftingBaseDataTypes.h"
#include "Definitions/FVCraftingBaseEnums.h"
#include "Definitions/FVInventoryBaseDataTypes.h"
#include "Definitions/FVInventoryItemTemplate.h"
#include "Definitions/FVInventoryItemTemplate_Recipe.h"
#include "Definitions/FVRecipeIngredient.h"
#include "Definitions/FVRecipeIngredientsList.h"
#include "Definitions/FVRecipeTemplate.h"
#include "Interfaces/Crafting/FVAdvancedCraftingParticipantInterface.h"
#include "Interfaces/Crafting/FVAdvancedCraftingStationInterface.h"
#include "Interfaces/Inventory/FVAdvancedInventoryInterface.h"
#include "Settings/FVAdvancedCraftingConfig.h"
#include "Settings/FVAdvancedInventorySettings.h"
#include "Statics/FVInventoryStatics.h"

#define FV_BIND_CRAFTING_DELEGATE(Target, Binding, HandleGetter) \
	if (!IsValid(Target) || !(Binding).IsBound()) \
		return false; \
	IFVAdvancedCraftingParticipantInterface* nativeInterface = Cast<IFVAdvancedCraftingParticipantInterface>(Target); \
	if (!nativeInterface) \
		return false; \
	nativeInterface->HandleGetter().AddUnique(Binding); \
	return true

#define FV_UNBIND_CRAFTING_DELEGATE(Target, Binding, HandleGetter) \
	if (!IsValid(Target) || !(Binding).IsBound()) \
		return false; \
	IFVAdvancedCraftingParticipantInterface* nativeInterface = Cast<IFVAdvancedCraftingParticipantInterface>(Target); \
	if (!nativeInterface) \
		return false; \
	nativeInterface->HandleGetter().Remove(Binding); \
	return true

bool UFVCraftingStatics::HasInventoryItemForRecipeSourceCached(
	const TScriptInterface<IFVAdvancedInventoryInterface>& Inventory,
	UFVInventoryItemTemplate_Recipe* RecipeSource,
	TMap<UFVInventoryItemTemplate_Recipe*, bool>& RecipeSourcePresenceCache)
{
	if (!IsValid(RecipeSource))
		return false;

	if (const bool* cachedPresence = RecipeSourcePresenceCache.Find(RecipeSource))
		return *cachedPresence;

	const bool bHasRecipeItem = UFVInventoryStatics::FindItems(
		Inventory,
		FInventoryItemSearchParams(RecipeSource)).Num() > 0;

	RecipeSourcePresenceCache.Add(RecipeSource, bHasRecipeItem);
	return bHasRecipeItem;
}

TArray<UFVRecipeTemplate*> UFVCraftingStatics::ApplyRecipeSourceGrantFilter(
	const TScriptInterface<IFVAdvancedInventoryInterface>& Inventory,
	const TArray<UFVRecipeTemplate*>& SourceRecipes)
{
	TMap<UFVInventoryItemTemplate_Recipe*, bool> recipeSourcePresenceCache;
	
	TArray<UFVRecipeTemplate*> result;
	result.Reserve(SourceRecipes.Num());
	Algo::CopyIf(SourceRecipes, result,
		[&Inventory, &recipeSourcePresenceCache](const UFVRecipeTemplate* recipe) -> bool
		{
			if (!IsValid(recipe))
				return false;

			if (recipe->RecipeSource.IsNull())
				return true;

			UFVInventoryItemTemplate_Recipe* recipeSource = recipe->RecipeSource.LoadSynchronous();
			if (!IsValid(recipeSource))
			{
				// invalid RecipeSource asset is treated as unavailable source and filtered out.
				return false;
			}

			if (!recipeSource->bTemporaryGrant)
				return true;

			return HasInventoryItemForRecipeSourceCached(Inventory, recipeSource, recipeSourcePresenceCache);
		});

	return result;
}

int32 UFVCraftingStatics::GetIngredientQuantityCached(
	const TScriptInterface<IFVAdvancedInventoryInterface>& Inventory,
	UFVInventoryItemTemplate* IngredientTemplate,
	TMap<UFVInventoryItemTemplate*, int32>& IngredientQuantityCache)
{
	if (!IsValid(IngredientTemplate))
		return 0;

	if (const int32* cachedQuantity = IngredientQuantityCache.Find(IngredientTemplate))
		return *cachedQuantity;

	const TArray<FFVInventoryItem> matchingItems =
		IFVAdvancedInventoryInterface::Execute_FindItems(
			Inventory.GetObject(),
			FInventoryItemSearchParams(IngredientTemplate));

	int32 totalQuantity = 0;
	for (const FFVInventoryItem& matchingItem : matchingItems)
		totalQuantity += matchingItem.Quantity;

	IngredientQuantityCache.Add(IngredientTemplate, totalQuantity);
	return totalQuantity;
}

bool UFVCraftingStatics::IsIngredientSatisfied(
	const TScriptInterface<IFVAdvancedInventoryInterface>& Inventory,
	const UFVRecipeIngredient* Ingredient,
	TMap<UFVInventoryItemTemplate*, int32>& IngredientQuantityCache)
{
	if (!IsValid(Ingredient))
		return false;

	UFVInventoryItemTemplate* ingredientTemplate = Ingredient->IngredientSource.LoadSynchronous();
	if (!IsValid(ingredientTemplate))
		return false;

	return GetIngredientQuantityCached(Inventory, ingredientTemplate, IngredientQuantityCache) >= Ingredient->RequiredQuantity;
}

bool UFVCraftingStatics::IsIngredientGroupSatisfied(
	const TScriptInterface<IFVAdvancedInventoryInterface>& Inventory,
	const UFVRecipeIngredientsList* IngredientGroup,
	TMap<UFVInventoryItemTemplate*, int32>& IngredientQuantityCache)
{
	if (!IsValid(IngredientGroup))
	{
		// invalid ingredient groups are treated as non-craftable options and skipped.
		return false;
	}

	return !IngredientGroup->RecipeIngredients.ContainsByPredicate(
		[&Inventory, &IngredientQuantityCache](const UFVRecipeIngredient* ingredient) -> bool
		{
			return !IsIngredientSatisfied(Inventory, ingredient, IngredientQuantityCache);
		});
}

bool UFVCraftingStatics::IsRecipeSatisfiedByIngredients(
	const TScriptInterface<IFVAdvancedInventoryInterface>& Inventory,
	const UFVRecipeTemplate* Recipe,
	TMap<UFVInventoryItemTemplate*, int32>& IngredientQuantityCache)
{
	if (!IsValid(Recipe))
		return false;

	if (Recipe->RecipeIngredientOptions.Num() == 0)
		return true;

	return Recipe->RecipeIngredientOptions.ContainsByPredicate(
		[&Inventory, &IngredientQuantityCache](const UFVRecipeIngredientsList* ingredientGroup) -> bool
		{
			return IsIngredientGroupSatisfied(Inventory, ingredientGroup, IngredientQuantityCache);
		});
}

TArray<UFVRecipeTemplate*> UFVCraftingStatics::ApplyIngredientAvailabilityFilter(
	const TScriptInterface<IFVAdvancedInventoryInterface>& Inventory,
	const TArray<UFVRecipeTemplate*>& SourceRecipes)
{
	TMap<UFVInventoryItemTemplate*, int32> ingredientQuantityCache;

	TArray<UFVRecipeTemplate*> result;
	result.Reserve(SourceRecipes.Num());
	Algo::CopyIf(SourceRecipes, result,
		[&Inventory, &ingredientQuantityCache](const UFVRecipeTemplate* recipe) -> bool
		{
			return IsRecipeSatisfiedByIngredients(Inventory, recipe, ingredientQuantityCache);
		});

	return result;
}

bool UFVCraftingStatics::IsRecipeInCategory(
	const UFVRecipeTemplate* Recipe,
	const FGameplayTag& CategoryTag)
{
	if (!IsValid(Recipe) || !CategoryTag.IsValid())
		return false;

	const UFVInventoryItemTemplate* resultItem = Recipe->ResultItem.LoadSynchronous();
	if (!IsValid(resultItem))
		return false;

	const UFVAdvancedInventorySettings* inventorySettings = GetDefault<UFVAdvancedInventorySettings>();
	if (!IsValid(inventorySettings))
		return false;

	const TMap<FString, FInventoryCategory>& allowedCategories = inventorySettings->GetAllowedCategories();
	const FInventoryCategory* allowedCategory = allowedCategories.Find(resultItem->ItemCategory);
	if (!allowedCategory)
		return false;

	return allowedCategory->CategoryData.CategoryTags.HasTag(CategoryTag);
}

TArray<UFVRecipeTemplate*> UFVCraftingStatics::ApplyCategoryFilter(
	const TArray<UFVRecipeTemplate*>& SourceRecipes,
	const FGameplayTag& CategoryTag)
{
	if (!CategoryTag.IsValid() || !IsAllowedInventoryCategoryTag(CategoryTag))
		return {};

	TArray<UFVRecipeTemplate*> result;
	result.Reserve(SourceRecipes.Num());
	Algo::CopyIf(SourceRecipes, result,
		[&CategoryTag](const UFVRecipeTemplate* recipe) -> bool
		{
			return IsRecipeInCategory(recipe, CategoryTag);
		});

	return result;
}

void UFVCraftingStatics::GetAllowedInventoryCategoryTags(TArray<FGameplayTag>& OutCategoryTags)
{
	OutCategoryTags.Empty();

	const UFVAdvancedInventorySettings* inventorySettings = GetDefault<UFVAdvancedInventorySettings>();
	if (!IsValid(inventorySettings))
		return;

	const TMap<FString, FInventoryCategory>& allowedCategories = inventorySettings->GetAllowedCategories();
	FGameplayTagContainer allowedCategoryTags;
	Algo::ForEach(allowedCategories,
		[&allowedCategoryTags](const TPair<FString, FInventoryCategory>& allowedCategory)
		{
			allowedCategoryTags.AppendTags(allowedCategory.Value.CategoryData.CategoryTags);
		});

	allowedCategoryTags.GetGameplayTagArray(OutCategoryTags);
	OutCategoryTags.RemoveAll([](const FGameplayTag& categoryTag)
	{
		return !categoryTag.IsValid();
	});

	OutCategoryTags.Sort([](const FGameplayTag& Left, const FGameplayTag& Right)
	{
		return Left.ToString() < Right.ToString();
	});
}

bool UFVCraftingStatics::IsAllowedInventoryCategoryTag(const FGameplayTag& CategoryTag)
{
	if (!CategoryTag.IsValid())
		return false;

	TArray<FGameplayTag> allowedCategoryTags;
	GetAllowedInventoryCategoryTags(allowedCategoryTags);

	return allowedCategoryTags.Contains(CategoryTag);
}

TArray<UFVRecipeTemplate*> UFVCraftingStatics::ApplyStationTypeFilter(
	const TArray<UFVRecipeTemplate*>& SourceRecipes,
	const FGameplayTag& StationType)
{
	TArray<UFVRecipeTemplate*> result;
	result.Reserve(SourceRecipes.Num());
	Algo::CopyIf(SourceRecipes, result,
		[&StationType](const UFVRecipeTemplate* recipe) -> bool
		{
			return IsValid(recipe)
				&& (!recipe->RequiredCraftingPlace.IsValid() || recipe->RequiredCraftingPlace == StationType);
		});

	return result;
}

TArray<UFVRecipeTemplate*> UFVCraftingStatics::ApplyRecipeSourceFilter(
	const TArray<UFVRecipeTemplate*>& SourceRecipes,
	const UFVRecipeTemplate* RecipeSource)
{
	TArray<UFVRecipeTemplate*> result;
	result.Reserve(SourceRecipes.Num());
	Algo::CopyIf(SourceRecipes, result,
		[RecipeSource](const UFVRecipeTemplate* recipe) -> bool
		{
			return recipe == RecipeSource;
		});

	return result;
}

/**
 * CRAFTING PARTICIPANT
 */

bool UFVCraftingStatics::IsValidRecipeHandler(const UObject* Target)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedCraftingParticipantInterface>();
}

bool UFVCraftingStatics::BindToOnCraftingFinished(UObject* Target, const FFVCraftingFinishedBinding& Binding)
{
	FV_BIND_CRAFTING_DELEGATE(Target, Binding, GetOnCraftingFinishedEventHandle);
}

bool UFVCraftingStatics::UnbindFromOnCraftingFinished(UObject* Target, const FFVCraftingFinishedBinding& Binding)
{
	FV_UNBIND_CRAFTING_DELEGATE(Target, Binding, GetOnCraftingFinishedEventHandle);
}

bool UFVCraftingStatics::BindToOnRecipeLearned(UObject* Target, const FFVRecipeLearnedBinding& Binding)
{
	FV_BIND_CRAFTING_DELEGATE(Target, Binding, GetOnRecipeLearnedEventHandle);
}

bool UFVCraftingStatics::UnbindFromOnRecipeLearned(UObject* Target, const FFVRecipeLearnedBinding& Binding)
{
	FV_UNBIND_CRAFTING_DELEGATE(Target, Binding, GetOnRecipeLearnedEventHandle);
}

bool UFVCraftingStatics::BindToOnRecipeForgotten(UObject* Target, const FFVRecipeForgottenBinding& Binding)
{
	FV_BIND_CRAFTING_DELEGATE(Target, Binding, GetOnRecipeForgottenEventHandle);
}

bool UFVCraftingStatics::UnbindFromOnRecipeForgotten(UObject* Target, const FFVRecipeForgottenBinding& Binding)
{
	FV_UNBIND_CRAFTING_DELEGATE(Target, Binding, GetOnRecipeForgottenEventHandle);
}

bool UFVCraftingStatics::DoesHaveAnyRecipes(UObject* Target)
{
	return IsValidRecipeHandler(Target) ? IFVAdvancedCraftingParticipantInterface::Execute_GetKnownRecipes(Target).Num() > 0 : false;
}

TArray<UFVRecipeTemplate*> UFVCraftingStatics::GetFilteredRecipes(UObject* Target, const FFVCraftingRecipeSearchFilter& SearchFilter)
{
	if (!IsValidRecipeHandler(Target))
		return {};

	const TArray<UFVRecipeTemplate*> knownRecipes = GetRecipes(Target, FGameplayTag::EmptyTag);
	if (knownRecipes.Num() == 0)
		return {};

	const TScriptInterface<IFVAdvancedInventoryInterface> parentInventory = GetParentInventory(Target);
	if (!parentInventory)
		return {};

	TArray<UFVRecipeTemplate*> filteredRecipes = ApplyRecipeSourceGrantFilter(parentInventory, knownRecipes);

	// Apply stacked filters in this order:
	// 1) ingredients
	// 2) category
	// 3) station type
	// 4) recipe source
	if (SearchFilter.bSearchByAvailableIngredients)
		filteredRecipes = ApplyIngredientAvailabilityFilter(parentInventory, filteredRecipes);

	if (SearchFilter.bSearchByCategory)
	{
		if (!SearchFilter.CategoryTag.IsValid())
			return {};

		filteredRecipes = ApplyCategoryFilter(filteredRecipes, SearchFilter.CategoryTag);
	}

	if (SearchFilter.bSearchByStationType)
	{
		if (!SearchFilter.StationType.IsValid())
			return {};
		
		filteredRecipes = ApplyStationTypeFilter(filteredRecipes, SearchFilter.StationType);
	}

	if (SearchFilter.bSearchByRecipeSource)
	{
		if (!IsValid(SearchFilter.RecipeSource))
			return {};
		
		filteredRecipes = ApplyRecipeSourceFilter(filteredRecipes, SearchFilter.RecipeSource);
	}

	return filteredRecipes;
}

TArray<UFVRecipeTemplate*> UFVCraftingStatics::GetFilteredRecipesByCategory(UObject* Target, const FGameplayTag& CategoryTag)
{
	if (!CategoryTag.IsValid())
		return {};

	FFVCraftingRecipeSearchFilter searchFilter;
	searchFilter.bSearchByAvailableIngredients = true;
	searchFilter.bSearchByCategory = true;
	searchFilter.CategoryTag = CategoryTag;

	return GetFilteredRecipes(Target, searchFilter);
}

TArray<FInventoryCategory> UFVCraftingStatics::GetAllowedCategoriesWithCraftableItems(UObject* Target)
{
	if (!IsValidRecipeHandler(Target))
		return {};

	const UFVAdvancedInventorySettings* settings = GetDefault<UFVAdvancedInventorySettings>();
	if (!IsValid(settings))
		return {};

	const TMap<FString, FInventoryCategory> allowedCategories = settings->GetAllowedCategories();
	if (allowedCategories.Num() == 0)
		return {};

	FFVCraftingRecipeSearchFilter searchFilter;
	searchFilter.bSearchByAvailableIngredients = true;

	const TArray<UFVRecipeTemplate*> craftableRecipes = GetFilteredRecipes(Target, searchFilter);
	if (craftableRecipes.Num() == 0)
		return {};

	TSet<FString> craftableCategoryIds;
	craftableCategoryIds.Reserve(FMath::Min(craftableRecipes.Num(), allowedCategories.Num()));

	for (const UFVRecipeTemplate* recipe : craftableRecipes)
	{
		if (!IsValid(recipe))
			continue;

		const UFVInventoryItemTemplate* resultItem = recipe->ResultItem.LoadSynchronous();
		if (!IsValid(resultItem))
			continue;

		const FString& itemCategory = resultItem->ItemCategory;
		if (craftableCategoryIds.Contains(itemCategory))
			continue;

		if (!allowedCategories.Contains(itemCategory))
			continue;

		craftableCategoryIds.Add(itemCategory);
		if (craftableCategoryIds.Num() == allowedCategories.Num())
			break;
	}

	if (craftableCategoryIds.IsEmpty())
		return {};

	TArray<FInventoryCategory> returnValue;
	returnValue.Reserve(allowedCategories.Num());

	Algo::TransformIf(
		allowedCategories,
		returnValue,
		[&craftableCategoryIds](const TPair<FString, FInventoryCategory>& category)
		{
			return craftableCategoryIds.Contains(category.Key);
		},
		[](const TPair<FString, FInventoryCategory>& category)
		{
			return category.Value;
		});

	return returnValue;
}

TArray<UFVRecipeTemplate*> UFVCraftingStatics::GetKnownRecipes(UObject* Target)
{
	return IsValidRecipeHandler(Target) ? IFVAdvancedCraftingParticipantInterface::Execute_GetKnownRecipes(Target) : TArray<UFVRecipeTemplate*>();
}

UFVRecipeTemplate* UFVCraftingStatics::GetRecipe(UObject* Target, const FGuid& RecipeGuid)
{
	return IsValidRecipeHandler(Target) ? IFVAdvancedCraftingParticipantInterface::Execute_GetRecipe(Target, RecipeGuid) : nullptr;
}

TArray<UFVRecipeTemplate*> UFVCraftingStatics::GetRecipes(UObject* Target, const FGameplayTag& CraftingStationType)
{
	return IsValidRecipeHandler(Target) ? IFVAdvancedCraftingParticipantInterface::Execute_GetRecipes(Target, CraftingStationType) : TArray<UFVRecipeTemplate*>();
}

bool UFVCraftingStatics::IsRecipeKnown(UObject* Target, UFVRecipeTemplate* RecipeTemplate)
{
	return IsValidRecipeHandler(Target) ? IFVAdvancedCraftingParticipantInterface::Execute_IsRecipeKnown(Target, RecipeTemplate) : false;
}

bool UFVCraftingStatics::LearnRecipe(UObject* Target, UFVRecipeTemplate* RecipeTemplate)
{
	return IsValidRecipeHandler(Target) ? IFVAdvancedCraftingParticipantInterface::Execute_LearnRecipe(Target, RecipeTemplate) : false;
}

bool UFVCraftingStatics::ForgetRecipe(UObject* Target, UFVRecipeTemplate* RecipeTemplate)
{
	return IsValidRecipeHandler(Target) ? IFVAdvancedCraftingParticipantInterface::Execute_ForgetRecipe(Target, RecipeTemplate) : false;
}

bool UFVCraftingStatics::IsCraftingPossible(UObject* Target, UFVRecipeTemplate* TemplateToCraft)
{
	return IsValidRecipeHandler(Target) ? IFVAdvancedCraftingParticipantInterface::Execute_IsCraftingPossible(Target, TemplateToCraft) : false;
}

FFVCraftingResult UFVCraftingStatics::StartCrafting(UObject* Target, UFVRecipeTemplate* TemplateToCraft, UFVRecipeIngredientsList* Ingredients)
{
	return IFVAdvancedCraftingParticipantInterface::Execute_StartCrafting(Target, TemplateToCraft, Ingredients);
}

TScriptInterface<IFVAdvancedInventoryInterface> UFVCraftingStatics::GetParentInventory(UObject* Target)
{
	return IsValidRecipeHandler(Target) ? IFVAdvancedCraftingParticipantInterface::Execute_GetParentInventory(Target) : TScriptInterface<IFVAdvancedInventoryInterface>();
}

bool UFVCraftingStatics::SetParentInventory(UObject* Target, const TScriptInterface<IFVAdvancedInventoryInterface>& NewParentInventory)
{
	return IsValidRecipeHandler(Target) ? IFVAdvancedCraftingParticipantInterface::Execute_SetParentInventory(Target, NewParentInventory) : false;
}

bool UFVCraftingStatics::StartUsingCraftingStation(UObject* Target, const TScriptInterface<IFVAdvancedCraftingStationInterface>& Station)
{
	return IsValidRecipeHandler(Target) ? IFVAdvancedCraftingParticipantInterface::Execute_StartUsingCraftingStation(Target, Station) : false;
}

bool UFVCraftingStatics::StopUsingCraftingStation(UObject* Target)
{
	return IsValidRecipeHandler(Target) ? IFVAdvancedCraftingParticipantInterface::Execute_StopUsingCraftingStation(Target) : false;
}

TScriptInterface<IFVAdvancedCraftingStationInterface> UFVCraftingStatics::GetCraftingStation(UObject* Target)
{
	return IsValidRecipeHandler(Target) ? IFVAdvancedCraftingParticipantInterface::Execute_GetCraftingStation(Target) : TScriptInterface<IFVAdvancedCraftingStationInterface>();
}

TSet<UFVRecipeTemplate*> UFVCraftingStatics::GetAllRecipeTemplates()
{
	const auto settings = GetDefault<UFVAdvancedInventorySettings>();
	if (!IsValid(settings))
		return TSet<UFVRecipeTemplate*>();
	const auto craftingConfig = settings->AdvancedCraftingSettingsConfig.LoadSynchronous();
	if (!IsValid(craftingConfig))
		return TSet<UFVRecipeTemplate*>();
	const auto& allowedRecipes = craftingConfig->AllowedRecipes;

	TSet<UFVRecipeTemplate*> returnValue;
	returnValue.Reserve(allowedRecipes.Num());

	for(const auto& recipe : allowedRecipes)
	{
		returnValue.Add(recipe.Get());
	}

	return returnValue;
}

FFVCraftingResult UFVCraftingStatics::CraftItem(const TScriptInterface<IFVAdvancedCraftingParticipantInterface>& Target, const UFVRecipeTemplate* TemplateToCraft, UFVRecipeIngredientsList* Ingredients)
{
	FFVCraftingResult result;

	if (!Target || !IsValid(TemplateToCraft) || !IsValid(Ingredients))
		return result;
	
	const auto craftingStation = Target->Execute_GetCraftingStation(Target.GetObject());
	if (TemplateToCraft->RequiredCraftingPlace.IsValid() && !IsValid(craftingStation.GetObject()))
		return result;
	
	if (IsValid(craftingStation.GetObject()) && TemplateToCraft->RequiredCraftingPlace != craftingStation->Execute_GetCraftingPlaceType(craftingStation.GetObject()))
		return result;

	const TScriptInterface<IFVAdvancedInventoryInterface> inventory =
		IFVAdvancedCraftingParticipantInterface::Execute_GetParentInventory(Target.GetObject());
	if (!inventory)
		return result;

	UFVInventoryItemTemplate* resultTemplate = TemplateToCraft->ResultItem.LoadSynchronous();
	if (!IsValid(resultTemplate))
		return result;

	for (const UFVRecipeIngredient* ingredient : Ingredients->RecipeIngredients)
	{
		if (!IsValid(ingredient))
			return result;

		UFVInventoryItemTemplate* ingredientTemplate = ingredient->IngredientSource.LoadSynchronous();
		if (!IsValid(ingredientTemplate))
			return result;

		const TArray<FFVInventoryItem> foundItems =
			IFVAdvancedInventoryInterface::Execute_FindItems(inventory.GetObject(), FInventoryItemSearchParams(ingredientTemplate));

		int32 totalQuantity = 0;
		for (const FFVInventoryItem& item : foundItems)
			totalQuantity += item.Quantity;

		if (totalQuantity < ingredient->RequiredQuantity)
			return result;
	}

	if (!IFVAdvancedInventoryInterface::Execute_CanAddItemFromTemplate(inventory.GetObject(), resultTemplate, TemplateToCraft->QuantityPerCreation))
		return result;

	for (const UFVRecipeIngredient* ingredient : Ingredients->RecipeIngredients)
	{
		UFVInventoryItemTemplate* ingredientTemplate = ingredient->IngredientSource.LoadSynchronous();
		if (!IFVAdvancedInventoryInterface::Execute_RemoveItemFromTemplate(inventory.GetObject(), ingredientTemplate, ingredient->RequiredQuantity))
			return result;
	}

	if (!IFVAdvancedInventoryInterface::Execute_AddItemFromTemplate(inventory.GetObject(), resultTemplate, TemplateToCraft->QuantityPerCreation, 1))
		return result;

	result.bCraftingSuccess = true;
	return result;
}

/**
 * CRAFTING PLACE
 */

bool UFVCraftingStatics::IsValidCraftingPlace(const UObject* Target)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedCraftingStationInterface>();
}

FGameplayTag UFVCraftingStatics::GetCraftingPlaceType(UObject* Target)
{
	return IsValidCraftingPlace(Target) ? IFVAdvancedCraftingStationInterface::Execute_GetCraftingPlaceType(Target) : FGameplayTag();
}

bool UFVCraftingStatics::IsCraftingPlaceOccupied(UObject* Target)
{
	return IsValidCraftingPlace(Target) ? IFVAdvancedCraftingStationInterface::Execute_IsCraftingPlaceOccupied(Target) : false;
}

int32 UFVCraftingStatics::GetCraftingPlaceCapacity(UObject* Target)
{
	return IsValidCraftingPlace(Target) ? IFVAdvancedCraftingStationInterface::Execute_GetCraftingPlaceCapacity(Target) : INDEX_NONE;
}

bool UFVCraftingStatics::CanBeUsed(UObject* Target)
{
	return IsValidCraftingPlace(Target) ? IFVAdvancedCraftingStationInterface::Execute_CanBeUsed(Target) : false;
}

ECraftingStationState UFVCraftingStatics::GetCraftingStationState(UObject* Target)
{
	return IsValidCraftingPlace(Target) ? IFVAdvancedCraftingStationInterface::Execute_GetCraftingStationState(Target) : ECraftingStationState::EASS_Inactive;
}

bool UFVCraftingStatics::SetCraftingStationState(UObject* Target, const ECraftingStationState NewState)
{
	return IsValidCraftingPlace(Target) ? IFVAdvancedCraftingStationInterface::Execute_SetCraftingStationState(Target, NewState) : false;
}

bool UFVCraftingStatics::StartUsing(UObject* Target, const TScriptInterface<IFVAdvancedCraftingParticipantInterface>& Participant)
{
	return IsValidCraftingPlace(Target) ? IFVAdvancedCraftingStationInterface::Execute_StartUsing(Target, Participant) : false;
}

bool UFVCraftingStatics::StopUsing(UObject* Target, const TScriptInterface<IFVAdvancedCraftingParticipantInterface>& Participant)
{
	return IsValidCraftingPlace(Target) ? IFVAdvancedCraftingStationInterface::Execute_StopUsing(Target, Participant) : false;
}

TArray<UFVRecipeTemplate*> UFVCraftingStatics::GetRecipesByCategory(UObject* Target, const FGameplayTag& CategoryTag)
{
	return GetFilteredRecipesByCategory(Target, CategoryTag);
}
