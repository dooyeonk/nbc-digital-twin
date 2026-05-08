// Copyright Epic Games, Inc. All Rights Reserved.


#include "DTPlayerController.h"
#include "DTPawn.h"
#include "DTUI.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Sensor/SensorViewWidget.h"

void ADTPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// spawn the UI widget and add it to the viewport
	VehicleUI = CreateWidget<UDTUI>(this, VehicleUIClass);

	check(VehicleUI);

	VehicleUI->AddToViewport();

	if (SensorViewWidgetClass)
	{
		SensorViewWidget = CreateWidget<USensorViewWidget>(this, SensorViewWidgetClass);
		if (SensorViewWidget)
		{
			SensorViewWidget->AddToViewport(10);
			SensorViewWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
}

void ADTPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);

		// optionally add the steering wheel context
		if (bUseSteeringWheelControls && SteeringWheelInputMappingContext)
		{
			Subsystem->AddMappingContext(SteeringWheelInputMappingContext, 1);
		}
	}
}

void ADTPlayerController::Tick(float Delta)
{
	Super::Tick(Delta);

	if (IsValid(VehiclePawn) && IsValid(VehicleUI))
	{
		VehicleUI->UpdateSpeed(VehiclePawn->GetChaosVehicleMovement()->GetForwardSpeed());
		VehicleUI->UpdateGear(VehiclePawn->GetChaosVehicleMovement()->GetCurrentGear());
	}
}

void ADTPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// get a pointer to the controlled pawn
	VehiclePawn = CastChecked<ADTPawn>(InPawn);
}

void ADTPlayerController::ToggleSensorView(UTextureRenderTarget2D* InCameraRenderTarget)
{
	if (!SensorViewWidget || !InCameraRenderTarget)
	{
		return;
	}

	SensorViewWidget->SetRenderTarget(InCameraRenderTarget);
	SensorViewWidget->ToggleCameraView();
}

void ADTPlayerController::ToggleLidarView(UTexture2D* InLidarTexture)
{
	if (!SensorViewWidget || !InLidarTexture)
	{
		return;
	}

	SensorViewWidget->SetLidarRenderTarget(InLidarTexture);
	SensorViewWidget->ToggleLidarView();
}
