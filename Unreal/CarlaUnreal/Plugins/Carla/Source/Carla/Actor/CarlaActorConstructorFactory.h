// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CarlaActor.h"

/// A view over an actor and its properties.
class CARLA_API CarlaActorConstructorFactory
{
public:

  using CreatorFunc = std::function<TSharedPtr<FCarlaActor>(FCarlaActor::IdType ActorId, 
                                      AActor* Actor, 
                                      TSharedPtr<const FActorInfo> Info, 
                                      carla::rpc::ActorState InState, 
                                      UWorld* World)>;

  static CarlaActorConstructorFactory& Instance() {
        static CarlaActorConstructorFactory instance;
        return instance;
    }

  virtual ~CarlaActorConstructorFactory() {};

  void Register(const FString& ClassName, UClass* ClassType, CreatorFunc Creator);

  TSharedPtr<FCarlaActor> Create(const FString& className, 
                                      FCarlaActor::IdType ActorId, 
                                      AActor* Actor, 
                                      TSharedPtr<const FActorInfo> Info, 
                                      carla::rpc::ActorState InState, 
                                      UWorld* World) const;

  FCarlaActor::ActorType GetActorType(const AActor *Actor);
  FString GetActorCustomType(const AActor *Actor);

  TSharedPtr<FCarlaActor> ConstructCarlaActor(
      FCarlaActor::IdType ActorId,
      AActor* Actor,
      TSharedPtr<const FActorInfo> Info,
      FCarlaActor::ActorType Type,
      FString CustomType,
      carla::rpc::ActorState InState,
      UWorld* World);

protected:
    TMap<FString, CreatorFunc> CreatorFunctions;
    TMap<FString, UClass*> RegisteredClasses;

    CarlaActorConstructorFactory() = default; // Private constructor for singleton
};

#define REGISTER_CLASS(className, classPointer) \
    namespace { \
        const bool registered_##className = []() { \
            CarlaActorConstructorFactory::Instance().Register(#className, #classPointer, []( \
                FCarlaActor::IdType ActorId, \
                AActor* Actor, \
                TSharedPtr<const FActorInfo> Info, \
                carla::rpc::ActorState InState, \
                UWorld* World) -> std::unique_ptr<FCarlaActor> { \
                  return MakeShared<className>(ActorId, Actor, std::move(Info), InState, World); \
            }); \
            return true; \
        }(); \
    }