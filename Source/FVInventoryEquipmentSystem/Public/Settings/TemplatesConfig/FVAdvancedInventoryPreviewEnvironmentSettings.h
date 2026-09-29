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
#include "Definitions/FVInventoryBaseUIDataTypes.h"
#include "Engine/DataAsset.h"
#include "FVAdvancedInventoryPreviewEnvironmentSettings.generated.h"

/**
 * UFVAdvancedInventoryPreviewEnvironmentSettings provides data-driven configuration
 * for preview scene environments in the FV Advanced Inventory System.
 * 
 * This data asset defines lighting, post-processing, and additional actor configurations
 * that compose the visual environment for 3D inventory item previews.
 *
 * Key features include:
 * - Configurable directional and ambient sky lighting
 * - Optional post-processing effects for enhanced visuals
 * - Support for spawning additional environmental actors (backdrops, props, etc.)
 * - Data-driven approach eliminates need for Blueprint subclassing
 * - Reusable configurations across different preview contexts
 *
 * @see 3D Item Preview
 * @see AFVAdvancedInventoryPreviewEnvironment
 * @see UFVAdvancedInventoryInteractableObjectWidget
 */
UCLASS(ClassGroup=(FV), BlueprintType, Blueprintable, DisplayName="Inventory Interactive Widget Environment Config", 
	meta=(ShortTooltip="Configuration asset for the FV Inventory System defining visual style of Interactive Environment Widget."))
class FVINVENTORYEQUIPMENTSYSTEM_API UFVAdvancedInventoryPreviewEnvironmentSettings : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	
	/**
	 * Defines the directional light configuration for the preview environment.
	 * Directional lights serve as the primary illumination source, simulating
	 * sunlight or key lighting to reveal form and detail of preview items.
	 * Default settings match Unreal Engine's standard preview lighting setup.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lighting")
	FFVPreviewDirectionalLightSettings DirectionalLight;

	/**
	 * Defines the ambient sky light configuration for the preview environment.
	 * Sky lights provide soft ambient illumination and optional image-based lighting
	 * through cubemaps, creating realistic material appearance with environmental reflections.
	 * Essential for proper visualization of metallic and reflective materials.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lighting")
	FFVPreviewSkyLightSettings SkyLight;

	/**
	 * Defines the post-processing configuration for the preview environment.
	 * Post-processing effects enhance visual quality through tone mapping, color grading,
	 * bloom, depth of field, and other cinematic effects. Disabled by default for performance.
	 * Enable only when specific visual enhancements are required for preview quality.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PostProcess")
	FFVPreviewPostProcessSettings PostProcess;
	
	/**
	 * Specifies a collection of additional actor classes to spawn in the preview environment.
	 * Enables placement of environmental elements like backdrops, floors, decorative props,
	 * or additional light sources that enhance the preview scene's visual context.
	 * Actors are spawned at world origin; use actor-specific positioning logic as needed.
	 * Commonly used for:
	 * - Ground planes or pedestals for item display
	 * - Backdrop meshes for environmental context
	 * - Additional fill lights or accent lighting
	 * - Decorative elements for thematic preview scenes
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Extras")
	TSet<TSoftClassPtr<AActor>> ExtraActorsToSpawn;	
};
