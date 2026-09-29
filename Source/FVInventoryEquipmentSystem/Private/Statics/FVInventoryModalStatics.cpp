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

#include "Statics/FVInventoryModalStatics.h"

#include "Blueprint/UserWidget.h"
#include "Definitions/FVInventoryBaseCommands.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Helpers/FVJsonObject.h"
#include "Interfaces/Widgets/FVInventoryGenericWidgetInterface.h"
#include "Interfaces/Widgets/Modal/FVAdvancedInventoryModalContentWidgetInterface.h"
#include "Interfaces/Widgets/Modal/FVAdvancedInventoryModalWidgetInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Settings/FVAdvancedInventoryGlobalUIConfig.h"
#include "Statics/FVInventoryBaseUIStatics.h"

#define FV_BIND_MODAL_CONTENT_DELEGATE(Target, Binding, HandleGetter) \
	if (!IsValid(Target) || !(Binding).IsBound()) \
		return false; \
	IFVAdvancedInventoryModalContentWidgetInterface* nativeInterface = Cast<IFVAdvancedInventoryModalContentWidgetInterface>(Target); \
	if (!nativeInterface) \
		return false; \
	nativeInterface->HandleGetter().AddUnique(Binding); \
	return true

#define FV_UNBIND_MODAL_CONTENT_DELEGATE(Target, Binding, HandleGetter) \
	if (!IsValid(Target) || !(Binding).IsBound()) \
		return false; \
	IFVAdvancedInventoryModalContentWidgetInterface* nativeInterface = Cast<IFVAdvancedInventoryModalContentWidgetInterface>(Target); \
	if (!nativeInterface) \
		return false; \
	nativeInterface->HandleGetter().Remove(Binding); \
	return true

#define FV_BIND_MODAL_WINDOW_DELEGATE(Target, Binding, HandleGetter) \
	if (!IsValid(Target) || !(Binding).IsBound()) \
		return false; \
	IFVAdvancedInventoryModalWidgetInterface* nativeInterface = Cast<IFVAdvancedInventoryModalWidgetInterface>(Target); \
	if (!nativeInterface) \
		return false; \
	nativeInterface->HandleGetter().AddUnique(Binding); \
	return true

#define FV_UNBIND_MODAL_WINDOW_DELEGATE(Target, Binding, HandleGetter) \
	if (!IsValid(Target) || !(Binding).IsBound()) \
		return false; \
	IFVAdvancedInventoryModalWidgetInterface* nativeInterface = Cast<IFVAdvancedInventoryModalWidgetInterface>(Target); \
	if (!nativeInterface) \
		return false; \
	nativeInterface->HandleGetter().Remove(Binding); \
	return true

UUserWidget* UFVInventoryModalStatics::CreateModalWindowWidget(UObject* Context, UObject* OptionalPayload)
{
	if (!IsValid(Context))
		return nullptr;

	UFVAdvancedInventoryGlobalUIConfig* globalUIConfig = GetGlobalUIConfig();
	if (!IsValid(globalUIConfig))
		return nullptr;

	if (globalUIConfig->ModalWindowWidgetClass.IsNull())
		return nullptr;

	TSubclassOf<UUserWidget> widgetClass = globalUIConfig->ModalWindowWidgetClass.LoadSynchronous();
	if (!widgetClass || !widgetClass->ImplementsInterface(UFVAdvancedInventoryModalWidgetInterface::StaticClass()))
		return nullptr;

	UUserWidget* newWidget = CreateWidgetFromClass(Context, widgetClass);
	if (!IsValid(newWidget))
		return nullptr;

	if (newWidget->Implements<UFVInventoryGenericWidgetInterface>())
		IFVInventoryGenericWidgetInterface::Execute_ProcessInventoryWidgetCommand(newWidget, InventoryUICommands::General::Create, OptionalPayload);

	return newWidget;
}

UUserWidget* UFVInventoryModalStatics::CreateModalContentWidget(UObject* Context, const FString& ModalType, UFVJsonObject* InPayload)
{
	if (!IsValid(Context))
		return nullptr;

	if (ModalType.IsEmpty() || ModalType.Equals(TEXT("none"), ESearchCase::IgnoreCase))
		return nullptr;

	UFVAdvancedInventoryGlobalUIConfig* globalUIConfig = GetGlobalUIConfig();
	if (!IsValid(globalUIConfig))
		return nullptr;

	const FFVModalDefinition* modalDefinition = globalUIConfig->Modals.Find(ModalType);
	if (!modalDefinition || modalDefinition->WidgetClass.IsNull())
		return nullptr;

	TSubclassOf<UUserWidget> widgetClass = modalDefinition->WidgetClass.LoadSynchronous();
	if (!widgetClass || !widgetClass->ImplementsInterface(UFVAdvancedInventoryModalContentWidgetInterface::StaticClass()))
		return nullptr;

	UUserWidget* newWidget = CreateWidgetFromClass(Context, widgetClass);
	if (!IsValid(newWidget))
		return nullptr;

	IFVAdvancedInventoryModalContentWidgetInterface::Execute_ConstructModalContent(newWidget, InPayload);

	return newWidget;
}

FString UFVInventoryModalStatics::GetModalTypeString(const FString& ModalType)
{
	return ModalType;
}

void UFVInventoryModalStatics::AddModalContentToModalWindow(UObject* Target, UUserWidget* ModalContentWidget, UFVJsonObject* Payload)
{
	if (!IsValid(Target) || !IsValid(ModalContentWidget))
		return;

	if (Target->Implements<UFVAdvancedInventoryModalWidgetInterface>())
		IFVAdvancedInventoryModalWidgetInterface::Execute_AddModalContentToModalWindow(Target, ModalContentWidget, Payload);
}

bool UFVInventoryModalStatics::BindToOnModalContentAddedToModalWindow(UObject* Target, const FFVModalContentAddedToModalWindowBinding& Binding)
{
	FV_BIND_MODAL_WINDOW_DELEGATE(Target, Binding, GetOnModalContentAddedToModalWindowHandle);
}

bool UFVInventoryModalStatics::UnbindFromOnModalContentAddedToModalWindow(UObject* Target, const FFVModalContentAddedToModalWindowBinding& Binding)
{
	FV_UNBIND_MODAL_WINDOW_DELEGATE(Target, Binding, GetOnModalContentAddedToModalWindowHandle);
}

bool UFVInventoryModalStatics::BindToOnModalConfirmed(UObject* Target, const FFVModalConfirmedBinding& Binding)
{
	FV_BIND_MODAL_WINDOW_DELEGATE(Target, Binding, GetOnModalConfirmedHandle);
}

bool UFVInventoryModalStatics::UnbindFromOnModalConfirmed(UObject* Target, const FFVModalConfirmedBinding& Binding)
{
	FV_UNBIND_MODAL_WINDOW_DELEGATE(Target, Binding, GetOnModalConfirmedHandle);
}

bool UFVInventoryModalStatics::BindToOnModalCancelled(UObject* Target, const FFVModalCancelledBinding& Binding)
{
	FV_BIND_MODAL_WINDOW_DELEGATE(Target, Binding, GetOnModalCancelledHandle);
}

bool UFVInventoryModalStatics::UnbindFromOnModalCancelled(UObject* Target, const FFVModalCancelledBinding& Binding)
{
	FV_UNBIND_MODAL_WINDOW_DELEGATE(Target, Binding, GetOnModalCancelledHandle);
}

bool UFVInventoryModalStatics::BindToOnModalContentConfirmed(UObject* Target, const FFVModalContentConfirmedBinding& Binding)
{
	FV_BIND_MODAL_CONTENT_DELEGATE(Target, Binding, GetOnModalContentConfirmedHandle);
}

bool UFVInventoryModalStatics::UnbindFromOnModalContentConfirmed(UObject* Target, const FFVModalContentConfirmedBinding& Binding)
{
	FV_UNBIND_MODAL_CONTENT_DELEGATE(Target, Binding, GetOnModalContentConfirmedHandle);
}

bool UFVInventoryModalStatics::BindToOnModalContentCancelled(UObject* Target, const FFVModalContentCancelledBinding& Binding)
{
	FV_BIND_MODAL_CONTENT_DELEGATE(Target, Binding, GetOnModalContentCancelledHandle);
}

bool UFVInventoryModalStatics::UnbindFromOnModalContentCancelled(UObject* Target, const FFVModalContentCancelledBinding& Binding)
{
	FV_UNBIND_MODAL_CONTENT_DELEGATE(Target, Binding, GetOnModalContentCancelledHandle);
}

bool UFVInventoryModalStatics::GetModalConfigFromDataTables(const FString& Key, FFVModalsConfig& OutConfig)
{
	if (Key.IsEmpty() || Key.Equals(TEXT("none"), ESearchCase::IgnoreCase))
		return false;

	const UFVAdvancedInventoryGlobalUIConfig* globalUIConfig = GetGlobalUIConfig();
	if (!IsValid(globalUIConfig))
		return false;

	const FName rowName(*Key);
	for (const TSoftObjectPtr<UDataTable>& softDataTable : globalUIConfig->ModalsData)
	{
		UDataTable* dataTable = softDataTable.LoadSynchronous();
		if (!IsValid(dataTable))
			continue;

		FString contextString;
		if (const FFVModalsConfig* row = dataTable->FindRow<FFVModalsConfig>(rowName, contextString))
		{
			OutConfig = *row;
			return true;
		}
	}

	return false;
}

void UFVInventoryModalStatics::StartModalExpiration(UObject* Target, const float WidgetLifetime)
{
	if (!IsValid(Target))
		return;
	
	IFVAdvancedInventoryModalContentWidgetInterface::Execute_StartExpiration(Target, WidgetLifetime);
}

UUserWidget* UFVInventoryModalStatics::CreateWidgetFromClass(UObject* Context, const TSubclassOf<UUserWidget> WidgetClass)
{
	if (!IsValid(Context) || !WidgetClass)
		return nullptr;

	if (APlayerController* owningPlayer = ResolveOwningPlayer(Context))
		return CreateWidget<UUserWidget>(owningPlayer, WidgetClass);

	if (UWorld* world = Context->GetWorld())
		return CreateWidget<UUserWidget>(world, WidgetClass);

	return nullptr;
}

APlayerController* UFVInventoryModalStatics::ResolveOwningPlayer(UObject* Context)
{
	if (!IsValid(Context))
		return nullptr;

	if (APlayerController* playerController = Cast<APlayerController>(Context))
		return playerController;

	if (const UUserWidget* userWidget = Cast<UUserWidget>(Context))
	{
		if (APlayerController* owningPlayer = userWidget->GetOwningPlayer())
			return owningPlayer;
	}

	return UGameplayStatics::GetPlayerController(Context, 0);
}

UFVAdvancedInventoryGlobalUIConfig* UFVInventoryModalStatics::GetGlobalUIConfig()
{
	return UFVInventoryBaseUIStatics::GetGlobalUIConfig();
}

