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


#include "Statics/FVInventoryGenericWidgetStatics.h"

#include "GameplayTagContainer.h"
#include "Components/Widget.h"

bool UFVInventoryGenericWidgetStatics::IsValidGenericWidget(const UObject* Target)
{
	return IsValid(Target) && Target->Implements<UFVInventoryGenericWidgetInterface>();
}

void UFVInventoryGenericWidgetStatics::ProcessInventoryWidgetCommand(UObject* GenericWidget, const FString& Command, UObject* OptionalPayload)
{
	if (IsValidGenericWidget(GenericWidget))
		return IFVInventoryGenericWidgetInterface::Execute_ProcessInventoryWidgetCommand(GenericWidget, Command, OptionalPayload);
}

void UFVInventoryGenericWidgetStatics::ConsumeUIInput(UWidget* Target, const FGameplayTag InputTag, 
	const FFVWidgetInputPayload& Payload, const float DeltaTime)
{
	if (IsValidGenericWidget(Target))
		IFVInventoryGenericWidgetInterface::Execute_ConsumeUIInput(Target, InputTag, Payload, DeltaTime);
}

void UFVInventoryGenericWidgetStatics::RefreshWidget(UWidget* Target)
{
	if (IsValidGenericWidget(Target))
		IFVInventoryGenericWidgetInterface::Execute_RefreshWidget(Target);
}

void UFVInventoryGenericWidgetStatics::ApplyTheme(UWidget* Target)
{
	if (IsValidGenericWidget(Target))
		IFVInventoryGenericWidgetInterface::Execute_ApplyTheme(Target);
}

void UFVInventoryGenericWidgetStatics::SetWidgetTag(UWidget* Target, const FGameplayTag& WidgetTag)
{
	if (IsValidGenericWidget(Target))
		IFVInventoryGenericWidgetInterface::Execute_SetWidgetTag(Target, WidgetTag);
}

FGameplayTag UFVInventoryGenericWidgetStatics::GetWidgetTag(UWidget* Target)
{
	return IsValidGenericWidget(Target) ? IFVInventoryGenericWidgetInterface::Execute_GetWidgetTag(Target) : FGameplayTag();
}
