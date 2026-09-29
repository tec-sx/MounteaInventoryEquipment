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
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "Helpers/FVJsonObject.h"
#include "Interfaces/Widgets/FVInventoryGenericWidgetInterface.h"
#include "Interfaces/Widgets/Modal/FVAdvancedInventoryModalContentWidgetInterface.h"
#include "FVAdvancedInventoryModalContentBaseWidget.generated.h"

/**
 * Base widget for modal content driven by a JSON payload.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class FVINVENTORYEQUIPMENTSYSTEM_API UFVAdvancedInventoryModalContentBaseWidget : public UUserWidget,
	public IFVAdvancedInventoryModalContentWidgetInterface, public IFVInventoryGenericWidgetInterface
{
	GENERATED_BODY()

protected:

	UFUNCTION()
	void OnModalExpired();

public:

	virtual void ConstructModalContent_Implementation(UFVJsonObject* InPayload) override;
	virtual FOnModalContentConfirmed& GetOnModalContentConfirmedHandle() override
	{
		return OnModalContentConfirmed;
	};
	virtual FOnModalContentCancelled& GetOnModalContentCancelledHandle() override
	{
		return OnModalContentCancelled;
	};

	virtual void ProcessInventoryWidgetCommand_Implementation(const FString& Command, UObject* OptionalPayload = nullptr) override {};
	virtual void ApplyTheme_Implementation() override {};
	virtual void RefreshWidget_Implementation() override {};
	virtual void ConsumeUIInput_Implementation(const FGameplayTag& InputTag, const FFVWidgetInputPayload& Payload, float DeltaTime) override {};

	virtual FGameplayTag GetWidgetTag_Implementation() const override
	{
		return WidgetTag;
	};
	virtual void SetWidgetTag_Implementation(const FGameplayTag& NewWidgetTag) override
	{
		if (WidgetTag != NewWidgetTag)
			WidgetTag = NewWidgetTag;
	};
	
	virtual void StartExpiration_Implementation(const float WidgetLifetime) override;

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Primary Data")
	TObjectPtr<UFVJsonObject> IncomingPayload;	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Primary Data",
		meta=(NoResetToDefault),
		meta=(Categories="FV_Inventory.WidgetState.Modal,State"))
	FGameplayTag WidgetTag;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Events|Modal Content")
	FOnModalContentConfirmed OnModalContentConfirmed;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Events|Modal Content")
	FOnModalContentCancelled OnModalContentCancelled;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Primary Data")
	FTimerHandle ModalContentExpiryTimerHandle;
};
