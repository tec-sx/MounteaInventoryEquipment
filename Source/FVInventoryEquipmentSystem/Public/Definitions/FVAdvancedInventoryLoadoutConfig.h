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
#include "Engine/DataAsset.h"
#include "FVAdvancedInventoryLoadoutConfig.generated.h"

class UFVAdvancedInventoryLoadoutItem;

/**
 * UFVAdvancedInventoryLoadoutConfig stores a predefined set of loadout entries as a data asset.
 * It defines which items should be granted by a loadout component, including per-item setup such as
 * quantity, durability, and optional equipment slot preferences.
 *
 * @see Loadout System
 * @see UFVAdvancedInventoryLoadoutComponent
 * @see UFVAdvancedInventoryLoadoutItem
 */
UCLASS(ClassGroup=(FV), Blueprintable, BlueprintType, 
	meta=(DisplayName = "FV Loadout Configuration"))
class FVINVENTORYEQUIPMENTSYSTEM_API UFVAdvancedInventoryLoadoutConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	
	/**
	 * Unique identifier for this Loadout Config entry, matching the referenced item template's GUID.
	 * Used by the import pipeline to match and update existing entries without loading the template.
	 */
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Configuration",
		meta=(NoResetToDefault))
	FGuid Guid;
	
	/**
	 * Ordered list of item entries that make up this loadout configuration.
	 *
	 * Each entry describes one item to add when the loadout is executed. Entries are instanced inline
	 * for direct editing in the asset and are consumed by UFVAdvancedInventoryLoadoutComponent
	 * during LoadLoadout.
	 */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "Configuration",
		Instanced,
		meta=(NoResetToDefault),
		meta=(TitleProperty="item: {DisplayName} | slot: {EquipmentSlot}"),
		meta=(ShowInnerProperties, ShowOnlyInnerProperties),
		meta=(FullyExpand=true))
	TArray<TObjectPtr<UFVAdvancedInventoryLoadoutItem>> Items;

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
};
