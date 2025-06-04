// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/CollisionSensor.h"
#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaEngine.h"
#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Game/CarlaGameModeBase.h"

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

void ACollisionSensor::SetOwner(AActor* NewOwner)
{
  Super::SetOwner(NewOwner);

  /// @todo Deregister previous owner if there was one.

  if (NewOwner != nullptr)
  {
    NewOwner->OnActorHit.AddDynamic(this, &ACollisionSensor::OnCollisionEvent);
  }
}

void ACollisionSensor::OnCollisionEvent(
  AActor* Actor,
  AActor* OtherActor,
  FVector NormalImpulse,
  const FHitResult& Hit)
{
  if (Actor == nullptr || OtherActor == nullptr)
  {
    return;
  }

  uint64_t CurrentFrame = FCarlaEngine::GetFrameCounter();

  // remove all items from previous frames
  CollisionRegistry.erase(
    std::remove_if(
      CollisionRegistry.begin(),
      CollisionRegistry.end(),
      [CurrentFrame](std::tuple<uint64_t, AActor*, AActor*> Item)
      {
        return std::get<0>(Item) < CurrentFrame;
      }),
    CollisionRegistry.end());

  // check if this collision has been procesed already in this frame
  for (auto& Collision : CollisionRegistry)
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
  if (CurrentEpisode.GetRecorder()->IsEnabled()) {
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
    auto NormalImpulseVector = carla::geom::Vector3D(
      (float)NormalImpulse.X,
      (float)NormalImpulse.Y,
      (float)NormalImpulse.Z);
    AActor* ParentActor = GetAttachParentActor();
    if (ParentActor)
    {
      FTransform LocalTransformRelativeToParent = GetActorTransform().GetRelativeTransform(ParentActor->GetActorTransform());
      ROS2->ProcessDataFromCollisionSensor(
        0, StreamId,
        LocalTransformRelativeToParent,
        OtherActor->GetUniqueID(),
        NormalImpulseVector,
        this);
    }
    else
    {
      ROS2->ProcessDataFromCollisionSensor(
        0, StreamId,
        GetActorTransform(),
        OtherActor->GetUniqueID(),
        NormalImpulseVector,
        this);
    }
  }
#endif
}
