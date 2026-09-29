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
#include "Definitions/FVAdvancedInventoryGlobalUIDataTypes.h"
#include "Definitions/FVInventoryBaseUIDataTypes.h"
#include "Engine/DataAsset.h"
#include "FVAdvancedInventoryGlobalUIConfig.generated.h"

class UMaterialInterface;
class UDataTable;
class UUserWidget;

/**
 * UFVAdvancedInventoryGlobalUIConfig manages global UI configuration for 
 * the FV Advanced Inventory & Equipment System.
 *
 * @see UI Configuration
 * @see UFVAdvancedInventorySettings
 */
UCLASS(ClassGroup=(FV), BlueprintType, Blueprintable, DisplayName="Global UI Config",
	meta=(ShortTooltip="Configuration asset for the FV Inventory & Equipment System UI."))
class FVINVENTORYEQUIPMENTSYSTEM_API UFVAdvancedInventoryGlobalUIConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UFVAdvancedInventoryGlobalUIConfig();

public:

	// --- UI Inputs

	/** Discrete UI actions (Close, Confirm, ContextMenu, NextTab, PreviousTab, etc.). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inputs",
		meta=(ShowOnlyInnerProperties),
		meta=(TitleProperty="ActionTag"),
		meta=(NoResetToDefault),
		DisplayName="Action Mappings for UI")
	TArray<FFVWidgetInputActionMapping> UIActionMappings;

	/** Unified deadzone used for Analog and Wheel inputs. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inputs|Config",
		meta=(NoResetToDefault),
		meta=(UIMin=0.f, ClampMin=0.f))
	float InputDeadzone = 0.1f;

	// --- Wrapper

	/** Widget class used as the main user interface wrapper (root HUD panel, container for other widgets,
	 * like Inventory, Equipment, Crafting menu etc.). This Wrapper should be used for other UI systems to
	 * avoid Z-oder fighting.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wrapper",
		meta = (MustImplement = "/Script/FVInventoryEquipmentSystem.FVInventorySystemWrapperWidgetInterface"),
		meta=(NoResetToDefault),
		meta=(ForceShowPluginContent),
		meta=(ForceShowEngineContent))
	TSoftClassPtr<UUserWidget> UserInterfaceWrapperClass;

	// ---- Notifications

	/** Widget class that acts as a container for all inventory notifications (list / stack of notification widgets). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Notifications",
		meta = (MustImplement = "/Script/FVInventoryEquipmentSystem.FVInventoryNotificationContainerWidgetInterface"),
		meta=(NoResetToDefault),
		meta=(ForceShowPluginContent),
		meta=(ForceShowEngineContent))
	TSoftClassPtr<UUserWidget> NotificationWidgetContainerClass;

	/** Widget class used for a single inventory notification entry (one message / card in the notification container). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Notifications",
		meta = (MustImplement = "/Script/FVInventoryEquipmentSystem.FVInventoryNotificationWidgetInterface"),
		meta=(NoResetToDefault),
		meta=(ForceShowPluginContent),
		meta=(ForceShowEngineContent))
	TSoftClassPtr<UUserWidget> NotificationWidgetClass;

	/** Material used for rendering individual notification cards (background, styling, effects). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Notifications|Notification Card")
	TSoftObjectPtr<UMaterialInterface> NotificationCardMaterial = nullptr;

	/** Default size (in Slate units) of the notification card widget when rendered on screen. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Notifications|Notification Card")
	FVector2f NotificationCardSize = FVector2f(150.f, 60.f);

	// ---- Fonts

	/** Default font settings used across the inventory user interface (labels, counters, titles). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Font",
		meta=(NoResetToDefault))
	FSlateFontInfo DefaultFont;

	// ---- Commands

	/** Defines list of available Widget Commands. Those are available using custom ProcessWidgetCommand Node.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config & Settings",
		meta=(NoResetToDefault))
	TSet<FString> WidgetCommands;

	// ---- Modals

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Modals",
		meta=(NoResetToDefault))
	TSet<FString> ModalTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Modals",
		meta=(NoResetToDefault),
		meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedInventoryModalWidgetInterface"))
	TSoftClassPtr<UUserWidget> ModalWindowWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Modals",
		meta=(NoResetToDefault),
		meta=(ShowOnlyInnerProperties),
		meta=(GetKeyOptions="GetModalTypes"))
	TMap<FString,FFVModalDefinition> Modals;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Modals",
		meta=(NoResetToDefault),
		meta=(RequiredAssetDataTags="RowStructure=/Script/FVInventoryEquipmentSystem.FVModalsConfig"))
	TSet<TSoftObjectPtr<UDataTable>> ModalsData;

protected:

	static const TArray<FString>& GetDefaultModalTypes();
	void ValidateModalTypes();
	void SetupWidgetCommands();

	UFUNCTION()
	TArray<FString> GetModalTypes() const;

#if WITH_EDITOR
public:
	UFUNCTION(CallInEditor, Category="Default Values")
	void SetDefaultModalTypes();
	
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
