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


#include "Statics/FVInventoryNotificationsStatics.h"

#include "Interfaces/Widgets/Notification/FVInventoryNotificationContainerWidgetInterface.h"

void UFVInventoryNotificationsStatics::CreateNotification(
	UObject* Notification,	const FInventoryNotificationData& NotificationData, const TScriptInterface<IFVInventoryNotificationContainerWidgetInterface>& Container)
{
	if (!IsValid(Notification))
		return;

	if (Notification->Implements<UFVInventoryNotificationWidgetInterface>())
		return IFVInventoryNotificationWidgetInterface::Execute_CreateNotification(Notification, NotificationData, Container);
}

void UFVInventoryNotificationsStatics::ExpireNotification(UObject* Notification)
{
	if (!IsValid(Notification))
		return;

	if (Notification->Implements<UFVInventoryNotificationWidgetInterface>())
		return IFVInventoryNotificationWidgetInterface::Execute_ExpireNotification(Notification);
}

void UFVInventoryNotificationsStatics::DeleteNotification(UObject* Notification)
{
	if (!IsValid(Notification))
		return;

	if (Notification->Implements<UFVInventoryNotificationWidgetInterface>())
		return IFVInventoryNotificationWidgetInterface::Execute_DeleteNotification(Notification);
}

void UFVInventoryNotificationsStatics::AddNotification(UObject* Container, UUserWidget* NewNotification)
{
	if (!IsValid(Container))
		return;

	if (Container->Implements<UFVInventoryNotificationContainerWidgetInterface>())
		return IFVInventoryNotificationContainerWidgetInterface::Execute_AddNotification(Container, NewNotification);
}

void UFVInventoryNotificationsStatics::RemoveNotification(UObject* Container, UUserWidget* Notification)
{
	if (!IsValid(Container))
		return;

	if (Container->Implements<UFVInventoryNotificationContainerWidgetInterface>())
		return IFVInventoryNotificationContainerWidgetInterface::Execute_RemoveNotification(Container, Notification);
}

void UFVInventoryNotificationsStatics::ClearNotifications(UObject* Container)
{
	if (!IsValid(Container))
		return;

	if (Container->Implements<UFVInventoryNotificationContainerWidgetInterface>())
		return IFVInventoryNotificationContainerWidgetInterface::Execute_ClearNotifications(Container);
}

void UFVInventoryNotificationsStatics::SetNotificationsContainerVisibility(UObject* Container,
	const bool bShowContainer)
{
	if (!IsValid(Container))
		return;

	if (Container->Implements<UFVInventoryNotificationContainerWidgetInterface>())
		return IFVInventoryNotificationContainerWidgetInterface::Execute_SetNotificationsContainerVisibility(Container, bShowContainer);
}
