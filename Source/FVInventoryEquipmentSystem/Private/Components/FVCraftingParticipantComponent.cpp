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


#include "Components/FVCraftingParticipantComponent.h"

#include "Net/UnrealNetwork.h"
#include "Definitions/FVCraftingBaseDataTypes.h"
#include "Definitions/FVCraftingBaseEnums.h"
#include "Definitions/FVRecipeTemplate.h"
#include "Interfaces/Crafting/FVAdvancedCraftingStationInterface.h"
#include "Interfaces/Inventory/FVAdvancedInventoryInterface.h"
#include "Logs/FVAdvancedInventoryLog.h"
#include "Statics/FVCraftingStatics.h"

UFVCraftingParticipantComponent::UFVCraftingParticipantComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	UFVCraftingParticipantComponent::SetComponentTickEnabled(false);

	bAutoActivate = true;

	SetIsReplicatedByDefault(true);
	SetActiveFlag(true);

	ComponentTags.Append( { TEXT("FV"), TEXT("Crafting") } );
}

void UFVCraftingParticipantComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeInventoryAndEquipment();
}

void UFVCraftingParticipantComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UFVCraftingParticipantComponent, KnownRecipes, COND_InitialOrOwner);
	DOREPLIFETIME_CONDITION(UFVCraftingParticipantComponent, CraftingStation, COND_InitialOrOwner);
}

void UFVCraftingParticipantComponent::OnRep_KnownRecipes()
{
}

void UFVCraftingParticipantComponent::OnRep_CraftingStation()
{
}

void UFVCraftingParticipantComponent::InitializeInventoryAndEquipment()
{
	auto inventoryComponent = GetOwner()->FindComponentByInterface(UFVAdvancedInventoryInterface::StaticClass());
	if (!IsValid(inventoryComponent))
		LOG_ERROR(TEXT("[FVInventoryUIComponent] Cannot find 'Inventory' component in Parent! Loadouts will NOT work!"))
	else
	{
		Execute_SetParentInventory(this, inventoryComponent);
		ensureMsgf(RelatedInventory.GetObject() != nullptr, TEXT("[FVAdvancedInventoryLoadoutComponent] Failed to update 'RelatedInventory'"));
	}
}

TArray<UFVRecipeTemplate*> UFVCraftingParticipantComponent::GetKnownRecipes_Implementation() const
{
	return ResolveKnownRecipeTemplates(KnownRecipes);
}

TArray<UFVRecipeTemplate*> UFVCraftingParticipantComponent::GetRecipes_Implementation(const FGameplayTag& CraftingStationType) const
{
	const TArray<UFVRecipeTemplate*> knownRecipes = ResolveKnownRecipeTemplates(KnownRecipes);
	if (knownRecipes.Num() == 0)
		return TArray<UFVRecipeTemplate*>();

	TArray<UFVRecipeTemplate*> result;
	result.Reserve(knownRecipes.Num());

	for (UFVRecipeTemplate* recipe : knownRecipes)
	{
		if (!IsValid(recipe))
			continue;

		if (!CraftingStationType.IsValid() || recipe->RequiredCraftingPlace == CraftingStationType)
			result.Add(recipe);
	}

	return result;
}

UFVRecipeTemplate* UFVCraftingParticipantComponent::GetRecipe_Implementation(const FGuid& RecipeGuid) const
{
	if (!KnownRecipes.Contains(RecipeGuid))
		return nullptr;

	const TArray<UFVRecipeTemplate*> knownRecipes = ResolveKnownRecipeTemplates(KnownRecipes);
	UFVRecipeTemplate* const* foundRecipe = knownRecipes.FindByPredicate(
		[&RecipeGuid](const UFVRecipeTemplate* recipe)
		{
			return IsValid(recipe) && recipe->RecipeGuid == RecipeGuid;
		});

	return foundRecipe != nullptr ? *foundRecipe : nullptr;
}

bool UFVCraftingParticipantComponent::IsRecipeKnown_Implementation(UFVRecipeTemplate* RecipeTemplate) const
{
	return KnownRecipes.Contains(RecipeTemplate->RecipeGuid);
}

bool UFVCraftingParticipantComponent::LearnRecipe_Implementation(UFVRecipeTemplate* RecipeTemplate)
{
	if (Execute_IsRecipeKnown(this, RecipeTemplate))
		return false;

	if (!GetOwner()->HasAuthority())
	{
		Server_LearnRecipe(RecipeTemplate);
		return true;
	}

	KnownRecipes.Add(RecipeTemplate->RecipeGuid);
	PostRecipeLearned_Client(RecipeTemplate);
	OnRecipeLearned.Broadcast(RecipeTemplate);
	return true;
}

bool UFVCraftingParticipantComponent::ForgetRecipe_Implementation(UFVRecipeTemplate* RecipeTemplate)
{
	if (!Execute_IsRecipeKnown(this, RecipeTemplate))
		return false;

	if (!GetOwner()->HasAuthority())
	{
		Server_ForgetRecipe(RecipeTemplate);
		return true;
	}

	KnownRecipes.Remove(RecipeTemplate->RecipeGuid);
	PostRecipeForgotten_Client(RecipeTemplate);
	OnRecipeForgotten.Broadcast(RecipeTemplate);
	return true;
}

bool UFVCraftingParticipantComponent::IsCraftingPossible_Implementation(UFVRecipeTemplate* TemplateToCraft) const
{
	return Execute_IsRecipeKnown(this, TemplateToCraft);
}

FFVCraftingResult UFVCraftingParticipantComponent::StartCrafting_Implementation(UFVRecipeTemplate* TemplateToCraft, UFVRecipeIngredientsList* Ingredients)
{
	if (!GetOwner()->HasAuthority())
	{
		Server_StartCrafting(TemplateToCraft, Ingredients);
		return FFVCraftingResult{};
	}

	OnCraftingStarted.Broadcast(CraftingStation);
	PostCraftingStarted_Client(CraftingStation);
	FFVCraftingResult result = UFVCraftingStatics::CraftItem(this, TemplateToCraft, Ingredients);
	
	// TODO: Add possible delay IF crafting recipe takes time
	
	PostCraftingFinished_Client(result);
	OnCraftingFinished.Broadcast(result);
	return result;
}

void UFVCraftingParticipantComponent::StopCrafting_Implementation()
{
	if (!GetOwner()->HasAuthority())
	{
		Server_StopCrafting();
		return;
	}
	
	PostCraftingStopped_Client(CraftingStation);
	OnCraftingStopped.Broadcast(CraftingStation);
	
	if (CraftingStation.GetObject())
		Execute_StopUsingCraftingStation(this);
	
	CraftingStation = nullptr;
	return;
}

bool UFVCraftingParticipantComponent::SetParentInventory_Implementation(const TScriptInterface<IFVAdvancedInventoryInterface>& NewParentInventory)
{
	if (RelatedInventory == NewParentInventory)
		return false;
	RelatedInventory = NewParentInventory;
	return true;
}

bool UFVCraftingParticipantComponent::StartUsingCraftingStation_Implementation(const TScriptInterface<IFVAdvancedCraftingStationInterface>& Station)
{
	if (!IsValid(Station.GetObject()))
		return false;
	if (Station == CraftingStation)
		return false;

	if (!UFVCraftingStatics::DoesHaveAnyRecipes(this))
		return false;
	
	if (!GetOwner()->HasAuthority())
	{
		Server_StartUsingCraftingStation(Station);
		return true;
	}

	CraftingStation = Station;
	IFVAdvancedCraftingStationInterface::Execute_StartUsing(Station.GetObject(), this);
	OnCraftingStarted.Broadcast(CraftingStation);
	PostCraftingStarted_Client(CraftingStation);
	return true;
}

bool UFVCraftingParticipantComponent::StopUsingCraftingStation_Implementation()
{
	if (!IsValid(CraftingStation.GetObject()))
		return false;

	if (!GetOwner()->HasAuthority())
	{
		Server_StopUsingCraftingStation();
		return true;
	}

	PostCraftingStopped_Client(CraftingStation);
	OnCraftingStopped.Broadcast(CraftingStation);
	IFVAdvancedCraftingStationInterface::Execute_StopUsing(CraftingStation.GetObject(), this);
	CraftingStation = nullptr;
	return true;
}

TArray<UFVRecipeTemplate*> UFVCraftingParticipantComponent::ResolveKnownRecipeTemplates(const TArray<FGuid>& KnownRecipeGuids)
{
	if (KnownRecipeGuids.Num() == 0)
		return TArray<UFVRecipeTemplate*>();

	const TArray<UFVRecipeTemplate*> allRecipes = UFVCraftingStatics::GetAllRecipeTemplates().Array();
	if (allRecipes.Num() == 0)
		return TArray<UFVRecipeTemplate*>();

	TArray<UFVRecipeTemplate*> result;
	result.Reserve(KnownRecipeGuids.Num());

	const auto allRecipesArray = allRecipes;
	for (const FGuid& knownRecipeGuid : KnownRecipeGuids)
	{
		UFVRecipeTemplate* const* foundRecipe = allRecipesArray.FindByPredicate(
			[&knownRecipeGuid](const UFVRecipeTemplate* recipe)
			{
				return IsValid(recipe) && recipe->RecipeGuid == knownRecipeGuid;
			});

		if (foundRecipe != nullptr && IsValid(*foundRecipe))
			result.Add(*foundRecipe);
	}

	return result;
}

void UFVCraftingParticipantComponent::Server_StopCrafting_Implementation()
{
	Execute_StopCrafting(this);
}

void UFVCraftingParticipantComponent::PostCraftingStopped_Client_Implementation(const TScriptInterface<IFVAdvancedCraftingStationInterface>& OptionalCraftingStation)
{
	OnCraftingStopped.Broadcast(OptionalCraftingStation);
}

void UFVCraftingParticipantComponent::PostCraftingStarted_Client_Implementation(const TScriptInterface<IFVAdvancedCraftingStationInterface>& OptionalCraftingStation)
{
	OnCraftingStarted.Broadcast(OptionalCraftingStation);
}

void UFVCraftingParticipantComponent::Server_LearnRecipe_Implementation(UFVRecipeTemplate* RecipeTemplate)
{
	Execute_LearnRecipe(this, RecipeTemplate);
}

void UFVCraftingParticipantComponent::Server_ForgetRecipe_Implementation(UFVRecipeTemplate* RecipeTemplate)
{
	Execute_ForgetRecipe(this, RecipeTemplate);
}

void UFVCraftingParticipantComponent::Server_StartCrafting_Implementation(UFVRecipeTemplate* TemplateToCraft, UFVRecipeIngredientsList* Ingredients)
{
	Execute_StartCrafting(this, TemplateToCraft, Ingredients);
}

void UFVCraftingParticipantComponent::Server_StartUsingCraftingStation_Implementation(const TScriptInterface<IFVAdvancedCraftingStationInterface>& Station)
{
	Execute_StartUsingCraftingStation(this, Station);
}

void UFVCraftingParticipantComponent::Server_StopUsingCraftingStation_Implementation()
{
	Execute_StopUsingCraftingStation(this);
}

void UFVCraftingParticipantComponent::PostRecipeLearned_Client_Implementation(UFVRecipeTemplate* RecipeTemplate)
{
	OnRecipeLearned.Broadcast(RecipeTemplate);
}

void UFVCraftingParticipantComponent::PostRecipeForgotten_Client_Implementation(UFVRecipeTemplate* RecipeTemplate)
{
	OnRecipeForgotten.Broadcast(RecipeTemplate);
}

void UFVCraftingParticipantComponent::PostCraftingFinished_Client_Implementation(const FFVCraftingResult& Result)
{
	OnCraftingFinished.Broadcast(Result);
}
