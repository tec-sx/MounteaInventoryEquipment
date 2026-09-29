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
#include "Definitions/FVCraftingBaseDataTypes.h"
#include "Definitions/FVInventoryBaseDataTypes.h"
#include "Interfaces/Crafting/FVAdvancedCraftingParticipantInterface.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVCraftingStatics.generated.h"

class UFVRecipeTemplate;
class UFVRecipeIngredientsList;
class UFVRecipeIngredient;
class UFVInventoryItemTemplate;
class UFVInventoryItemTemplate_Recipe;
class IFVAdvancedCraftingParticipantInterface;
class IFVAdvancedCraftingStationInterface;
class IFVAdvancedInventoryInterface;

struct FGameplayTag;
struct FFVCraftingResult;
struct FFVCraftingRecipeSearchFilter;
enum class ECraftingStationState : uint8;

/**
 *
 */
UCLASS()
class FVINVENTORYEQUIPMENTSYSTEM_API UFVCraftingStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

/**
 * CRAFTING PARTICIPANT
 */

public:

	static bool IsValidRecipeHandler(const UObject* Target);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|Bindings",
		meta=(FVBinding),
		meta=(DefaultToSelf="Target"),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Bind On Crafting Finished")
	static bool BindToOnCraftingFinished(UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedCraftingParticipantInterface")) UObject* Target, const FFVCraftingFinishedBinding& Binding);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|Bindings",
		meta=(FVBinding),
		meta=(DefaultToSelf="Target"),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Unbind From On Crafting Finished")
	static bool UnbindFromOnCraftingFinished(UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedCraftingParticipantInterface")) UObject* Target, const FFVCraftingFinishedBinding& Binding);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|Bindings",
		meta=(FVBinding),
		meta=(DefaultToSelf="Target"),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Bind On Recipe Learned")
	static bool BindToOnRecipeLearned(UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedCraftingParticipantInterface")) UObject* Target, const FFVRecipeLearnedBinding& Binding);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|Bindings",
		meta=(FVBinding),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(DefaultToSelf="Target"),
		DisplayName="Unbind From On Recipe Learned")
	static bool UnbindFromOnRecipeLearned(UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedCraftingParticipantInterface")) UObject* Target, const FFVRecipeLearnedBinding& Binding);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|Bindings",
		meta=(FVBinding),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(DefaultToSelf="Target"),
		DisplayName="Bind On Recipe Forgotten")
	static bool BindToOnRecipeForgotten(UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedCraftingParticipantInterface")) UObject* Target, const FFVRecipeForgottenBinding& Binding);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|Bindings",
		meta=(FVBinding),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(DefaultToSelf="Target"),
		DisplayName="Unbind From On Recipe Forgotten")
	static bool UnbindFromOnRecipeForgotten(UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedCraftingParticipantInterface")) UObject* Target, const FFVRecipeForgottenBinding& Binding);
	
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|Participant",
		meta=(FVValidate),
		meta=(DefaultToSelf="Target"),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Does Have Any Recipes")
	static bool DoesHaveAnyRecipes(UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedCraftingParticipantInterface")) UObject* Target);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|Participant",
		meta=(FVGetter),
		meta=(AutoCreateRefTerm="SearchFilter"),
		meta=(DefaultToSelf="Target"),
		DisplayName="Get All Available Recipes")
	static TArray<UFVRecipeTemplate*> GetFilteredRecipes(UObject* Target, const FFVCraftingRecipeSearchFilter& SearchFilter);

	/**
	 * Returns all recipes known by the crafting participant where the craftable item would be in specified Category.
	 * @param Target Crafting participant to query for known recipes.
	 * @param CategoryTag Category to search by.
	 * @return Filtered list of available recipes by single category.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|Participant",
		meta=(FVGetter, FVInventoryCategoryPin="CategoryTag"),
		meta=(AutoCreateRefTerm="CategoryTag"),
		meta=(DefaultToSelf="Target"),
		DisplayName="Get All Available Recipes By Category")
	static TArray<UFVRecipeTemplate*> GetFilteredRecipesByCategory(
		UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedCraftingParticipantInterface")) UObject* Target,
		UPARAM(meta=(Categories="FV_Inventory.Categories,FV_Inventory.Category,Categories,Category")) const FGameplayTag& CategoryTag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|Participant",
		meta=(FVGetter),
		meta=(DefaultToSelf="Target"),
		DisplayName="Get Categories With Craftbale Items")
	static TArray<FInventoryCategory> GetAllowedCategoriesWithCraftableItems(UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedCraftingParticipantInterface")) UObject* Target);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|Participant",
		meta=(FVGetter),
		meta=(DefaultToSelf="Target"),
		DisplayName="Get All Known Recipes")
	static TArray<UFVRecipeTemplate*> GetKnownRecipes(UObject* Target);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|Participant",
		meta=(FVGetter),
		meta=(DefaultToSelf="Target"),
		meta=(AutoCreateRefTerm="RecipeGuid"),
		DisplayName="Get Recipe")
	static UFVRecipeTemplate* GetRecipe(UObject* Target, const FGuid& RecipeGuid);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|Participant",
		meta=(FVGetter),
		meta=(DefaultToSelf="Target"),
		meta=(AutoCreateRefTerm="CraftingStationType"),
		DisplayName="Get Recipes")
	static TArray<UFVRecipeTemplate*> GetRecipes(UObject* Target, const FGameplayTag& CraftingStationType);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|Participant",
		meta=(FVValidate),
		meta=(DefaultToSelf="Target"),
		DisplayName="Is Recipe Known")
	static bool IsRecipeKnown(UObject* Target, UFVRecipeTemplate* RecipeTemplate);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|Participant",
		meta=(FVSetter),
		meta=(DefaultToSelf="Target"),
		DisplayName="Learn Recipe")
	static bool LearnRecipe(UObject* Target, UFVRecipeTemplate* RecipeTemplate);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|Participant",
		meta=(FVSetter),
		meta=(DefaultToSelf="Target"),
		DisplayName="Forget Recipe")
	static bool ForgetRecipe(UObject* Target, UFVRecipeTemplate* RecipeTemplate);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|Participant",
		meta=(FVValidate),
		meta=(DefaultToSelf="Target"),
		DisplayName="Is Crafting Possible")
	static bool IsCraftingPossible(UObject* Target, UFVRecipeTemplate* TemplateToCraft);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|Participant",
		meta=(FVSetter),
		meta=(DefaultToSelf="Target"),
		DisplayName="Start Crafting")
	static FFVCraftingResult StartCrafting(UObject* Target, UFVRecipeTemplate* TemplateToCraft, UFVRecipeIngredientsList* Ingredients);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|Participant",
		meta=(FVGetter),
		meta=(DefaultToSelf="Target"),
		DisplayName="Get Parent Inventory")
	static TScriptInterface<IFVAdvancedInventoryInterface> GetParentInventory(UObject* Target);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|Participant",
		meta=(FVSetter),
		meta=(DefaultToSelf="Target"),
		DisplayName="Set Parent Inventory")
	static bool SetParentInventory(UObject* Target, const TScriptInterface<IFVAdvancedInventoryInterface>& NewParentInventory);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|Participant",
		meta=(FVSetter),
		meta=(DefaultToSelf="Target"),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Start Using Crafting Station")
	static bool StartUsingCraftingStation(UObject* Target, const TScriptInterface<IFVAdvancedCraftingStationInterface>& Station);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|Participant",
		meta=(FVSetter),
		meta=(DefaultToSelf="Target"),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Stop Using Crafting Station")
	static bool StopUsingCraftingStation(UObject* Target);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|Participant",
		meta=(FVGetter),
		meta=(DefaultToSelf="Target"),
		DisplayName="Get Crafting Station")
	static TScriptInterface<IFVAdvancedCraftingStationInterface> GetCraftingStation(UObject* Target);

public:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|Participant",
		meta=(FVGetter),
		DisplayName="Get All Recipe Templates")
	static TSet<UFVRecipeTemplate*> GetAllRecipeTemplates();

	static FFVCraftingResult CraftItem(const TScriptInterface<IFVAdvancedCraftingParticipantInterface>& Target, const UFVRecipeTemplate* TemplateToCraft, UFVRecipeIngredientsList* Ingredients);
	
	static bool HasInventoryItemForRecipeSourceCached(
		const TScriptInterface<IFVAdvancedInventoryInterface>& Inventory,
		UFVInventoryItemTemplate_Recipe* RecipeSource,
		TMap<UFVInventoryItemTemplate_Recipe*, bool>& RecipeSourcePresenceCache);
	
	static TArray<UFVRecipeTemplate*> ApplyRecipeSourceGrantFilter(
		const TScriptInterface<IFVAdvancedInventoryInterface>& Inventory,
		const TArray<UFVRecipeTemplate*>& SourceRecipes);

	static int32 GetIngredientQuantityCached(
		const TScriptInterface<IFVAdvancedInventoryInterface>& Inventory,
		UFVInventoryItemTemplate* IngredientTemplate,
		TMap<UFVInventoryItemTemplate*, int32>& IngredientQuantityCache);

	static bool IsIngredientSatisfied(
		const TScriptInterface<IFVAdvancedInventoryInterface>& Inventory,
		const UFVRecipeIngredient* Ingredient,
		TMap<UFVInventoryItemTemplate*, int32>& IngredientQuantityCache);

	static bool IsIngredientGroupSatisfied(
		const TScriptInterface<IFVAdvancedInventoryInterface>& Inventory,
		const UFVRecipeIngredientsList* IngredientGroup,
		TMap<UFVInventoryItemTemplate*, int32>& IngredientQuantityCache);

	static bool IsRecipeSatisfiedByIngredients(
		const TScriptInterface<IFVAdvancedInventoryInterface>& Inventory,
		const UFVRecipeTemplate* Recipe,
		TMap<UFVInventoryItemTemplate*, int32>& IngredientQuantityCache);

	static TArray<UFVRecipeTemplate*> ApplyIngredientAvailabilityFilter(
		const TScriptInterface<IFVAdvancedInventoryInterface>& Inventory,
		const TArray<UFVRecipeTemplate*>& SourceRecipes);

	static bool IsRecipeInCategory(
		const UFVRecipeTemplate* Recipe,
		const FGameplayTag& CategoryTag);

	static TArray<UFVRecipeTemplate*> ApplyCategoryFilter(
		const TArray<UFVRecipeTemplate*>& SourceRecipes,
		const FGameplayTag& CategoryTag);

	static void GetAllowedInventoryCategoryTags(TArray<FGameplayTag>& OutCategoryTags);

	static bool IsAllowedInventoryCategoryTag(const FGameplayTag& CategoryTag);

	static TArray<UFVRecipeTemplate*> ApplyStationTypeFilter(
		const TArray<UFVRecipeTemplate*>& SourceRecipes,
		const FGameplayTag& StationType);

	static TArray<UFVRecipeTemplate*> ApplyRecipeSourceFilter(
		const TArray<UFVRecipeTemplate*>& SourceRecipes,
		const UFVRecipeTemplate* RecipeSource);

/**
 * CRAFTING PLACE
 */

public:

	static bool IsValidCraftingPlace(const UObject* Target);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|Station",
		meta=(FVGetter),
		meta=(DefaultToSelf="Target"),
		DisplayName="Get Crafting Place Type")
	static FGameplayTag GetCraftingPlaceType(UObject* Target);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|Station",
		meta=(FVValidate),
		meta=(DefaultToSelf="Target"),
		DisplayName="Is Crafting Place Occupied")
	static bool IsCraftingPlaceOccupied(UObject* Target);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|Station",
		meta=(FVGetter),
		meta=(DefaultToSelf="Target"),
		DisplayName="Get Crafting Place Capacity")
	static int32 GetCraftingPlaceCapacity(UObject* Target);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|Station",
		meta=(FVValidate),
		meta=(DefaultToSelf="Target"),
		DisplayName="Can Be Used")
	static bool CanBeUsed(UObject* Target);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|Station",
		meta=(FVGetter),
		meta=(DefaultToSelf="Target"),
		DisplayName="Get Crafting Station State")
	static ECraftingStationState GetCraftingStationState(UObject* Target);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|Station",
		meta=(FVSetter),
		meta=(DefaultToSelf="Target"),
		meta=(AutoCreateRefTerm="NewState"),
		DisplayName="Set Crafting Station State")
	static bool SetCraftingStationState(UObject* Target, ECraftingStationState NewState);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|Station",
		meta=(FVSetter),
		meta=(DefaultToSelf="Target"),
		DisplayName="Start Using Station")
	static bool StartUsing(UObject* Target, const TScriptInterface<IFVAdvancedCraftingParticipantInterface>& Participant);

	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|Station",
		meta=(FVSetter),
		meta=(DefaultToSelf="Target"),
		DisplayName="Stop Using Station")
	static bool StopUsing(UObject* Target, const TScriptInterface<IFVAdvancedCraftingParticipantInterface>& Participant);
	
/**
 * HELPERS
 */

public:
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|Helpers",
		meta=(FVGetter, FVInventoryCategoryPin="CategoryTag"),
		meta=(DefaultToSelf="Target"),
		meta=(AutoCreateRefTerm="CategoryTag"),
		DisplayName="Get Recipes By Category")
	static TArray<UFVRecipeTemplate*> GetRecipesByCategory(
		UObject* Target,
		UPARAM(meta=(Categories="FV_Inventory.Categories,FV_Inventory.Category,Categories,Category")) const FGameplayTag& CategoryTag);
};
