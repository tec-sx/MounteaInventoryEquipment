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
#include "UObject/Interface.h"
#include "FVAdvancedCraftingParticipantInterface.generated.h"

class IFVAdvancedCraftingStationInterface;
class IFVAdvancedInventoryInterface;
struct FGameplayTag;
struct FFVCraftingResult;

class UFVRecipeTemplate;
class UFVRecipeIngredientsList;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftingFinished, const FFVCraftingResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecipeLearned, const UFVRecipeTemplate*, RecipeTemplate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecipeForgotten, const UFVRecipeTemplate*, RecipeTemplate);
DECLARE_DYNAMIC_DELEGATE_OneParam(FFVCraftingFinishedBinding, const FFVCraftingResult&, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FFVRecipeLearnedBinding, const UFVRecipeTemplate*, RecipeTemplate);
DECLARE_DYNAMIC_DELEGATE_OneParam(FFVRecipeForgottenBinding, const UFVRecipeTemplate*, RecipeTemplate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVCraftingStarted, const TScriptInterface<IFVAdvancedCraftingStationInterface>&, CraftingStation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVCraftingStopped, const TScriptInterface<IFVAdvancedCraftingStationInterface>&, CraftingStation);

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UFVAdvancedCraftingParticipantInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FVINVENTORYEQUIPMENTSYSTEM_API IFVAdvancedCraftingParticipantInterface
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	TArray<UFVRecipeTemplate*> GetKnownRecipes() const;
	virtual TArray<UFVRecipeTemplate*> GetKnownRecipes_Implementation() const = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	TArray<UFVRecipeTemplate*> GetRecipes(const FGameplayTag& CraftingStationType) const;
	virtual TArray<UFVRecipeTemplate*> GetRecipes_Implementation(const FGameplayTag& CraftingStationType) const = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	UFVRecipeTemplate* GetRecipe(const FGuid& RecipeGuid) const;
	virtual UFVRecipeTemplate* GetRecipe_Implementation(const FGuid& RecipeGuid) const = 0;

	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	bool IsRecipeKnown(UFVRecipeTemplate* RecipeTemplate) const;
	virtual bool IsRecipeKnown_Implementation(UFVRecipeTemplate* RecipeTemplate) const = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	bool LearnRecipe(UFVRecipeTemplate* RecipeTemplate);
	virtual bool LearnRecipe_Implementation(UFVRecipeTemplate* RecipeTemplate) = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	bool ForgetRecipe(UFVRecipeTemplate* RecipeTemplate);
	virtual bool ForgetRecipe_Implementation(UFVRecipeTemplate* RecipeTemplate) = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	bool IsCraftingPossible(UFVRecipeTemplate* TemplateToCraft) const;
	virtual bool IsCraftingPossible_Implementation(UFVRecipeTemplate* TemplateToCraft) const = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	FFVCraftingResult StartCrafting(UFVRecipeTemplate* TemplateToCraft, UFVRecipeIngredientsList* Ingredients);
	virtual FFVCraftingResult StartCrafting_Implementation(UFVRecipeTemplate* TemplateToCraft, UFVRecipeIngredientsList* Ingredients) = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	void StopCrafting();
	virtual void StopCrafting_Implementation() = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	TScriptInterface<IFVAdvancedInventoryInterface> GetParentInventory() const;
	virtual TScriptInterface<IFVAdvancedInventoryInterface> GetParentInventory_Implementation() const = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	bool SetParentInventory(const TScriptInterface<IFVAdvancedInventoryInterface>& NewParentInventory);
	virtual bool SetParentInventory_Implementation(const TScriptInterface<IFVAdvancedInventoryInterface>& NewParentInventory) = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	bool StartUsingCraftingStation(const TScriptInterface<IFVAdvancedCraftingStationInterface>& Station);
	virtual bool StartUsingCraftingStation_Implementation(const TScriptInterface<IFVAdvancedCraftingStationInterface>& Station) = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	bool StopUsingCraftingStation();
	virtual bool StopUsingCraftingStation_Implementation() = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	TScriptInterface<IFVAdvancedCraftingStationInterface> GetCraftingStation() const;
	virtual TScriptInterface<IFVAdvancedCraftingStationInterface> GetCraftingStation_Implementation() const = 0;
	
public:
	
	virtual FOnCraftingFinished& GetOnCraftingFinishedEventHandle() = 0;
	virtual FOnRecipeLearned& GetOnRecipeLearnedEventHandle() = 0;
	virtual FOnRecipeForgotten& GetOnRecipeForgottenEventHandle() = 0;
};
