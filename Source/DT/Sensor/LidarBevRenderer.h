
#pragma once

#include "CoreMinimal.h"
#include "CameraSensorTypes.h"
#include "UObject/Object.h"
#include "LidarBevRenderer.generated.h"


UCLASS()
class DT_API ULidarBevRenderer : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(const FBevRenderConfig& InConfig);
	void RenderPointCloud(const FLidarPointCloudData& PointCloud, const FTransform& SensorTransform);
	UTexture2D* GetRenderTarget() const { return DynamicTexture; }
	void UpdateConfig(const FBevRenderConfig& InConfig);

private:
	void CreateTexture();
	void BuildColorLUT();

	UPROPERTY()
	TObjectPtr<UTexture2D> DynamicTexture;

	TArray<FColor> PixelBuffer;
	FUpdateTextureRegion2D UpdateRegion;
	FBevRenderConfig Config;
	FColor ColorLUT[256];
};
