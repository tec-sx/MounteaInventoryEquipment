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
#include "Definitions/FVInventoryBaseDataTypes.h"
#include "Engine/DeveloperSettings.h"
#include "Logs/FVInventoryEquipmentSystemLogTypes.h"
#include "FVAdvancedInventorySettings.generated.h"

class UFVAdvancedInventoryGlobalUIConfig;
class UFVAdvancedInventoryGlobalConfig;
class UFVAdvancedInventoryUIConfig;
class UFVAdvancedEquipmentSettingsConfig;
class UInputMappingContext;
class UFVAdvancedInventorySettingsConfig;
class UFVAdvancedCraftingConfig;
class UFVAdvancedCraftingUIConfig;

/**
 * UFVAdvancedInventorySettings provides centralized configuration for the FV Advanced Inventory System.
 * Settings control input mappings, logging verbosity, asset references, and system-wide behavior
 * through Unreal's developer settings framework for project-wide inventory configuration.
 *
 * @see System Settings
 * @see UFVAdvancedInventorySettingsConfig
 * @see UDeveloperSettings
 */
UCLASS(config = FVSettings, defaultconfig, meta = (DisplayName = "FV Inventory System Settings"))
class FVINVENTORYEQUIPMENTSYSTEM_API UFVAdvancedInventorySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UFVAdvancedInventorySettings();

public:

#if WITH_EDITOR
	virtual FText GetSectionText() const override
	{
		return NSLOCTEXT("FVInventoryEquipmentSystem", "FVSettingsDescription", "FV Inventory System");
	}

	virtual FText GetSectionDescription() const override
	{
		return NSLOCTEXT("FVInventoryEquipmentSystem", "FVSettingsDescription", "Default values for FV Advanced Inventory Plugin.");
	}

	virtual FName GetContainerName() const override
	{
		return "Project";
	}
#endif

public:
	
	/**
	 * Represents a soft reference to an input mapping context used specifically for managing
	 * equipment-related input within the FV Advanced Inventory system.
	 * This can be configured to define custom input actions for inventory equipment handling.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Config", 
		meta=(ForceShowPluginContent),
		meta=(ForceShowEngineContent))
	TSoftObjectPtr<UInputMappingContext> AdvancedInventoryEquipmentInputMapping;

	/**
	 * Holds a reference to the configuration settings for FV Advanced Inventory.
	 * This reference is used to define and load inventory-specific configurations.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Config", 
		meta=(ForceShowPluginContent),
		meta=(ForceShowEngineContent))
	TSoftObjectPtr<UFVAdvancedInventorySettingsConfig> AdvancedInventorySettingsConfig;

	/**
	 * Holds a reference to the configuration settings for FV Advanced Equipment.
	 * This reference is used to define and load equipment-specific configurations.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Config", 
		meta=(ForceShowPluginContent),
		meta=(ForceShowEngineContent))
	TSoftObjectPtr<UFVAdvancedEquipmentSettingsConfig> AdvancedEquipmentSettingsConfig;
	
	/**
	 * Holds a reference to the configuration settings for FV Advanced Inventory UI.
	 * This reference is used to define and load inventory-specific configurations.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Config", 
		meta=(ForceShowPluginContent),
		meta=(ForceShowEngineContent),
		DisplayName="Advanced Inventory UI Settings Config")
	TSoftObjectPtr<UFVAdvancedInventoryUIConfig> AdvancedInventoryUISettingsConfig;

	/**
	 * Holds a reference to the configuration settings for FV Advanced Crafting.
	 * This reference is used to define and load crafting-specific configurations.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Config", 
		meta=(ForceShowPluginContent),
		meta=(ForceShowEngineContent))
	TSoftObjectPtr<UFVAdvancedCraftingConfig> AdvancedCraftingSettingsConfig;
	
	/**
	 * Holds a reference to the configuration settings for FV Advanced Crafting.
	 * This reference is used to define and load crafting-specific configurations.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Config", 
		meta=(ForceShowPluginContent),
		meta=(ForceShowEngineContent))
	TSoftObjectPtr<UFVAdvancedCraftingUIConfig> AdvancedCraftingUISettingsConfig;

	/**
	 * Holds a reference to the configuration settings for FV Advanced Inventory & Equipment Global UI.
	 * This reference is used to define and load global UI specific configurations.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Config", 
		meta=(ForceShowPluginContent),
		meta=(ForceShowEngineContent))
	TSoftObjectPtr<UFVAdvancedInventoryGlobalUIConfig> GlobalUIConfig;

	/**
	 * Holds a reference to the global configuration asset for shared Inventory & Equipment definitions.
	 * This includes named JSON object definitions used by JSON generation and validation helpers.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Config",
		meta=(ForceShowPluginContent),
		meta=(ForceShowEngineContent))
	TSoftObjectPtr<UFVAdvancedInventoryGlobalConfig> GlobalConfig;
	
	/**
	 * Defines logging level that is allowed to be shown.
	 * Affects on-screen messages.
	 */
	UPROPERTY(config, EditDefaultsOnly, Category = "Logging", 
		meta=(Bitmask, BitmaskEnum="/Script/FVInventoryEquipmentSystem.EFVAdvancedInventoryLoggingVerbosity"))
	uint8 LogVerbosity;

public:
	
	/**
	 * Retrieves the allowed logging verbosity level from the FV Advanced Inventory settings.
	 * This level determines which messages are permitted to be displayed, helping to manage verbosity levels for logging purposes.
	 *
	 * @return The allowed logging verbosity as an enum value of EFVAdvancedInventoryLoggingVerbosity.
	 */
	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|Inventory|Settings",  meta=(FVGetter))
	EFVAdvancedInventoryLoggingVerbosity GetAllowedLoggVerbosity() const;

	/**
	 * Retrieves the list of allowed rarities for inventory items as defined in the FV Advanced Inventory settings.
	 * If no valid settings configuration is found, a default rarity of "Common" is returned.
	 *
	 * @return An array of strings representing the allowed item rarities.
	 */
	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|Inventory|Settings",  meta=(FVGetter))
	TMap<FString, FInventoryRarity> GetAllowedRarities() const;

	/**
	 * Retrieves the list of allowed categories for inventory items as defined in the FV Advanced Inventory settings.
	 * If no valid settings configuration is found, a default category of "Miscellaneous" is returned.
	 *
	 * @return An list of strings representing the allowed inventory categories.
	 */
	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|Inventory|Settings",  meta=(FVGetter))
	TMap<FString, FInventoryCategory> GetAllowedCategories() const;

	/**
	 * Retrieves the input mapping context used for handling equipment-related actions
	 * within the FV Advanced Inventory system.
	 *
	 * @return A soft object pointer to the input mapping context for inventory equipment management.
	 */
	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|Inventory|Settings",  meta=(FVGetter))
	TSoftObjectPtr<UInputMappingContext> GetAdvancedInventoryEquipmentInputMapping() const;
};
