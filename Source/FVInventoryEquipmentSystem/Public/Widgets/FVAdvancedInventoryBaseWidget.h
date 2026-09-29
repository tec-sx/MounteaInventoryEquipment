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
#include "Blueprint/UserWidget.h"
#include "Interfaces/Widgets/FVInventoryGenericWidgetInterface.h"
#include "Interfaces/Widgets/BaseWidget/FVAdvancedBaseInventoryWidgetInterface.h"
#include "FVAdvancedInventoryBaseWidget.generated.h"

/**
 * UFVAdvancedInventoryBaseWidget is the base class for all inventory-related UI widgets.
 * Base widgets provide core functionality including theme application, widget refresh, command processing,
 * and parent UI component management for consistent inventory interface behavior.
 *
 * @see Inventory UI System
 * @see IFVAdvancedInventoryUIManagerInterface
 * @see IFVInventoryGenericWidgetInterface
 */
UCLASS()
class FVINVENTORYEQUIPMENTSYSTEM_API UFVAdvancedInventoryBaseWidget : public UUserWidget,
	public IFVInventoryGenericWidgetInterface,
	public IFVAdvancedBaseInventoryWidgetInterface
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

	virtual void ProcessInventoryWidgetCommand_Implementation(const FString& Command, UObject* OptionalPayload = nullptr) override {};
	virtual void ApplyTheme_Implementation() override {};
	virtual void RefreshWidget_Implementation() override {};
	virtual void ConsumeUIInput_Implementation(const FGameplayTag& InputTag, const FFVWidgetInputPayload& Payload, float DeltaTime) override {};

	virtual void SetOwningInventoryUI_Implementation(const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& NewOwningInventoryUI) override
	{
		if (ParentUIComponent != NewOwningInventoryUI)
			ParentUIComponent = NewOwningInventoryUI;
	};

	virtual TScriptInterface<IFVAdvancedInventoryUIManagerInterface> GetOwningInventoryUI_Implementation() const override
	{ return ParentUIComponent; };
	
	virtual FGameplayTag GetWidgetTag_Implementation() const override { return Tag; };
	virtual void SetWidgetTag_Implementation(const FGameplayTag& WidgetTag) override
	{
		if (WidgetTag != Tag)
			Tag = WidgetTag;
	};

protected:
	
	/** Reference to the owning inventory UI interface.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FV|Primary Data",
		AdvancedDisplay, 
		DisplayName="UI Manager",
		meta=(DisplayThumbnail=false))
	TScriptInterface<IFVAdvancedInventoryUIManagerInterface> ParentUIComponent = nullptr;
	
	/** Defines type of the Widget. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FV|Primary Data",
		AdvancedDisplay, 
		DisplayName="Widget Tag")
	FGameplayTag Tag;
};
