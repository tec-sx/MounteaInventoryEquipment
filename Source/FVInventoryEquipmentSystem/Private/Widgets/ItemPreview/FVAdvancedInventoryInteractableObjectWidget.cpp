// FVAdvancedInventoryInteractableObjectWidget.cpp

#include "Widgets/ItemPreview/FVAdvancedInventoryInteractableObjectWidget.h"

#include "AssetDefinitionAssetInfo.h"
#include "Actors/ItemPreview/FVAdvancedInventoryItemPreviewRenderer.h"
#include "Actors/ItemPreview/FVAdvancedInventoryPreviewEnvironment.h"
#include "Components/Image.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Settings/TemplatesConfig/FVAdvancedInventoryInteractiveWidgetConfig.h"
#include "Settings/TemplatesConfig/FVAdvancedInventoryPreviewEnvironmentSettings.h"
#include "Statics/FVInventoryStatics.h"

void UFVAdvancedInventoryInteractableObjectWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(false);
}

void UFVAdvancedInventoryInteractableObjectWidget::CleanUpPreviewScene()
{
	if (!PreviewScene.IsValid())
		return;
	
	if(RendererActor)
		PreviewScene->GetWorld()->DestroyActor( RendererActor );
	if(EnvironmentActor)
		PreviewScene->GetWorld()->DestroyActor( EnvironmentActor );
	if (PreviewRenderTarget)
		PreviewRenderTarget->MarkAsGarbage();
	
	EnvironmentActor = nullptr;
	RendererActor = nullptr;
	PreviewRenderTarget	= nullptr;
	
	PreviewScene.Reset();
}

void UFVAdvancedInventoryInteractableObjectWidget::NativeDestruct()
{
	Super::NativeDestruct();
	CleanUpPreviewScene();
}

bool UFVAdvancedInventoryInteractableObjectWidget::InitializeInteractableWidget()
{
	auto templateConfig = UFVInventoryStatics::GetTemplateConfig(TEXT("InteractivePreview"));
	if (!templateConfig) return false;

	auto interactiveConfig = Cast<UFVAdvancedInventoryInteractiveWidgetConfig>(templateConfig);
	if (!interactiveConfig) return false;
	
	UTextureRenderTarget2D* templateRT = interactiveConfig->DefaultRenderTarget.LoadSynchronous();
	if (!templateRT) return false;

	PreviewRenderTarget = DuplicateObject<UTextureRenderTarget2D>(templateRT, this, TEXT("PreviewRT"));
	PreviewRenderTarget->UpdateResourceImmediate(true);
	
	const auto previewMaterial = interactiveConfig->DefaultRenderTargetMaterial.LoadSynchronous();
	if (!previewMaterial) return false;
	
	ControlSettings = interactiveConfig->PreviewCameraControlSettings;

	if (!EnvironmentActorClass)
		EnvironmentActorClass = interactiveConfig->EnvironmentActor.LoadSynchronous();
	if (!EnvironmentActorClass)
		EnvironmentActorClass = AFVAdvancedInventoryPreviewEnvironment::StaticClass();
	
	if (!RendererActorClass)
		RendererActorClass = interactiveConfig->RendererActor.LoadSynchronous();
	if (!RendererActorClass)
		RendererActorClass = AFVAdvancedInventoryItemPreviewRenderer::StaticClass();

	PreviewRenderTarget->ClearColor = FLinearColor::Transparent;

	PreviewMaterialInstance = UMaterialInstanceDynamic::Create(previewMaterial, this);
	PreviewMaterialInstance->SetTextureParameterValue(TEXT("SceneCapture"), PreviewRenderTarget);
	
	PreviewScene = MakeUnique<FPreviewScene>(FPreviewScene::ConstructionValues()
		.SetLightBrightness(EnvironmentActorClass ? 0.0f : 3.0f)
		.SetSkyBrightness(EnvironmentActorClass ? 0.0f : 1.0f)
	);
	UWorld* previewWorld = PreviewScene->GetWorld();
	
	if (EnvironmentActorClass)
		EnvironmentActor = previewWorld->SpawnActor<AFVAdvancedInventoryPreviewEnvironment>(EnvironmentActorClass);
	
	const auto environmentalSettings = interactiveConfig->EnvironmentSettings.LoadSynchronous();
	if (EnvironmentActor && environmentalSettings)
		EnvironmentActor->InitializeFromSettings(interactiveConfig->EnvironmentSettings.LoadSynchronous());
	
	RendererActor = previewWorld->SpawnActor<AFVAdvancedInventoryItemPreviewRenderer>(RendererActorClass);
	if (!IsValid(RendererActor))
	{
		CleanUpPreviewScene();
		return false;
	}
	
	PreviewScene->UpdateCaptureContents();
	
	RendererActor->SetRenderTarget(PreviewRenderTarget);
	PreviewImage->SetBrushFromMaterial(PreviewMaterialInstance);

	if (ControlSettings.bAutoStartTick)
		StartPreview();
	
	return true;
}

void UFVAdvancedInventoryInteractableObjectWidget::SetPreviewMesh(UStaticMesh* StaticMesh) const
{
	if (RendererActor)
		RendererActor->SetStaticMesh(StaticMesh);
}

void UFVAdvancedInventoryInteractableObjectWidget::SetPreviewSkeletalMesh(USkeletalMesh* SkeletalMesh) const
{
	if (RendererActor)
		RendererActor->SetSkeletalMesh(SkeletalMesh);
}

void UFVAdvancedInventoryInteractableObjectWidget::ClearPreview() const
{
	if (RendererActor)
		RendererActor->ClearMesh();
}

void UFVAdvancedInventoryInteractableObjectWidget::StartPreview()
{
	if (!PreviewScene.IsValid() || !IsValid(RendererActor))
	{
		LOG_ERROR(TEXT("Preview scene or renderer actor is invalid!"));
		return;
	}
	const UWorld* contextWorld = GetWorld();
	if (!contextWorld) return;
	
	if (FMath::IsNearlyZero(LastInteractionTime))
		LastInteractionTime = GetWorld()->GetTimeSeconds();
		
	if (!contextWorld->GetTimerManager().IsTimerActive(TimerHandle_PreviewTick) && !FMath::IsNearlyEqual(ControlSettings.PreviewTickFrequency, 0.f, KINDA_SMALL_NUMBER))
	{
		const float interval = 1.f / ControlSettings.PreviewTickFrequency;
		contextWorld->GetTimerManager().SetTimer(
			TimerHandle_PreviewTick,
			this,
			&UFVAdvancedInventoryInteractableObjectWidget::TickPreview,
			interval,
			true
		);

		FVector4 currentValues;
		RendererActor->GetCurrentValues(currentValues);
		CurrentZoom = (ControlSettings.ZoomLimits.Y + ControlSettings.ZoomLimits.X) / 2;
		CurrentCameraHeight = currentValues.Y;
		CurrentPitch = currentValues.Z;
		CurrentYaw = currentValues.W;
	}
}

void UFVAdvancedInventoryInteractableObjectWidget::PausePreview()
{
	if (const UWorld* contextWorld = GetWorld())
		contextWorld->GetTimerManager().ClearTimer(TimerHandle_PreviewTick);
}

void UFVAdvancedInventoryInteractableObjectWidget::TickPreview()
{
	if (!PreviewScene || !RendererActor)
		return;
	
	const float currentTime = GetWorld()->GetTimeSeconds();
	if ((currentTime - LastInteractionTime) > ControlSettings.IdleThreshold)
	{
		PausePreview();
		return;
	}
	
	PreviewScene->UpdateCaptureContents();
	RendererActor->CaptureScene();
}

void UFVAdvancedInventoryInteractableObjectWidget::UpdateLastInteractionAndStartPreview()
{
	if (const UWorld* contextWorld = GetWorld())
	{
		LastInteractionTime = contextWorld->GetTimeSeconds();
		StartPreview();
	}
}

void UFVAdvancedInventoryInteractableObjectWidget::ResetCameraToDefaults()
{
	if (!RendererActor)
		return;
	
	if (RendererActor)
	{
		RendererActor->ResetToDefaults();
		RendererActor->CaptureScene();
	}

	FVector4 currentValues;
	RendererActor->GetCurrentValues(currentValues);
	CurrentZoom = currentValues.X;
	CurrentCameraHeight = currentValues.Y;
	CurrentPitch = currentValues.Z;
	CurrentYaw = currentValues.W;

	OnPreviewReset();
}

bool UFVAdvancedInventoryInteractableObjectWidget::CanConsumeNativeInput() const
{
	return IsValid(RendererActor);
}

void UFVAdvancedInventoryInteractableObjectWidget::ProcessRotationInput(const FVector2f& Delta)
{
	if (!RendererActor || !ControlSettings.bAllowRotation) return;
	
	CurrentYaw += Delta.X * ControlSettings.CameraRotationSensitivity;
	CurrentPitch = FMath::Clamp(CurrentPitch - Delta.Y * ControlSettings.CameraRotationSensitivity, -ControlSettings.RotationLimits.Y, ControlSettings.RotationLimits.Y);
	
	RendererActor->SetCameraRotation(CurrentYaw, CurrentPitch);
	UpdateLastInteractionAndStartPreview();
}

void UFVAdvancedInventoryInteractableObjectWidget::ProcessHeightInput(const FVector2f& Delta)
{
	if (!RendererActor || !ControlSettings.bAllowHeightAdjustment) return;
	
	const float heightSensitivity = ControlSettings.CameraHeightSensitivity;
	const float heightIncrement = (Delta.Y > 0) ? heightSensitivity : -heightSensitivity;
	
	CurrentCameraHeight = FMath::Clamp(CurrentCameraHeight + heightIncrement, -ControlSettings.HeightLimit, ControlSettings.HeightLimit);
	RendererActor->SetCameraHeight(CurrentCameraHeight);
	UpdateLastInteractionAndStartPreview();
}

void UFVAdvancedInventoryInteractableObjectWidget::ProcessZoomInput(const float Delta)
{
	if (!RendererActor || !ControlSettings.bAllowZoom) return;
	
	const float zoomRange = (ControlSettings.ZoomLimits.Y + ControlSettings.ZoomLimits.X) / 2;
	const float stepPerUnit = zoomRange / 10.0f;
	const float zoomDelta = Delta * stepPerUnit;
	
	CurrentZoom = FMath::Clamp(CurrentZoom + zoomDelta, ControlSettings.ZoomLimits.X, ControlSettings.ZoomLimits.Y);
	
	RendererActor->SetCameraDistance(CurrentZoom);
	UpdateLastInteractionAndStartPreview();
}

void UFVAdvancedInventoryInteractableObjectWidget::ProcessAnalogRotation(const FVector2f& AnalogInput, const float DeltaTime)
{
	if (!RendererActor || !ControlSettings.bAllowRotation) return;
	
	const float rotationSpeed = ControlSettings.RotationLimits.Y;
	const float adjustedSpeed = rotationSpeed / FMath::Max(CurrentZoom, SMALL_NUMBER);
	
	CurrentYaw += AnalogInput.X * adjustedSpeed * DeltaTime;
	CurrentPitch = FMath::Clamp(CurrentPitch - AnalogInput.Y * adjustedSpeed * DeltaTime, -ControlSettings.RotationLimits.Y, ControlSettings.RotationLimits.Y);
	
	RendererActor->SetCameraRotation(CurrentYaw, CurrentPitch);
	UpdateLastInteractionAndStartPreview();
}

void UFVAdvancedInventoryInteractableObjectWidget::ProcessAnalogHeight(const float AnalogInput, const float DeltaTime)
{
	if (!RendererActor || !ControlSettings.bAllowHeightAdjustment) return;
	
	const float heightSpeed = ControlSettings.HeightLimit * (ControlSettings.ZoomLimits.Y - ControlSettings.ZoomLimits.X);
	const float adjustedSpeed = heightSpeed / FMath::Max(CurrentZoom, SMALL_NUMBER);
	
	CurrentCameraHeight = FMath::Clamp(CurrentCameraHeight + AnalogInput * adjustedSpeed * DeltaTime, -ControlSettings.HeightLimit, ControlSettings.HeightLimit);
	RendererActor->SetCameraHeight(CurrentCameraHeight);
	UpdateLastInteractionAndStartPreview();
}

void UFVAdvancedInventoryInteractableObjectWidget::ProcessAnalogZoom(const float AnalogInput, const float DeltaTime)
{
	if (!RendererActor || !ControlSettings.bAllowZoom) return;
	
	const float zoomRange = ControlSettings.ZoomLimits.Y - ControlSettings.ZoomLimits.X;
	const float unitsToTraverseFullRange = 10.0f;
	const float stepPerUnit = zoomRange / unitsToTraverseFullRange;
	const float zoomDelta = AnalogInput * stepPerUnit * DeltaTime * 10.0f;
	
	CurrentZoom = FMath::Clamp(CurrentZoom - zoomDelta, ControlSettings.ZoomLimits.X, ControlSettings.ZoomLimits.Y);
	
	RendererActor->SetCameraDistance(CurrentZoom);
	UpdateLastInteractionAndStartPreview();
}

void UFVAdvancedInventoryInteractableObjectWidget::SetCameraRotationAbsolute(const float YawNormalized, const float PitchNormalized)
{
	if (!RendererActor) return;
	
	CurrentYaw = FMath::Lerp(-ControlSettings.RotationLimits.Y, ControlSettings.RotationLimits.Y, YawNormalized);
	CurrentPitch = FMath::Lerp(-ControlSettings.RotationLimits.Y, ControlSettings.RotationLimits.Y, PitchNormalized);
	
	RendererActor->SetCameraRotation(CurrentYaw, CurrentPitch);
	UpdateLastInteractionAndStartPreview();
}

void UFVAdvancedInventoryInteractableObjectWidget::SetCameraHeightAbsolute(const float HeightNormalized)
{
	if (!RendererActor) return;
	
	CurrentCameraHeight = FMath::Lerp(-ControlSettings.HeightLimit, ControlSettings.HeightLimit, HeightNormalized);
	RendererActor->SetCameraHeight(CurrentCameraHeight);
	UpdateLastInteractionAndStartPreview();
}

void UFVAdvancedInventoryInteractableObjectWidget::SetCameraZoomAbsolute(const float ZoomNormalized)
{
	if (!RendererActor) return;
	
	CurrentZoom = FMath::Lerp(ControlSettings.ZoomLimits.X, ControlSettings.ZoomLimits.Y, ZoomNormalized);
	RendererActor->SetCameraDistance(CurrentZoom);
	UpdateLastInteractionAndStartPreview();
}

FReply UFVAdvancedInventoryInteractableObjectWidget::NativeOnMouseMove(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (!CanConsumeNativeInput()) return FReply::Unhandled();
	
	const FVector2f currentMousePos = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	const FVector2f mouseDelta = currentMousePos - LastMousePosition;
	
	if (bIsMousePressed)
	{
		ProcessRotationInput(mouseDelta);
		LastMousePosition = currentMousePos;
		return FReply::Handled();
	}
	else if (bIsMiddleMousePressed)
	{
		ProcessHeightInput(mouseDelta);
		LastMousePosition = currentMousePos;
		return FReply::Handled();
	}
	
	LastMousePosition = currentMousePos;
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply UFVAdvancedInventoryInteractableObjectWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (!CanConsumeNativeInput()) return FReply::Unhandled();
	
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsMousePressed = true;
		UpdateLastInteractionAndStartPreview();
		return FReply::Handled().CaptureMouse(TakeWidget());
	}
	else if (InMouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton)
	{
		bIsMiddleMousePressed = true;
		UpdateLastInteractionAndStartPreview();
		return FReply::Handled().CaptureMouse(TakeWidget());
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UFVAdvancedInventoryInteractableObjectWidget::NativeOnMouseButtonUp(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (!CanConsumeNativeInput()) return FReply::Unhandled();
	
	if (bIsMousePressed && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsMousePressed = false;
		UpdateLastInteractionAndStartPreview();
		return FReply::Handled().ReleaseMouseCapture();
	}
	else if (bIsMiddleMousePressed && InMouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton)
	{
		bIsMiddleMousePressed = false;
		UpdateLastInteractionAndStartPreview();
		return FReply::Handled().ReleaseMouseCapture();
	}
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UFVAdvancedInventoryInteractableObjectWidget::NativeOnMouseWheel(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (!CanConsumeNativeInput()) return FReply::Unhandled();
	
	ProcessZoomInput(InMouseEvent.GetWheelDelta());
	return FReply::Handled();
}

FReply UFVAdvancedInventoryInteractableObjectWidget::NativeOnFocusReceived(const FGeometry& InGeometry,
	const FFocusEvent& InFocusEvent)
{
	StartPreview();
	return Super::NativeOnFocusReceived(InGeometry, InFocusEvent);
}

void UFVAdvancedInventoryInteractableObjectWidget::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusLost(InFocusEvent);
	PausePreview();
}