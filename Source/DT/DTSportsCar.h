// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DTPawn.h"
#include "DTSportsCar.generated.h"

/**
 *  Sports car wheeled vehicle implementation
 */
UCLASS(abstract)
class DT_API ADTSportsCar : public ADTPawn
{
	GENERATED_BODY()
	
public:

	ADTSportsCar();
};
