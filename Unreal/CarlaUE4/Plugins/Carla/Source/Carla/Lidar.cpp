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

void ALidar::ReadPoints(float DeltaTime, FCapturedLidarSegment& LidarSegmentData)
{
  int PointsToScanWithOneLaser = int(FMath::RoundHalfFromZero(PointsPerSecond * DeltaTime / float(Channels)));

  float AngleDistanceOfTick = RotationFrequency * 360 * DeltaTime;
  float AngleDistanceOfLaserMeasure = AngleDistanceOfTick / PointsToScanWithOneLaser;

  LidarSegmentData.LidarLasersSegments.Empty();

  auto NumOfLasers = Lasers.Num();
  LidarSegmentData.LidarLasersSegments.AddDefaulted(NumOfLasers);
  for (int li=0; li<NumOfLasers; li++)
  {
    auto& Laser = Lasers[li];
    auto& LaserPoints = LidarSegmentData.LidarLasersSegments[li].Points;
    LaserPoints.AddDefaulted(PointsToScanWithOneLaser);
    for (int i=0; i<PointsToScanWithOneLaser; i++)
    {
      Laser.Measure(
        this,
        CurrentHorizontalAngle + AngleDistanceOfLaserMeasure * i,
        LaserPoints[i],
        ShowDebugPoints
      );
    }
  }

  CurrentHorizontalAngle = fmod(CurrentHorizontalAngle + AngleDistanceOfTick, 360.0f);
  LidarSegmentData.HorizontalAngle = CurrentHorizontalAngle;
}
