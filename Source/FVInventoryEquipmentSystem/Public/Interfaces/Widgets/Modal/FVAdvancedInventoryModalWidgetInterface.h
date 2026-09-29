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
#include "FVAdvancedInventoryModalWidgetInterface.generated.h"

class UUserWidget;
class UFVJsonObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModalContentAddedToModalWindow, UUserWidget*, ModalContentWidget, UFVJsonObject*, Payload);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModalConfirmed, UObject*, Payload);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnModalCancelled);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FFVModalContentAddedToModalWindowBinding, UUserWidget*, ModalContentWidget, UFVJsonObject*, Payload);
DECLARE_DYNAMIC_DELEGATE_OneParam(FFVModalConfirmedBinding, UObject*, Payload);
DECLARE_DYNAMIC_DELEGATE(FFVModalCancelledBinding);

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UFVAdvancedInventoryModalWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

class FVINVENTORYEQUIPMENTSYSTEM_API IFVAdvancedInventoryModalWidgetInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|UI|Modal")
	void AddModalContentToModalWindow(UUserWidget* ModalContentWidget, UFVJsonObject* Payload);
	virtual void AddModalContentToModalWindow_Implementation(UUserWidget* ModalContentWidget, UFVJsonObject* Payload) = 0;

	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|UI|Modal")
	void ModalContentConfirmed(UObject* Payload);
	virtual void ModalContentConfirmed_Implementation(UObject* Payload) = 0;

	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|UI|Modal")
	void ModalContentCancelled();
	virtual void ModalContentCancelled_Implementation() = 0;
	
	virtual FOnModalContentAddedToModalWindow& GetOnModalContentAddedToModalWindowHandle() = 0;
	virtual FOnModalConfirmed& GetOnModalConfirmedHandle() = 0;
	virtual FOnModalCancelled& GetOnModalCancelledHandle() = 0;
};
