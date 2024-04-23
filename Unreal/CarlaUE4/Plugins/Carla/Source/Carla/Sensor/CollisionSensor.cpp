// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/CollisionSensor.h"
#include "Carla.h"
#include "CoreMinimal.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaEngine.h"
#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Walker/WalkerBase.h"

ACollisionSensor::ACollisionSensor(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
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
  if (IsValid(NewOwner))
  {
    ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle>(NewOwner);
    if(IsValid(Vehicle))
    {
      Vehicle->GetMesh()->OnComponentHit.AddDynamic(this, &ACollisionSensor::OnComponentCollisionEvent);
    }
    else
    {
      AWalkerBase* Walker = Cast<AWalkerBase>(NewOwner);
      if(IsValid(Walker))
      {
        Walker->GetMesh()->OnComponentHit.AddDynamic(this, &ACollisionSensor::OnComponentCollisionEvent);
      }
      else
      {
        OnActorHit.AddDynamic(this, &ACollisionSensor::OnActorCollisionEvent);
      }
    }
  }
  else
  {
    UE_LOG(LogCarla, Log, TEXT("ACollisionSensor::SetOwner New owner is not valid or you are destroying collision sensor") );
  }
}

void ACollisionSensor::PrePhysTick(float DeltaSeconds) {
  Super::PrePhysTick(DeltaSeconds);

  // remove all items from previous frames
  uint64_t CurrentFrame = FCarlaEngine::GetFrameCounter();
  CollisionRegistry.erase(
      std::remove_if(
          CollisionRegistry.begin(),
          CollisionRegistry.end(),
          [CurrentFrame](std::tuple<uint64_t, AActor*, AActor*> Item)
          {
            return std::get<0>(Item) < CurrentFrame;
          }),
      CollisionRegistry.end());
}

void ACollisionSensor::OnCollisionEvent(
    AActor *Actor,
    AActor *OtherActor,
    FVector NormalImpulse,
    const FHitResult &Hit)
{
  if (!IsValid(OtherActor))
  {
    UE_LOG(LogCarla, Error, TEXT("ACollisionSensor::OnActorCollisionEvent Error with collided actor; Not valid.\n Collider actor %s"),
      *(Actor->GetName()) );
    return;
  }

  if (!IsValid(Actor))
  {
    UE_LOG(LogCarla, Error, TEXT("ACollisionSensor::OnActorCollisionEvent Error with collider actor; Not valid.\n Collided actor %s"),
      *(OtherActor->GetName()) );
    return;
  }

  uint64_t CurrentFrame = FCarlaEngine::GetFrameCounter();

  // check if this collision has been procesed already in this frame
  for (auto& Collision: CollisionRegistry)
  {
    if (std::get<0>(Collision) == CurrentFrame &&
        std::get<1>(Collision) == Actor &&
        std::get<2>(Collision) == OtherActor)
    {
      return;
    }
  }

  const auto& CurrentEpisode = GetEpisode();
  constexpr float TO_METERS = 1e-2;
  NormalImpulse *= TO_METERS;
  GetDataStream(*this).SerializeAndSend(
      *this,
      CurrentEpisode.SerializeActor(Actor),
      CurrentEpisode.SerializeActor(OtherActor),
      carla::geom::Vector3D(
          (float)NormalImpulse.X,
          (float)NormalImpulse.Y,
          (float)NormalImpulse.Z));

  // record the collision event
  if (CurrentEpisode.GetRecorder()->IsEnabled()){
      CurrentEpisode.GetRecorder()->AddCollision(Actor, OtherActor);
  }

  CollisionRegistry.emplace_back(CurrentFrame, Actor, OtherActor);

  // ROS2
#if defined(WITH_ROS2)
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  if (ROS2->IsEnabled())
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("ROS2 Send");
    auto StreamId = carla::streaming::detail::token_type(GetToken()).get_stream_id();
    AActor* ParentActor = GetAttachParentActor();
    if (ParentActor)
    {
      FTransform LocalTransformRelativeToParent = GetActorTransform().GetRelativeTransform(ParentActor->GetActorTransform());
      ROS2->ProcessDataFromCollisionSensor(0, StreamId, LocalTransformRelativeToParent, OtherActor->GetUniqueID(), carla::geom::Vector3D{NormalImpulse.X, NormalImpulse.Y, NormalImpulse.Z}, this);
    }
    else
    {
      ROS2->ProcessDataFromCollisionSensor(0, StreamId, GetActorTransform(), OtherActor->GetUniqueID(), carla::geom::Vector3D{NormalImpulse.X, NormalImpulse.Y, NormalImpulse.Z}, this);
    }
  }
#endif
}

void ACollisionSensor::OnActorCollisionEvent(
    AActor *Actor,
    AActor *OtherActor,
    FVector NormalImpulse,
    const FHitResult &Hit)
{
  OnCollisionEvent(Actor, OtherActor, NormalImpulse, Hit);
}

void ACollisionSensor::OnComponentCollisionEvent(
      UPrimitiveComponent* HitComp,
      AActor* OtherActor,
      UPrimitiveComponent* OtherComp,
      FVector NormalImpulse,
      const FHitResult& Hit)
{
  AActor* Actor = HitComp->GetOwner();
  OnCollisionEvent(Actor, OtherActor, NormalImpulse, Hit);
}
