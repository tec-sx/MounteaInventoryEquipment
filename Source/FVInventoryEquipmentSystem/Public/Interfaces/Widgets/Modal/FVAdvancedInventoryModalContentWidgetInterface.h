// Copyright (C) 2026 Dominik (Pavlicek) Morse. All rights reserved.
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
#include "FVAdvancedInventoryModalContentWidgetInterface.generated.h"

class UFVJsonObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModalContentConfirmed, UObject*, Payload);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnModalContentCancelled);
DECLARE_DYNAMIC_DELEGATE_OneParam(FFVModalContentConfirmedBinding, UObject*, Payload);
DECLARE_DYNAMIC_DELEGATE(FFVModalContentCancelledBinding);

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UFVAdvancedInventoryModalContentWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class FVINVENTORYEQUIPMENTSYSTEM_API IFVAdvancedInventoryModalContentWidgetInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|UI|Modal|Content")
	void ConstructModalContent(UFVJsonObject* InPayload);
	virtual void ConstructModalContent_Implementation(UFVJsonObject* InPayload) = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|UI|Modal")
	void StartExpiration(const float WidgetLifetime);
	virtual void StartExpiration_Implementation(const float WidgetLifetime) = 0;

	virtual FOnModalContentConfirmed& GetOnModalContentConfirmedHandle() = 0;
	virtual FOnModalContentCancelled& GetOnModalContentCancelledHandle() = 0;
};
