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


#include "Statics/FVInventoryUIStatics.h"
#include "Statics/FVInventorySystemStatics.h"
#include "Statics/FVInventoryStatics.h"

#include "Algo/Copy.h"
#include "Algo/MaxElement.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"

#include "CommonInputSubsystem.h"

#include "Definitions/FVInventoryBaseUIDataTypes.h"
#include "Definitions/FVInventoryItemTemplate.h"

#include "GameFramework/PlayerState.h"

#include "Components/PanelWidget.h"
#include "Definitions/FVInventoryBaseUIEnums.h"
#include "Interfaces/Crafting/FVAdvancedCraftingParticipantUIInterface.h"

#include "Interfaces/Inventory/FVAdvancedInventoryInterface.h"
#include "Interfaces/Inventory/FVAdvancedInventoryUIManagerInterface.h"
#include "Interfaces/Widgets/BaseWidget/FVAdvancedBaseInventoryWidgetInterface.h"
#include "Interfaces/Widgets/Category/FVAdvancedInventoryCategoriesWrapperWidgetInterface.h"
#include "Interfaces/Widgets/Category/FVAdvancedInventoryCategoryWidgetInterface.h"
#include "Interfaces/Widgets/ItemActions/FVAdvancedInventoryItemActionsContainerWidgetInterface.h"
#include "Interfaces/Widgets/ItemActions/FVAdvancedInventoryItemActionWidgetInterface.h"
#include "Interfaces/Widgets/Items/FVAdvancedInventoryItemSlotWidgetInterface.h"
#include "Interfaces/Widgets/Items/FVAdvancedInventoryItemWidgetInterface.h"
#include "Interfaces/Widgets/Items/FVAdvancedInventoryItemsGridWidgetInterface.h"
#include "Interfaces/Widgets/Items/FVAdvancedInventoryItemSlotsWrapperWidgetInterface.h"
#include "Interfaces/Widgets/Tooltip/FVAdvancedInventoryTooltipWidgetInterface.h"
#include "Interfaces/Inventory/FVAdvancedInventoryUIManagerInterface.h"

#include "Settings/FVAdvancedInventorySettings.h"
#include "Settings/FVAdvancedInventorySettingsConfig.h"
#include "Settings/FVAdvancedInventoryGlobalUIConfig.h"
#include "Settings/FVAdvancedInventoryUIConfig.h"
#include "Settings/FVAdvancedInventoryThemeConfig.h"
#include "Statics/FVInventoryBaseUIStatics.h"

#include "Slate/FVInventoryScrollBox.h"

#include "Subsystems/FVAdvancedInventoryUISubsystem.h"
#include "Subsystems/FVAdvancedInventorySharedHUDSubsystem.h"
#include "Widgets/ItemPreview/FVAdvancedInventoryInteractableObjectWidget.h"

#define FV_BIND_INVENTORY_UI_DELEGATE(Target, Binding, HandleGetter) \
	if (!IsValid(Target) || !(Binding).IsBound()) \
		return false; \
	IFVAdvancedInventoryUIManagerInterface* nativeInterface = Cast<IFVAdvancedInventoryUIManagerInterface>(Target); \
	if (!nativeInterface) \
		return false; \
	nativeInterface->HandleGetter().AddUnique(Binding); \
	return true

#define FV_UNBIND_INVENTORY_UI_DELEGATE(Target, Binding, HandleGetter) \
	if (!IsValid(Target) || !(Binding).IsBound()) \
		return false; \
	IFVAdvancedInventoryUIManagerInterface* nativeInterface = Cast<IFVAdvancedInventoryUIManagerInterface>(Target); \
	if (!nativeInterface) \
		return false; \
	nativeInterface->HandleGetter().Remove(Binding); \
	return true

#define FV_BIND_CATEGORY_UI_DELEGATE(Target, Binding, HandleGetter) \
	if (!IsValid(Target) || !(Binding).IsBound()) \
		return false; \
	IFVAdvancedInventoryCategoryWidgetInterface* nativeInterface = Cast<IFVAdvancedInventoryCategoryWidgetInterface>(Target); \
		if (!nativeInterface) \
			return false; \
		nativeInterface->HandleGetter().AddUnique(Binding); \
	return true

#define FV_UNBIND_CATEGORY_UI_DELEGATE(Target, Binding, HandleGetter) \
	if (!IsValid(Target) || !(Binding).IsBound()) \
		return false; \
	IFVAdvancedInventoryCategoryWidgetInterface* nativeInterface = Cast<IFVAdvancedInventoryCategoryWidgetInterface>(Target); \
		if (!nativeInterface) \
			return false; \
	nativeInterface->HandleGetter().Remove(Binding); \
		return true

APlayerController* UFVInventoryUIStatics::FindPlayerController(AActor* Actor, int SearchDepth)
{
	SearchDepth++;
	if (SearchDepth >= 8)
		return nullptr;
	
	if (APlayerController* playerController = Cast<APlayerController>(Actor))
		return playerController;

	if (APlayerState* playerState = Cast<APlayerState>(Actor))
		return playerState->GetPlayerController();

	if (APawn* actorPawn = Cast<APawn>(Actor))
		return Cast<APlayerController>(actorPawn->GetController());
	
	if (AActor* ownerActor = Actor->GetOwner())
		return FindPlayerController(ownerActor, SearchDepth);
	
	return nullptr;
}

void UFVInventoryUIStatics::SetOwningInventoryUIInternal(UWidget* Target,
	const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& NewOwningInventoryUI)
{
	if (!IsValid(Target))
	{
		LOG_ERROR(TEXT("[SetOwningInventoryUI] Failed to set Owning Inventory UI. Reason: Target is null."));
		return;
	}

	if (!Target->Implements<UFVAdvancedBaseInventoryWidgetInterface>())
	{
		LOG_ERROR(TEXT("[SetOwningInventoryUI] Failed to set Owning Inventory UI for %s. Reason: Target does not implement IFVAdvancedBaseInventoryWidgetInterface."),
			*Target->GetName());
		return;
	}

	IFVAdvancedBaseInventoryWidgetInterface::Execute_SetOwningInventoryUI(Target, NewOwningInventoryUI);
}

bool UFVInventoryUIStatics::IsItemWidgetDetachedFromValidItems(const TArray<FFVInventoryItem>& ValidItems,
	const FInventoryItemData& InventoryItemData)
{
	if (ValidItems.IsEmpty())
		return true;
	if (!InventoryItemData.ContainingItem.IsItemValid())
		return true;
	
	return !ValidItems.ContainsByPredicate([&InventoryItemData](const FFVInventoryItem& Item)
	{
		return Item.Guid == InventoryItemData.ContainingItem.Guid;
	});
}

bool UFVInventoryUIStatics::InInputTypeActive(const UUserWidget* ContextObject, const ECommonInputType& InputType)
{
	if (!IsValid(ContextObject))
		return false;

	const ULocalPlayer* localPlayer = ContextObject->GetOwningLocalPlayer();
	if (!localPlayer)
		return false;

	const UCommonInputSubsystem* inputSubsystem = UCommonInputSubsystem::Get(localPlayer);
	if (!inputSubsystem)
		return false;

	return InputType == inputSubsystem->GetCurrentInputType();
}

void UFVInventoryUIStatics::CenterListItemAtIndex(UPanelWidget* ListWidget, const int32 SelectedIndex)
{
	if (!ListWidget || SelectedIndex < 0)
		return;
    
	if (ListWidget->GetChildrenCount() == 0)
		return;
    
	UWidget* FirstChild = ListWidget->GetChildAt(0);
	if (!FirstChild)
		return;
    
	const float ItemHeight = FirstChild->GetDesiredSize().Y;
	const float ViewportHeight = ListWidget->GetCachedGeometry().GetLocalSize().Y;
    
	if (ViewportHeight <= 0.0f || ItemHeight <= 0.0f)
		return;
    
	const float ViewportCenter = ViewportHeight * 0.5f;
	const float ItemCenter = (SelectedIndex * ItemHeight) + (ItemHeight * 0.5f);
	const float TranslationY = ViewportCenter - ItemCenter;
    
	ListWidget->SetRenderTranslation(FVector2D(0.0f, TranslationY));
}

FVector2D UFVInventoryUIStatics::CalculateCenteredListTranslation(UPanelWidget* ListWidget, const int32 SelectedIndex)
{
	if (!ListWidget || SelectedIndex < 0 || ListWidget->GetChildrenCount() == 0)
		return FVector2D::ZeroVector;

	const UWidget* firstChild = ListWidget->GetChildAt(0);
	if (!firstChild)
		return FVector2D::ZeroVector;
    
	const float itemHeight = firstChild->GetDesiredSize().Y;
	const float viewportHeight = ListWidget->GetCachedGeometry().GetLocalSize().Y;
    
	if (viewportHeight <= 0.0f || itemHeight <= 0.0f)
		return FVector2D::ZeroVector;
    
	const float viewportCenter = viewportHeight * 0.5f;
	const float itemCenter = (SelectedIndex * itemHeight) + (itemHeight * 0.5f);
	const float translationY = (viewportCenter - itemCenter);
    
	return FVector2D(0.0f, translationY);
}

FVector2D UFVInventoryUIStatics::CalculateCenteredListTranslation(const TSharedPtr<SWidget>& ListWidget, 
	const FGeometry& ListGeometry, const int32 SelectedIndex)
{
	TSharedPtr<SPanel> panel = StaticCastSharedPtr<SPanel>(ListWidget);
	
	if (!panel.IsValid() || SelectedIndex < 0)
		return FVector2D::ZeroVector;

	FChildren* children = panel->GetChildren();
	if (!children || children->Num() == 0 || SelectedIndex >= children->Num())
		return FVector2D::ZeroVector;

	const TSharedRef<SWidget> firstChild = children->GetChildAt(0);
	const float itemHeight = firstChild->GetDesiredSize().Y;
	const float viewportHeight = ListGeometry.GetLocalSize().Y;
    
	if (viewportHeight <= 0.0f || itemHeight <= 0.0f)
		return FVector2D::ZeroVector;
    
	const float viewportCenter = viewportHeight * 0.5f;
	const float itemCenter = (SelectedIndex * itemHeight) + (itemHeight * 0.5f);
	const float translationY = viewportCenter - itemCenter;
    
	return FVector2D(0.0f, translationY);
}

FSlateFontInfo UFVInventoryUIStatics::SetFontSize(const FSlateFontInfo& Font, const int32 NewSize)
{
	auto returnValue = Font;
	returnValue.Size = NewSize;
	return returnValue;
}

FSlateFontInfo UFVInventoryUIStatics::SetFontTypeface(const FSlateFontInfo& Font, const FName& NewTypeface)
{
	auto returnValue = Font;
	returnValue.TypefaceFontName = NewTypeface;
	return returnValue;
}

UFVAdvancedInventoryUIConfig* UFVInventoryUIStatics::GetInventoryUISettingsConfig()
{
	const auto settings = GetDefault<UFVAdvancedInventorySettings>();
	if (!settings) return nullptr;
	return settings->AdvancedInventoryUISettingsConfig.LoadSynchronous();
}

float UFVInventoryUIStatics::GetSignedStepFromPayload(const FFVWidgetInputPayload& Payload)
{
	if (Payload.InputKey != EFVWidgetInputKey::Wheel && Payload.InputKey != EFVWidgetInputKey::Analog)
		return 0.f;
	
	if (FMath::IsNearlyZero(Payload.FloatValue))
		return 0.f;

	const auto* globalUIConfig = UFVInventoryBaseUIStatics::GetGlobalUIConfig();
	const float deadZone = IsValid(globalUIConfig) ? globalUIConfig->InputDeadzone : 0.1f;

	const float returnValue = Payload.FloatValue;
	if (FMath::Abs(returnValue) <= deadZone)
		return 0.f;

	return FMath::Sign(returnValue); // -1.f or +1.f
}

TScriptInterface<IFVAdvancedInventoryInterface> UFVInventoryUIStatics::GetParentInventory(
	const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target)
{
	return Target.GetObject() ? Target->Execute_GetParentInventory(Target.GetObject()) : nullptr;
}

void UFVInventoryUIStatics::SetParentInventory(const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target,
	const TScriptInterface<IFVAdvancedInventoryInterface>& NewParentInventory)
{
	if (Target.GetObject())
		Target->Execute_SetParentInventory(Target.GetObject(), NewParentInventory);
}

bool UFVInventoryUIStatics::BindToOnCategorySelected(UObject* Target, const FFVInventoryCategorySelectedBinding& Binding)
{
	FV_BIND_INVENTORY_UI_DELEGATE(Target, Binding, GetOnCategorySelectedHandle);
}

bool UFVInventoryUIStatics::UnbindFromOnCategorySelected(UObject* Target, const FFVInventoryCategorySelectedBinding& Binding)
{
	FV_UNBIND_INVENTORY_UI_DELEGATE(Target, Binding, GetOnCategorySelectedHandle);
}

bool UFVInventoryUIStatics::BindToOnItemSelected(UObject* Target, const FFVInventoryItemSelectedBinding& Binding)
{
	FV_BIND_INVENTORY_UI_DELEGATE(Target, Binding, GetOnItemSelectedHandle);
}

bool UFVInventoryUIStatics::UnbindFromOnItemSelected(UObject* Target, const FFVInventoryItemSelectedBinding& Binding)
{
	FV_UNBIND_INVENTORY_UI_DELEGATE(Target, Binding, GetOnItemSelectedHandle);
}

#pragma region InventoryWidget

bool UFVInventoryUIStatics::CreateInventoryWidget(
	const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target)
{
	return (Target.GetObject() ? Target->Execute_CreateInventoryWidget(Target.GetObject()) : false);
}

UUserWidget* UFVInventoryUIStatics::GetInventoryWidget(
	const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target)
{
	return Target.GetObject() ? Target->Execute_GetInventoryWidget(Target.GetObject()) : nullptr;
}

void UFVInventoryUIStatics::RemoveInventoryWidget(
	const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target)
{
	if (Target.GetObject())
		Target->Execute_RemoveInventoryWidget(Target.GetObject());
}

bool UFVInventoryUIStatics::SetInventoryWidget(
	const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target, UUserWidget* NewInventoryWidget)
{
	return (Target.GetObject() ? Target->Execute_SetInventoryWidget(Target.GetObject(), NewInventoryWidget) : false);
}

void UFVInventoryUIStatics::CategorySelected(const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target,
	const FString& SelectedCategoryId)
{
	if (Target.GetObject())
		IFVAdvancedInventoryUIManagerInterface::Execute_CategorySelected(Target.GetObject(), SelectedCategoryId);
}

FString UFVInventoryUIStatics::GetSelectedCategoryId(
	const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target)
{
	return Target.GetObject() ? Target->Execute_GetSelectedCategoryId(Target.GetObject()) : TEXT("none");
}

void UFVInventoryUIStatics::ItemSelected(const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target, 
	const FGuid& ItemGuid)
{
	if (Target.GetObject())
		IFVAdvancedInventoryUIManagerInterface::Execute_ItemSelected(Target.GetObject(), ItemGuid);
}

FGuid UFVInventoryUIStatics::GetSelectedItemGuid(
	const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target)
{
	return Target.GetObject() ? Target->Execute_GetActiveItemGuid(Target.GetObject()) : FGuid();
}

void UFVInventoryUIStatics::SetInventoryOwningInventoryUI(UWidget* Target, 
	const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& OwningInventoryUI)
{
	SetOwningInventoryUIInternal(Target, OwningInventoryUI);
}

void UFVInventoryUIStatics::SetCategoriesWrapperOwningInventoryUI(UWidget* Target,
	const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& OwningInventoryUI)
{
	SetOwningInventoryUIInternal(Target, OwningInventoryUI);
}

void UFVInventoryUIStatics::SetActiveCategoryId(UWidget* Target, const FString& ActiveCategoryId)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryCategoriesWrapperWidgetInterface>())
		IFVAdvancedInventoryCategoriesWrapperWidgetInterface::Execute_SetActiveCategoryId(Target, ActiveCategoryId);
}

FString UFVInventoryUIStatics::GetActiveCategoryId(UWidget* Target)
{
	return (IsValid(Target) && Target->Implements<UFVAdvancedInventoryCategoriesWrapperWidgetInterface>())
		? IFVAdvancedInventoryCategoriesWrapperWidgetInterface::Execute_GetActiveCategoryId(Target) : TEXT("none");
}

FFVInventoryItem UFVInventoryUIStatics::FindItem(const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target,
	const FInventoryItemSearchParams& SearchParams)
{
	if (!IsValid(Target.GetObject()))
		return {};

	const auto& inventory = Target->Execute_GetParentInventory(Target.GetObject());
	if (!IsValid(inventory.GetObject()))
		return {};

	return inventory->Execute_FindItem(inventory.GetObject(), SearchParams);
}

TMap<FGameplayTag, FInventoryUICustomData> UFVInventoryUIStatics::GetCustomItemsMap(
	const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target)
{
	return Target.GetObject() ? 
		IFVAdvancedInventoryUIManagerInterface::Execute_GetCustomItemsMap(Target.GetObject()) : TMap<FGameplayTag, FInventoryUICustomData>();
}

void UFVInventoryUIStatics::AddCustomItem(const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target,
	const FGameplayTag& ItemTag, const FGuid& ItemId)
{
	if (Target.GetObject())
		IFVAdvancedInventoryUIManagerInterface::Execute_AddCustomItemToMap(Target.GetObject(), ItemTag, ItemId);
}

void UFVInventoryUIStatics::AppendCustomItems(const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target,
	const TMap<FGameplayTag, FInventoryUICustomData>& OtherItems)
{
	if (Target.GetObject())
		IFVAdvancedInventoryUIManagerInterface::Execute_AppendCustomItemsMap(Target.GetObject(), OtherItems);
}

void UFVInventoryUIStatics::ClearCustomItems(const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target)
{
	if (Target.GetObject())
		IFVAdvancedInventoryUIManagerInterface::Execute_ClearCustomItemsMap(Target.GetObject());
}

bool UFVInventoryUIStatics::RemoveCustomItem(const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target,
	const FGameplayTag& ItemTag, const FGuid& ItemId)
{
	if (Target.GetObject())
		return IFVAdvancedInventoryUIManagerInterface::Execute_RemoveCustomItemFromMap(Target.GetObject(), ItemTag, ItemId);
	return false;
}

bool UFVInventoryUIStatics::IsItemStoredInCustomMap(const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target, 
	const FGameplayTag& ItemTag, const FGuid& ItemId)
{
	if (Target.GetObject())
		return IFVAdvancedInventoryUIManagerInterface::Execute_IsItemStoredInCustomMap(Target.GetObject(), ItemTag, ItemId);
	return false;
}

TArray<UFVSelectableInventoryItemAction*> UFVInventoryUIStatics::GetItemActionsQueue(const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target)
{
	return Target.GetObject() ? IFVAdvancedInventoryUIManagerInterface::Execute_GetItemActionsQueue(Target.GetObject()) : TArray<UFVSelectableInventoryItemAction*>();
}

bool UFVInventoryUIStatics::EnqueueItemAction(const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target,
	UFVSelectableInventoryItemAction* ItemAction, UObject* Payload)
{
	return Target.GetObject() ? IFVAdvancedInventoryUIManagerInterface::Execute_EnqueueItemAction(Target.GetObject(), ItemAction, Payload) : false;
}

void UFVInventoryUIStatics::EmptyItemActionsQueue(const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target)
{
	if (Target.GetObject())
		IFVAdvancedInventoryUIManagerInterface::Execute_EmptyItemActionsQueue(Target.GetObject());
}

void UFVInventoryUIStatics::CompleteQueuedAction(const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target,
	UFVSelectableInventoryItemAction* ItemAction, UObject* Payload)
{
	if (Target.GetObject())
		IFVAdvancedInventoryUIManagerInterface::Execute_CompleteQueuedAction(Target.GetObject(), ItemAction, Payload);
}

void UFVInventoryUIStatics::CancelQueuedAction(const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target,
	UFVSelectableInventoryItemAction* ItemAction)
{
	if (Target.GetObject())
		IFVAdvancedInventoryUIManagerInterface::Execute_CancelQueuedAction(Target.GetObject(), ItemAction);
}

void UFVInventoryUIStatics::RemoveQueuedAction(const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target,
	UFVSelectableInventoryItemAction* ItemAction)
{
	if (Target.GetObject())
		IFVAdvancedInventoryUIManagerInterface::Execute_CancelQueuedAction(Target.GetObject(), ItemAction);
}

#pragma endregion

UFVAdvancedInventoryThemeConfig* UFVInventoryUIStatics::GetThemeConfig()
{
	const auto settings = GetMutableDefault<UFVAdvancedInventorySettings>();
	if (!IsValid(settings))
	{
		LOG_ERROR(TEXT("[GetThemeConfig] Settings not found!"))
		return nullptr;
	}
	const auto config = settings->AdvancedInventorySettingsConfig.LoadSynchronous();
	if (!IsValid(config))
	{
		LOG_ERROR(TEXT("[GetThemeConfig] Config not found!"))
		return nullptr;
	}
	return config->BaseTheme.LoadSynchronous();
}

void UFVInventoryUIStatics::SetGridSlotData(FFVInventoryGridSlot& SourceData, 
	const FFVInventoryGridSlot& TargetData)
{
	SourceData = TargetData;
}

void UFVInventoryUIStatics::StoreItem(FFVInventoryGridSlot& SourceData, const FGuid& ItemId)
{
	//SourceData.OccupiedItemId = ItemId;
}

void UFVInventoryUIStatics::ResetItem(FFVInventoryGridSlot& SourceData)
{
	SourceData.ResetSlot();
}

FString UFVInventoryUIStatics::GridSlot_ToString(const FFVInventoryGridSlot& SourceData)
{
	return SourceData.ToString();
}

#pragma region UI_Input

bool UFVInventoryUIStatics::FindUIActionMappingForItemAction(const FGameplayTagContainer& PreferredInputActions,
	const FGameplayTagContainer& WidgetStates, FFVWidgetInputActionMapping& OutMapping)
{
	OutMapping = FFVWidgetInputActionMapping();

	const auto globalUIConfig = UFVInventoryBaseUIStatics::GetGlobalUIConfig();
	if (!IsValid(globalUIConfig))
		return false;

	const auto& definedMappings = globalUIConfig->UIActionMappings;
	if (definedMappings.Num() == 0)
		return false;

	if (PreferredInputActions.Num() == 0)
		return false;

	const bool bUseStateFiltering = (WidgetStates.Num() > 0);

	const auto foundMapping = Algo::FindByPredicate(definedMappings,
		[&PreferredInputActions, &WidgetStates, bUseStateFiltering](const FFVWidgetInputActionMapping& Mapping)
		{
			if (!PreferredInputActions.HasTagExact(Mapping.ActionTag))
				return false;

			if (!bUseStateFiltering)
				return true;

			return !Mapping.BlacklistedWidgetStates.HasAnyExact(WidgetStates);
		});

	if (foundMapping == nullptr)
		return false;

	OutMapping = *foundMapping;
	return true;
}

bool UFVInventoryUIStatics::FindUIActionMappingFromKeyEvent(const FKeyEvent& KeyEvent, const FGameplayTagContainer& WidgetStates,
	const TArray<FFVWidgetInputActionMapping>& Mappings, FFVWidgetInputActionMapping& OutMapping)
{
	return FindUIActionMappingByKey(KeyEvent.GetKey(), KeyEvent.GetModifierKeys(), WidgetStates, Mappings, OutMapping);
}

bool UFVInventoryUIStatics::FindUIActionMappingFromAnalogEvent(const FAnalogInputEvent& AnalogEvent, const FGameplayTagContainer& WidgetStates,
	const TArray<FFVWidgetInputActionMapping>& Mappings, FFVWidgetInputActionMapping& OutMapping)
{
	return FindUIActionMappingByKey(AnalogEvent.GetKey(), AnalogEvent.GetModifierKeys(), WidgetStates, Mappings, OutMapping);
}

bool UFVInventoryUIStatics::FindUIActionMappingFromMouseEvent(const FPointerEvent& MouseEvent, const FGameplayTagContainer& WidgetStates,
	const TArray<FFVWidgetInputActionMapping>& Mappings, FFVWidgetInputActionMapping& OutMapping)
{
	// Wheel events (NativeOnMouseWheel) don't have an "effecting button".
	// Convention: bind wheel mappings using EKeys::MouseWheelAxis.
	const float WheelDelta = MouseEvent.GetWheelDelta();
	const FKey PressedKey = !FMath::IsNearlyZero(WheelDelta)
		? EKeys::MouseWheelAxis
		: MouseEvent.GetEffectingButton();

	return FindUIActionMappingByKey(PressedKey, MouseEvent.GetModifierKeys(), WidgetStates, Mappings, OutMapping);
}

bool UFVInventoryUIStatics::FindUIActionMappingsFromKeyEvent(const FKeyEvent& KeyEvent, const FGameplayTagContainer& WidgetStates,
	const TArray<FFVWidgetInputActionMapping>& Mappings, TArray<FFVWidgetInputActionMapping>& OutMappings)
{
	return FindUIActionMappingsByKey(KeyEvent.GetKey(), KeyEvent.GetModifierKeys(), WidgetStates, Mappings, OutMappings);
}

bool UFVInventoryUIStatics::FindUIActionMappingsFromAnalogEvent(const FAnalogInputEvent& AnalogEvent, const FGameplayTagContainer& WidgetStates,
	const TArray<FFVWidgetInputActionMapping>& Mappings, TArray<FFVWidgetInputActionMapping>& OutMappings)
{
	return FindUIActionMappingsByKey(AnalogEvent.GetKey(), AnalogEvent.GetModifierKeys(), WidgetStates, Mappings, OutMappings);
}

bool UFVInventoryUIStatics::FindUIActionMappingsFromMouseEvent(const FPointerEvent& MouseEvent, const FGameplayTagContainer& WidgetStates,
	const TArray<FFVWidgetInputActionMapping>& Mappings, TArray<FFVWidgetInputActionMapping>& OutMappings)
{
	const float WheelDelta = MouseEvent.GetWheelDelta();
	const FKey PressedKey = !FMath::IsNearlyZero(WheelDelta)
		? EKeys::MouseWheelAxis
		: MouseEvent.GetEffectingButton();

	return FindUIActionMappingsByKey(PressedKey, MouseEvent.GetModifierKeys(), WidgetStates, Mappings, OutMappings);
}

bool UFVInventoryUIStatics::FindUIActionMappingByKey(const FKey& PressedKey, const FModifierKeysState& Modifiers,
	const FGameplayTagContainer& WidgetStates, const TArray<FFVWidgetInputActionMapping>& Mappings, FFVWidgetInputActionMapping& OutMapping)
{
	if (!PressedKey.IsValid())
		return false;
	
	if (Mappings.Num() <= 0)
		return false;

	const bool bUseStateFiltering = (WidgetStates.Num() > 0);

	const auto matchingMapping = [&PressedKey, &WidgetStates, bUseStateFiltering](const FFVWidgetInputActionMapping& M)
	{
		const bool bKeyFound = M.Keys.Contains(PressedKey);
		return bUseStateFiltering ? bKeyFound && !M.BlacklistedWidgetStates.HasAnyExact(WidgetStates) : bKeyFound;
	};

	TArray<const FFVWidgetInputActionMapping*> candidateMappings;
	candidateMappings.Reserve(Mappings.Num());

	Algo::TransformIf(
		Mappings,
		candidateMappings,
		matchingMapping,
		[](const FFVWidgetInputActionMapping& M) { return &M; }
	);

	const FFVWidgetInputActionMapping* const* bestMapping =
		Algo::MaxElementBy(candidateMappings, [](const auto* M) { return M->InputPriority; });

	if (!bestMapping)
		return false;

	OutMapping = **bestMapping;
	return true;
}

bool UFVInventoryUIStatics::FindUIActionMappingsByKey(const FKey& PressedKey, const FModifierKeysState& Modifiers,
	const FGameplayTagContainer& WidgetStates, const TArray<FFVWidgetInputActionMapping>& Mappings, TArray<FFVWidgetInputActionMapping>& OutMappings)
{
	OutMappings.Reset();

	if (!PressedKey.IsValid())
		return false;

	if (Mappings.Num() <= 0)
		return false;
	
	const bool bUseStateFiltering = (WidgetStates.Num() > 0);

	const auto matchingMapping = [&PressedKey, &WidgetStates, bUseStateFiltering](const FFVWidgetInputActionMapping& M)
	{
		const bool bKeyFound = M.Keys.Contains(PressedKey);
		return bUseStateFiltering ? bKeyFound && !M.BlacklistedWidgetStates.HasAnyExact(WidgetStates) : bKeyFound;
	};

	TArray<const FFVWidgetInputActionMapping*> candidateMappings;
	candidateMappings.Reserve(Mappings.Num());

	Algo::TransformIf(
		Mappings,
		candidateMappings,
		matchingMapping,
		[](const FFVWidgetInputActionMapping& M) { return &M; }
	);

	if (candidateMappings.IsEmpty())
		return false;

	Algo::Sort(candidateMappings, [](const auto* A, const auto* B)
	{
		return A->InputPriority > B->InputPriority;
	});

	OutMappings.Reserve(candidateMappings.Num());

	Algo::Transform(
		candidateMappings,
		OutMappings,
		[](const FFVWidgetInputActionMapping* M) { return *M; }
	);

	return true;
}

#pragma endregion 

FVector2D UFVInventoryUIStatics::GetActionsListSpawnLocation(UWidget* ParentWidget)
{
	if (!IsValid(ParentWidget))
	{
		LOG_ERROR(TEXT("[GetActionsListSpawnLocation] Invalid Parent Widget!"));
		return FVector2D::ZeroVector;
	}

	if (!IsValid(ParentWidget->GetWorld()))
	{
		LOG_ERROR(TEXT("[GetActionsListSpawnLocation] Invalid World!"));
		return FVector2D::ZeroVector;
	}
	const auto Geometry = ParentWidget->GetCachedGeometry();
	FVector2D localCoordinates = FVector2D(Geometry.GetLocalSize().X, 0.f);
	FVector2D pixelPosition, viewportPosition;
	USlateBlueprintLibrary::LocalToViewport(ParentWidget->GetWorld(), Geometry, localCoordinates, pixelPosition, viewportPosition);

	return pixelPosition;
}

UObject* UFVInventoryUIStatics::GetInventoryUIManager(AActor* FromActor)
{
	if (!FromActor) return nullptr;

	APlayerController* playerController = FindPlayerController(FromActor, 3);
	if (!playerController) return nullptr;

	const UFVAdvancedInventoryUISubsystem* localSubsystem = GetInventoryUISubsystem(playerController);
	return localSubsystem ? localSubsystem->GetInventoryUIManager() : nullptr;
}

UFVAdvancedInventoryUISubsystem* UFVInventoryUIStatics::GetInventoryUISubsystem(UObject* Context)
{
	if (!IsValid(Context))
		return nullptr;

	APlayerController* playerController = Cast<APlayerController>(Context);

	if (!IsValid(playerController))
	{
		if (AActor* actor = Cast<AActor>(Context))
			playerController = FindPlayerController(actor, 3);
		else if (const UActorComponent* actorComp = Cast<UActorComponent>(Context))
		{
			if (AActor* ownerActor = actorComp->GetOwner())
				playerController = FindPlayerController(ownerActor, 3);
		}
		else if (const UUserWidget* userWidget = Cast<UUserWidget>(Context))
			playerController = FindPlayerController(userWidget->GetOwningPlayer(), 2);
	}

	if (!IsValid(playerController))
		return nullptr;

	const ULocalPlayer* localPlayer = playerController->GetLocalPlayer();
	return IsValid(localPlayer) ? localPlayer->GetSubsystem<UFVAdvancedInventoryUISubsystem>() : nullptr;
}

int32 UFVInventoryUIStatics::FVInventoryScrollBox_GetChildrenCount(
	const UFVInventoryScrollBox* ScrollBox)
{
	return ScrollBox ? ScrollBox->GetChildrenCount() : INDEX_NONE;
}

int32 UFVInventoryUIStatics::FVInventoryScrollBox_GetActiveIndex(const UFVInventoryScrollBox* ScrollBox)
{
	return ScrollBox ? ScrollBox->GetActiveIndex() : INDEX_NONE;
}

void UFVInventoryUIStatics::FVInventoryScrollBox_SetActiveIndex(UFVInventoryScrollBox* ScrollBox,
	const int32 NewActiveIndex)
{
	if (ScrollBox)
		ScrollBox->SetActiveIndex(NewActiveIndex);
}

void UFVInventoryUIStatics::FVInventoryScrollBox_AddChild(UFVInventoryScrollBox* ScrollBox, UWidget* Content)
{
	if (!ScrollBox || !Content)
		return;
	ScrollBox->AddChild(Content);
}

void UFVInventoryUIStatics::FVInventoryScrollBox_ResetChildren(UFVInventoryScrollBox* ScrollBox)
{
	if (!ScrollBox)
		return;
	ScrollBox->ResetChildren();
}

TArray<UWidget*> UFVInventoryUIStatics::FVInventoryScrollBox_GetAllChildren(UFVInventoryScrollBox* ScrollBox)
{
	return ScrollBox ? ScrollBox->GetChildWidgets() : TArray<UWidget*>();
}

void UFVInventoryUIStatics::FVInventoryScrollBox_RemoveChild(UFVInventoryScrollBox* ScrollBox, UWidget* Content)
{
	if (ScrollBox)
		return ScrollBox->RemoveChild(Content);
}

void UFVInventoryUIStatics::FVInventoryScrollBox_RemoveChildAt(UFVInventoryScrollBox* ScrollBox, const int32 Index)
{
	if (ScrollBox)
		return ScrollBox->RemoveChildAt(Index);
}

void UFVInventoryUIStatics::SetOwningInventoryUI(UWidget* Target,
	const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& NewOwningInventoryUI)
{
	SetOwningInventoryUIInternal(Target, NewOwningInventoryUI);
}

void UFVInventoryUIStatics::SetSourceInventory(
	const TScriptInterface<IFVAdvancedBaseInventoryWidgetInterface>& Target,
	const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& ParentInventory)
{
	if (Target.GetObject())
		Target->Execute_SetOwningInventoryUI(Target.GetObject(), ParentInventory);
}

#pragma endregion 

void UFVInventoryUIStatics::SetInventoryCategoryKey(UWidget* Target, const FString& CategoryId)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryCategoryWidgetInterface>())
		IFVAdvancedInventoryCategoryWidgetInterface::Execute_SetInventoryCategoryKey(Target, CategoryId);
}

FString UFVInventoryUIStatics::GetInventoryCategoryKey(UWidget* Target)
{
	return (
		IsValid(Target) && Target->Implements<UFVAdvancedInventoryCategoryWidgetInterface>()
		) ? IFVAdvancedInventoryCategoryWidgetInterface::Execute_GetInventoryCategoryKey(Target) : TEXT("none");
}

void UFVInventoryUIStatics::SetCategoryOwningInventoryUI(UWidget* Target,
	const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& OwningInventoryUI)
{
	SetOwningInventoryUIInternal(Target, OwningInventoryUI);
}

void UFVInventoryUIStatics::SetActiveState(UWidget* Target, const bool bIsActive)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryCategoryWidgetInterface>())
		IFVAdvancedInventoryCategoryWidgetInterface::Execute_SetActiveState(Target, bIsActive);
}

FInventoryItemData UFVInventoryUIStatics::MakeInventoryItemWidgetData(const FFVInventoryItem& Item, const int32 Quantity)
{
	return FInventoryItemData(Quantity, Item);
}

bool UFVInventoryUIStatics::CompareItemWidgetData(const FInventoryItemData& NewState, const FInventoryItemData& OldState)
{
	return !NewState.IsDirty(OldState);
}

UWidget* UFVInventoryUIStatics::GetActiveItemWidget(const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target)
{
	return (
		IsValid(Target.GetObject()) )
	? IFVAdvancedInventoryUIManagerInterface::Execute_GetActiveItemWidget(Target.GetObject()) : nullptr;
}

void UFVInventoryUIStatics::SetActiveItemWidget(
	const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& Target, UWidget* NewActiveItemWidget)
{
	if (!IsValid(Target.GetObject())) return;

	Target->Execute_SetActiveItemWidget(Target.GetObject(), NewActiveItemWidget);
}

void UFVInventoryUIStatics::ItemWidget_SelectItem(UWidget* Target)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemWidgetInterface>())
		IFVAdvancedInventoryItemWidgetInterface::Execute_SelectItem(Target);
}

FGuid UFVInventoryUIStatics::ItemWidget_GetInventoryItemId(UWidget* Target)
{
	return (
		IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemWidgetInterface>()
		) ? IFVAdvancedInventoryItemWidgetInterface::Execute_GetInventoryItemId(Target) : FGuid();
}

void UFVInventoryUIStatics::SetItemOwningInventoryUI(UWidget* Target,
	const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& OwningInventoryUI)
{
	SetOwningInventoryUIInternal(Target, OwningInventoryUI);
}

void UFVInventoryUIStatics::ItemWidget_InitializeItemWidget(UWidget* Target, const FFVInventoryItem& Item,
	const int32 Quantity)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemWidgetInterface>())
		IFVAdvancedInventoryItemWidgetInterface::Execute_InitializeItemWidget(Target, Item, Quantity);
}

FInventoryItemData UFVInventoryUIStatics::ItemWidget_GetInventoryData(UWidget* Target)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemWidgetInterface>())
		return IFVAdvancedInventoryItemWidgetInterface::Execute_GetInventoryData(Target);
	return FInventoryItemData();
}

void UFVInventoryUIStatics::ItemWidget_SetInventoryData(UWidget* Target, const FInventoryItemData& InventoryItemData)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemWidgetInterface>())
		IFVAdvancedInventoryItemWidgetInterface::Execute_SetInventoryData(Target, InventoryItemData);
}

void UFVInventoryUIStatics::Item_RefreshWidget(UWidget* Target, const int32 Quantity)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemWidgetInterface>())
		IFVAdvancedInventoryItemWidgetInterface::Execute_RefreshItemWidget(Target, Quantity);
}

void UFVInventoryUIStatics::Item_SetParentSlot(UWidget* Target, UUserWidget* ParentSlot)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemWidgetInterface>())
		IFVAdvancedInventoryItemWidgetInterface::Execute_SetParentSlot(Target, ParentSlot);
}

void UFVInventoryUIStatics::Item_HighlightItem(UWidget* Target, const bool bIsSelected)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemWidgetInterface>())
		IFVAdvancedInventoryItemWidgetInterface::Execute_HighlightItem(Target, bIsSelected);
}

void UFVInventoryUIStatics::ItemAction_InitializeItemAction(UWidget* Target,
                                                                 const UFVSelectableInventoryItemAction* ItemAction, const FGuid& SelectedItem)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemActionWidgetInterface>())
		IFVAdvancedInventoryItemActionWidgetInterface::Execute_InitializeItemAction(
				Target, ItemAction, SelectedItem);
	else
		LOG_ERROR(TEXT("[InitializeItemAction] Target does not implement IFVAdvancedInventoryItemActionWidgetInterface!"));
}

bool UFVInventoryUIStatics::ItemAction_IsActionEnabled(UWidget* Target)
{
	return IsValid(Target) &&
		Target->Implements<UFVAdvancedInventoryItemActionWidgetInterface>() && 
		IFVAdvancedInventoryItemActionWidgetInterface::Execute_IsActionEnabled(Target);
}

bool UFVInventoryUIStatics::ItemAction_IsActionValid(UWidget* Target)
{
	return IsValid(Target) && 
		Target->Implements<UFVAdvancedInventoryItemActionWidgetInterface>() && 
		IFVAdvancedInventoryItemActionWidgetInterface::Execute_IsActionValid(Target);
}

void UFVInventoryUIStatics::ItemAction_ExecuteItemAction(UWidget* Target)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemActionWidgetInterface>())
		IFVAdvancedInventoryItemActionWidgetInterface::Execute_ExecuteItemAction(Target);
	else
		LOG_ERROR(TEXT("[ExecuteItemAction] Target does not implement IFVAdvancedInventoryItemActionWidgetInterface!"));
}

TSoftClassPtr<UFVSelectableInventoryItemAction> UFVInventoryUIStatics::ItemAction_GetItemAction(UWidget* Target)
{
	if (!IsValid(Target))
	{
		LOG_ERROR(TEXT("[GetItemAction] Target is not valid!"));
		return nullptr;
	}

	if (!Target->Implements<UFVAdvancedInventoryItemActionWidgetInterface>())
	{
		LOG_ERROR(TEXT("[GetItemAction] Target does not implement IFVAdvancedInventoryItemActionWidgetInterface!"));
		return nullptr;
	}

	return IFVAdvancedInventoryItemActionWidgetInterface::Execute_GetItemAction(Target);
}

#pragma region ItemActionsContainer

bool UFVInventoryUIStatics::IsValidActionsContainerWidget(const UObject* Target)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemActionsContainerWidgetInterface>();
}

void UFVInventoryUIStatics::ItemActionsContainer_ConstructFromActionsList(UWidget* Target, 
	const TArray<UFVSelectableInventoryItemAction*>& ItemActionsList)
{
	if (IsValidActionsContainerWidget(Target))
		IFVAdvancedInventoryItemActionsContainerWidgetInterface::Execute_ConstructFromActionsList(Target, ItemActionsList);
}

UWidget* UFVInventoryUIStatics::ItemActionsContainer_AddItemActionToContainer(UWidget* Target, UFVSelectableInventoryItemAction* ItemAction, 
	const FGuid& ItemId)
{
	return IsValidActionsContainerWidget(Target) ? 
		IFVAdvancedInventoryItemActionsContainerWidgetInterface::Execute_AddItemActionToContainer(Target, ItemAction, ItemId) :
		nullptr;
}

void UFVInventoryUIStatics::ItemActionsContainer_RemoveItemActionFromContainer(UWidget* Target, UWidget* ItemActionWidget)
{
	if (!IsValid(Target) || !Target->Implements<UFVAdvancedInventoryItemActionsContainerWidgetInterface>())
	{
		LOG_ERROR(TEXT("[RemoveItemActionFromContainer] Target does not implement IFVAdvancedInventoryItemActionsContainerWidgetInterface or is invalid!"));
		return;
	}

	if (!IsValid(ItemActionWidget))
	{
		LOG_WARNING(TEXT("[RemoveItemActionFromContainer] ItemActionWidget is invalid!"));
		return;
	}

	IFVAdvancedInventoryItemActionsContainerWidgetInterface::Execute_RemoveItemActionFromContainer(Target, ItemActionWidget);
}

void UFVInventoryUIStatics::ItemActionsContainer_ClearItemActionsContainer(UWidget* Target)
{
	if (!IsValid(Target) || !Target->Implements<UFVAdvancedInventoryItemActionsContainerWidgetInterface>())
	{
		LOG_ERROR(TEXT("[ClearItemActionsContainer] Target does not implement IFVAdvancedInventoryItemActionsContainerWidgetInterface or is invalid!"));
		return;
	}

	IFVAdvancedInventoryItemActionsContainerWidgetInterface::Execute_ClearItemActionsContainer(Target);
}

void UFVInventoryUIStatics::ItemActionsContainer_SelectItemAction(UWidget* Target, UWidget* ItemActionWidget)
{
	if (!IsValid(Target) || !Target->Implements<UFVAdvancedInventoryItemActionsContainerWidgetInterface>())
	{
		LOG_ERROR(TEXT("[SelectItemAction] Target does not implement IFVAdvancedInventoryItemActionsContainerWidgetInterface or is invalid!"));
		return;
	}

	if (!IsValid(ItemActionWidget))
	{
		LOG_WARNING(TEXT("[SelectItemAction] ItemActionWidget is invalid!"));
		return;
	}

	IFVAdvancedInventoryItemActionsContainerWidgetInterface::Execute_SelectItemAction(Target, ItemActionWidget);
}

TArray<UWidget*> UFVInventoryUIStatics::ItemActionsContainer_GetItemActionsInContainer(UWidget* Target)
{
	if (!IsValid(Target) || !Target->Implements<UFVAdvancedInventoryItemActionsContainerWidgetInterface>())
	{
		LOG_ERROR(TEXT("[GetItemActionsInContainer] Target does not implement IFVAdvancedInventoryItemActionsContainerWidgetInterface or is invalid!"));
		return {};
	}

	return IFVAdvancedInventoryItemActionsContainerWidgetInterface::Execute_GetItemActionsInContainer(Target);
}

void UFVInventoryUIStatics::ItemActionsContainer_SelectItemActionByIndex(UWidget* Target, const int32 ItemActionIndex)
{
	if (!IsValid(Target) || !Target->Implements<UFVAdvancedInventoryItemActionsContainerWidgetInterface>())
	{
		LOG_ERROR(TEXT("[SelectItemActionByIndex] Target does not implement IFVAdvancedInventoryItemActionsContainerWidgetInterface or is invalid!"));
		return;
	}

	const auto itemActions = ItemActionsContainer_GetItemActionsInContainer(Target);
	if (itemActions.Num() == 0 || ItemActionIndex < 0 || ItemActionIndex >= itemActions.Num() || !itemActions.IsValidIndex(ItemActionIndex))
	{
		LOG_WARNING(TEXT("[SelectItemActionByIndex] Invalid ItemActionIndex or no actions in container!"));
		return;
	}

	ItemActionsContainer_SelectItemAction(Target, itemActions[ItemActionIndex]);
}

#pragma endregion

void UFVInventoryUIStatics::ItemTooltip_SetTooltipItem(UWidget* Target, const FInventorySlot& SourceSlot)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryTooltipWidgetInterface>())
		IFVAdvancedInventoryTooltipWidgetInterface::Execute_SetTooltipItem(Target, SourceSlot);
}

FInventorySlot UFVInventoryUIStatics::MakeInventoryGridSlotData(const FInventorySlot& SlotData)
{
	return FFVInventoryGridSlot(SlotData);
}

FInventorySlot UFVInventoryUIStatics::MakeInventorySlotData(const FFVInventoryGridSlot& GridSlotData)
{
	return FInventorySlot(GridSlotData);
}

FInventorySlot UFVInventoryUIStatics::MakeInventorySlot(UUserWidget* UserWidget, const FGuid& ItemId)
{
	FInventorySlot returnValue;
	{
		returnValue.OverrideGuid(ItemId);
		returnValue.ItemWidget = UserWidget;
	}
	return returnValue;
}

void UFVInventoryUIStatics::ItemSlot_SetItemSlotOwningInventoryUI(UWidget* Target,
	const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& OwningInventoryUI)
{
	SetOwningInventoryUIInternal(Target, OwningInventoryUI);
}

void UFVInventoryUIStatics::ItemSlot_AddItemToSlot(UWidget* Target, const FGuid& ItemId)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemSlotWidgetInterface>())
		IFVAdvancedInventoryItemSlotWidgetInterface::Execute_AddItemToSlot(Target, ItemId);
}

void UFVInventoryUIStatics::ItemSlot_RemoveItemFromSlot(UWidget* Target, const FGuid& ItemId)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemSlotWidgetInterface>())
		IFVAdvancedInventoryItemSlotWidgetInterface::Execute_RemoveItemFromSlot(Target, ItemId);
}

void UFVInventoryUIStatics::ItemSlot_StoreGridSlotData(UWidget* Target, const FFVInventoryGridSlot& SlotData)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemSlotWidgetInterface>())
		IFVAdvancedInventoryItemSlotWidgetInterface::Execute_StoreGridSlotData(Target, SlotData);
}

FFVInventoryGridSlot UFVInventoryUIStatics::ItemSlot_GetGridSlotData(UWidget* Target)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemSlotWidgetInterface>())
		return IFVAdvancedInventoryItemSlotWidgetInterface::Execute_GetGridSlotData(Target);
	return FFVInventoryGridSlot();
}

UUserWidget* UFVInventoryUIStatics::ItemSlot_GetItemWidgetInSlot(UWidget* Target)
{
	return (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemSlotWidgetInterface>())
	? IFVAdvancedInventoryItemSlotWidgetInterface::Execute_GetItemWidgetInSlot(Target) : nullptr;
}

void UFVInventoryUIStatics::ItemSlot_SelectItemInSlot(UWidget* Target)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemSlotWidgetInterface>())
		IFVAdvancedInventoryItemSlotWidgetInterface::Execute_SelectItemInSlot(Target);
}

void UFVInventoryUIStatics::ItemSlot_SetParentSlotsWrapper(UWidget* Target, UUserWidget* ParentSlotsWrapper)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemSlotWidgetInterface>())
		IFVAdvancedInventoryItemSlotWidgetInterface::Execute_SetParentSlotsWrapper(Target, ParentSlotsWrapper);
}

void UFVInventoryUIStatics::SetItemSlotsWrapperOwningInventoryUI(UWidget* Target, 
	const TScriptInterface<IFVAdvancedInventoryUIManagerInterface>& OwningInventoryUI)
{
	SetOwningInventoryUIInternal(Target, OwningInventoryUI);
}

void UFVInventoryUIStatics::SlotsWrapper_AddItem(UWidget* Target, const FGuid& ItemId)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemSlotsWrapperWidgetInterface>())
		IFVAdvancedInventoryItemSlotsWrapperWidgetInterface::Execute_AddItem(Target, ItemId);
}

void UFVInventoryUIStatics::SlotsWrapper_UpdateItem(UWidget* Target, const FGuid& ItemId, const int32 OptionalItemSlot)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemSlotsWrapperWidgetInterface>())
		IFVAdvancedInventoryItemSlotsWrapperWidgetInterface::Execute_UpdateItem(Target, ItemId, OptionalItemSlot);
}

void UFVInventoryUIStatics::SlotsWrapper_RemoveItem(UWidget* Target, const FGuid& ItemId, const int32 Quantity)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemSlotsWrapperWidgetInterface>())
		IFVAdvancedInventoryItemSlotsWrapperWidgetInterface::Execute_RemoveItem(Target, ItemId, Quantity);
}

UWidget* UFVInventoryUIStatics::SlotsWrapper_GetSelectedItemWidget(UWidget* Target)
{
	return (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemSlotsWrapperWidgetInterface>())
	? IFVAdvancedInventoryItemSlotsWrapperWidgetInterface::Execute_GetSelectedItemWidget(Target) : nullptr;
}

void UFVInventoryUIStatics::SlotsWrapper_SetSelectedItemWidget(UWidget* Target,
	UWidget* NewSelectedItemWidget)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemSlotsWrapperWidgetInterface>())
		IFVAdvancedInventoryItemSlotsWrapperWidgetInterface::Execute_SetSelectedItemWidget(
			Target, NewSelectedItemWidget);
}

bool UFVInventoryUIStatics::ItemsGrid_AddItemToEmptySlot(UWidget* Target, const FGuid& ItemId)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>()
	? IFVAdvancedInventoryItemsGridWidgetInterface::Execute_AddItemToEmptySlot(Target, ItemId) : false;
}

bool UFVInventoryUIStatics::ItemsGrid_AddItemToSlot(UWidget* Target, const FGuid& ItemId,
	const int32 SlotIndex)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>()
	? IFVAdvancedInventoryItemsGridWidgetInterface::Execute_AddItemToSlot(Target, ItemId, SlotIndex) : false;
}

bool UFVInventoryUIStatics::ItemsGrid_RemoveItemFromSlot(UWidget* Target, const int32 SlotIndex)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>()
	? IFVAdvancedInventoryItemsGridWidgetInterface::Execute_RemoveItemFromSlot(Target, SlotIndex) : false;
}

bool UFVInventoryUIStatics::ItemsGrid_RemoveItem(UWidget* Target, const FGuid& ItemId, const int32 Quantity)
{
	return (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>())
	? IFVAdvancedInventoryItemsGridWidgetInterface::Execute_RemoveItemFromGrid(Target, ItemId, Quantity) : false;
}

FGuid UFVInventoryUIStatics::ItemsGrid_GetItemInSlot(UWidget* Target, const int32 SlotIndex)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>()
	? IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetItemInSlot(Target, SlotIndex) : FGuid();
}

bool UFVInventoryUIStatics::ItemsGrid_SwapItemsBetweenSlots(UWidget* Target, const int32 SlotIndex1,
	const int32 SlotIndex2)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>()
	? IFVAdvancedInventoryItemsGridWidgetInterface::Execute_SwapItemsBetweenSlots(Target, SlotIndex1, SlotIndex2) : false;
}

void UFVInventoryUIStatics::ItemsGrid_ClearAllSlots(UWidget* Target)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>())
		IFVAdvancedInventoryItemsGridWidgetInterface::Execute_ClearAllSlots(Target);
}

int32 UFVInventoryUIStatics::ItemsGrid_GetTotalSlots(UWidget* Target)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>()
	? IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetTotalSlots(Target) : INDEX_NONE;
}

bool UFVInventoryUIStatics::ItemsGrid_IsSlotEmpty(UWidget* Target, const int32 SlotIndex)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>()
	? IFVAdvancedInventoryItemsGridWidgetInterface::Execute_IsSlotEmpty(Target, SlotIndex) : false;
}

int32 UFVInventoryUIStatics::ItemsGrid_GetSlotIndexByItem(UWidget* Target, const FGuid& ItemId)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>()
	? IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetSlotIndexByItem(Target, ItemId) : INDEX_NONE;
}

int32 UFVInventoryUIStatics::ItemsGrid_GetGridSlotIndexByCoords(UWidget* Target, const FIntPoint& SlotCoords)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>()
	? IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetGridSlotIndexByCoords(Target, SlotCoords) : INDEX_NONE;
}

bool UFVInventoryUIStatics::ItemsGrid_IsItemInGrid(UWidget* Target, const FGuid& ItemId)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>()
	? IFVAdvancedInventoryItemsGridWidgetInterface::Execute_IsItemInGrid(Target, ItemId) : false;
}

FFVInventoryGridSlot UFVInventoryUIStatics::ItemsGrid_GetGridSlotData(UWidget* Target,
	const int32 SlotIndex)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>()
	? IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetGridSlotData(Target, SlotIndex) : FFVInventoryGridSlot();
}

TSet<FFVInventoryGridSlot> UFVInventoryUIStatics::ItemsGrid_GetGridSlotsData(UWidget* Target)
{
	return IsValid(Target) ? IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetGridSlotsData(Target) : TSet<FFVInventoryGridSlot>();
}

UUserWidget* UFVInventoryUIStatics::ItemsGrid_GetItemWidgetInSlot(UWidget* Target, const int32 SlotIndex)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>()
	? IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetItemWidgetInSlot(Target, SlotIndex) : nullptr;
}

UUserWidget* UFVInventoryUIStatics::ItemsGrid_FindEmptyWidgetSlot(UWidget* Target)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>()
	? IFVAdvancedInventoryItemsGridWidgetInterface::Execute_FindEmptyWidgetSlot(Target) : nullptr;
}

int32 UFVInventoryUIStatics::ItemsGrid_FindEmptySlotIndex(UWidget* Target, const FGuid& ItemId)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>()
	? IFVAdvancedInventoryItemsGridWidgetInterface::Execute_FindEmptySlotIndex(Target, ItemId) : INDEX_NONE;
}

void UFVInventoryUIStatics::ItemsGrid_AddSlot(UWidget* Target, const FFVInventoryGridSlot& SlotData)
{
	if (IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>())
		IFVAdvancedInventoryItemsGridWidgetInterface::Execute_AddSlot(Target, SlotData);
}

bool UFVInventoryUIStatics::ItemsGrid_UpdateItemInSlot(UWidget* Target, const FGuid& ItemId,
	const int32 SlotIndex)
{
	return IsValid(Target) && Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>()
	? IFVAdvancedInventoryItemsGridWidgetInterface::Execute_UpdateItemInSlot(Target, ItemId, SlotIndex) : false;
}

int32 UFVInventoryUIStatics::Helper_FindEmptyGridSlotIndex(const UWidget* Target, const FGuid& ItemId, UObject* ParentInventory)
{
	if (!IsValid(Target)) return INDEX_NONE;
	if (!Target->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>()) return INDEX_NONE;
	if (!ItemId.IsValid()) return INDEX_NONE;
	if (!IsValid(ParentInventory)) return INDEX_NONE;

	const bool bHasUIInterface = ParentInventory->Implements<UFVAdvancedInventoryUIManagerInterface>();
	const bool bHasInventoryInterface = ParentInventory->Implements<UFVAdvancedInventoryInterface>();
	if (!bHasUIInterface && !bHasInventoryInterface) return INDEX_NONE;

	auto settings = GetMutableDefault<UFVAdvancedInventorySettings>();
	if (!IsValid(settings)) return INDEX_NONE;

	auto config = settings->AdvancedInventorySettingsConfig.LoadSynchronous();
	if (!IsValid(config)) return INDEX_NONE;
	
	UObject* inventoryObject = nullptr;
	TScriptInterface<IFVAdvancedInventoryInterface> InventoryInterface = nullptr;
	if (bHasUIInterface)
	{
		auto inventoryRef = IFVAdvancedInventoryUIManagerInterface::Execute_GetParentInventory(ParentInventory);
		inventoryObject = inventoryRef.GetObject();
		InventoryInterface = inventoryRef;
	}
	else
	{
		inventoryObject = ParentInventory;
		InventoryInterface = inventoryObject;
	}
	
	if (!IsValid(inventoryObject)) return INDEX_NONE;

	const auto inventoryItem = IFVAdvancedInventoryInterface::Execute_FindItem(inventoryObject, FInventoryItemSearchParams(ItemId));
	if (!inventoryItem.IsItemValid() || !inventoryItem.Template) return INDEX_NONE;

	const auto gridSlots = IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetGridSlotsData(Target).Array();
	if (gridSlots.Num() == 0) return INDEX_NONE;
	
	const bool bIsStackable = UFVInventorySystemStatics::HasFlag(inventoryItem.Template->ItemFlags, EInventoryItemFlags::EIIF_Stackable);
	const bool bAlwaysStack = true; //config->bAlwaysStackStackableItems; This is now in UI config! And it should be in specific UI config class!
	
	return FindEmptyGridSlotRecursive(InventoryInterface, inventoryItem, gridSlots, bIsStackable, bAlwaysStack);
}

int32 UFVInventoryUIStatics::FindEmptyGridSlotRecursive(TScriptInterface<IFVAdvancedInventoryInterface>& InventoryInterface, 
	const FFVInventoryItem& InventoryItem, const TArray<FFVInventoryGridSlot>& GridSlots, const bool bIsStackable, 
	const bool bAlwaysStackItems)
{
	return INDEX_NONE;
	/*
	// For non-stackable items, just find first empty slot
	if (!bIsStackable || !bAlwaysStackItems)
	{
		for (int i = 0; i < GridSlots.Num(); i++)
		{
			const auto& gridSlot = GridSlots[i];
			if (!IsValid(gridSlot.SlotWidget)) continue;
			if (!gridSlot.SlotWidget->Implements<UFVAdvancedInventoryItemSlotWidgetInterface>()) continue;
			
			if (IFVAdvancedInventoryItemSlotWidgetInterface::Execute_IsSlotEmpty(gridSlot.SlotWidget))
				return i;
		}
		return INDEX_NONE;
	}
	
	// For stackable items
	int32 firstEmptySlotIndex = INDEX_NONE;
	int32 leastFilledStackIndex = INDEX_NONE;
	int32 leastFilledStackAmount = MAX_int32;
	const int32 maxStack = InventoryItem.Template->MaxStackSize;
	
	TSet<FFVInventoryGridSlot> itemSlots;
	
	Algo::CopyIf(GridSlots, itemSlots, [&InventoryItem](const FFVInventoryGridSlot& gridSlot) {
		return gridSlot.OccupiedItemId == InventoryItem.GetGuid();
	});
	
	// Storing itemSlots in a temp array first to avoid a dangling reference in line 1475
	TArray<FFVInventoryGridSlot> slotsArray = itemSlots.Array();
	
	// Find slot with this item that has the most available space
	if (bAlwaysStackItems && !slotsArray.IsEmpty())
	{
		for (int i = 0; i < slotsArray.Num(); i++)
		{
			const auto& gridSlot = slotsArray[i];
			if (gridSlot.OccupiedItemId != InventoryItem.GetGuid()) continue;
			
			const int32 currentStack = gridSlot.SlotQuantity;
			if (currentStack >= maxStack) continue;
			
			if (currentStack < leastFilledStackAmount)
			{
				leastFilledStackAmount = currentStack;
				leastFilledStackIndex = i;
			}
		}
	}
	
	// Find first empty slot
	for (int i = 0; i < GridSlots.Num(); i++)
	{
		const auto& gridSlot = GridSlots[i];
		if (!IsValid(gridSlot.SlotWidget)) continue;
		if (!gridSlot.SlotWidget->Implements<UFVAdvancedInventoryItemSlotWidgetInterface>()) continue;
		
		if (IFVAdvancedInventoryItemSlotWidgetInterface::Execute_IsSlotEmpty(gridSlot.SlotWidget))
		{
			firstEmptySlotIndex = i;
			break;
		}
	}
	
	// Return the appropriate slot index
	if (bAlwaysStackItems && leastFilledStackIndex != INDEX_NONE)
		return leastFilledStackIndex;
	return firstEmptySlotIndex;
	*/
}

bool UFVInventoryUIStatics::Helper_ItemsGrid_UpdateItemInSlot(UUserWidget* GridWidget, const FGuid& ItemId, 
	int32 SlotIndex, TScriptInterface<IFVAdvancedInventoryUIManagerInterface> ParentUIComponent)
{
	return false;
	/*
	if (!ItemId.IsValid() || !IsValid(GridWidget)) return false;
	if (!GridWidget->Implements<UFVAdvancedInventoryItemsGridWidgetInterface>()) return false;
	if (!IsValid(ParentUIComponent.GetObject())) return false;
	
	const auto parentInventory = ParentUIComponent->Execute_GetParentInventory(ParentUIComponent.GetObject());
	if (!IsValid(parentInventory.GetObject())) return false;

	const auto item = parentInventory->Execute_FindItem(parentInventory.GetObject(), FInventoryItemSearchParams(ItemId));
	if (!item.IsItemValid()) return false;

	const auto settings = GetMutableDefault<UFVAdvancedInventorySettings>();
	const auto config = settings ? settings->AdvancedInventorySettingsConfig.LoadSynchronous() : nullptr;
		
	const bool bIsStackable = UFVInventoryStatics::HasInventoryFlags(
		item.Template->ItemFlags, static_cast<int32>(EInventoryItemFlags::EIIF_Stackable)
	) && config ? config->bAlwaysStackStackableItems : true;
	
	auto itemSlots = IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetGridSlotsDataForItem(GridWidget, ItemId);
	const int32 currentTotalQuantity = IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetStacksSizeForItem(GridWidget, ItemId);
	const int32 targetTotalQuantity = item.GetQuantity();
	const int32 maxStackSize = item.Template->MaxStackSize;
	
	if (currentTotalQuantity == targetTotalQuantity && itemSlots.Num() > 0)
		return false;

	bool bAnySlotUpdated = false;

	// Helper function to update a slot's quantity and refresh UI
	auto updateSlotQuantity = [&](FFVInventoryGridSlot& slot, const int32 newQuantity) -> void {
		slot.SlotQuantity = newQuantity;
		IFVAdvancedInventoryItemsGridWidgetInterface::Execute_AddSlot(GridWidget, slot);
		IFVAdvancedInventoryItemSlotWidgetInterface::Execute_StoreGridSlotData(slot.SlotWidget, slot);
		ParentUIComponent->Execute_UpdateSlot(ParentUIComponent.GetObject(), slot);
		
		if (IsValid(slot.SlotWidget))
			IFVInventoryGenericWidgetInterface::Execute_RefreshWidget(slot.SlotWidget);
			
		bAnySlotUpdated = true;
	};

	itemSlots.Sort([](const FFVInventoryGridSlot& a, const FFVInventoryGridSlot& b) {
			if (a.SlotPosition.Y != b.SlotPosition.Y)
				return a.SlotPosition.Y > b.SlotPosition.Y; // Sort by Y descending first
			return a.SlotPosition.X > b.SlotPosition.X; // Then by X descending
		});

	if (SlotIndex == INDEX_NONE)
		SlotIndex = 0;
	
	// Case 1: We need to add items (SUM < item Quantity)
	if (targetTotalQuantity > currentTotalQuantity)
	{
		int32 quantityToAdd = targetTotalQuantity - currentTotalQuantity;
		
		// Handle non-stackable items differently
		if (!bIsStackable)
		{
			bool bSpecifiedSlotUsed = false;
			
			while (quantityToAdd > 0)
			{
				int32 emptySlotIndex = INDEX_NONE;
				
				// Use specified slot first if available and not used yet
				if (SlotIndex != INDEX_NONE && !bSpecifiedSlotUsed && 
					IFVAdvancedInventoryItemsGridWidgetInterface::Execute_IsSlotEmpty(GridWidget, SlotIndex))
				{
					emptySlotIndex = SlotIndex;
					bSpecifiedSlotUsed = true;
				}
				else
					emptySlotIndex = IFVAdvancedInventoryItemsGridWidgetInterface::Execute_FindEmptySlotIndex(GridWidget, ItemId);
				
				if (emptySlotIndex == INDEX_NONE) break; // No more empty slots
				
				if (IFVAdvancedInventoryItemsGridWidgetInterface::Execute_AddItemToSlot(GridWidget, ItemId, emptySlotIndex))
				{
					FFVInventoryGridSlot slotData = IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetGridSlotData(GridWidget, emptySlotIndex);
					updateSlotQuantity(slotData, 1); // Each non-stackable item gets quantity of 1
					quantityToAdd--;
				}
				else
				{
					break; // Failed to add to slot
				}
			}
		}
		// Handle stackable items
		else
		{
			// If no slots exist for this item, add it to an empty slot
			if (itemSlots.Num() == 0)
			{
				int32 emptySlotIndex = SlotIndex != INDEX_NONE ? SlotIndex : 
					IFVAdvancedInventoryItemsGridWidgetInterface::Execute_FindEmptySlotIndex(GridWidget, ItemId);
				
				if (emptySlotIndex != INDEX_NONE && 
					IFVAdvancedInventoryItemsGridWidgetInterface::Execute_IsSlotEmpty(GridWidget, emptySlotIndex))
				{
					int32 quantityToAddToSlot = FMath::Min(maxStackSize, quantityToAdd);
					if (IFVAdvancedInventoryItemsGridWidgetInterface::Execute_AddItemToSlot(GridWidget, ItemId, emptySlotIndex))
					{
						FFVInventoryGridSlot slotData = IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetGridSlotData(GridWidget, emptySlotIndex);
						updateSlotQuantity(slotData, quantityToAddToSlot);
						quantityToAdd -= quantityToAddToSlot;
					}
				}
			}
			// Otherwise, first try to add to the specified slot if provided
			else if (SlotIndex != INDEX_NONE && 
					IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetGridSlotsData(GridWidget).Array().IsValidIndex(SlotIndex))
			{
				FFVInventoryGridSlot slotData = IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetGridSlotData(GridWidget, SlotIndex);
				
				if (slotData.IsEmpty())
				{
					int32 quantityToAddToSlot = FMath::Min(maxStackSize, quantityToAdd);
					if (IFVAdvancedInventoryItemsGridWidgetInterface::Execute_AddItemToSlot(GridWidget, ItemId, SlotIndex))
					{
						slotData = IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetGridSlotData(GridWidget, SlotIndex);
						updateSlotQuantity(slotData, quantityToAddToSlot);
						quantityToAdd -= quantityToAddToSlot;
					}
				}
				else if (slotData.OccupiedItemId == ItemId)
				{
					int32 availableSpace = maxStackSize - slotData.SlotQuantity;
					int32 quantityToAddToSlot = FMath::Min(availableSpace, quantityToAdd);
					
					if (quantityToAddToSlot > 0)
					{
						updateSlotQuantity(slotData, slotData.SlotQuantity + quantityToAddToSlot);
						quantityToAdd -= quantityToAddToSlot;
					}
				}

				itemSlots.Remove(slotData);
			}
			
			// Fill up existing item slots
			if (quantityToAdd > 0)
			{
				for (auto& slot : itemSlots)
				{
					if (quantityToAdd <= 0) break;
					
					const int32 availableSpace = maxStackSize - slot.SlotQuantity;
					const int32 quantityToAddToSlot = FMath::Min(availableSpace, quantityToAdd);
					
					if (quantityToAddToSlot > 0)
					{
						updateSlotQuantity(slot, slot.SlotQuantity + quantityToAddToSlot);
						quantityToAdd -= quantityToAddToSlot;
					}
				}
			}
			
			// Create new slots if needed
			while (quantityToAdd > 0)
			{
				int32 emptySlotIndex = IFVAdvancedInventoryItemsGridWidgetInterface::Execute_FindEmptySlotIndex(GridWidget, ItemId);
				if (emptySlotIndex == INDEX_NONE) break;
				
				int32 quantityToAddToSlot = FMath::Min(maxStackSize, quantityToAdd);

				if (IFVAdvancedInventoryItemsGridWidgetInterface::Execute_AddItemToSlot(GridWidget, ItemId, emptySlotIndex))
				{
					FFVInventoryGridSlot slotData = IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetGridSlotData(GridWidget, emptySlotIndex);
					updateSlotQuantity(slotData, quantityToAddToSlot);
					quantityToAdd -= quantityToAddToSlot;
				}
				else
				{
					break;
				}
			}
		}
	}
	
	// Case 2: We need to remove items (SUM > item Quantity)
	else if (targetTotalQuantity < currentTotalQuantity)
	{
		int32 quantityToRemove = currentTotalQuantity - targetTotalQuantity;
	
		if (SlotIndex != INDEX_NONE && 
			IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetGridSlotsData(GridWidget).Array().IsValidIndex(SlotIndex))
		{
			FFVInventoryGridSlot slotData = IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetGridSlotData(GridWidget, SlotIndex);
			if (slotData.OccupiedItemId == ItemId)
			{
				const int32 quantityToRemoveFromSlot = FMath::Min(slotData.SlotQuantity, quantityToRemove);
			
				if (quantityToRemoveFromSlot >= slotData.SlotQuantity)
				{
					IFVAdvancedInventoryItemsGridWidgetInterface::Execute_RemoveItemFromSlot(GridWidget, SlotIndex);
					bAnySlotUpdated = true;
				}
				else
					updateSlotQuantity(slotData, slotData.SlotQuantity - quantityToRemoveFromSlot);
			
				quantityToRemove -= quantityToRemoveFromSlot;
				itemSlots.Remove(slotData);
			}
		}
		
		if (quantityToRemove > 0)
		{
			auto itemSlotsArray = itemSlots.Array();
			for (int32 i = 0; i < itemSlots.Num(); i++)
			{
				if (!itemSlotsArray.IsValidIndex(i)) continue;
				
				auto& slot =itemSlotsArray[i];
				if (quantityToRemove <= 0) break;
			
				const int32 slotIndex = IFVAdvancedInventoryItemsGridWidgetInterface::Execute_GetGridSlotIndexByCoords(GridWidget, slot.SlotPosition);
				if (slotIndex == INDEX_NONE) continue;
			
				const int32 quantityToRemoveFromSlot = FMath::Min(slot.SlotQuantity, quantityToRemove);
			
				if (quantityToRemoveFromSlot >= slot.SlotQuantity)
				{
					IFVAdvancedInventoryItemsGridWidgetInterface::Execute_RemoveItemFromSlot(GridWidget, slotIndex);
					bAnySlotUpdated = true;
				}
				else
					updateSlotQuantity(slot, slot.SlotQuantity - quantityToRemoveFromSlot);
			
				quantityToRemove -= quantityToRemoveFromSlot;
			}
		}
	}

	return bAnySlotUpdated;
	*/
}

bool UFVInventoryUIStatics::ItemPreview_InitializeInteractableWidget(UFVAdvancedInventoryInteractableObjectWidget* Target)
{
	return IsValid(Target) ? Target->InitializeInteractableWidget() : false;
}

void UFVInventoryUIStatics::ItemPreview_SetPreviewMesh(UFVAdvancedInventoryInteractableObjectWidget* Target,
	UStaticMesh* StaticMesh)
{
	if (IsValid(Target))
		Target->SetPreviewMesh(StaticMesh);
}

void UFVInventoryUIStatics::ItemPreview_SetPreviewSkeletalMesh(
	UFVAdvancedInventoryInteractableObjectWidget* Target, USkeletalMesh* SkeletalMesh)
{
	if (IsValid(Target))
		Target->SetPreviewSkeletalMesh(SkeletalMesh);
}

void UFVInventoryUIStatics::ItemPreview_ClearPreview(UFVAdvancedInventoryInteractableObjectWidget* Target)
{
	if (IsValid(Target))
		Target->ClearPreview();
}

void UFVInventoryUIStatics::ItemPreview_ResetPreview(UFVAdvancedInventoryInteractableObjectWidget* Target)
{
	if (IsValid(Target))
		Target->ResetCameraToDefaults();
}

void UFVInventoryUIStatics::ItemPreview_UpdateCameraRotation(UFVAdvancedInventoryInteractableObjectWidget* Target, 
	const FVector2f& MouseDelta)
{
	if (IsValid(Target))
		Target->ProcessRotationInput(MouseDelta);
}

void UFVInventoryUIStatics::ItemPreview_UpdateCameraHeight(UFVAdvancedInventoryInteractableObjectWidget* Target, 
	const FVector2f& MouseDelta)
{
	if (IsValid(Target))
		Target->ProcessHeightInput(MouseDelta);
}

void UFVInventoryUIStatics::ItemPreview_UpdateCameraZoom(UFVAdvancedInventoryInteractableObjectWidget* Target,
	const float WheelDelta)
{
	if (IsValid(Target))
		ItemPreview_UpdateCameraZoom(Target, WheelDelta);
}

void UFVInventoryUIStatics::ItemPreview_SetCameraRotationAbsolute(UFVAdvancedInventoryInteractableObjectWidget* Target, 
	const float YawNormalized, const float PitchNormalized)
{
	if (IsValid(Target))
		Target->SetCameraRotationAbsolute(YawNormalized, PitchNormalized);
}

void UFVInventoryUIStatics::ItemPreview_SetCameraHeightAbsolute(UFVAdvancedInventoryInteractableObjectWidget* Target, 
	const float HeightNormalized)
{
	if (IsValid(Target))
		Target->SetCameraHeightAbsolute(HeightNormalized);
}

void UFVInventoryUIStatics::ItemPreview_SetCameraZoomAbsolute(UFVAdvancedInventoryInteractableObjectWidget* Target, 
	const float ZoomNormalized)
{
	if (IsValid(Target))
		Target->SetCameraZoomAbsolute(ZoomNormalized);
}

void UFVInventoryUIStatics::ItemPreview_UpdateCameraRotationAnalog(UFVAdvancedInventoryInteractableObjectWidget* Target, 
	const FVector2f& AnalogInput, const float DeltaTime)
{
	if (IsValid(Target))
		Target->ProcessAnalogRotation(AnalogInput, DeltaTime);
}

void UFVInventoryUIStatics::ItemPreview_UpdateCameraHeightAnalog(UFVAdvancedInventoryInteractableObjectWidget* Target, 
	const float AnalogInput, const float DeltaTime)
{
	if (IsValid(Target))
		Target->ProcessAnalogHeight(AnalogInput, DeltaTime);
}

void UFVInventoryUIStatics::ItemPreview_UpdateCameraZoomAnalog(UFVAdvancedInventoryInteractableObjectWidget* Target, 
	const float AnalogInput, const float DeltaTime)
{
	if (IsValid(Target))
		Target->ProcessAnalogZoom(AnalogInput, DeltaTime);
}
