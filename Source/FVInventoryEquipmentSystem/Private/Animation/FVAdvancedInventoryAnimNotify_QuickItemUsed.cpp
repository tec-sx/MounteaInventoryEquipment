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


#include "Animation/FVAdvancedInventoryAnimNotify_QuickItemUsed.h"

#include "Components/SkeletalMeshComponent.h"
#include "Statics/FVEquipmentStatics.h"

void UFVAdvancedInventoryAnimNotify_QuickItemUsed::Notify(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	ExecuteQuickItemUsed(MeshComp);
	Super::Notify(MeshComp, Animation, EventReference);
}

void UFVAdvancedInventoryAnimNotify_QuickItemUsed::BranchingPointNotify(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotify(BranchingPointPayload);
}

void UFVAdvancedInventoryAnimNotify_QuickItemUsed::ExecuteQuickItemUsed(USkeletalMeshComponent* MeshComp)
{
	if (!IsValid(MeshComp))
		return;

	UObject* interfaceSource = MeshComp->GetOwner();
	if (!IsValid(interfaceSource))
		interfaceSource = MeshComp;

	const TScriptInterface<IFVAdvancedEquipmentInterface> equipmentInterface =
		UFVEquipmentStatics::FindEquipmentInterface(interfaceSource);
	if (!equipmentInterface.GetObject())
		return;

	UFVEquipmentStatics::AnimQuickItemUsed(equipmentInterface);
}
