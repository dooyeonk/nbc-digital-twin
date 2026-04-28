// Copyright Epic Games, Inc. All Rights Reserved.

#include "DTWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UDTWheelFront::UDTWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}