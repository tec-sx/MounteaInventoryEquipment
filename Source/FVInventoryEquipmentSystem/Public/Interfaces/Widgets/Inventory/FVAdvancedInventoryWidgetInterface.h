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
#include "FVAdvancedInventoryWidgetInterface.generated.h"

class IFVAdvancedInventoryUIManagerInterface;

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UFVAdvancedInventoryWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * IFVAdvancedInventoryWidgetInterface provides core functionality for main inventory display widget.
 * Inventory widget interfaces define the primary inventory display container with comprehensive
 * inventory management and presentation capabilities for the main inventory interface.
 *
 * @see Inventory Widgets
 * @see IFVAdvancedInventoryUIManagerInterface
 */
class FVINVENTORYEQUIPMENTSYSTEM_API IFVAdvancedInventoryWidgetInterface
{
	GENERATED_BODY()

public:

};
