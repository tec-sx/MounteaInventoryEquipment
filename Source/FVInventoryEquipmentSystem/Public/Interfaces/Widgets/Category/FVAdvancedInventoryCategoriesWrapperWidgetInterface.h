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
#include "FVAdvancedInventoryCategoriesWrapperWidgetInterface.generated.h"

class IFVAdvancedInventoryUIManagerInterface;

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UFVAdvancedInventoryCategoriesWrapperWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * IFVAdvancedInventoryCategoriesWrapperWidgetInterface manages category selection wrapper widgets.
 * Categories wrapper interfaces handle active category tracking and state management
 * for category selection containers within inventory interfaces.
 *
 * @see Category Widgets
 * @see IFVAdvancedInventoryCategoryWidgetInterface
 */
class FVINVENTORYEQUIPMENTSYSTEM_API IFVAdvancedInventoryCategoriesWrapperWidgetInterface
{
	GENERATED_BODY()

public:

	/**
	 * 
	 * @param ActiveCategoryId 
	 */
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|UI|Inventory|Categories")
	void SetActiveCategoryId(const FString& ActiveCategoryId);
	virtual void SetActiveCategoryId_Implementation(const FString& ActiveCategoryId) = 0;

	/**
	 * 
	 * @return 
	 */
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|UI|Inventory|Categories")
	FString GetActiveCategoryId() const;
	virtual FString GetActiveCategoryId_Implementation() const = 0;
};
