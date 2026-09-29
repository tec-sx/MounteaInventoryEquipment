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
#include "FVInventorySystemWrapperWidgetInterface.generated.h"

struct FGameplayTagContainer;
struct FGameplayTag;
class IFVAdvancedInventoryInterface;
class IFVAdvancedInventorySharedHUDInterface;

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UFVInventorySystemWrapperWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * IFVInventorySystemWrapperWidgetInterface manages main inventory system wrapper widget.
 * System base interfaces handle initialization, visibility management, and inventory source assignment
 * for top-level inventory UI wrappers and container widgets.
 *
 * @see System Base Widgets
 * @see IFVAdvancedInventoryUIManagerInterface
 */
class FVINVENTORYEQUIPMENTSYSTEM_API IFVInventorySystemWrapperWidgetInterface
{
	GENERATED_BODY()

public:
	
	/**
	 * Initializes the Wrapper widget for the FV inventory system.
	 * UI Manager (IFVAdvancedInventoryUIManagerInterface) knows this wrapper/container
	 * and knows it processes all internal logic.
	 * 
	 * This is the main UI screen/container which should handle all UI related
	 * visual logic, like creating Inventory etc.
	 *
	 * @param Parent The parent UI Manager that implements the IFVAdvancedInventoryUIManagerInterface.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|UI|Wrapper")
	void InitializeWrapperWidget(const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Parent);
	virtual void InitializeWrapperWidget_Implementation(const TScriptInterface<IFVAdvancedInventorySharedHUDInterface>& Parent) = 0;

	/**
	 * Removes the Wrapper UI from the viewport and cleans up resources.
	 * This will destroy all child UI elements inside the Main Wrapper!
	 */
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|UI|Wrapper")
	void RemoveWrapperWidget();
	virtual void RemoveWrapperWidget_Implementation() = 0;

	/**
	 * Adds new modal widget to global UI wrapper.
	 * @param ModalWindow Modal window to display.
	 * @return True if modal window is valid and modal can be displayed.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|UI|Wrapper")
	bool AddModalWindow(UUserWidget* ModalWindow);
	virtual bool AddModalWindow_Implementation(UUserWidget* ModalWindow) = 0;

	/**
	 * Clears currently displayed modal window.
	 * @return False if no window is present to be cleared OR something goes wrong.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|UI|Wrapper")
	bool CleanupModalWindow();
	virtual bool CleanupModalWindow_Implementation() = 0;
};
