// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "Util/SensorSpawnerActor.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Game/CarlaGameModeBase.h"
#include "Sensor/SceneCaptureCamera.h"
#include "Sensor/Sensor.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogSensorSpawnerActor, Verbose, All);

ASensorSpawnerActor::ASensorSpawnerActor()
{
	PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.bStartWithTickEnabled = false;
  PrimaryActorTick.TickInterval = TickInterval;
  
  SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
  RootComponent = SceneComp;

  SensorClassToCapture = ASceneCaptureCamera::StaticClass();
}

void ASensorSpawnerActor::BeginPlay()
{
  Super::BeginPlay();

  // Wait for the CarlaEpisode initialisation. It is done on CarlaGameMode BeginPlay().
  if(ACarlaGameModeBase* CarlaGameMode = Cast<ACarlaGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
  {
    CarlaGameMode->OnEpisodeInitialisedDelegate.AddDynamic(this, &ASensorSpawnerActor::OnEpisodeInitialised);
  }

  SetActorTickInterval(TickInterval);
  SaveImagePath = FPaths::ProjectSavedDir() + "/SensorSpawnerCaptures/" + FString::Printf(TEXT("%lld"), FDateTime::Now().ToUnixTimestamp());
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
        SpawnSensorActor(SensorDescription);
      }
    }
  }
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

void ASensorSpawnerActor::SpawnSensorActor(const FActorDescription& SensorDescription)
{
  if(IsValid(CarlaEpisode))
  {
    FTransform Transform;
    GetRandomTransform(Transform);
    
    const TPair<EActorSpawnResultStatus, FCarlaActor*> SpawnResult = CarlaEpisode->SpawnActorWithInfo(Transform, SensorDescription);
    
    if(bSaveCameraToDisk && SpawnResult.Value)
    {
      StartSavingCapturesToDisk(SpawnResult.Value->GetActor());
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
    SpawnSensorActor(SensorDescription);
  }

  SensorsToSpawnCopy[0].Amount--;

  if(SensorsToSpawnCopy[0].Amount <= 0)
  {
    SensorsToSpawnCopy.RemoveAt(0);
  }
}

void ASensorSpawnerActor::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  for(const ASceneCaptureSensor* CaptureSensor : SceneCaptureSensor)
  {
    if(CaptureSensor)
    {
      CaptureSensor->SaveCaptureToDisk(SaveImagePath + "/" + CaptureSensor->GetName() + "/" + FString::Printf(TEXT("%lld"), FDateTime::Now().ToUnixTimestamp()) + ".png" );
    }
  }
}

void ASensorSpawnerActor::RemoveSceneCaptureCameras()
{
  if(SceneCaptureSensor.IsValidIndex(0))
  {
    SceneCaptureSensor.RemoveAt(0);
  }

  if(SceneCaptureSensor.IsEmpty())
  {
    SetActorTickEnabled(false);
  }
}

void ASensorSpawnerActor::StartSavingCapturesToDisk(const AActor* Actor)
{
  if(const ASceneCaptureSensor* CaptureSensor = Cast<ASceneCaptureSensor>(Actor))
  {
    if(SensorClassToCapture == CaptureSensor->GetClass() || SensorClassToCapture == nullptr)
    {
      SceneCaptureSensor.Add(CaptureSensor);
      SetActorTickEnabled(true);
      FTimerHandle CaptureTimerHandle;
      GetWorldTimerManager().SetTimer(CaptureTimerHandle, this, &ASensorSpawnerActor::RemoveSceneCaptureCameras, CaptureTime);
    }
  }
}
