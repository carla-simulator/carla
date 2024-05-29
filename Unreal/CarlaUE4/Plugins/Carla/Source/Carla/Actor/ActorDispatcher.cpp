// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Actor/ActorDispatcher.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Actor/CarlaActorFactory.h"
#include "Carla/Util/BoundingBoxCalculator.h"

#include "Carla/Game/Tagger.h"
#include "Carla/Vehicle/VehicleControl.h"

#include "GameFramework/Controller.h"

#if defined(WITH_ROS2)
#  include <compiler/disable-ue4-macros.h>
#  include "carla/ros2/ROS2.h"
#  include "carla/ros2/types/PublisherSensorType.h"
#  include "carla/ros2/types/SensorActorDefinition.h"
#  include "carla/ros2/types/VehicleActorDefinition.h"
#  include "carla/ros2/types/WalkerActorDefinition.h"
#  include "carla/ros2/types/TrafficSignActorDefinition.h"
#  include "carla/ros2/types/TrafficLightActorDefinition.h"
#  include <compiler/enable-ue4-macros.h>

#  include "Carla/Sensor/CollisionSensor.h"
#  include "Carla/Sensor/DepthCamera.h"
#  include "Carla/Sensor/NormalsCamera.h"
#  include "Carla/Sensor/DVSCamera.h"
#  include "Carla/Sensor/GnssSensor.h"
#  include "Carla/Sensor/InertialMeasurementUnit.h"
#  include "Carla/Sensor/LaneInvasionSensor.h"
#  include "Carla/Sensor/ObstacleDetectionSensor.h"
#  include "Carla/Sensor/OpticalFlowCamera.h"
#  include "Carla/Sensor/Radar.h"
#  include "Carla/Sensor/RayCastLidar.h"
#  include "Carla/Sensor/RayCastSemanticLidar.h"
#  include "Carla/Sensor/RssSensor.h"
#  include "Carla/Sensor/SceneCaptureCamera.h"
#  include "Carla/Sensor/SemanticSegmentationCamera.h"
#  include "Carla/Sensor/InstanceSegmentationCamera.h"
#endif

void UActorDispatcher::Bind(FActorDefinition Definition, SpawnFunctionType Functor)
{
  if (UActorBlueprintFunctionLibrary::CheckActorDefinition(Definition))
  {
    Definition.UId = static_cast<uint32>(SpawnFunctions.Num()) + 1u;
    Definitions.Emplace(Definition);
    SpawnFunctions.Emplace(Functor);
    Classes.Emplace(Definition.Class);
  }
  else
  {
    UE_LOG(LogCarla, Warning, TEXT("Invalid definition '%s' ignored"), *Definition.Id);
  }
}

void UActorDispatcher::Bind(ACarlaActorFactory &ActorFactory)
{
  for (const auto &Definition : ActorFactory.GetDefinitions())
  {
    Bind(Definition, [&](const FTransform &Transform, const FActorDescription &Description) {
      return ActorFactory.SpawnActor(Transform, Description);
    });
  }
}

TPair<EActorSpawnResultStatus, FCarlaActor*> UActorDispatcher::SpawnActor(
    const FTransform &Transform,
    FActorDescription Description,
    FCarlaActor::IdType DesiredId)
{
  if ((Description.UId == 0u) || (Description.UId > static_cast<uint32>(SpawnFunctions.Num())))
  {
    UE_LOG(LogCarla, Error, TEXT("Invalid ActorDescription '%s' (UId=%d)"), *Description.Id, Description.UId);
    return MakeTuple(EActorSpawnResultStatus::InvalidDescription, nullptr);
  }

  UE_LOG(LogCarla, Log, TEXT("Spawning actor '%s'"), *Description.Id);

  Description.Class = Classes[Description.UId - 1];
  FActorSpawnResult Result = SpawnFunctions[Description.UId - 1](Transform, Description);

  if ((Result.Status == EActorSpawnResultStatus::Success) && (Result.Actor == nullptr))
  {
    UE_LOG(LogCarla, Warning, TEXT("ActorSpawnResult: Trying to spawn '%s'"), *Description.Id);
    UE_LOG(LogCarla, Warning, TEXT("ActorSpawnResult: Reported success but did not return an actor"));
    Result.Status = EActorSpawnResultStatus::UnknownError;
  }

  FCarlaActor* View = Result.IsValid() ?
      RegisterActor(*Result.Actor, std::move(Description), DesiredId) : nullptr;
  if (!View)
  {
    UE_LOG(LogCarla, Warning, TEXT("Failed to spawn actor '%s'"), *Description.Id);
    check(Result.Status != EActorSpawnResultStatus::Success);
  }
  else
  {
    ATagger::TagActor(*View->GetActor(), true);
  }

  return MakeTuple(Result.Status, View);
}

AActor* UActorDispatcher::ReSpawnActor(
    const FTransform &Transform,
    FActorDescription Description)
{
  if ((Description.UId == 0u) || (Description.UId > static_cast<uint32>(SpawnFunctions.Num())))
  {
    UE_LOG(LogCarla, Error, TEXT("Invalid ActorDescription '%s' (UId=%d)"), *Description.Id, Description.UId);
    return nullptr;
  }

  UE_LOG(LogCarla, Log, TEXT("Spawning actor '%s'"), *Description.Id);

  Description.Class = Classes[Description.UId - 1];
  FActorSpawnResult Result = SpawnFunctions[Description.UId - 1](Transform, Description);

  if ((Result.Status == EActorSpawnResultStatus::Success) && (Result.Actor == nullptr))
  {
    UE_LOG(LogCarla, Warning, TEXT("ActorSpawnResult: Trying to spawn '%s'"), *Description.Id);
    UE_LOG(LogCarla, Warning, TEXT("ActorSpawnResult: Reported success but did not return an actor"));
    Result.Status = EActorSpawnResultStatus::UnknownError;
    return nullptr;
  }

  if (Result.Status == EActorSpawnResultStatus::Success)
  {
    return Result.Actor;
  }

  return nullptr;
}

bool UActorDispatcher::DestroyActor(FCarlaActor::IdType ActorId)
{
  // Check if the actor is in the registry.
  FCarlaActor* View = Registry.FindCarlaActor(ActorId);

  // Invalid destruction if is not marked to PendingKill (except is dormant, dormant actors can be destroyed)
  if (!View)
  {
    UE_LOG(LogCarla, Warning, TEXT("Trying to destroy actor that is not in the registry"));
    return false;
  }

  const FString &Id = View->GetActorInfo()->Description.Id;

  // Destroy its controller if present.
  AActor* Actor = View->GetActor();
  if(Actor)
  {
    APawn* Pawn = Cast<APawn>(Actor);
    AController* Controller = (Pawn != nullptr ? Pawn->GetController() : nullptr);
    if (Controller != nullptr)
    {
      UE_LOG(LogCarla, Log, TEXT("Destroying actor's controller: '%s'"), *Id);
      bool Success = Controller->Destroy();
      if (!Success)
      {
        UE_LOG(LogCarla, Error, TEXT("Failed to destroy actor's controller: '%s'"), *Id);
      }
    }

    // Destroy the actor.
    UE_LOG(LogCarla, Log, TEXT("UActorDispatcher::Destroying actor: '%s' %x"), *Id, Actor);
    UE_LOG(LogCarla, Log, TEXT("            %s"), Actor?*Actor->GetName():*FString("None"));
    if (!Actor || !Actor->Destroy())
    {
      UE_LOG(LogCarla, Error, TEXT("Failed to destroy actor: '%s'"), *Id);
      return false;
    }
  }

  Registry.Deregister(ActorId);

  return true;
}

#if defined(WITH_ROS2)
carla::ros2::types::PublisherSensorType GetPublisherSensorType(ASensor * Sensor) {
      // map the Ue4 sensors to their ESensor type and stream_id
  carla::ros2::types::PublisherSensorType SensorType = carla::ros2::types::PublisherSensorType::Unknown;
  if ( dynamic_cast<ACollisionSensor *>(Sensor) ) {
    SensorType = carla::ros2::types::PublisherSensorType::CollisionSensor;
  } else if ( dynamic_cast<ADepthCamera *>(Sensor) ) {
    SensorType = carla::ros2::types::PublisherSensorType::DepthCamera;
  } else if ( dynamic_cast<ANormalsCamera *>(Sensor) ) {
    SensorType = carla::ros2::types::PublisherSensorType::NormalsCamera;
  } else if ( dynamic_cast<ADVSCamera *>(Sensor) ) {
    SensorType = carla::ros2::types::PublisherSensorType::DVSCamera;
  } else if ( dynamic_cast<AGnssSensor *>(Sensor) ) {
    SensorType = carla::ros2::types::PublisherSensorType::GnssSensor;
  } else if ( dynamic_cast<AInertialMeasurementUnit *>(Sensor) ) {
    SensorType = carla::ros2::types::PublisherSensorType::InertialMeasurementUnit;
  } else if ( dynamic_cast<ALaneInvasionSensor *>(Sensor) ) {
    SensorType = carla::ros2::types::PublisherSensorType::LaneInvasionSensor;
  } else if ( dynamic_cast<AObstacleDetectionSensor *>(Sensor) ) {
    SensorType = carla::ros2::types::PublisherSensorType::ObstacleDetectionSensor;
  } else if ( dynamic_cast<AOpticalFlowCamera *>(Sensor) ) {
    SensorType = carla::ros2::types::PublisherSensorType::OpticalFlowCamera;
  } else if ( dynamic_cast<ARadar *>(Sensor) ) {
    SensorType = carla::ros2::types::PublisherSensorType::Radar;
  // BE CAREFUL: FIRST CHECK ARayCastLidar, because that's derived from RayCastSemanticLidar!!
  } else if ( dynamic_cast<ARayCastLidar *>(Sensor) ) {
    SensorType = carla::ros2::types::PublisherSensorType::RayCastLidar;
  } else if ( dynamic_cast<ARayCastSemanticLidar *>(Sensor) ) {
    SensorType = carla::ros2::types::PublisherSensorType::RayCastSemanticLidar;
  } else if ( dynamic_cast<ARssSensor *>(Sensor) ) {
    SensorType = carla::ros2::types::PublisherSensorType::RssSensor;
  } else if ( dynamic_cast<ASceneCaptureCamera *>(Sensor) ) {
    SensorType = carla::ros2::types::PublisherSensorType::SceneCaptureCamera;
  } else if ( dynamic_cast<ASemanticSegmentationCamera *>(Sensor) ) {
    SensorType = carla::ros2::types::PublisherSensorType::SemanticSegmentationCamera;
  } else if ( dynamic_cast<AInstanceSegmentationCamera *>(Sensor) ) {
    SensorType = carla::ros2::types::PublisherSensorType::InstanceSegmentationCamera;
  } else if ( dynamic_cast<FCameraGBufferUint8 *>(Sensor) ) {
    SensorType = carla::ros2::types::PublisherSensorType::CameraGBufferUint8;
  } else if ( dynamic_cast<FCameraGBufferFloat *>(Sensor) ) {
    SensorType = carla::ros2::types::PublisherSensorType::CameraGBufferFloat;
  } else {
     // not derived from ASensor, is initialized in each case separately
     //carla::ros2::types::PublisherSensorType::WorldObserver

     carla::log_error("Getcarla::ros2::types::PublisherSensorType : invalid sensor type");
  }
  return SensorType;
}

void RegisterActorROS2(std::shared_ptr<carla::ros2::ROS2> ROS2, FCarlaActor* CarlaActor, carla::ros2::types::ActorNameDefinition ActorNameDefinition) {
  auto *Sensor = Cast<ASensor>(CarlaActor->GetActor());
  auto *Vehicle = Cast<ACarlaWheeledVehicle>(CarlaActor->GetActor());
  auto *Walker = Cast<AWalkerBase>(CarlaActor->GetActor());
  auto *TrafficLight = Cast<ATrafficLightBase>(CarlaActor->GetActor());
  auto *TrafficSign = Cast<ATrafficSignBase>(CarlaActor->GetActor());
  if ( Sensor != nullptr ) {
    auto SensorActorDefinition = std::make_shared<carla::ros2::types::SensorActorDefinition>(
      ActorNameDefinition,
      GetPublisherSensorType(Sensor),
      carla::streaming::detail::token_type(Sensor->GetToken()).get_stream_id());
    ROS2->AddSensorUe(SensorActorDefinition);
  }
  else if (Vehicle != nullptr ) {
    FVehiclePhysicsControl PhysicsControl;
    CarlaActor->GetPhysicsControl(PhysicsControl);

    auto VehicleActorDefinition = std::make_shared<carla::ros2::types::VehicleActorDefinition>(
      carla::ros2::types::ActorDefinition(ActorNameDefinition,
        CarlaActor->GetActorInfo()->BoundingBox,
        carla::ros2::types::Polygon()),
        PhysicsControl);
    auto SkeletalMeshComponent = Vehicle->GetMesh();
    if (SkeletalMeshComponent != nullptr) {
      VehicleActorDefinition->vertex_polygon.SetGlobalVertices(UBoundingBoxCalculator::GetSkeletalMeshVertices(SkeletalMeshComponent->SkeletalMesh));
    }

    carla::ros2::types::VehicleControlCallback VehicleControlCallback = [Vehicle](carla::ros2::types::VehicleControl const &Source) -> void {
      EVehicleInputPriority InputPriority = EVehicleInputPriority(Source.ControlPriority());
      if ( InputPriority <= EVehicleInputPriority::User) {
        // priority at least on User level, but allow multiple input prios to allow e.g. manual overrides
        InputPriority = EVehicleInputPriority::User;
      }
      Vehicle->ApplyVehicleControl(Source.GetVehicleControl(), InputPriority);
    };

    carla::ros2::types::VehicleAckermannControlCallback VehicleAckermannControlCallback = [Vehicle](carla::ros2::types::VehicleAckermannControl const &Source) -> void {
      Vehicle->ApplyVehicleAckermannControl(Source.GetVehicleAckermannControl());
    };

    ROS2->AddVehicleUe(VehicleActorDefinition, VehicleControlCallback, VehicleAckermannControlCallback);
  }
  else if ( Walker != nullptr ) {
    auto WalkerActorDefinition = std::make_shared<carla::ros2::types::WalkerActorDefinition>(
        carla::ros2::types::ActorDefinition(ActorNameDefinition,
                          CarlaActor->GetActorInfo()->BoundingBox,
              carla::ros2::types::Polygon()));
    auto SkeletalMeshComponent = Walker->GetMesh();
    if (SkeletalMeshComponent != nullptr) {
      WalkerActorDefinition->vertex_polygon.SetGlobalVertices(UBoundingBoxCalculator::GetSkeletalMeshVertices(SkeletalMeshComponent->SkeletalMesh));
    }

    auto WalkerController = Cast<AWalkerController>(Walker->GetController());
    carla::ros2::types::WalkerControlCallback walker_control_callback = [WalkerController](carla::ros2::types::WalkerControl const &Source) -> void {
      WalkerController->ApplyWalkerControl(Source.GetWalkerControl());
    };

    ROS2->AddWalkerUe(WalkerActorDefinition, walker_control_callback);
  }
  else if ( TrafficLight != nullptr ) {
    auto TrafficLightActorDefinition = std::make_shared<carla::ros2::types::TrafficLightActorDefinition>(ActorNameDefinition);
    ROS2->AddTrafficLightUe(TrafficLightActorDefinition);
  }
  else if ( TrafficSign != nullptr ) {
    auto TrafficSignActorDefinition = std::make_shared<carla::ros2::types::TrafficSignActorDefinition>(ActorNameDefinition);
    ROS2->AddTrafficSignUe(TrafficSignActorDefinition);
  }
}
#endif

FCarlaActor* UActorDispatcher::RegisterActor(
    AActor &Actor, FActorDescription Description,
    FActorRegistry::IdType DesiredId)
{
  FCarlaActor* View = Registry.Register(Actor, Description, DesiredId);
  if (View)
  {
    // TODO: support external actor destruction
    Actor.OnDestroyed.AddDynamic(this, &UActorDispatcher::OnActorDestroyed);

    #if defined(WITH_ROS2)
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    if (ROS2->IsEnabled())
    {
      std::string const EnabledForRosString = TCHAR_TO_UTF8(*Description.GetAttribute("enabled_for_ros").Value);
      bool EnabledForRos = false;
      if ( (EnabledForRosString == "") && (ROS2->VisibilityDefaultMode() == carla::ros2::ROS2::TopicVisibilityDefaultMode::eOn )) {
          EnabledForRos = true;
      }
      else {
        EnabledForRos = static_cast<bool>(std::atoi(EnabledForRosString.c_str()));
      }

      carla::ros2::types::ActorNameDefinition ActorNameDefinition( 
        View->GetActorId(),
        std::string(TCHAR_TO_UTF8(*View->GetActorInfo()->Description.Id)),
        std::string(TCHAR_TO_UTF8(*Description.GetAttribute("ros_name").Value)),
        std::string(TCHAR_TO_UTF8(*Description.GetAttribute("role_name").Value)),
        std::string(TCHAR_TO_UTF8(*Description.GetAttribute("object_type").Value)),
        std::string(TCHAR_TO_UTF8(*Description.GetAttribute("base_type").Value)),
        EnabledForRos);
      RegisterActorROS2(ROS2, View, ActorNameDefinition);
    }
    #endif
  }
  return View;
}

void UActorDispatcher::PutActorToSleep(FCarlaActor::IdType Id, UCarlaEpisode* CarlaEpisode)
{
  Registry.PutActorToSleep(Id, CarlaEpisode);
}

void UActorDispatcher::WakeActorUp(FCarlaActor::IdType Id, UCarlaEpisode* CarlaEpisode)
{
  Registry.WakeActorUp(Id, CarlaEpisode);
}

void UActorDispatcher::OnActorDestroyed(AActor *Actor)
{
  FCarlaActor* CarlaActor = Registry.FindCarlaActor(Actor);
  if (CarlaActor)
  {
    auto const ActorId = CarlaActor->GetActorId();

    #if defined(WITH_ROS2)
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    if (ROS2->IsEnabled())
    {
      ROS2->RemoveActor(ActorId);
    }
    #endif

    if (CarlaActor->IsActive())
    {
      Registry.Deregister(ActorId);
    }
  }

}
