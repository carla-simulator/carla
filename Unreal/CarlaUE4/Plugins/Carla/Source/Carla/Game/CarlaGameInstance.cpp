// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Game/CarlaGameInstance.h"
#include "Carla.h"
#include "CarlaEpisode.h"

#include "Carla/Settings/CarlaSettings.h"
#include "Sensor/SceneCaptureCamera.h"
#include "Sensor/SensorFactory.h"

UCarlaGameInstance::UCarlaGameInstance() {
  CarlaSettings = CreateDefaultSubobject<UCarlaSettings>(TEXT("CarlaSettings"));
  Recorder = CreateDefaultSubobject<ACarlaRecorder>(TEXT("Recorder"));
  CarlaEngine.SetRecorder(Recorder);

  check(CarlaSettings != nullptr);
  CarlaSettings->LoadSettings();
  CarlaSettings->LogSettings();
}

UCarlaGameInstance::~UCarlaGameInstance() = default;

void UCarlaGameInstance::StartSpawningCameras()
{
  // Save pointer to sensors data that we want to spawn later
  if(const FActorDefinition* Definition = GetActorDefinitionByClass(ASceneCaptureCamera::StaticClass()))
  {
    SensorsDefinitionsToSpawn.Add(Definition);
  }
  if(const FActorDefinition* Definition = GetActorDefinitionByClass(AGnssSensor::StaticClass()))
  {
    SensorsDefinitionsToSpawn.Add(Definition);
  }
  if(const FActorDefinition* Definition = GetActorDefinitionByClass(AInertialMeasurementUnit::StaticClass()))
  {
    SensorsDefinitionsToSpawn.Add(Definition);
  }
  /*SensorsToSpawn.Add({55, ASceneCaptureCamera::GetSensorDefinition()});
  SensorsToSpawn.Add({46, AGnssSensor::GetSensorDefinition()});
  SensorsToSpawn.Add({47, AInertialMeasurementUnit::GetSensorDefinition()});*/
  
  UE_LOG(LogTemp, Log, TEXT("UCarlaGameInstance::StartSpawningCameras - Start spawning cameras"));
  GetTimerManager().SetTimer(SpawnCameraTimerHandle, this, &UCarlaGameInstance::SpawnMultipleSensorCameras, InitialDelay - SpawnRate);
  
  GetTimerManager().SetTimer(SpawnDelayedCameraTimerHandle, this, &UCarlaGameInstance::SpawnSensorCamera, SpawnRate, true, InitialDelay);

  const float InvalidateDelay = SpawnRate * static_cast<float>(NumCameras) + InitialDelay + 0.5f;
  GetTimerManager().SetTimer(ResetSpawnDelayedCameraTimerHandle, this, &UCarlaGameInstance::InvalidateDelayedCameraTimerHandle, InvalidateDelay);
  
}

const FActorDefinition* UCarlaGameInstance::GetActorDefinitionByClass(const TSubclassOf<AActor> ActorClass)
{
  if(!ActorClass)
  {
    return nullptr;
  }
  
  const TArray<FActorDefinition>& ActorDefinitions = GetCarlaEpisode()->GetActorDefinitions();
  const FActorDefinition* ActorDefinition = ActorDefinitions.FindByPredicate([&](const FActorDefinition& ActorDef){ return ActorDef.Class->IsChildOf(ActorClass); });

  return ActorDefinition;
}

void UCarlaGameInstance::InvalidateDelayedCameraTimerHandle()
{
  GetTimerManager().ClearTimer(SpawnDelayedCameraTimerHandle);
}

void UCarlaGameInstance::SpawnMultipleSensorCameras()
{
  for(int i = 0; i < NumCameras; i++)
  {
    SpawnSensorCamera();
  }
}

void UCarlaGameInstance::SpawnSensorCamera()
{
  FTransform Transform = FTransform::Identity;
  const float PosX = FMath::FRandRange(-140000.f, -110000.f);
  const float PosY = FMath::FRandRange(-140000.f, -110000.f);
  Transform.SetLocation(FVector(PosX,PosY,9400.0f));

  FActorDescription CameraActorDescription;
  GenerateSensorActorDescription(CameraActorDescription);

  // Add custom values to CameraActorDescription;
  
  SpawnCameraActor(Transform, CameraActorDescription);
}

void UCarlaGameInstance::GenerateSensorActorDescription(FActorDescription& ActorDescription) const
{
  check(!SensorsDefinitionsToSpawn.IsEmpty());
  const int RandSensorToSpawn = FMath::RandRange(0, SensorsDefinitionsToSpawn.Num() - 1);
  
  const FActorDefinition* ActorDefinition = SensorsDefinitionsToSpawn[RandSensorToSpawn];
  ActorDescription.UId = ActorDefinition->UId;
  ActorDescription.Id = ActorDefinition->Id;
  ActorDescription.Class = ActorDefinition->Class;
  ActorDescription.Variations.Reserve(ActorDefinition->Variations.Num());

  FActorAttribute CreatedAttribute;
  for(const FActorVariation& Variation : ActorDefinition->Variations)
  {
    if(Variation.RecommendedValues.IsValidIndex(0))
    {
      CreatedAttribute.Id = Variation.Id;
      CreatedAttribute.Type = Variation.Type;
      CreatedAttribute.Value = Variation.RecommendedValues[0];
      ActorDescription.Variations.Emplace(CreatedAttribute.Id, CreatedAttribute);
    }
  }
}

void UCarlaGameInstance::SpawnCameraActor(const FTransform &Transform, FActorDescription ThisActorDescription)
{
  if(UCarlaEpisode* CarlaEpisode = GetCarlaEpisode())
  {
    TPair<EActorSpawnResultStatus, FCarlaActor*> SpawnPair = CarlaEpisode->SpawnActorWithInfo(Transform, ThisActorDescription);
    UE_LOG(LogTemp, Log, TEXT("UCarlaGameInstance::SpawnCameraActor: Id: %s, SpawnResult: %s, Valid: %s"), *ThisActorDescription.Id,
      *FString::FromInt(static_cast<uint8>(SpawnPair.Key)), SpawnPair.Value ? TEXT("true") : TEXT("false"));
  }
}


