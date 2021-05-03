// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/Sensor.h"

#include "Carla/Actor/ActorDescription.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

ASensor::ASensor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  auto *Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CamMesh"));
  Mesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
  Mesh->bHiddenInGame = true;
  Mesh->CastShadow = false;
  RootComponent = Mesh;
}

void ASensor::BeginPlay()
{
  Super::BeginPlay();
  OnPostTickDelegate = FWorldDelegates::OnWorldPostActorTick.AddUObject(
      this, &ASensor::PostPhysTickInternal);
}

void ASensor::Set(const FActorDescription &Description)
{
  // set the tick interval of the sensor
  if (Description.Variations.Contains("sensor_tick"))
  {
    SetActorTickInterval(
        UActorBlueprintFunctionLibrary::ActorAttributeToFloat(Description.Variations["sensor_tick"],
        0.0f));
  }
}

void ASensor::Tick(const float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ASensor::Tick);
  Super::Tick(DeltaTime);
  ReadyToTick = true;
  PrePhysTick(DeltaTime);
}

void ASensor::SetSeed(const int32 InSeed)
{
  check(RandomEngine != nullptr);
  Seed = InSeed;
  RandomEngine->Seed(InSeed);
}

void ASensor::PostActorCreated()
{
  Super::PostActorCreated();

#if WITH_EDITOR
  auto *StaticMeshComponent = Cast<UStaticMeshComponent>(RootComponent);
  if (StaticMeshComponent && !IsRunningCommandlet() && !StaticMeshComponent->GetStaticMesh())
  {
    UStaticMesh *CamMesh = LoadObject<UStaticMesh>(
        NULL,
        TEXT("/Engine/EditorMeshes/MatineeCam_SM.MatineeCam_SM"),
        NULL,
        LOAD_None,
        NULL);
    StaticMeshComponent->SetStaticMesh(CamMesh);
  }
#endif // WITH_EDITOR
}

void ASensor::EndPlay(EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
  Stream = FDataStream();

  FWorldDelegates::OnWorldPostActorTick.Remove(OnPostTickDelegate);
}

void ASensor::PostPhysTickInternal(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ASensor::PostPhysTickInternal);
  if(ReadyToTick)
  {
    PostPhysTick(World, TickType, DeltaSeconds);
    ReadyToTick = false;
  }
}
