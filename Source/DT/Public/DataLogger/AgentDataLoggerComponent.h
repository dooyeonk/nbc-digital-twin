// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AgentDataLoggerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DT_API UAgentDataLoggerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAgentDataLoggerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	static int32 GetUtmZone(double Longitude);
	static void LatLonToUtm(double Lat, double Lon, int32 Zone, double& OutEasting, double& OutNorthing);
	void WorldToUtm(const FVector& WorldLocation, double& OutEasting, double& OutNorthing) const;
	void CreateCsvFile();
	void AppendRow();

	UFUNCTION(BlueprintCallable, Category="Data Logger")
	void StartRecording();

	UFUNCTION(BlueprintCallable, Category="Data Logger")
	void StopRecording();

	UFUNCTION(BlueprintPure, Category="Data Logger")
	bool IsRecording() const { return bIsRecording; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data Logger", meta=(AllowPrivateAccess="true"))
	bool bEnableLogging = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data Logger", meta=(ClampMin="0.1", ClampMax="100.0", Units="Hz", AllowPrivateAccess="true"))
	float SaveFrequencyHz = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data Logger|UTM Reference", meta=(ClampMin="-90.0", ClampMax="90.0", Units="deg", AllowPrivateAccess="true"))
	double OriginLatitude = 36.4800;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data Logger|UTM Reference", meta=(ClampMin="-180.0", ClampMax="180.0", Units="deg", AllowPrivateAccess="true"))
	double OriginLongitude = 127.0000;

	// 속도 0 → Blue, MaxSpeedForColorKmh → Red 색상 매핑 기준
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data Logger|Visualization", meta=(ClampMin="1.0", Units="km/h", AllowPrivateAccess="true"))
	float MaxSpeedForColorKmh = 120.f;

	// 이 거리(cm)마다 속도/Yaw 수치를 월드에 표시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data Logger|Visualization", meta=(ClampMin="100.0", Units="cm", AllowPrivateAccess="true"))
	float StringDisplayDistanceCm = 1000.f;

	// 이 감속도(km/h/s) 초과 시 급감속 마커 표시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data Logger|Visualization", meta=(ClampMin="1.0", AllowPrivateAccess="true"))
	float BrakingThresholdKmhPerSec = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data Logger|Visualization", meta=(ClampMin="0.5", AllowPrivateAccess="true"))
	float TrajectoryLineThickness = 3.f;

private:
	double OriginUtmEasting = 0.0;
	double OriginUtmNorthing = 0.0;
	int32 OriginUtmZone = 0;

	FString CsvFilePath;
	bool bIsRecording = false;
	float TimeSinceLastSave = 0.0f;
	float ElapsedRecordingTime = 0.0f;

	FVector LastRecordedLocation = FVector::ZeroVector;
	float LastRecordedSpeedKmh = 0.f;
	float AccumulatedDistForString = 0.f;
	bool bHasPrevRecord = false;
};
