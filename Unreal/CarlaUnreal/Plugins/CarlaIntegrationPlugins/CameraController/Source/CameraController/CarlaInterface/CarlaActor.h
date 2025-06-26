// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/CarlaActor.h"
#include "Carla/Actor/CarlaActorConstructorFactory.h"

class FCameraControllerActor : public FCarlaActor
{
public:
  FCameraControllerActor(
      IdType ActorId,
      AActor* Actor,
      TSharedPtr<const FActorInfo> Info,
      carla::rpc::ActorState InState,
      UWorld* World);

  static TSharedPtr<FCameraControllerActor> CreateInstance(FCarlaActor::IdType ActorId, 
                AActor* Actor,
                TSharedPtr<const FActorInfo> Info,
                carla::rpc::ActorState InState,
                UWorld* World);

  static void RegisterClassWithFactory();

  protected:
  static const FString CustomType;

};
