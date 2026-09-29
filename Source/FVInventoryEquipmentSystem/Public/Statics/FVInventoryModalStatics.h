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
#include "Engine/DataTable.h"
#include "Interfaces/Widgets/Modal/FVAdvancedInventoryModalContentWidgetInterface.h"
#include "Interfaces/Widgets/Modal/FVAdvancedInventoryModalWidgetInterface.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVInventoryModalStatics.generated.h"

class APlayerController;
class UFVAdvancedInventoryGlobalUIConfig;
class UFVJsonObject;
class UUserWidget;

/**
 *
 */
UCLASS()
class FVINVENTORYEQUIPMENTSYSTEM_API UFVInventoryModalStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|UI|Modal",
		meta=(FVSetter),
		meta=(WorldContext="Context"),
		meta=(DefaultToSelf="Context"),
		meta=(DisplayName="Modal - Create Modal Window Widget"))
	static UUserWidget* CreateModalWindowWidget(UObject* Context, UObject* OptionalPayload);

	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|UI|Modal",
		meta=(FVSetter),
		meta=(FVModalType),
		meta=(FVModalTypePin="ModalType"),
		meta=(WorldContext="Context"),
		meta=(DefaultToSelf="Context"),
		meta=(DisplayName="Modal - Create Modal Content Widget"))
	static UUserWidget* CreateModalContentWidget(UObject* Context, const FString& ModalType, UFVJsonObject* InPayload);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FV|Inventory & Equipment|UI|Modal",
		meta=(FVGetter),
		meta=(FVModalType),
		meta=(FVModalTypePin="ModalType"),
		meta=(DisplayName="Modal - Get Modal Type String"))
	static FString GetModalTypeString(const FString& ModalType);

	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|UI|Modal",
		meta=(FVSetter),
		meta=(DefaultToSelf="Target"),
		meta=(DisplayName="Modal Window - Add Modal Content"))
	static void AddModalContentToModalWindow(
		UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedInventoryModalWidgetInterface")) UObject* Target,
		UUserWidget* ModalContentWidget, UFVJsonObject* Payload);

	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|UI|Modal|Bindings",
		meta=(FVBinding),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(DisplayName="Bind On Modal Content Added To Modal Window"))
	static bool BindToOnModalContentAddedToModalWindow(
		UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedInventoryModalWidgetInterface")) UObject* Target,
		const FFVModalContentAddedToModalWindowBinding& Binding);

	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|UI|Modal|Bindings",
		meta=(FVBinding),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(DisplayName="Unbind From On Modal Content Added To Modal Window"))
	static bool UnbindFromOnModalContentAddedToModalWindow(
		UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedInventoryModalWidgetInterface")) UObject* Target,
		const FFVModalContentAddedToModalWindowBinding& Binding);

	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|UI|Modal|Bindings",
		meta=(FVBinding),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(DisplayName="Bind On Modal Confirmed"))
	static bool BindToOnModalConfirmed(
		UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedInventoryModalWidgetInterface")) UObject* Target,
		const FFVModalConfirmedBinding& Binding);

	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|UI|Modal|Bindings",
		meta=(FVBinding),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(DisplayName="Unbind From On Modal Confirmed"))
	static bool UnbindFromOnModalConfirmed(
		UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedInventoryModalWidgetInterface")) UObject* Target,
		const FFVModalConfirmedBinding& Binding);

	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|UI|Modal|Bindings",
		meta=(FVBinding),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(DisplayName="Bind On Modal Cancelled"))
	static bool BindToOnModalCancelled(
		UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedInventoryModalWidgetInterface")) UObject* Target,
		const FFVModalCancelledBinding& Binding);

	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|UI|Modal|Bindings",
		meta=(FVBinding),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(DisplayName="Unbind From On Modal Cancelled"))
	static bool UnbindFromOnModalCancelled(
		UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedInventoryModalWidgetInterface")) UObject* Target,
		const FFVModalCancelledBinding& Binding);

	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|UI|Modal|Bindings",
		meta=(FVBinding),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(DisplayName="Bind On Modal Content Confirmed"))
	static bool BindToOnModalContentConfirmed(
		UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedInventoryModalContentWidgetInterface")) UObject* Target,
		const FFVModalContentConfirmedBinding& Binding);

	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|UI|Modal|Bindings",
		meta=(FVBinding),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(DisplayName="Unbind From On Modal Content Confirmed"))
	static bool UnbindFromOnModalContentConfirmed(
		UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedInventoryModalContentWidgetInterface")) UObject* Target,
		const FFVModalContentConfirmedBinding& Binding);

	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|UI|Modal|Bindings",
		meta=(FVBinding),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(DisplayName="Bind On Modal Content Cancelled"))
	static bool BindToOnModalContentCancelled(
		UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedInventoryModalContentWidgetInterface")) UObject* Target,
		const FFVModalContentCancelledBinding& Binding);

	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|UI|Modal|Bindings",
		meta=(FVBinding),
		meta=(ExpandBoolAsExecs="ReturnValue"),
		meta=(DisplayName="Unbind From On Modal Content Cancelled"))
	static bool UnbindFromOnModalContentCancelled(
		UPARAM(meta=(MustImplement="/Script/FVInventoryEquipmentSystem.FVAdvancedInventoryModalContentWidgetInterface")) UObject* Target,
		const FFVModalContentCancelledBinding& Binding);

	/** Finds the first FFVModalsConfig row whose name matches Key across all configured ModalsData tables. */
	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|UI|Modal",
		meta=(FVGetter),
		meta=(FVModalRow),
		meta=(FVModalRowPin="Key"),
		meta=(CallableWithoutWorldContext),
		meta=(DisplayName="Modal - Get Config From Data Tables"))
	static bool GetModalConfigFromDataTables(const FString& Key, FFVModalsConfig& OutConfig);
	
	UFUNCTION(BlueprintCallable, Category="FV|Inventory & Equipment|UI|Modal|Bindings",
		meta=(FVSetter),
		meta=(DefaultToSelf="Target"),
		meta=(DisplayName="Modal Content - Start Expiration"))
	static void StartModalExpiration(UObject* Target, const float WidgetLifetime);
	

private:

	static UUserWidget* CreateWidgetFromClass(UObject* Context, TSubclassOf<UUserWidget> WidgetClass);
	static APlayerController* ResolveOwningPlayer(UObject* Context);
	static UFVAdvancedInventoryGlobalUIConfig* GetGlobalUIConfig();
};
