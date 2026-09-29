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
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "FVAdvancedCraftingStationInterface.generated.h"

class IFVAdvancedCraftingParticipantInterface;
enum class ECraftingStationState : uint8;

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UFVAdvancedCraftingStationInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FVINVENTORYEQUIPMENTSYSTEM_API IFVAdvancedCraftingStationInterface
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	bool CanBeUsed() const;
	virtual bool CanBeUsed_Implementation() const = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	bool StartUsing(const TScriptInterface<IFVAdvancedCraftingParticipantInterface>& Participant);
	virtual bool StartUsing_Implementation(const TScriptInterface<IFVAdvancedCraftingParticipantInterface>& Participant) = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	bool StopUsing(const TScriptInterface<IFVAdvancedCraftingParticipantInterface>& Participant);
	virtual bool StopUsing_Implementation(const TScriptInterface<IFVAdvancedCraftingParticipantInterface>& Participant) = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	FGameplayTag GetCraftingPlaceType() const;
	virtual FGameplayTag GetCraftingPlaceType_Implementation() const = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	bool IsCraftingPlaceOccupied() const;
	virtual bool IsCraftingPlaceOccupied_Implementation() const = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	int32 GetCraftingPlaceCapacity() const;
	virtual int32 GetCraftingPlaceCapacity_Implementation() const = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	ECraftingStationState GetCraftingStationState() const;
	virtual ECraftingStationState GetCraftingStationState_Implementation() const = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Crafting")
	bool SetCraftingStationState(const ECraftingStationState NewCraftingStationState);
	virtual bool SetCraftingStationState_Implementation(const ECraftingStationState NewCraftingStationState) = 0;
	
	
};
