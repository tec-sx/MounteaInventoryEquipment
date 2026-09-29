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

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "FVCraftingBaseDataTypes.generated.h"

class UFVRecipeTemplate;

/**
 * 
 */
USTRUCT(BlueprintType)
struct FFVCraftingResult
{
	GENERATED_BODY()
	
	FFVCraftingResult() 
		: bCraftingSuccess(false),
		ResultItemId(FGuid::NewGuid())
	{};
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Crafting")
	uint8 bCraftingSuccess : 1;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Crafting")
	FGuid ResultItemId;	
};

/**
 * Search parameters for filtering known crafting recipes.
 * Filters can be stacked and are expected to run in this order:
 * 1) Available ingredients
 * 2) Category
 * 3) Station type
 * 4) Recipe source
 */
USTRUCT(BlueprintType)
struct FFVCraftingRecipeSearchFilter
{
	GENERATED_BODY()

	FFVCraftingRecipeSearchFilter()
		: bSearchByStationType(false)
		, bSearchByAvailableIngredients(false)
		, bSearchByCategory(false)
		, bSearchByRecipeSource(false)
		, RecipeSource(nullptr)
	{
	}

	explicit FFVCraftingRecipeSearchFilter(const FGameplayTag& InStationType)
		: bSearchByStationType(true)
		, StationType(InStationType)
		, bSearchByAvailableIngredients(false)
		, bSearchByCategory(false)
		, bSearchByRecipeSource(false)
		, RecipeSource(nullptr)
	{
	}

	explicit FFVCraftingRecipeSearchFilter(UFVRecipeTemplate* InRecipeSource)
		: bSearchByStationType(false)
		, bSearchByAvailableIngredients(false)
		, bSearchByCategory(false)
		, bSearchByRecipeSource(true)
		, RecipeSource(InRecipeSource)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
	uint8 bSearchByStationType : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search",
		meta=(Categories="FV_Inventory.Crafting,Crafting"))
	FGameplayTag StationType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
	uint8 bSearchByAvailableIngredients : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
	uint8 bSearchByCategory : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search",
		meta=(Categories="FV_Inventory.Categories,FV_Inventory.Category,Categories,Category"))
	FGameplayTag CategoryTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
	uint8 bSearchByRecipeSource : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
	TObjectPtr<UFVRecipeTemplate> RecipeSource;
};
