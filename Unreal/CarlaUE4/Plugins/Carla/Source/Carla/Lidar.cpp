// Fill out your copyright notice in the Description page of Project Settings.


#include "Carla.h"
#include "Lidar.h"
#include "StaticMeshResources.h"

// Sets default values
ALidar::ALidar()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

  auto MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CamMesh0"));
  MeshComp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
  MeshComp->bHiddenInGame = true;
  MeshComp->CastShadow = false;
  MeshComp->PostPhysicsComponentTick.bCanEverTick = false;
  RootComponent = MeshComp;

  CreateLasers();
}

void ALidar::Set(const FLidarDescription &LidarDescription)
{
  // UE_LOG(LogCarla, Log, TEXT("--- Lidar settings --------------------------"));
  // UE_LOG(LogCarla, Log, TEXT("pos x %f"), LidarDescription.Position.X);
  // UE_LOG(LogCarla, Log, TEXT("pos y %f"), LidarDescription.Position.Y);
  // UE_LOG(LogCarla, Log, TEXT("pos z %f"), LidarDescription.Position.Z);
  // UE_LOG(LogCarla, Log, TEXT("rot p %f"), LidarDescription.Rotation.Pitch);
  // UE_LOG(LogCarla, Log, TEXT("rot r %f"), LidarDescription.Rotation.Roll);
  // UE_LOG(LogCarla, Log, TEXT("rot y %f"), LidarDescription.Rotation.Yaw);
  //
  // UE_LOG(LogCarla, Log, TEXT("ch %d"), LidarDescription.Channels);
  // UE_LOG(LogCarla, Log, TEXT("r %f"), LidarDescription.Range);
  // UE_LOG(LogCarla, Log, TEXT("pts %f"), LidarDescription.PointsPerSecond);
  // UE_LOG(LogCarla, Log, TEXT("freq %f"), LidarDescription.RotationFrequency);
  // UE_LOG(LogCarla, Log, TEXT("upper l %f"), LidarDescription.UpperFovLimit);
  // UE_LOG(LogCarla, Log, TEXT("lower l %f"), LidarDescription.LowerFovLimit);
  // UE_LOG(LogCarla, Log, TEXT("d %d"), LidarDescription.ShowDebugPoints?1:0);

  Channels = LidarDescription.Channels;
	Range = LidarDescription.Range;
	PointsPerSecond = LidarDescription.PointsPerSecond;
	RotationFrequency = LidarDescription.RotationFrequency;
	UpperFovLimit = LidarDescription.UpperFovLimit;
	LowerFovLimit = LidarDescription.LowerFovLimit;
	ShowDebugPoints = LidarDescription.ShowDebugPoints;
  CreateLasers();
}

void ALidar::CreateLasers()
{
  float dAngle = (UpperFovLimit - LowerFovLimit) / (Channels - 1);

  Lasers.Empty();
	for(int i=0; i<Channels; i++)
	{
		Lasers.Emplace(i, UpperFovLimit - i * dAngle);
	}
}

// Called when the game starts or when spawned
void ALidar::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ALidar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

  // UE_LOG(LogCarla, Log, TEXT("--- Lidar tick %d"), ShowDebugPoints?1:0);
  // FVector LidarBodyLoc = RootComponent->GetComponentLocation();
  // UE_LOG(LogCarla, Log, TEXT("--- location: %f %f %f"), lidar_body_loc.X, lidar_body_loc.Y, lidar_body_loc.Z);

	int PointsToScanWithOneLaser = int(FMath::RoundHalfFromZero(PointsPerSecond * DeltaTime / float(Channels)));
	// float HorizontalAngle = CurrentHorizontalAngle;

	float AngleDistanceOfTick = RotationFrequency * 360 * DeltaTime;
	// PrintString(FString::Printf(TEXT("tick %f %f %d"), DeltaTime, angle_distance_of_tick, points_to_scan_with_one_laser));
	float AngleDistanceOfLaserMeasure = AngleDistanceOfTick / PointsToScanWithOneLaser;

	for (auto& Laser : Lasers)
	{
		for (int i=0; i<PointsToScanWithOneLaser; i++)
		{
			float Distance = Laser.Measure(this, CurrentHorizontalAngle + AngleDistanceOfLaserMeasure * i, ShowDebugPoints);
		}
	}

	// lidar_body_->AddLocalRotation(FRotator(0, angle_distance_of_tick, 0), false);
	CurrentHorizontalAngle += AngleDistanceOfTick;

}
