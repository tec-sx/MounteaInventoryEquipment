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
#include "Engine/DataAsset.h"
#include "FVRecipeIngredientsList.generated.h"

class UFVRecipeIngredient;

/**
 * 
 */
UCLASS(ClassGroup=(FV), BlueprintType, DefaultToInstanced, EditInlineNew,
	AutoExpandCategories=("FV"),
	HideCategories=("Cooking","Collision","Private"),
	meta=(DisplayName="FV Inventory Recipe Ingredients"))
class FVINVENTORYEQUIPMENTSYSTEM_API UFVRecipeIngredientsList : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "Configuration",
		Instanced,
		meta=(NoResetToDefault),
		meta=(ShowInnerProperties, ShowOnlyInnerProperties),
		meta=(FullyExpand=true))
	TArray<TObjectPtr<UFVRecipeIngredient>> RecipeIngredients;

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
};
