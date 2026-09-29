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
#include "FVAdvancedCraftingUIConfig.generated.h"

/**
 * 
 */
UCLASS(ClassGroup=(FV), BlueprintType, Blueprintable, DisplayName="Crafting UI Settings Config",
	meta=(ShortTooltip="Configuration asset for the FV Inventory System defining crafting UI."))
class FVINVENTORYEQUIPMENTSYSTEM_API UFVAdvancedCraftingUIConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "User Interface",
		meta = (MustImplement = "/Script/FVInventoryEquipmentSystem.FVAdvancedInventoryCraftingWidgetInterface"),
		meta=(NoResetToDefault),
		meta=(ForceShowPluginContent),
		meta=(ForceShowEngineContent))
	TSoftClassPtr<UUserWidget> CraftingUserInterfaceClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "User Interface",
		meta = (MustImplement = "/Script/FVInventoryEquipmentSystem.FVAdvancedInventoryCategoriesWrapperWidgetInterface"),
		meta=(NoResetToDefault),
		meta=(ForceShowPluginContent),
		meta=(ForceShowEngineContent))
	TSoftClassPtr<UUserWidget> CraftingCategoriesUserInterfaceClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "User Interface",
		meta = (MustImplement = "/Script/FVInventoryEquipmentSystem.FVAdvancedInventoryCategoryWidgetInterface"),
		meta=(NoResetToDefault),
		meta=(ForceShowPluginContent),
		meta=(ForceShowEngineContent))
	TSoftClassPtr<UUserWidget> CraftingCategoryUserInterfaceClass;
};
