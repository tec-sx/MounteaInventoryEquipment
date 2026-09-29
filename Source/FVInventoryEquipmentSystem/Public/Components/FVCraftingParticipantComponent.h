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
#include "Components/ActorComponent.h"
#include "Interfaces/Crafting/FVAdvancedCraftingParticipantInterface.h"
#include "FVCraftingParticipantComponent.generated.h"

class IFVAdvancedCraftingStationInterface;
struct FFVCraftingResult;
class IFVAdvancedInventoryInterface;

/**
 * UFVCraftingParticipantComponent represents an actor capable of interacting with crafting stations.
 * It manages known recipes, crafting actions, and the currently-used station reference,
 * replicated with server authority and client notification RPCs.
 *
 * @see IFVAdvancedCraftingParticipantInterface
 * @see UFVCraftingStationComponent
 */
UCLASS(ClassGroup=(FV), Blueprintable,
	AutoExpandCategories=("FV","Crafting","FV|Crafting"),
	AutoCollapseCategories=("Variable,Sockets,Tags,Component Tick,Component Replication,Activation,Events,Replication,Asset User Data,Navigation"),
	HideCategories=("Cooking","Collision"),
	meta=(BlueprintSpawnableComponent, DisplayName="FV Crafting Participant Component"))
class FVINVENTORYEQUIPMENTSYSTEM_API UFVCraftingParticipantComponent : public UActorComponent, public IFVAdvancedCraftingParticipantInterface
{
	GENERATED_BODY()

public:
	UFVCraftingParticipantComponent();

public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	void InitializeInventoryAndEquipment();

	UFUNCTION()
	void OnRep_KnownRecipes();

	UFUNCTION()
	void OnRep_CraftingStation();

public:

	virtual TArray<UFVRecipeTemplate*> GetKnownRecipes_Implementation() const override;
	virtual TArray<UFVRecipeTemplate*> GetRecipes_Implementation(const FGameplayTag& CraftingStationType) const override;
	virtual UFVRecipeTemplate* GetRecipe_Implementation(const FGuid& RecipeGuid) const override;
	virtual bool IsRecipeKnown_Implementation(UFVRecipeTemplate* RecipeTemplate) const override;
	virtual bool LearnRecipe_Implementation(UFVRecipeTemplate* RecipeTemplate) override;
	virtual bool ForgetRecipe_Implementation(UFVRecipeTemplate* RecipeTemplate) override;
	virtual bool IsCraftingPossible_Implementation(UFVRecipeTemplate* TemplateToCraft) const override;
	virtual FFVCraftingResult StartCrafting_Implementation(UFVRecipeTemplate* TemplateToCraft, UFVRecipeIngredientsList* Ingredients) override;
	virtual void StopCrafting_Implementation() override;
	virtual TScriptInterface<IFVAdvancedInventoryInterface> GetParentInventory_Implementation() const override
	{
		return RelatedInventory;
	}
	virtual bool SetParentInventory_Implementation(const TScriptInterface<IFVAdvancedInventoryInterface>& NewParentInventory) override;

	virtual FOnCraftingFinished& GetOnCraftingFinishedEventHandle() override
	{
		return OnCraftingFinished;
	}
	virtual FOnRecipeLearned& GetOnRecipeLearnedEventHandle() override
	{
		return OnRecipeLearned;
	}
	virtual FOnRecipeForgotten& GetOnRecipeForgottenEventHandle() override
	{
		return OnRecipeForgotten;
	}
	virtual bool StartUsingCraftingStation_Implementation(const TScriptInterface<IFVAdvancedCraftingStationInterface>& Station) override;
	virtual bool StopUsingCraftingStation_Implementation() override;
	virtual TScriptInterface<IFVAdvancedCraftingStationInterface> GetCraftingStation_Implementation() const override
	{
		return CraftingStation;
	}

protected:
	static TArray<UFVRecipeTemplate*> ResolveKnownRecipeTemplates(const TArray<FGuid>& KnownRecipeGuids);

public:

	UPROPERTY(BlueprintAssignable, Category="Crafting")
	FOnCraftingFinished OnCraftingFinished;

	UPROPERTY(BlueprintAssignable, Category="Crafting")
	FOnRecipeLearned OnRecipeLearned;

	UPROPERTY(BlueprintAssignable, Category="Crafting")
	FOnRecipeForgotten OnRecipeForgotten;
	
	UPROPERTY(BlueprintAssignable, Category="Crafting")
	FFVCraftingStarted OnCraftingStarted;
	
	UPROPERTY(BlueprintAssignable, Category="Crafting")
	FFVCraftingStopped OnCraftingStopped;

	UPROPERTY(SaveGame, ReplicatedUsing=OnRep_KnownRecipes, VisibleAnywhere, BlueprintReadOnly, Category="Crafting",
		meta=(NoResetToDefault))
	TArray<FGuid> KnownRecipes;

	/**
	 * Cached inventory interface found on the owning actor.
	 *
	 * This is initialized during BeginPlay and used as the target inventory when loading
	 * items from the configured loadout.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Configuration",
		meta=(NoResetToDefault),
		meta=(DisplayThumbnail=false))
	TScriptInterface<IFVAdvancedInventoryInterface> RelatedInventory;

	UPROPERTY(ReplicatedUsing=OnRep_CraftingStation, VisibleAnywhere, BlueprintReadOnly, Category="Configuration",
		meta=(NoResetToDefault),
		meta=(DisplayThumbnail=false))
	TScriptInterface<IFVAdvancedCraftingStationInterface> CraftingStation;

protected:

	UFUNCTION(Server, Reliable)
	void Server_LearnRecipe(UFVRecipeTemplate* RecipeTemplate);

	UFUNCTION(Server, Reliable)
	void Server_ForgetRecipe(UFVRecipeTemplate* RecipeTemplate);

	UFUNCTION(Server, Reliable)
	void Server_StartCrafting(UFVRecipeTemplate* TemplateToCraft, UFVRecipeIngredientsList* Ingredients);
	
	UFUNCTION(Server, Reliable)
	void Server_StopCrafting();

	UFUNCTION(Server, Reliable)
	void Server_StartUsingCraftingStation(const TScriptInterface<IFVAdvancedCraftingStationInterface>& Station);

	UFUNCTION(Server, Reliable)
	void Server_StopUsingCraftingStation();

	UFUNCTION(Client, Unreliable)
	void PostRecipeLearned_Client(UFVRecipeTemplate* RecipeTemplate);

	UFUNCTION(Client, Unreliable)
	void PostRecipeForgotten_Client(UFVRecipeTemplate* RecipeTemplate);

	UFUNCTION(Client, Unreliable)
	void PostCraftingFinished_Client(const FFVCraftingResult& Result);
	
	UFUNCTION(Client, Unreliable)
	void PostCraftingStarted_Client(const TScriptInterface<IFVAdvancedCraftingStationInterface>& OptionalCraftingStation);
	
	UFUNCTION(Client, Unreliable)
	void PostCraftingStopped_Client(const TScriptInterface<IFVAdvancedCraftingStationInterface>& OptionalCraftingStation);
};
