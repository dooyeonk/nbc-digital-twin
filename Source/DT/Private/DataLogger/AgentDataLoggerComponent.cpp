#include "DataLogger/AgentDataLoggerComponent.h"
#include "WheeledVehiclePawn.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

UAgentDataLoggerComponent::UAgentDataLoggerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}


void UAgentDataLoggerComponent::BeginPlay()
{
	Super::BeginPlay();

	OriginUtmZone = GetUtmZone(OriginLongitude);
	LatLonToUtm(OriginLatitude, OriginLongitude, OriginUtmZone, OriginUtmEasting, OriginUtmNorthing);

	if (bEnableLogging)
	{
		StartRecording();
	}
}

void UAgentDataLoggerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopRecording();
	Super::EndPlay(EndPlayReason);
}

void UAgentDataLoggerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsRecording)
	{
		return;
	}

	ElapsedRecordingTime += DeltaTime;
	TimeSinceLastSave += DeltaTime;

	const float SaveInterval = 1.0f / FMath::Max(SaveFrequencyHz, 0.1f);
	if (TimeSinceLastSave >= SaveInterval)
	{
		AppendRow();
		TimeSinceLastSave -= SaveInterval;
	}
}

int32 UAgentDataLoggerComponent::GetUtmZone(double Longitude)
{
	return FMath::FloorToInt((Longitude + 180.0) / 6.0) + 1;
}

void UAgentDataLoggerComponent::LatLonToUtm(double Lat, double Lon, int32 Zone, double& OutEasting, double& OutNorthing)
{
	// WGS-84 ellipsoid constants
	constexpr double a  = 6378137.0;            // semi-major axis (m)
	constexpr double f  = 1.0 / 298.257223563;  // flattening
	constexpr double k0 = 0.9996;               // UTM scale factor

	const double e2 = 2.0 * f - f * f;          // first eccentricity squared
	const double ep2 = e2 / (1.0 - e2);         // second eccentricity squared

	const double LatRad = FMath::DegreesToRadians(Lat);
	const double CentralMeridian = (Zone - 1) * 6.0 - 180.0 + 3.0;
	const double DeltaLon = FMath::DegreesToRadians(Lon - CentralMeridian);

	const double SinLat = FMath::Sin(LatRad);
	const double CosLat = FMath::Cos(LatRad);
	const double TanLat = FMath::Tan(LatRad);

	const double N = a / FMath::Sqrt(1.0 - e2 * SinLat * SinLat);
	const double T = TanLat * TanLat;
	const double C = ep2 * CosLat * CosLat;
	const double A = CosLat * DeltaLon;

	// Meridional arc (M) — series expansion
	const double e4 = e2 * e2;
	const double e6 = e4 * e2;
	const double M = a * (
		(1.0 - e2 / 4.0 - 3.0 * e4 / 64.0  - 5.0 * e6 / 256.0) * LatRad
		- (3.0 * e2 / 8.0 + 3.0 * e4 / 32.0 + 45.0 * e6 / 1024.0) * FMath::Sin(2.0 * LatRad)
		+ (15.0 * e4 / 256.0 + 45.0 * e6 / 1024.0) * FMath::Sin(4.0 * LatRad)
		- (35.0 * e6 / 3072.0) * FMath::Sin(6.0 * LatRad));

	const double A2 = A * A;
	const double A4 = A2 * A2;
	const double A6 = A4 * A2;

	OutEasting = k0 * N * (
		A
		+ (1.0 - T + C) * A2 * A / 6.0
		+ (5.0 - 18.0 * T + T * T + 72.0 * C - 58.0 * ep2) * A4 * A / 120.0
	) + 500000.0;   // false easting

	OutNorthing = k0 * (M + N * TanLat * (
		A2 / 2.0
		+ (5.0 - T + 9.0 * C + 4.0 * C * C) * A4 / 24.0
		+ (61.0 - 58.0 * T + T * T + 600.0 * C - 330.0 * ep2) * A6 / 720.0
	));

	// Southern hemisphere offset
	if (Lat < 0.0)
		OutNorthing += 10000000.0;
}

void UAgentDataLoggerComponent::WorldToUtm(const FVector& WorldLocation, double& OutEasting, double& OutNorthing) const
{
	const double OffsetEastM  =  WorldLocation.X * 0.01;
	const double OffsetNorthM = -WorldLocation.Y * 0.01;

	OutEasting  = OriginUtmEasting  + OffsetEastM;
	OutNorthing = OriginUtmNorthing + OffsetNorthM;
}

void UAgentDataLoggerComponent::CreateCsvFile()
{
	const FString OutputDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Output"));
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*OutputDir);

	const FDateTime Now = FDateTime::Now();
	const FString FileName = FString::Printf(
		TEXT("AgentData-%04d_%02d-%02d-%02d-%02d-%02d.csv"),
		Now.GetYear(), Now.GetMonth(), Now.GetDay(),
		Now.GetHour(), Now.GetMinute(), Now.GetSecond()
	);

	CsvFilePath = FPaths::Combine(OutputDir, FileName);

	const FString Header =
		TEXT("Timestamp,World_X,World_Y,World_Z,UTM_Easting,UTM_Northing,UTM_Zone,Velocity_kmh,Yaw,Acceleration_kmhps,Steering\n"
	);
	FFileHelper::SaveStringToFile(Header, *CsvFilePath,
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM
	);

	UE_LOG(LogTemp, Log, TEXT("[AgentDataLogger] Recording to: %s  (%.1f Hz)"), *CsvFilePath, SaveFrequencyHz);
}

void UAgentDataLoggerComponent::AppendRow()
{
	const AActor* Owner = GetOwner();
	if (!Owner)
		return;

	const FVector WorldLoc = Owner->GetActorLocation();
	const FRotator WorldRot = Owner->GetActorRotation();
	const FVector Velocity = Owner->GetVelocity();

	const float SpeedKmh = Velocity.Size() * 0.01f * 3.6f;
	const double Yaw = WorldRot.Yaw;

	// 가속도 (km/h/s)
	const float SaveInterval = 1.0f / FMath::Max(SaveFrequencyHz, 0.1f);
	const float AccelKmhps = bHasPrevRecord ? (SpeedKmh - LastRecordedSpeedKmh) / SaveInterval : 0.f;

	// 조향 입력값
	float SteeringInput = 0.f;
	if (const AWheeledVehiclePawn* VehiclePawn = Cast<AWheeledVehiclePawn>(Owner))
	{
		if (UChaosWheeledVehicleMovementComponent* MoveComp = Cast<UChaosWheeledVehicleMovementComponent>(VehiclePawn->GetVehicleMovementComponent()))
		{
			SteeringInput = MoveComp->GetSteeringInput();
		}
	}

	double UtmEasting = 0.0;
	double UtmNorthing = 0.0;
	WorldToUtm(WorldLoc, UtmEasting, UtmNorthing);

	// ── DrawDebug 시각화 ──────────────────────────────────────────
	if (bHasPrevRecord && GetWorld())
	{
		// 속도 → 색상 (느리면 파랑, 빠르면 빨강)
		const float NormalizedSpeed = FMath::Clamp(SpeedKmh / FMath::Max(MaxSpeedForColorKmh, 1.f), 0.f, 1.f);
		const FLinearColor LineColorLinear = FLinearColor::LerpUsingHSV(
			FLinearColor(0.f, 0.f, 1.f),
			FLinearColor(1.f, 0.f, 0.f),
			NormalizedSpeed
		);
		const FColor LineColor = LineColorLinear.ToFColor(true);

		// 주행 궤적 선
		DrawDebugLine(
			GetWorld(),
			LastRecordedLocation, WorldLoc,
			LineColor,
			/*bPersistentLines=*/ true,
			/*LifeTime=*/ -1.f,
			/*DepthPriority=*/ 0,
			TrajectoryLineThickness
		);

		// 급감속 마커
		if (AccelKmhps < -BrakingThresholdKmhPerSec)
		{
			DrawDebugPoint(
				GetWorld(),
				WorldLoc,
				20.f,
				FColor::Yellow,
				/*bPersistentLines=*/ true,
				/*LifeTime=*/ -1.f
			);
		}

		// 일정 거리마다 속도/Yaw 표시
		AccumulatedDistForString += FVector::Dist(LastRecordedLocation, WorldLoc);
		if (AccumulatedDistForString >= StringDisplayDistanceCm)
		{
			const FString InfoText = FString::Printf(TEXT("%.0f km/h\nYaw: %.1f"), SpeedKmh, Yaw);
			DrawDebugString(
				GetWorld(),
				WorldLoc + FVector(0.f, 0.f, 100.f),
				InfoText,
				nullptr,
				FColor::White,
				/*TextDuration=*/ -1.f
			);
			AccumulatedDistForString = 0.f;
		}
	}

	// 이전 위치/속도 갱신
	LastRecordedLocation = WorldLoc;
	LastRecordedSpeedKmh = SpeedKmh;
	bHasPrevRecord = true;

	// ── CSV 저장 ─────────────────────────────────────────────────
	const FString Row = FString::Printf(
		TEXT("%.3f,%.2f,%.2f,%.2f,%.4f,%.4f,%d,%.2f,%.4f,%.4f,%.4f\n"),
		ElapsedRecordingTime,
		WorldLoc.X, WorldLoc.Y, WorldLoc.Z,
		UtmEasting, UtmNorthing, OriginUtmZone,
		SpeedKmh,
		Yaw,
		AccelKmhps,
		SteeringInput
	);

	FFileHelper::SaveStringToFile(Row, *CsvFilePath,
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,
		&IFileManager::Get(),
		EFileWrite::FILEWRITE_Append
	);
}

void UAgentDataLoggerComponent::StartRecording()
{
	if (bIsRecording)
	{
		return;
	}

	CreateCsvFile();
	bIsRecording = true;
	TimeSinceLastSave = 0.0f;
	ElapsedRecordingTime = 0.0f;
	bHasPrevRecord = false;
	LastRecordedSpeedKmh = 0.f;
	AccumulatedDistForString = 0.f;
}

void UAgentDataLoggerComponent::StopRecording()
{
	bIsRecording = false;
}




