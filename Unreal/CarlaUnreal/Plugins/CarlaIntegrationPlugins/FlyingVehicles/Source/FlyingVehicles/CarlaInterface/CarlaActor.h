// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/CarlaActor.h"
#include "Carla/Actor/CarlaActorConstructorFactory.h"

class FMultirotorActor : public FCarlaActor
{
public:
  FMultirotorActor(
      IdType ActorId,
      AActor* Actor,
      TSharedPtr<const FActorInfo> Info,
      carla::rpc::ActorState InState,
      UWorld* World);

  static TSharedPtr<FMultirotorActor> CreateInstance(FCarlaActor::IdType ActorId, 
                AActor* Actor, 
                TSharedPtr<const FActorInfo> Info, 
                carla::rpc::ActorState InState, 
                UWorld* World);

  static void RegisterClassWithFactory();

  virtual ECarlaServerResponse GetMultirotorPhysicsControl(FMultirotorPhysicsControl& PhysicsControl) final;

  virtual ECarlaServerResponse ApplyMultirotorPhysicsControl(
      const FMultirotorPhysicsControl& PhysicsControl) final;

  virtual ECarlaServerResponse ApplyControlToMultirotor(
      const FMultirotorControl&, const EVehicleInputPriority&) final;

  virtual ECarlaServerResponse GetMultirotorControl(FMultirotorControl&) final;

  protected:
  static const FString CustomType;

};


