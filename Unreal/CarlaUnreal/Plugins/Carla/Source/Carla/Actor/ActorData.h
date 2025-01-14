// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Vehicle/AckermannControllerSettings.h"
#include "Carla/Vehicle/VehicleAckermannControl.h"
#include "Carla/Vehicle/VehicleControl.h"
#include "Carla/Vehicle/VehicleLightState.h"
#include "Carla/Vehicle/VehicleInputPriority.h"
#include "Carla/Vehicle/VehiclePhysicsControl.h"
#include "Carla/Sensor/DataStream.h"
#include "Carla/Traffic/TrafficLightState.h"
#include "Carla/Actor/ActorInfo.h"
#include "Carla/Math/DVector.h"

#include <util/disable-ue4-macros.h>
#include <carla/rpc/WalkerControl.h>
#include <util/enable-ue4-macros.h>

class UCarlaEpisode;
class UTrafficLightController;
class FCarlaActor;

class FActorData
{
public:

  FDVector Location;

  FQuat Rotation;

  FVector Scale;

  FVector Velocity;

  FVector AngularVelocity = FVector(0,0,0);

  bool bSimulatePhysics = false;

  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode);

  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode);

  virtual AActor* RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info);

  FTransform GetLocalTransform(UCarlaEpisode* CarlaEpisode) const;

  virtual ~FActorData(){};
};

class FVehicleData : public FActorData
{
public:

  FVehiclePhysicsControl PhysicsControl;

  FVehicleControl Control;

  FVehicleAckermannControl AckermannControl;

  bool bAckermannControlActive = false;

  FAckermannControllerSettings AckermannControllerSettings;

  FVehicleLightState LightState;

  float SpeedLimit = 30;

  carla::rpc::VehicleFailureState FailureState;

  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;

  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;
};

class FWalkerData : public FActorData
{
public:

  carla::rpc::WalkerControl WalkerControl;

  bool bAlive = true;

  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;

  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;
};

class FTrafficSignData : public FActorData
{
public:
  FString SignId;

  TSubclassOf<AActor> Model;

  TSubclassOf<UObject> SignModel;

  virtual AActor* RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info) override;

  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;

  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;
};

class FTrafficLightData : public FActorData
{
public:

  UTrafficLightController* Controller;

  ETrafficLightState LightState;

  FString SignId;

  TSubclassOf<AActor> Model;

  int PoleIndex;

  virtual AActor* RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info) override;

  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;

  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;

};

class FActorSensorData : public FActorData
{
public:

  FDataStream Stream;

  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;

  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;
};
