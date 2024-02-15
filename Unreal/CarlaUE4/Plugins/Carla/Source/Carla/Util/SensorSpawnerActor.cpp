// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "Util/SensorSpawnerActor.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Game/CarlaGameModeBase.h"
#include "Sensor/Sensor.h"

ASensorSpawnerActor::ASensorSpawnerActor()
{
	PrimaryActorTick.bCanEverTick = false;

  SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
  RootComponent = SceneComp;
}

void ASensorSpawnerActor::BeginPlay()
{
  Super::BeginPlay();

  
  // Wait for the CarlaEpisode initialisation. It is done on CarlaGameMode BeginPlay().
  if(ACarlaGameModeBase* CarlaGameMode = Cast<ACarlaGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
  {
    CarlaGameMode->OnEpisodeInitialisedDelegate.AddDynamic(this, &ASensorSpawnerActor::OnEpisodeInitialised);
  }
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
    UE_LOG(LogTemp, Warning, TEXT("Warning: ASensorSpawnerActor::SpawnSensors - Delayed spawn already in progress, wait until it ends"));
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

void ASensorSpawnerActor::SpawnSensorActor(const FActorDescription& SensorDescription) const
{
  if(IsValid(CarlaEpisode))
  {
    FTransform Transform;
    GetRandomTransform(Transform);
    
    TPair<EActorSpawnResultStatus, FCarlaActor*> SpawnPair = CarlaEpisode->SpawnActorWithInfo(Transform, SensorDescription);
    
    UE_LOG(LogTemp, Log, TEXT("ASensorSpawnerActor::SpawnSensorActor: Id: %s, SpawnResult: %s, Valid: %s"), *SensorDescription.Id,
      *FString::FromInt(static_cast<uint8>(SpawnPair.Key)), SpawnPair.Value ? TEXT("true") : TEXT("false"));
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