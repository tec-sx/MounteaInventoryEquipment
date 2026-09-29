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


#include "Actors/Attachable/FVEquipmentItemBase.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/FVEquipmentItemComponent.h"


AFVEquipmentItemBase::AFVEquipmentItemBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	StaticMeshComponent->SetupAttachment(Root);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMeshComponent->SetGenerateOverlapEvents(false);
	StaticMeshComponent->SetVisibility(false);

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SkeletalMeshComponent->SetupAttachment(Root);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMeshComponent->SetGenerateOverlapEvents(false);
	SkeletalMeshComponent->SetVisibility(false);
	
	EquipmentItemComponent = CreateDefaultSubobject<UFVEquipmentItemComponent>(TEXT("Equipment Item"));
}

bool AFVEquipmentItemBase::SetStaticMesh_Implementation(UStaticMesh* NewStaticMesh)
{
	if (!IsValid(StaticMeshComponent) || !IsValid(SkeletalMeshComponent))
		return false;

	SkeletalMeshComponent->SetSkeletalMesh(nullptr);
	SkeletalMeshComponent->SetVisibility(false, true);

	StaticMeshComponent->SetStaticMesh(NewStaticMesh);
	StaticMeshComponent->SetVisibility(IsValid(NewStaticMesh), true);
	return true;
}

bool AFVEquipmentItemBase::SetSkeletalMesh_Implementation(USkeletalMesh* NewSkeletalMesh)
{
	if (!IsValid(StaticMeshComponent) || !IsValid(SkeletalMeshComponent))
		return false;

	StaticMeshComponent->SetStaticMesh(nullptr);
	StaticMeshComponent->SetVisibility(false, true);

	SkeletalMeshComponent->SetSkeletalMesh(NewSkeletalMesh);
	SkeletalMeshComponent->SetVisibility(IsValid(NewSkeletalMesh), true);
	return true;
}

void AFVEquipmentItemBase::BeginPlay()
{
	Super::BeginPlay();
	
}

