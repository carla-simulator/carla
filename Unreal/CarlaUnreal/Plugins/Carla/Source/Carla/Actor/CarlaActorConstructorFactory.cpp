// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaActorConstructorFactory.h"
#include "CarlaActor.h"


void CarlaActorConstructorFactory::Register(const FString& ClassName, UClass* ClassType, CreatorFunc CreatorFunction) {
    CreatorFunctions.Add(ClassName, CreatorFunction);
    RegisteredClasses.Add(ClassName, ClassType);
}

TSharedPtr<FCarlaActor> CarlaActorConstructorFactory::Create(const FString& className, 
                                                              FCarlaActor::IdType ActorId, 
                                                              AActor* Actor, 
                                                              TSharedPtr<const FActorInfo> Info, 
                                                              carla::rpc::ActorState InState, 
                                                              UWorld* World) const {
    if (CreatorFunctions.Contains(className))
    {
      return (*CreatorFunctions.Find(className))(ActorId, Actor, std::move(Info), InState, World);
    }
    return nullptr;
}

TSharedPtr<FCarlaActor> CarlaActorConstructorFactory::ConstructCarlaActor(
      FCarlaActor::IdType ActorId,
      AActor* Actor,
      TSharedPtr<const FActorInfo> Info,
      FCarlaActor::ActorType Type,
      FString CustomType,
      carla::rpc::ActorState InState,
      UWorld* World)
{
  switch(Type)
  {
  case FCarlaActor::ActorType::TrafficSign:
    return MakeShared<FTrafficSignActor>(ActorId, Actor, std::move(Info), InState, World);
    break;
  case FCarlaActor::ActorType::TrafficLight:
    return MakeShared<FTrafficLightActor>(ActorId, Actor, std::move(Info), InState, World);
    break;
  case FCarlaActor::ActorType::Vehicle:
    return MakeShared<FVehicleActor>(ActorId, Actor, std::move(Info), InState, World);
    break;
  case FCarlaActor::ActorType::Walker:
    return MakeShared<FWalkerActor>(ActorId, Actor, std::move(Info), InState, World);
    break;
  case FCarlaActor::ActorType::Sensor:
    return MakeShared<FSensorActor>(ActorId, Actor, std::move(Info), InState, World);
    break;
  case FCarlaActor::ActorType::Custom:
    return Create(CustomType, ActorId, Actor, std::move(Info), InState, World);
    break;
  default:
    return MakeShared<FOtherActor>(ActorId, Actor, std::move(Info), InState, World);
    break;
  }
}

FCarlaActor::ActorType CarlaActorConstructorFactory::GetActorType(const AActor *Actor)
{
  if (!Actor)
  {
    return FCarlaActor::ActorType::INVALID;
  }

  if (nullptr != Cast<ACarlaWheeledVehicle>(Actor))
  {
    return FCarlaActor::ActorType::Vehicle;
  }
  else if (nullptr != Cast<ACharacter>(Actor))
  {
    return FCarlaActor::ActorType::Walker;
  }
  else if (nullptr != Cast<ATrafficLightBase>(Actor))
  {
    return FCarlaActor::ActorType::TrafficLight;
  }
  else if (nullptr != Cast<ATrafficSignBase>(Actor))
  {
    return FCarlaActor::ActorType::TrafficSign;
  }
  else if (nullptr != Cast<ASensor>(Actor))
  {
    return FCarlaActor::ActorType::Sensor;
  }
  else
  {
    for (const auto& Pair : RegisteredClasses) {
      UClass* ClassType = Pair.Value; // UClass* from registration
      if (Actor->IsA(ClassType)) {
          return FCarlaActor::ActorType::Custom;
      }
    }
    return FCarlaActor::ActorType::Other;
  }
}

FString CarlaActorConstructorFactory::GetActorCustomType(const AActor *Actor)
{
  for (const auto& Pair : RegisteredClasses) {
      UClass* ClassType = Pair.Value; // UClass* from registration
      if (Actor->IsA(ClassType)) {
          return Pair.Key;
      }
    }
  return "";
}