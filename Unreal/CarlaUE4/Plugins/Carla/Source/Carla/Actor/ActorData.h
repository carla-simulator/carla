// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Math/DVector.h"
#include "Carla/Vehicle/VehicleControl.h"
#include "Carla/Vehicle/VehicleLightState.h"
#include "Vehicle/VehicleInputPriority.h"
#include "Vehicle/VehiclePhysicsControl.h"
#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/WalkerControl.h>
#include <compiler/enable-ue4-macros.h>

class UCarlaEpisode;
class UTrafficLightController;

class FActorData
{
public:

  FDVector Location;

  FQuat Rotation;

  FVector Scale;

  FVector Velocity;

  FVector AngularVelocity = FVector(0,0,0);

  bool bSimulatePhysics = false;

  virtual void RecordActorData(AActor* Actor, UCarlaEpisode* CarlaEpisode);

  virtual void RestoreActorData(AActor* Actor, UCarlaEpisode* CarlaEpisode);

  virtual AActor* RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info);

  FTransform GetLocalTransform(UCarlaEpisode* CarlaEpisode) const;

  virtual ~FActorData(){};
};

class FVehicleData : public FActorData
{
public:

  FVehiclePhysicsControl PhysicsControl;

  FVehicleControl Control;

  FVehicleLightState LightState;

  virtual void RecordActorData(AActor* Actor, UCarlaEpisode* CarlaEpisode) override;

  virtual void RestoreActorData(AActor* Actor, UCarlaEpisode* CarlaEpisode) override;
};

class FWalkerData : public FActorData
{
public:

  carla::rpc::WalkerControl WalkerControl;

  bool bAlive = true;

  virtual void RecordActorData(AActor* Actor, UCarlaEpisode* CarlaEpisode) override;

  virtual void RestoreActorData(AActor* Actor, UCarlaEpisode* CarlaEpisode) override;
};

class FTrafficSignData : public FActorData
{
public:
  FString SignId;

  TSubclassOf<AActor> Model;

  TSubclassOf<UObject> SignModel;

  virtual AActor* RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info) override;

  virtual void RecordActorData(AActor* Actor, UCarlaEpisode* CarlaEpisode) override;

  virtual void RestoreActorData(AActor* Actor, UCarlaEpisode* CarlaEpisode) override;
};

class FTrafficLightData : public FActorData
{
public:

  UTrafficLightController* Controller;

  FString SignId;

  TSubclassOf<AActor> Model;

  int PoleIndex;

  virtual AActor* RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info) override;

  virtual void RecordActorData(AActor* Actor, UCarlaEpisode* CarlaEpisode) override;

  virtual void RestoreActorData(AActor* Actor, UCarlaEpisode* CarlaEpisode) override;

};
