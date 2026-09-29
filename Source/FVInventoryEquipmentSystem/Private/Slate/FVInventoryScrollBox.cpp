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


#include "Slate/FVInventoryScrollBox.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/VerticalBox.h"
#include "Statics/FVInventoryUIStatics.h"

void UFVInventoryScrollBox::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (bCaptureInput)
		SetIsFocusable(true);
}

int32 UFVInventoryScrollBox::GetChildrenCount() const
{
	return VerticalBox ? VerticalBox->GetChildrenCount() : INDEX_NONE;
}

void UFVInventoryScrollBox::AddChild(UWidget* Content) const
{
	if (!Content || !VerticalBox)
		return;
	VerticalBox->AddChild(Content);
}

void UFVInventoryScrollBox::RemoveChild(UWidget* Content) const
{
	if (Content && VerticalBox)
		VerticalBox->RemoveChild(Content);
}

void UFVInventoryScrollBox::RemoveChildAt(const int32 Index) const
{
	if (VerticalBox && Index != INDEX_NONE && VerticalBox->GetChildAt(Index) != nullptr)
		VerticalBox->RemoveChildAt(Index);
}

void UFVInventoryScrollBox::ResetChildren()
{
	if (VerticalBox)
	{
		ActiveIndex = INDEX_NONE;
		VerticalBox->ClearChildren();
	}
	
	UnlockScroll();
}

TSharedRef<SWidget> UFVInventoryScrollBox::RebuildWidget()
{
	if (WidgetTree && !VerticalBox)
	{
		VerticalBox = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			TEXT("VerticalBox")
			);
		WidgetTree->RootWidget = VerticalBox;
	}
	
	return Super::RebuildWidget();
}

void UFVInventoryScrollBox::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	InterpolateToTarget(InDeltaTime);
}

FReply UFVInventoryScrollBox::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bCaptureInput)
		return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
	
	if (bUseScrollDelay && bScrollLocked)
		return FReply::Handled();

	if (bUseScrollDelay)
	{
		bScrollLocked = true;

		if (UWorld* world = GetWorld())
		{
			world->GetTimerManager().SetTimer(
				ScrollDelayTimerHandle,
				this,
				&UFVInventoryScrollBox::UnlockScroll,
				ScrollDelay,
				false
			);
		}
	}

	const float wheelDelta = InMouseEvent.GetWheelDelta();
	BroadcastIndexChange(wheelDelta > 0 ? -1 : 1);

	return FReply::Handled();
}


FReply UFVInventoryScrollBox::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (!bCaptureInput)
		return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
	
	const FKey key = InKeyEvent.GetKey();

	// Keys that should move the selection
	const bool bUpKey =
		(key == EKeys::Up || key == EKeys::W || key == EKeys::Gamepad_DPad_Up);
	const bool bDownKey =
		(key == EKeys::Down || key == EKeys::S || key == EKeys::Gamepad_DPad_Down);

	if (!bUpKey && !bDownKey)
		return Super::NativeOnKeyDown(InGeometry, InKeyEvent);

	// Delay
	if (bUseScrollDelay && bScrollLocked)
		return FReply::Handled();

	if (bUseScrollDelay)
	{
		bScrollLocked = true;

		if (UWorld* world = GetWorld())
		{
			world->GetTimerManager().SetTimer(
				ScrollDelayTimerHandle,
				this,
				&UFVInventoryScrollBox::UnlockScroll,
				ScrollDelay,
				false
			);
		}
	}

	const int32 deltaValue = bUpKey ? -1 : 1;
	BroadcastIndexChange(deltaValue);

	return FReply::Handled();
}


FReply UFVInventoryScrollBox::NativeOnAnalogValueChanged(const FGeometry& InGeometry,
	const FAnalogInputEvent& InAnalogEvent)
{
	if (!bCaptureInput)
		return Super::NativeOnAnalogValueChanged(InGeometry, InAnalogEvent);
	
	const FKey key = InAnalogEvent.GetKey();
	const float value = InAnalogEvent.GetAnalogValue();

	if (key != EKeys::Gamepad_LeftY || FMath::Abs(value) <= 0.5f)
		return Super::NativeOnAnalogValueChanged(InGeometry, InAnalogEvent);
	
	if (bUseScrollDelay && bScrollLocked)
		return FReply::Handled();

	if (bUseScrollDelay)
	{
		bScrollLocked = true;

		if (UWorld* world = GetWorld())
		{
			world->GetTimerManager().SetTimer(
				ScrollDelayTimerHandle,
				this,
				&UFVInventoryScrollBox::UnlockScroll,
				ScrollDelay,
				false
			);
		}
	}

	BroadcastIndexChange(value > 0 ? -1 : 1);

	return FReply::Handled();
}


void UFVInventoryScrollBox::SetActiveIndex(const int32 NewIndex)
{
	if (!VerticalBox)
		return;
	
	const int32 maxIndex = VerticalBox->GetChildrenCount() - 1;
	ActiveIndex = FMath::Clamp(NewIndex, 0, maxIndex);
	CalculateDesiredTransform();
}

TArray<UWidget*> UFVInventoryScrollBox::GetChildWidgets() const
{
	return VerticalBox ? VerticalBox->GetAllChildren() : TArray<UWidget*>();
}

void UFVInventoryScrollBox::BroadcastIndexChange(const int32 Delta) const
{
	int32 newIndex = ActiveIndex + Delta;
	
	if (bWrapAround)
	{
		if (newIndex < 0)
			newIndex = VerticalBox->GetChildrenCount() - 1 ;
		else if (newIndex > VerticalBox->GetChildrenCount() - 1)
			newIndex = 0;
	}
	else
		newIndex = FMath::Clamp(newIndex, 0, VerticalBox->GetChildrenCount() - 1);
	
	if (newIndex != ActiveIndex)
		OnNewIndexCalculated.Broadcast(newIndex);
}

void UFVInventoryScrollBox::CalculateDesiredTransform()
{
	if (!VerticalBox)
		return;
	
	TargetTranslation = UFVInventoryUIStatics::CalculateCenteredListTranslation(
		VerticalBox,
		ActiveIndex
	);
}

void UFVInventoryScrollBox::InterpolateToTarget(const float DeltaTime)
{
	if (!VerticalBox)
		return;
	
	const float currentLength = CurrentTranslation.Size();
	const float targetLength = TargetTranslation.Size();
	
	if (FMath::IsNearlyEqual(currentLength, targetLength, 0.1f))
		return;
	
	CurrentTranslation = FMath::Vector2DInterpTo(CurrentTranslation, TargetTranslation, DeltaTime, InterpolationSpeed);
	VerticalBox->SetRenderTranslation(CurrentTranslation);
}

void UFVInventoryScrollBox::UnlockScroll()
{
	bScrollLocked = false;
}
