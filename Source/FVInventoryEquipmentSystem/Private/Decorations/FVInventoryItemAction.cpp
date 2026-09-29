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


#include "Decorations/FVInventoryItemAction.h"

#include "Definitions/FVInventoryItemTemplate.h"
#include "Interfaces/Widgets/Items/FVAdvancedInventoryItemWidgetInterface.h"
#include "Logs/FVAdvancedInventoryLog.h"
#include "Statics/FVInventoryStatics.h"

bool UFVInventoryItemAction::InitializeItemAction_Implementation(const FFVInventoryItem& NewTargetItem,
 const TScriptInterface<IFVAdvancedInventoryInterface>& NewOwningInventory, UObject* ContextPayload)
{
	if (!NewTargetItem.IsItemValid() || NewTargetItem.OwningInventory != NewOwningInventory || !IsValid(NewOwningInventory.GetObject()))
		return false;
	
	return true;
}

TScriptInterface<IFVAdvancedInventoryInterface> UFVInventoryItemAction::GetOwningInventory_Implementation() const
{
	return nullptr;
}

bool UFVInventoryItemAction::IsAllowed_Implementation(const FFVInventoryItem& TargetItem) const
{
	if (!TargetItem.IsItemValid())
		return false;

	if (!IsValid(TargetItem.Template))
		return false;

	return false;
}

FText UFVInventoryItemAction::GetDisallowedReason_Implementation(const FFVInventoryItem& TargetItem) const
{
	if (!TargetItem.IsItemValid())
		return FText::FromString(TEXT("Invalid target item"));

	return FText::FromString(TEXT("Action is not currently available"));
}

bool UFVInventoryItemAction::ExecuteInventoryAction_Implementation(const FFVInventoryItem& TargetItem)
{
	if (!IsAllowed(TargetItem))
	{
		LOG_WARNING(TEXT("[%s] Action not allowed: %s"), *GetName(), *GetDisallowedReason(TargetItem).ToString())
		return false;
	}
	
	return true;
}

void UFVInventoryItemAction::CancelInventoryAction_Implementation()
{
	CleanupInventoryAction();
}

