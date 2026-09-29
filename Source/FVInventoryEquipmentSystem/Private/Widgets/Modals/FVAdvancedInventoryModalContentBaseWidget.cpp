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

#include "Widgets/Modals/FVAdvancedInventoryModalContentBaseWidget.h"

void UFVAdvancedInventoryModalContentBaseWidget::OnModalExpired()
{
	ModalContentExpiryTimerHandle.Invalidate();
	OnModalContentCancelled.Broadcast();
}

void UFVAdvancedInventoryModalContentBaseWidget::ConstructModalContent_Implementation(UFVJsonObject* InPayload)
{
	IncomingPayload = InPayload;
}

void UFVAdvancedInventoryModalContentBaseWidget::StartExpiration_Implementation(const float WidgetLifetime)
{
	if (!GetWorld())
		return;
	
	GetWorld()->GetTimerManager().SetTimer(ModalContentExpiryTimerHandle, this, &UFVAdvancedInventoryModalContentBaseWidget::OnModalExpired, WidgetLifetime);
}
