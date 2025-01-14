// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Actor/ActorDispatcher.h"
#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Actor/ActorROS2Handler.h"
#include "Carla/Actor/CarlaActorFactory.h"
#include "Carla/Game/Tagger.h"
#include "Carla/Vehicle/VehicleControl.h"

#include <util/ue-header-guard-begin.h>
#include "GameFramework/Controller.h"
#ifdef WITH_ROS2
  #include <util/disable-ue4-macros.h>
  #include "carla/ros2/ROS2.h"
  #include <util/enable-ue4-macros.h>
  #include <variant>
#endif
#include <util/ue-header-guard-end.h>

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

FCarlaActor* UActorDispatcher::RegisterActor(
    AActor &Actor, FActorDescription Description,
    FActorRegistry::IdType DesiredId)
{
  FCarlaActor* View = Registry.Register(Actor, Description, DesiredId);
  if (View)
  {
    // TODO: support external actor destruction
    Actor.OnDestroyed.AddDynamic(this, &UActorDispatcher::OnActorDestroyed);

    // ROS2 mapping of actor->ros_name
    #ifdef WITH_ROS2
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    if (ROS2->IsEnabled())
    {
      // actor ros_name
      std::string RosName;
      for (auto &&Attr : Description.Variations)
      {
        if (Attr.Key == "ros_name")
        {
          RosName = std::string(TCHAR_TO_UTF8(*Attr.Value.Value));
        }
      }
      const std::string id = std::string(TCHAR_TO_UTF8(*Description.Id));
      if (RosName == id) {
        if(RosName.find("vehicle") != std::string::npos)
        {
          std::string VehicleName = "vehicle" + std::to_string(View->GetActorId());
          ROS2->AddActorRosName(static_cast<void*>(&Actor), VehicleName);
        }
        else
        {
          size_t pos = RosName.find_last_of('.');
          if (pos != std::string::npos) {
            std::string lastToken = RosName.substr(pos + 1) + "__";
            ROS2->AddActorRosName(static_cast<void*>(&Actor), lastToken);
          }
        }
      } else {
        ROS2->AddActorRosName(static_cast<void*>(&Actor), RosName);
      }

      // vehicle controller for hero
      for (auto &&Attr : Description.Variations)
      {
        if (Attr.Key == "role_name" && (Attr.Value.Value == "hero" || Attr.Value.Value == "ego"))
        {
          ROS2->AddActorCallback(static_cast<void*>(&Actor), RosName, [RosName](void *Actor, carla::ros2::ROS2CallbackData Data) -> void
          {
            AActor *UEActor = reinterpret_cast<AActor *>(Actor);
            ActorROS2Handler Handler(UEActor, RosName);
            std::visit(Handler, Data);
          });
          #if defined(WITH_ROS2_DEMO)
          ROS2->AddBasicSubscriberCallback(static_cast<void*>(&Actor), RosName, [RosName](void *Actor, carla::ros2::ROS2MessageCallbackData Data) -> void
          {
            AActor *UEActor = reinterpret_cast<AActor *>(Actor);
            ActorROS2Handler Handler(UEActor, RosName);
            std::visit(Handler, Data);
          });
          #endif
        }
      }
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
    if (CarlaActor->IsActive())
    {
      Registry.Deregister(CarlaActor->GetActorId());
    }
  }

  #ifdef WITH_ROS2
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  if (ROS2->IsEnabled())
  {
    ROS2->RemoveActorRosName(reinterpret_cast<void *>(Actor));
  }
  #endif
}
