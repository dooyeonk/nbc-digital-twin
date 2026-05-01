
#pragma once

#include "CoreMinimal.h"
#include "CameraSensorTypes.h"
#include "Components/SceneComponent.h"
#include "CameraSensorComponent.generated.h"


UCLASS(ClassGroup = (Sensor), meta = (BlueprintSpawnableComponent), BlueprintType)
class DT_API UCameraSensorComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UCameraSensorComponent();

	UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "CameraSensor")
	void ApplyPreset(ECameraSensorPreset NewPreset);

	void InitializeCapture();
	void CreateRenderTarget();
	void ConfigureSceneCapture();
	void ApplyPostProcessSettings();
	void ApplyLensDistortion();

	void StartCaptureTimer();
	void StopCaptureTimer();
	void OnCaptureTimer();
	void SaveCameraImage();

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Config",
	meta=(AllowPrivateAccess="true"))
	ECameraSensorPreset Preset = ECameraSensorPreset::TeslaHW3_Wide;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Config",
		meta=(AllowPrivateAccess="true"))
	bool bSensorEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Distortion",
	meta=(AllowPrivateAccess="true"))
	FLensDistortionParams Distortion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Noise",
	meta=(AllowPrivateAccess="true"))
	FSensorNoiseParams Noise;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|PostProcess",
	meta=(AllowPrivateAccess="true"))
	FCameraPostProcessEffects PostProcess;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Exposure",
	meta=(AllowPrivateAccess="true"))
	FAutoExposureParams Exposure;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|DataSave",
	meta=(AllowPrivateAccess="true"))
	bool bIsDataSaving = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CameraSensor|Output",
		meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|DataSave",
	meta=(AllowPrivateAccess="true"))
	FSensorDataSaveConfig DataSaveConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSensor|Distortion",
		meta=(AllowPrivateAccess="true"))
	TObjectPtr<UMaterialInterface> LensDistortionMaterial;

	FCameraSensorIntrinsics Intrinsics;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CameraSensor|Output",
	meta=(AllowPrivateAccess="true"))
	int64 FrameCount = 0;

	UPROPERTY()
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DistortionMID;

	FTimerHandle CaptureTimerHandle;
};
