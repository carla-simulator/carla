// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "Util/SensorSpawnerActor.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Sensor/RayCastLidar.h"
#include "Carla/Sensor/GnssSensor.h"
#include "Carla/Sensor/Radar.h"
#include "Carla/Sensor/InertialMeasurementUnit.h"

#include <util/ue-header-guard-begin.h>
#include "JsonFileManagerLibrary.h"
#include "Sensor/SceneCaptureCamera.h"
#include "Sensor/Sensor.h"
#include "Kismet/GameplayStatics.h"
#include <util/ue-header-guard-end.h>

DEFINE_LOG_CATEGORY_STATIC(LogSensorSpawnerActor, Verbose, All);

ASensorSpawnerActor::ASensorSpawnerActor()
{
	PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.bStartWithTickEnabled = true;
  
  SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
  RootComponent = SceneComp;
  SaveImagePath = FPaths::ProjectSavedDir() + "SensorSpawnerCaptures/";

  SensorClassToCapture = nullptr;
}

void ASensorSpawnerActor::BeginPlay()
{
  Super::BeginPlay();

  // Wait for the CarlaEpisode initialisation. It is done on CarlaGameMode BeginPlay().
  if(ACarlaGameModeBase* CarlaGameMode = Cast<ACarlaGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
  {
    CarlaGameMode->OnEpisodeInitialisedDelegate.AddDynamic(this, &ASensorSpawnerActor::OnEpisodeInitialised);
  }

  SaveImagePath = SaveImagePath + FString::Printf(TEXT("%lld"), FDateTime::Now().ToUnixTimestamp());
  bRecordingData = bSaveDataAtBeginPlay;
}

void ASensorSpawnerActor::OnEpisodeInitialised(UCarlaEpisode* InitialisedEpisode)
{
  if(IsValid(InitialisedEpisode))
  {
    CarlaEpisode = InitialisedEpisode;

    // Spawn cameras with initial delay if set.
    GetWorldTimerManager().SetTimer(InitialDelaySpawnTimerHandle, this, &ASensorSpawnerActor::SpawnSensors, InitialDelay);
  }
}

void ASensorSpawnerActor::SpawnSensors()
{
  // Check if we are doing a delayed spawn. If so, don't do nothing.
  if(!SensorsToSpawnCopy.IsEmpty())
  {
    UE_LOG(LogSensorSpawnerActor, Warning, TEXT("Warning: ASensorSpawnerActor::SpawnSensors - Delayed spawn already in progress, wait until it ends"));
    return;
  }
  
  if(DelayBetweenSpawns > 0.f)
  {
    SensorsToSpawnCopy = SensorsToSpawn;
    GetWorldTimerManager().SetTimer(SpawnSensorsDelayedTimerHandle, this, &ASensorSpawnerActor::SpawnSensorsDelayed, DelayBetweenSpawns, true);
    return;
  }
  
  for(const auto& SensorStruct : SensorsToSpawn)
  {
    if(const FActorDefinition* SensorDefinition = GetActorDefinitionByClass(SensorStruct.SensorClass))
    {
      FActorDescription SensorDescription;
      GenerateSensorActorDescription(SensorDefinition, SensorDescription);
      
      for(int i = 0; i < SensorStruct.Amount; i++)
      {
        SpawnSensorActor(SensorDescription, SensorStruct.bAttachToActor);
      }
    }
  }
}

void ASensorSpawnerActor::StartRecordingSensorData()
{
  bRecordingData = true;

  for(ASensor* CurrentSensor : SpawnedSensorsArray)
  {
    if(CurrentSensor)
    {
      CurrentSensor->SetSavingDataToDisk(bRecordingData);
    }
  }
}

void ASensorSpawnerActor::StopRecordingSensorData()
{
  bRecordingData = false;

  for(ASensor* CurrentSensor : SpawnedSensorsArray)
  {
    if(CurrentSensor)
    {
      CurrentSensor->SetSavingDataToDisk(bRecordingData);
    }
  }
}

void ASensorSpawnerActor::ToggleRadarVisibility()
{
  bRadarVisibility = !bRadarVisibility;
}

const FActorDefinition* ASensorSpawnerActor::GetActorDefinitionByClass(const TSubclassOf<AActor> ActorClass) const
{
  if(!ActorClass || !IsValid(CarlaEpisode))
  {
    return nullptr;
  }
  
  const TArray<FActorDefinition>& ActorDefinitions = CarlaEpisode->GetActorDefinitions();
  // Checks that the class is exactly the same. If we want to allow also child classes use: ActorDef.Class->IsChildOf(ActorClass)
  const FActorDefinition* ActorDefinition = ActorDefinitions.FindByPredicate([&](const FActorDefinition& ActorDef){ return ActorDef.Class == ActorClass; });

  return ActorDefinition;
}

void ASensorSpawnerActor::SpawnSensorActor(const FActorDescription& SensorDescription, bool bAttachToActor)
{
  if(IsValid(CarlaEpisode))
  {
    FTransform Transform;
    GetRandomTransform(Transform);
    
    const TPair<EActorSpawnResultStatus, FCarlaActor*> SpawnResult = CarlaEpisode->SpawnActorWithInfo(Transform, SensorDescription);

    // After spawn handle other logic if spawn was success.
    if(SpawnResult.Value)
    {
      AddSensorToSaveDataArray(SpawnResult.Value->GetActor());

      if(bAttachToActor)
      {
        AttachSensorToActor(SpawnResult.Value->GetActor());
      }
    }
  }
}

void ASensorSpawnerActor::GenerateSensorActorDescription(const FActorDefinition* Definition, FActorDescription& SensorDescription) const
{
  SensorDescription.UId = Definition->UId;
  SensorDescription.Id = Definition->Id;
  SensorDescription.Class = Definition->Class;
  SensorDescription.Variations.Reserve(Definition->Variations.Num());

  FActorAttribute CreatedAttribute;
  for(const FActorVariation& Variation : Definition->Variations)
  {
    if(Variation.RecommendedValues.IsValidIndex(0))
    {
      CreatedAttribute.Id = Variation.Id;
      CreatedAttribute.Type = Variation.Type;
      CreatedAttribute.Value = Variation.RecommendedValues[0];
      SensorDescription.Variations.Emplace(CreatedAttribute.Id, CreatedAttribute);
    }
  }
}

void ASensorSpawnerActor::GetRandomTransform(FTransform &Transform) const
{
  Transform = FTransform::Identity;
  const float PosX = FMath::FRandRange(MinSpawnLocation.X, MaxSpawnLocation.X);
  const float PosY = FMath::FRandRange(MinSpawnLocation.Y, MaxSpawnLocation.Y);
  const float PosZ = FMath::FRandRange(MinSpawnLocation.Z, MaxSpawnLocation.Z);
  Transform.SetLocation(FVector(PosX, PosY, PosZ));
}

void ASensorSpawnerActor::SpawnSensorsDelayed()
{
  if(SensorsToSpawnCopy.IsEmpty())
  {
    GetWorldTimerManager().ClearTimer(SpawnSensorsDelayedTimerHandle);
    return;
  }

  if(const FActorDefinition* SensorDefinition = GetActorDefinitionByClass(SensorsToSpawnCopy[0].SensorClass))
  {
    FActorDescription SensorDescription;
    GenerateSensorActorDescription(SensorDefinition, SensorDescription);
    SpawnSensorActor(SensorDescription, SensorsToSpawnCopy[0].bAttachToActor);
  }

  SensorsToSpawnCopy[0].Amount--;

  if(SensorsToSpawnCopy[0].Amount <= 0)
  {
    SensorsToSpawnCopy.RemoveAt(0);
  }
}

void ASensorSpawnerActor::AddSensorToSaveDataArray(AActor* Actor)
{
  if(ASensor* CaptureSensor = Cast<ASensor>(Actor))
  {
    if(SensorClassToCapture == CaptureSensor->GetClass() || SensorClassToCapture == nullptr)
    {
      SpawnedSensorsArray.Add(CaptureSensor);
      CaptureSensor->SetSavingDataToDisk(bRecordingData);
    }
  }
}

void ASensorSpawnerActor::Tick(float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ASensorSpawnerActor::Tick);
  Super::Tick(DeltaSeconds);

  if(bRecordingData)
  {
    SaveSensorData(DeltaSeconds);
  }
  if(bRadarVisibility)
  {
    DrawRadarSensorPoints();
  }
}

void ASensorSpawnerActor::SaveSensorData(float DeltaSeconds)
{
  const FString FrameNumber = FString::Printf(TEXT("%lld"), FCarlaEngine::GetFrameCounter());
  for(ASensor* CurrentSensor : SpawnedSensorsArray)
  {
    if(ASceneCaptureSensor* CaptureSensor = Cast<ASceneCaptureSensor>(CurrentSensor))
    {
      const FString FinalPath = FPaths::Combine(SaveImagePath, CaptureSensor->GetName(), FString::Printf(TEXT("%lld"), FDateTime::Now().ToUnixTimestamp()) + "-Frame_" + FrameNumber + ".png");
      CaptureSensor->SaveCaptureToDisk(FinalPath);
      continue;
    }

    if(const AInertialMeasurementUnit* IMUSensor = Cast<AInertialMeasurementUnit>(CurrentSensor))
    {
      const FVector Accelerometer = IMUSensor->GetAccelerometerValue().ToFVector();
      const FVector Gyroscope = IMUSensor->GetGyroscopeValue().ToFVector();
      float Compass = IMUSensor->GetCompassValue();
      Compass = FMath::RadiansToDegrees(Compass);

      const FString FilePath = FPaths::Combine(SaveImagePath, IMUSensor->GetName(), IMUSensor->GetName() + ".json");
      UJsonFileManagerLibrary::SaveIMUDataToJson(FilePath, Accelerometer, Gyroscope, Compass, FrameNumber);
      continue;
    }

    if(const AGnssSensor* GnssSensor = Cast<AGnssSensor>(CurrentSensor))
    {
      const FString FilePath = FPaths::Combine(SaveImagePath, GnssSensor->GetName(), GnssSensor->GetName() + ".json");
      UJsonFileManagerLibrary::SaveGnssDataToJson(FilePath, GnssSensor->GetAltitudeValue(), GnssSensor->GetLatitudeValue(), GnssSensor->GetLongitudeValue(), FrameNumber);
    }

    if(const ARayCastLidar* LidarSensor = Cast<ARayCastLidar>(CurrentSensor))
    {
      const FString FilePath = FPaths::Combine(SaveImagePath, LidarSensor->GetName(), FString::Printf(TEXT("%lld"), FDateTime::Now().ToUnixTimestamp()) + "-Frame_" + FrameNumber + ".ply");
      UJsonFileManagerLibrary::SaveLidarDataToPly(FilePath, LidarSensor->GetTestPointCloud(), 4); 
    }

    if(const ARadar* RadarSensor = Cast<ARadar>(CurrentSensor))
    {
      const FString FilePath = FPaths::Combine(SaveImagePath, RadarSensor->GetName(), FString::Printf(TEXT("%lld"), FDateTime::Now().ToUnixTimestamp()) + "-Frame_" + FrameNumber + ".json");
      UJsonFileManagerLibrary::SaveRadarDataToJson(FilePath, RadarSensor->GetRadarData(), FrameNumber); 
    }
  }
}

void ASensorSpawnerActor::AttachSensorToActor(AActor* SensorActor)
{
  const UWorld* World = GetWorld();
  if(SensorActor && GetWorld() && AttachActorClass)
  {
    AActor* SensorAttachParent = UGameplayStatics::GetActorOfClass(World, AttachActorClass);
    if(SensorAttachParent)
    {
      SensorActor->AttachToActor(SensorAttachParent, FAttachmentTransformRules::KeepWorldTransform);
      SensorActor->SetOwner(SensorAttachParent);
      SensorActor->SetActorRelativeTransform(FTransform::Identity);
    }
  }
}

void ASensorSpawnerActor::DrawRadarSensorPoints() const
{
  float velocity_range = VelocityRange; // m/s
  for(const ASensor* CurrentSensor : SpawnedSensorsArray)
  {
    if(const ARadar* RadarSensor = Cast<ARadar>(CurrentSensor); CurrentSensor)
    {
      FRotator CurrentRot = RadarSensor->GetActorRotation(); 
      for( auto& detect : RadarSensor->GetRadarData().GetDetections() )
      {
        float Azi = FMath::RadiansToDegrees(detect.azimuth);
        float Alt = FMath::RadiansToDegrees(detect.altitude);

        FVector FWVector( (detect.depth - 0.25f) * 100.0f, 0.f, 0.f);
        FTransform SensorTransform(FRotator(CurrentRot.Pitch + Alt,
                                            CurrentRot.Yaw + Azi,
                                            CurrentRot.Roll));

        FWVector = SensorTransform.GetRotation().RotateVector(FWVector);

        float Norm_velocity = detect.velocity / velocity_range; 
        FColor ColorPoint;
        ColorPoint.R = FMath::Clamp(Norm_velocity, 0.0f, 1.0f) * 255;
        ColorPoint.G = FMath::Clamp( 1.0f - abs(Norm_velocity), 0.0f, 1.0f) * 255;
        ColorPoint.B = FMath::Clamp(abs(- 1.0f - Norm_velocity), 0.0f, 1.0f) * 255;
        
        DrawDebugPoint(GetWorld(), RadarSensor->GetActorLocation() + FWVector , PointSize, ColorPoint, false, 0.06);
      }
    }
  }
}
