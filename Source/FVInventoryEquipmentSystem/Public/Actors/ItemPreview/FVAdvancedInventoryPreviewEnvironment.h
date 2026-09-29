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
#include "GameFramework/Actor.h"
#include "FVAdvancedInventoryPreviewEnvironment.generated.h"

class UDirectionalLightComponent;
class USkyLightComponent;
class UPostProcessComponent;
class UFVAdvancedInventoryPreviewEnvironmentSettings;

UCLASS()
class FVINVENTORYEQUIPMENTSYSTEM_API AFVAdvancedInventoryPreviewEnvironment : public AActor
{
	GENERATED_BODY()
    
public:
	
	AFVAdvancedInventoryPreviewEnvironment();
	
	void InitializeFromSettings(const UFVAdvancedInventoryPreviewEnvironmentSettings* Settings);
	
public:
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> RootSceneComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lighting")
	TObjectPtr<UDirectionalLightComponent> DirectionalLight;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lighting")
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PostProcess")
	TObjectPtr<UPostProcessComponent> PostProcessComponent;
	
private:
	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedExtraActors;
};
