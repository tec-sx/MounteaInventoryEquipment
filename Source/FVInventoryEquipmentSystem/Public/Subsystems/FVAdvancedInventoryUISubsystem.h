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
#include "Subsystems/LocalPlayerSubsystem.h"
#include "FVAdvancedInventoryUISubsystem.generated.h"

class IFVAdvancedInventoryUIManagerInterface;

/**
 * 
 */
UCLASS(ClassGroup=(FV),
	meta=(DisplayName="FV Inventory UI Subsystem"))
class FVINVENTORYEQUIPMENTSYSTEM_API UFVAdvancedInventoryUISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	

	/**
	 * Registers an Inventory UI Manager instance.
	 *
	 * Stores the provided object as the active Inventory UI Manager used
	 * to control inventory-related user interface logic.
	 *
	 * @param NewInventoryUIManager Object instance to register as the Inventory UI Manager.
	 */
	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|UI|Inventory|Subsystem",
		meta=(FVSetter),
		DisplayName="Register Inventory UI Manager")
	void RegisterInventoryUIManager(UObject* NewInventoryUIManager);

	/**
	 * Unregisters the currently registered Inventory UI Manager.
	 *
	 * If the provided object matches the currently registered Inventory UI Manager,
	 * it will be cleared and no longer used by the inventory system.
	 *
	 * @param DirtyInventoryUIManager Inventory UI Manager instance to unregister.
	 */
	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|UI|Inventory|Subsystem",
		meta=(FVSetter),
		DisplayName="Unregister Inventory UI Manager")
	void UnregisterInventoryUIManager(const UObject* DirtyInventoryUIManager);

	/**
	 * Returns the currently registered Inventory UI Manager.
	 *
	 * @return The active Inventory UI Manager instance, or nullptr if none is registered.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FV|Inventory & Equipment|UI|Inventory|Subsystem",
		meta=(FVGetter),
		DisplayName="Get Inventory UI Manager")
	UObject* GetInventoryUIManager() const;
	
	/**
	 * Returns the currently registered Inventory UI Manager.
	 *
	 * @return The active Inventory UI Manager instance, or nullptr if none is registered.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FV|Inventory & Equipment|UI|Inventory|Subsystem",
		meta=(FVGetter),
		DisplayName="Get Inventory UI Manager")
	TScriptInterface<IFVAdvancedInventoryUIManagerInterface> GetInventoryUIManager_Interface() const;


private:

	// Reference to UI Manager, usually lives withing Player Pawn.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Managers",
		meta=(AllowPrivateAccess))
	TScriptInterface<IFVAdvancedInventoryUIManagerInterface> InventoryUIManager;
};
