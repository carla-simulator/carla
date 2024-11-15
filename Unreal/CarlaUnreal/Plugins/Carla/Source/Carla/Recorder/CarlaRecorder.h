// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// #include "GameFramework/Actor.h"
#include <fstream>

#include "Carla/Actor/ActorDescription.h"
#include "CarlaRecorderTraficLightTime.h"
#include "CarlaRecorderPhysicsControl.h"
#include "CarlaRecorderPlatformTime.h"
#include "CarlaRecorderBoundingBox.h"
#include "CarlaRecorderKinematics.h"
#include "CarlaRecorderLightScene.h"
#include "CarlaRecorderLightVehicle.h"
#include "CarlaRecorderAnimVehicle.h"
#include "CarlaRecorderAnimVehicleWheels.h"
#include "CarlaRecorderAnimWalker.h"
#include "CarlaRecorderAnimBiker.h"
#include "CarlaRecorderCollision.h"
#include "CarlaRecorderEventAdd.h"
#include "CarlaRecorderEventDel.h"
#include "CarlaRecorderEventParent.h"
#include "CarlaRecorderFrames.h"
#include "CarlaRecorderInfo.h"
#include "CarlaRecorderPosition.h"
#include "CarlaRecorderQuery.h"
#include "CarlaRecorderState.h"
#include "CarlaRecorderVisualTime.h"
#include "CarlaRecorderWalkerBones.h"
#include "CarlaRecorderDoorVehicle.h"
#include "CarlaReplayer.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"

#include "CarlaRecorder.generated.h"

class AActor;
class UCarlaEpisode;
class ACarlaWheeledVehicle;
class UCarlaLight;
class ATrafficSignBase;
class ATrafficLightBase;

enum class CarlaRecorderPacketId : uint8_t
{
  FrameStart = 0,
  FrameEnd,
  EventAdd,
  EventDel,
  EventParent,
  Collision,
  Position,
  State,
  AnimVehicle,
  AnimWalker,
  VehicleLight,
  SceneLight,
  Kinematics,
  BoundingBox,
  PlatformTime,
  PhysicsControl,
  TrafficLightTime,
  TriggerVolume,
  FrameCounter,
  WalkerBones,
  VisualTime,
  VehicleDoor,
  AnimVehicleWheels,
  AnimBiker
};

/// Recorder for the simulation
UCLASS()
class CARLA_API ACarlaRecorder : public AActor
{
  GENERATED_BODY()

public:

  ACarlaRecorder(void);
  ACarlaRecorder(const FObjectInitializer &InObjectInitializer);

  // enable / disable
  bool IsEnabled(void)
  {
    return Enabled;
  }
  void Enable(void);

  void Disable(void);

  // start / stop
  std::string Start(std::string Name, FString MapName, bool AdditionalData = false);

  void Stop(void);

  void Clear(void);

  void Write(double DeltaSeconds);

  // events
  void AddEvent(const CarlaRecorderEventAdd &Event);

  void AddEvent(const CarlaRecorderEventDel &Event);

  void AddEvent(const CarlaRecorderEventParent &Event);

  void AddCollision(AActor *Actor1, AActor *Actor2);

  void AddPosition(const CarlaRecorderPosition &Position);

  void AddState(const CarlaRecorderStateTrafficLight &State);

  void AddAnimVehicle(const CarlaRecorderAnimVehicle &Vehicle);

  void AddAnimVehicleWheels(const CarlaRecorderAnimWheels &VehicleWheels);

  void AddAnimWalker(const CarlaRecorderAnimWalker &Walker);

  void AddAnimBiker(const CarlaRecorderAnimBiker &Biker);

  void AddLightVehicle(const CarlaRecorderLightVehicle &LightVehicle);

  void AddEventLightSceneChanged(const UCarlaLight* Light);

  void AddKinematics(const CarlaRecorderKinematics &ActorKinematics);

  void AddBoundingBox(const CarlaRecorderActorBoundingBox &ActorBoundingBox);

  void AddTriggerVolume(const ATrafficSignBase &TrafficSign);

  void AddPhysicsControl(const ACarlaWheeledVehicle& Vehicle);

  void AddTrafficLightTime(const ATrafficLightBase& TrafficLight);

  void AddActorBones(FCarlaActor *CarlaActor);

  void AddVehicleDoor(const ACarlaWheeledVehicle& Vehicle, const EVehicleDoor SDoors, bool bIsOpen);

  void AddDoorVehicle(const CarlaRecorderDoorVehicle &DoorVehicle);

  // set episode
  void SetEpisode(UCarlaEpisode *ThisEpisode)
  {
    Episode = ThisEpisode;
    Replayer.SetEpisode(ThisEpisode);
  }

  void CreateRecorderEventAdd(
      uint32_t DatabaseId,
      uint8_t Type,
      const FTransform &Transform,
      FActorDescription ActorDescription);

  // replayer
  CarlaReplayer *GetReplayer(void)
  {
    return &Replayer;
  }

  // queries
  std::string ShowFileInfo(std::string Name, bool bShowAll = false);
  std::string ShowFileCollisions(std::string Name, char Type1, char Type2);
  std::string ShowFileActorsBlocked(std::string Name, double MinTime = 30, double MinDistance = 10);

  // replayer
  std::string ReplayFile(std::string Name, double TimeStart, double Duration,
      uint32_t FollowId, bool ReplaySensors);
  void SetReplayerTimeFactor(double TimeFactor);
  void SetReplayerIgnoreHero(bool IgnoreHero);
  void SetReplayerIgnoreSpectator(bool IgnoreSpectator);
  void StopReplayer(bool KeepActors = false);

  void Ticking(float DeltaSeconds);

private:

  bool Enabled;   // enabled or not

  // enabling this records additional data (kinematics, bounding boxes, etc)
  bool bAdditionalData = false;

  uint32_t NextCollisionId = 0;

  // files
  std::ofstream File;

  UCarlaEpisode *Episode = nullptr;

  // structures
  CarlaRecorderInfo Info;
  CarlaRecorderFrames Frames;
  CarlaRecorderEventsAdd EventsAdd;
  CarlaRecorderEventsDel EventsDel;
  CarlaRecorderEventsParent EventsParent;
  CarlaRecorderCollisions Collisions;
  CarlaRecorderPositions Positions;
  CarlaRecorderStates States;
  CarlaRecorderAnimVehicles Vehicles;
  CarlaRecorderAnimVehicleWheels Wheels;
  CarlaRecorderAnimWalkers Walkers;
  CarlaRecorderAnimBikers Bikers;
  CarlaRecorderLightVehicles LightVehicles;
  CarlaRecorderLightScenes LightScenes;
  CarlaRecorderActorsKinematics Kinematics;
  CarlaRecorderActorBoundingBoxes BoundingBoxes;
  CarlaRecorderActorTriggerVolumes TriggerVolumes;
  CarlaRecorderPlatformTime PlatformTime;
  CarlaRecorderPhysicsControls PhysicsControls;
  CarlaRecorderTrafficLightTimes TrafficLightTimes;
  CarlaRecorderWalkersBones WalkersBones;
  CarlaRecorderVisualTime VisualTime;
  CarlaRecorderDoorVehicles DoorVehicles;

  // replayer
  CarlaReplayer Replayer;

  // query tools
  CarlaRecorderQuery Query;

  void AddExistingActors(void);
  void AddActorPosition(FCarlaActor *CarlaActor);
  void AddWalkerAnimation(FCarlaActor *CarlaActor);
  void AddBikerAnimation(FCarlaActor *CarlaActor);
  void AddVehicleAnimation(FCarlaActor *CarlaActor);
  void AddVehicleWheelsAnimation(FCarlaActor *CarlaActor);
  void AddTrafficLightState(FCarlaActor *CarlaActor);
  void AddVehicleLight(FCarlaActor *CarlaActor);
  void AddActorKinematics(FCarlaActor *CarlaActor);
  void AddActorBoundingBox(FCarlaActor *CarlaActor);
};
