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
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVCraftingUIStatics.generated.h"

class IFVAdvancedCraftingParticipantInterface;
class IFVAdvancedCraftingParticipantUIInterface;
class UFVAdvancedCraftingUIConfig;
class UFVAdvancedCraftingUISubsystem;
class UUserWidget;

/**
 * 
 */
UCLASS()
class FVINVENTORYEQUIPMENTSYSTEM_API UFVCraftingUIStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	static bool IsValidCraftingUIHandler(const UObject* Target);

	/**
	 * Retrieves the crafting UI subsystem associated with the provided context.
	 *
	 * This function determines the player controller tied to the given context and returns
	 * the associated FV advanced crafting UI subsystem if available.
	 *
	 * @param Context The context object used to locate the player controller. This can be an actor,
	 *                actor component, or user widget. If invalid, the function returns nullptr.
	 * @return A pointer to the UFVAdvancedCraftingUISubsystem if found; otherwise, nullptr.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FV|Inventory & Equipment|UI|Subsystems", 
		meta=(FVGetter),
		meta=(DefaultToSelf="Context"),
		meta=(CompactNodeTitle="Crafting UI Subsystem"),
		DisplayName="Get Crafting UI Subsystem")
	static UFVAdvancedCraftingUISubsystem* GetCraftingUISubsystem(UObject* Context);
		
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|Config", 
		meta=(FVGetter),
		DisplayName="Get Crafting UI Config")
	static UFVAdvancedCraftingUIConfig* GetCraftingUISettingsConfig();
	
	// --- Crafting Participant

	/**
	 * Retrieves the parent crafting participant associated with a specified object.
	 *
	 * This function checks if the given object is valid and implements the
	 * FVAdvancedCraftingParticipantUIInterface. If so, it retrieves the parent crafting participant from the object.
	 *
	 * @param Target The object for which to retrieve the parent crafting participant.
	 * @return A TScriptInterface of IFVAdvancedCraftingParticipantInterface
	 *         if a parent crafting participant exists, or nullptr if the object is invalid
	 *         or does not implement the required interface.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FV|Inventory & Equipment|UI|Manager", 
		meta=(FVGetter),
		meta=(DefaultToSelf="Target"),
		DisplayName="Get Parent Crafting Participant")
	static TScriptInterface<IFVAdvancedCraftingParticipantInterface> GetParentCraftingParticipant(const UObject* Target);
	
	/**
	 * Retrieves the parent crafting participant associated with a specified object.
	 *
	 * This function checks if the given object is valid and implements the
	 * FVAdvancedCraftingParticipantUIInterface. If so, it retrieves the parent crafting participant from the object.
	 *
	 * @param Target The object for which to retrieve the parent crafting participant.
	 * @return A UObject of IFVAdvancedCraftingParticipantInterface
	 *         if a parent crafting participant exists, or nullptr if the object is invalid
	 *         or does not implement the required interface.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FV|Inventory & Equipment|UI|Manager", 
		meta=(FVGetter),
		meta=(DefaultToSelf="Target"),
		DisplayName="Get Parent Crafting Participant")
	static UObject* GetParentCraftingParticipantObject(const UObject* Target);

	/**
	 * Attempts to associate a crafting participant with a target object that implements the required interface.
	 *
	 * This function verifies the validity of the provided target object and checks if it implements the
	 * UFVAdvancedCraftingParticipantUIInterface. If the target is valid and compatible, it sets the crafting participant for the target object.
	 *
	 * @param Target The object to which the crafting participant will be assigned. Must implement UFVAdvancedCraftingParticipantUIInterface.
	 * @param Participant The crafting participant to be associated with the target.
	 * @return True if the crafting participant was successfully set; otherwise, false.
	 */
	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|UI|Manager",
		meta=(FVSetter),
		meta=(DefaultToSelf="Target"),
		DisplayName="Set Parent Crafting Participant")
	static bool SetParentCraftingParticipant(UObject* Target, const TScriptInterface<IFVAdvancedCraftingParticipantInterface>& Participant);
	
/**
* CRAFTING UI
*/
	
	/**
	 * Creates and initializes the Crafting widget inside the wrapper.
	 *
	 * @return True if the Crafting widget was created successfully, otherwise false.
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|UI",
		meta=(FVSetter),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Create Crafting Widget")
	static bool CreateCraftingWidget(const TScriptInterface<IFVAdvancedCraftingParticipantUIInterface>& Target);
	
	/**
	 * Returns the currently active Crafting widget.
	 *
	 * @return Crafting widget if available, otherwise nullptr.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FV|Inventory & Equipment|Crafting|UI",
		meta=(FVGetter),
		DisplayName="Get Crafting Widget")
	static UUserWidget* GetCraftingWidget(const TScriptInterface<IFVAdvancedCraftingParticipantUIInterface>& Target);

	/**
	 * Sets the Crafting UI from outside.
	 * @return True if UI was successfully set, otherwise false.
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|UI",
		meta=(FVSetter),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		DisplayName="Set Crafting Widget")
	static bool SetCraftingWidget(const TScriptInterface<IFVAdvancedCraftingParticipantUIInterface>& Target, UUserWidget* NewCraftingWidget);

	/**
	 * Removes and cleans up the Crafting widget.
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|UI",
		meta=(FVSetter),
		DisplayName="Remove Crafting Widget")
	static void RemoveCraftingWidget(const TScriptInterface<IFVAdvancedCraftingParticipantUIInterface>& Target);

	/**
	 * Retrieves the currently selected Crafting category ID.
	 * 
	 * @param Target The Crafting Participant UI object to retrieve the activecategory from.
	 * @return The ID of the currently selected Crafting category.
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|UI",
		meta=(FVGetter),
		DisplayName="Get Selected Crafting Category")
	static FString GetCraftingSelectedCategoryId(UObject* Target);
	
	/**
	 * Handles the selection of a Crafting category in the Crafting UI.
	 * 
	 * @param Target The Crafting Participant UI object.
	 * @param SelectedCategoryId The ID of the selected category.
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|UI",
		meta=(FVSetter),
		DisplayName="Select Crafting Category")
	static bool CraftingCategorySelected(UObject* Target, const FString& SelectedCategoryId);

	/**
	 * Retrieves the currently active Craftable item GUID.
	 *
	 * @param Target The Crafting Participan tUI object to retrieve the active craftable item's GUID from.
	 * @return  The GUID of the currently active Craftable item.
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|UI",
		meta=(FVGetter),
		DisplayName="Get Active Craftable Item Guid")
	static FGuid GetActiveCraftableItemGuid_Implementation(UObject* Target);
	
	/**
	 * Handles the selection of a Craftable item in the Crafting UI.
	 * 
	 * @param Target The Crafting Participant UI object.
	 * @param SelectedItem The GUID of the selected Craftable item.
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory & Equipment|Crafting|UI",
		meta=(FVSetter),
		DisplayName="Select Craftable Item")
	static bool CraftableItemSelected(UObject* Target, const FGuid& SelectedItem);
};
