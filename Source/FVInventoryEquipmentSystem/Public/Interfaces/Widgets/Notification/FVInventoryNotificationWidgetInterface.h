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
#include "Definitions/FVAdvancedInventoryNotification.h"
#include "UObject/Interface.h"
#include "FVInventoryNotificationWidgetInterface.generated.h"

class IFVInventoryNotificationContainerWidgetInterface;
struct FInventoryNotificationData;

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UFVInventoryNotificationWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FVINVENTORYEQUIPMENTSYSTEM_API IFVInventoryNotificationWidgetInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Notification")
	void CreateNotification(const FInventoryNotificationData& NotificationData, const TScriptInterface<IFVInventoryNotificationContainerWidgetInterface>& Container);
	virtual void CreateNotification_Implementation(const FInventoryNotificationData& NotificationData, const TScriptInterface<IFVInventoryNotificationContainerWidgetInterface>& Container) = 0;

	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Notification")
	void ExpireNotification();
	virtual void ExpireNotification_Implementation() = 0;
	
	UFUNCTION(BlueprintNativeEvent, Category="FV|Inventory & Equipment|Notification")
	void DeleteNotification();
	virtual void DeleteNotification_Implementation() = 0;
};
