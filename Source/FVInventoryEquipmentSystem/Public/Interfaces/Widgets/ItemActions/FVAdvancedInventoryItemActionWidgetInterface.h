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
#include "FVAdvancedInventoryItemActionWidgetInterface.generated.h"

struct FFVItemActionData;

class UWidget;
class UFVSelectableInventoryItemAction;
class IFVAdvancedInventoryUIManagerInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemActionSelected, const UUserWidget*, SelectedItemActionWidget);

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UFVAdvancedInventoryItemActionWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * IFVAdvancedInventoryItemActionWidgetInterface defines individual item action widget functionality.
 * Item action interfaces manage individual actionable buttons and controls for inventory item operations
 * such as use, equip, drop, and other context-specific item interactions.
 *
 * @see Item Actions
 * @see IFVAdvancedInventoryItemActionsContainerWidgetInterface
 */
class FVINVENTORYEQUIPMENTSYSTEM_API IFVAdvancedInventoryItemActionWidgetInterface
{
	GENERATED_BODY()

public:

	/**
	 * Initializes the item action widget with the Initiator UI and item action data.
	 * 
	 * @param ItemAction The item action associated with this widget.
	 * @param SelectedItem Inventory Item which this action affects.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|UI|Inventory|Item Actions")
	void InitializeItemAction(
		const UFVSelectableInventoryItemAction* ItemAction, const FGuid& SelectedItem);
	virtual void InitializeItemAction_Implementation(
		const UFVSelectableInventoryItemAction* ItemAction, const FGuid& SelectedItem) = 0;

	/**
	 * Retrieves the item action associated with this widget.
	 * 
	 * @return The item action data associated with this widget.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|UI|Inventory|Item Actions")
	bool IsActionEnabled() const;
	virtual bool IsActionEnabled_Implementation() const = 0;

	/**
	 * Checks if the item action is valid for execution.
	 * 
	 * @return True if the action is valid, false otherwise.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|UI|Inventory|Item Actions")
	bool IsActionValid() const;
	virtual bool IsActionValid_Implementation() const = 0;

	/**
	 * Executes the item action when triggered by the user.
	 * 
	 * This function is typically called when the user clicks or interacts with the item action button.
	 * It should handle the logic for performing the action, such as using, equipping, or dropping the item.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|UI|Inventory|Item Actions")
	void ExecuteItemAction();
	virtual void ExecuteItemAction_Implementation() = 0;

	/**
	 * Retrieves the item action class associated with this widget.
	 * 
	 * @return The soft class reference to the item action.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|UI|Inventory|Item Actions")
	TSoftClassPtr<UFVSelectableInventoryItemAction> GetItemAction() const;
	virtual TSoftClassPtr<UFVSelectableInventoryItemAction> GetItemAction_Implementation() const = 0;
};
