// Copyright Epic Games, Inc. All Rights Reserved.

#include "DTWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UDTWheelRear::UDTWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}