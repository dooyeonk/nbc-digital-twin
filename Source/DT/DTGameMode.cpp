// Copyright Epic Games, Inc. All Rights Reserved.

#include "DTGameMode.h"
#include "DTPlayerController.h"

ADTGameMode::ADTGameMode()
{
	PlayerControllerClass = ADTPlayerController::StaticClass();
}
