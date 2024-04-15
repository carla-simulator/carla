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
  if (NewOwner != nullptr)
  {
    NewOwner->OnActorHit.AddDynamic(this, &ACollisionSensor::OnCollisionEvent);
    UE_LOG(LogCarla, Error, TEXT("New owner is valid") );
  }
  else
  {
    UE_LOG(LogCarla, Error, TEXT("New owner is not valid") );
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

  if(!GetOwner()->OnActorHit.IsBound()){
    GetOwner()->OnActorHit.AddDynamic(this, &ACollisionSensor::OnCollisionEvent);
  }

}

void ACollisionSensor::OnCollisionEvent(
    AActor *Actor,
    AActor *OtherActor,
    FVector NormalImpulse,
    const FHitResult &Hit)
{
  UE_LOG(LogCarla, Log, TEXT("Collision size %d"), CollisionRegistry.size() );

  if (OtherActor == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("ACollisionSensor::OnCollisionEvent Error with collided actor; Not valid.\n Collider actor %s"),
      *(Actor->GetName()) );
    return;
  }

  if (Actor == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("ACollisionSensor::OnCollisionEvent Error with collider actor; Not valid.\n Collided actor %s"),
      *(OtherActor->GetName()) );
    return;
  }

  uint64_t CurrentFrame = FCarlaEngine::GetFrameCounter();

  UE_LOG(LogCarla, Warning, TEXT("Collision detected betweem %s and %s"),
    *(Actor->GetName()), *(OtherActor->GetName()) );
  // check if this collision has been procesed already in this frame
  for (auto& Collision: CollisionRegistry)
  {

    UE_LOG(LogCarla, Error, TEXT("Collision repeated betweem %s and %s"),
      *(std::get<1>(Collision)->GetName()), *(std::get<2>(Collision)->GetName()) );

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
