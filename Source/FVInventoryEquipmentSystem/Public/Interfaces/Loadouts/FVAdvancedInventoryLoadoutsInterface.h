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
#include "FVAdvancedInventoryLoadoutsInterface.generated.h"

class UFVAdvancedInventoryLoadoutConfig;
class UFVAdvancedInventoryLoadoutItem;

UINTERFACE(MinimalAPI, Blueprintable, BlueprintType)
class UFVAdvancedInventoryLoadoutsInterface : public UInterface
{
	GENERATED_BODY()
};

class FVINVENTORYEQUIPMENTSYSTEM_API IFVAdvancedInventoryLoadoutsInterface
{
	GENERATED_BODY()
	
public:
	
	/**
	 * Executes loadout processing for this component.
	 *
	 * Adds configured loadout items to inventory and attempts auto-equip for eligible entries.
	 * On non-authority instances this routes execution to server RPC.
	 *
	 * @return True if processing was started/completed, otherwise false.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Loadouts")
	bool LoadLoadout();
	virtual bool LoadLoadout_Implementation() = 0;
	
	/**
	 * Returns the currently configured loadout asset.
	 *
	 * @return Resolved loadout configuration, or nullptr if not set.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Loadouts")
	UFVAdvancedInventoryLoadoutConfig* GetLoadout() const;
	virtual UFVAdvancedInventoryLoadoutConfig* GetLoadout_Implementation() const = 0;
	
	/**
	 * Returns all loadout entries from the configured loadout asset.
	 *
	 * @return Array of loadout items, or empty array when configuration is invalid.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Loadouts")
	TArray<UFVAdvancedInventoryLoadoutItem*> GetLoadoutItems() const;
	virtual TArray<UFVAdvancedInventoryLoadoutItem*> GetLoadoutItems_Implementation() const = 0;
	
	/**
	 * Indicates whether the loadout should be applied automatically on BeginPlay.
	 *
	 * @return True if automatic loading is enabled.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Loadouts")
	bool ShouldLoadAutomatically() const;
	virtual bool ShouldLoadAutomatically_Implementation() const = 0;
	
	/**
	 * Updates automatic load behavior.
	 *
	 * @param bNewValue New automatic load flag value.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Loadouts")
	void SetAutomaticLoad(const bool bNewValue);
	virtual void SetAutomaticLoad_Implementation(const bool bNewValue) = 0;
};
