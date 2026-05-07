
#pragma once

#include "CoreMinimal.h"
#include "CameraSensorTypes.h"
#include "Components/SceneComponent.h"
#include "LidarSensorComponent.generated.h"


class ULidarBevRenderer;

UCLASS(ClassGroup = (Sensor), meta = (BlueprintSpawnableComponent), BlueprintType)
class DT_API ULidarSensorComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	ULidarSensorComponent();

	UFUNCTION(BlueprintCallable, Category = "LidarSensor")
	void StartScan();

	UFUNCTION(BlueprintCallable, Category = "LidarSensor")
	void StopScan();

	UFUNCTION(BlueprintPure, Category = "LidarSensor")
	UTexture2D* GetBevRenderTarget() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION(BlueprintCallable, Category = "LidarSensor")
	void ApplyPreset(ELidarSensorPreset NewPreset);

	void InitializeSensor();
	void SavePointCloudData();
	void CollectAsyncResults();

	void RebuildDirectionCache();
	void FireAsyncTraces();

	void StartScanTimer();
	void StopScanTimer();
	void OnScanTimer();
	void SetScanRate(float Hz);
	void RefreshSettings();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LidarSensor|Config",
	meta=(AllowPrivateAccess="true"))
	bool bSensorEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LidarSensor|BEV",
	meta=(AllowPrivateAccess="true"))
	FBevRenderConfig BevConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LidarSensor|Output",
		meta=(AllowPrivateAccess="true"))
	FLidarPointCloudData LastPointCloud;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LidarSensor|Config",
	meta=(AllowPrivateAccess="true"))
	ELidarSensorPreset Preset = ELidarSensorPreset::VelodyneVLP16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LidarSensor|Config",
		meta=(AllowPrivateAccess="true"))
	FLidarSensorConfig Config;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LidarSensor|Output",
	meta=(AllowPrivateAccess="true"))
	int64 FrameCount = 0;

	UFUNCTION(BlueprintPure, Category = "LidarSensor")
	const FLidarPointCloudData& GetPointCloud() const { return LastPointCloud; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LidarSensor|DataSave",
	meta=(AllowPrivateAccess="true"))
	bool bIsDataSaving = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LidarSensor|DataSave",
	meta=(AllowPrivateAccess="true"))
	FSensorDataSaveConfig DataSaveConfig;

	UPROPERTY()
	TObjectPtr<ULidarBevRenderer> BevRenderer;
	bool bDirectionsDirty = true;
	bool bHasPendingTraces = false;
	uint64 FireFrameNumber = 0;

	TArray<FVector> CachedLocalDirections;
	FTransform PendingTransform;

	TArray<FTraceHandle> PendingHandles;
	TArray<FVector> PendingWorldDirs;
	TArray<FVector> ScanPoints;
	TArray<float>   ScanIntensities;

	FTimerHandle ScanTimerHandle;
};
