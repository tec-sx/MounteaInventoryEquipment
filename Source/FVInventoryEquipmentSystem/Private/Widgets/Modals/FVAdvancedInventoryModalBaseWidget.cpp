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

#include "Widgets/Modals/FVAdvancedInventoryModalBaseWidget.h"

#include "Components/PanelWidget.h"
#include "Interfaces/Widgets/Modal/FVAdvancedInventoryModalContentWidgetInterface.h"

void UFVAdvancedInventoryModalBaseWidget::AddModalContentToModalWindow_Implementation(UUserWidget* ModalContentWidget, UFVJsonObject* Payload)
{
	if (!IsValid(ModalContentWidget))
		return;

	if (IFVAdvancedInventoryModalContentWidgetInterface* previousContent = Cast<IFVAdvancedInventoryModalContentWidgetInterface>(CurrentModalContentWidget))
	{
		previousContent->GetOnModalContentConfirmedHandle().RemoveDynamic(this, &UFVAdvancedInventoryModalBaseWidget::HandleModalContentConfirmed);
		previousContent->GetOnModalContentCancelledHandle().RemoveDynamic(this, &UFVAdvancedInventoryModalBaseWidget::HandleModalContentCancelled);
	}

	CurrentModalContentWidget = ModalContentWidget;

	if (IFVAdvancedInventoryModalContentWidgetInterface* newContent = Cast<IFVAdvancedInventoryModalContentWidgetInterface>(CurrentModalContentWidget))
	{
		newContent->GetOnModalContentConfirmedHandle().AddUniqueDynamic(this, &UFVAdvancedInventoryModalBaseWidget::HandleModalContentConfirmed);
		newContent->GetOnModalContentCancelledHandle().AddUniqueDynamic(this, &UFVAdvancedInventoryModalBaseWidget::HandleModalContentCancelled);
	}
	
	OnModalContentAddedToModalWindow.Broadcast(ModalContentWidget, Payload);
}

void UFVAdvancedInventoryModalBaseWidget::HandleModalContentConfirmed(UObject* Payload)
{
	Execute_ModalContentConfirmed(this, Payload);
	OnModalConfirmed.Broadcast(Payload);
}

void UFVAdvancedInventoryModalBaseWidget::HandleModalContentCancelled()
{
	Execute_ModalContentCancelled(this);
	OnModalCancelled.Broadcast();
}
