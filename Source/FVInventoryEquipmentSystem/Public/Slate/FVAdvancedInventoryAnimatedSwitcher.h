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
#include "CommonAnimatedSwitcher.h"

#include "FVAdvancedInventoryAnimatedSwitcher.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAnimationFinished, UWidget*, ActiveWidget, int32, ActiveIndex);

/**
 * UFVAdvancedInventoryAnimatedSwitcher extends UCommonAnimatedSwitcher with enhanced animation event handling.
 * Animated switchers provide smooth transitions between inventory UI panels with completion callbacks
 * for coordinated interface updates and responsive user experience in complex inventory layouts.
 *
 * @see Inventory UI Transitions
 * @see UCommonAnimatedSwitcher
 * @see FOnAnimationFinished
 */
UCLASS()
class FVINVENTORYEQUIPMENTSYSTEM_API UFVAdvancedInventoryAnimatedSwitcher : public UCommonAnimatedSwitcher
{
	GENERATED_BODY()

public:
	
	UFVAdvancedInventoryAnimatedSwitcher(const FObjectInitializer& ObjectInitializer);

protected:

	UFUNCTION()
	void ProcessIndexChanged(UWidget* Widget, int32 Index) const;

private:
	
#if WITH_EDITOR
	
	virtual const FText GetPaletteCategory() override;

#endif

protected:

	UPROPERTY(BlueprintAssignable, Category="AnimatedSwitcher|Event")
	FOnAnimationFinished OnAnimationFinished;
};
