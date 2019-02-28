// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/CollisionSensor.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Game/TheNewCarlaGameModeBase.h"

ACollisionSensor::ACollisionSensor(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;
}

FActorDefinition ACollisionSensor::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeGenericSensorDefinition(
      TEXT("other"),
      TEXT("collision"));
}

void ACollisionSensor::SetOwner(AActor *NewOwner)
{
  Super::SetOwner(NewOwner);

  /// @todo Deregister previous owner if there was one.

  if (NewOwner != nullptr)
  {
    NewOwner->OnActorHit.AddDynamic(this, &ACollisionSensor::OnCollisionEvent);
  }
}

void ACollisionSensor::BeginPlay()
{
  Super::BeginPlay();

  auto *GameMode = Cast<ATheNewCarlaGameModeBase>(GetWorld()->GetAuthGameMode());

  if (GameMode == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("ACollisionSensor: Game mode not compatible with this sensor"));
    return;
  }
  Episode = &GameMode->GetCarlaEpisode();
}

void ACollisionSensor::OnCollisionEvent(
    AActor *Actor,
    AActor *OtherActor,
    FVector NormalImpulse,
    const FHitResult &Hit)
{
  if ((Episode != nullptr) && (Actor != nullptr) && (OtherActor != nullptr))
  {
    constexpr float TO_METERS = 1e-2;
    NormalImpulse *= TO_METERS;
    GetDataStream(*this).Send(
        *this,
        Episode->SerializeActor(Episode->FindOrFakeActor(Actor)),
        Episode->SerializeActor(Episode->FindOrFakeActor(OtherActor)),
        carla::geom::Vector3D{NormalImpulse.X, NormalImpulse.Y, NormalImpulse.Z});
    // record the collision event
    if (Episode->GetRecorder()->IsEnabled())
      Episode->GetRecorder()->AddCollision(Actor, OtherActor);
  }
}
