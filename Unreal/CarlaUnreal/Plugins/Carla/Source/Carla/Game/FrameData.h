// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Recorder/CarlaRecorderTraficLightTime.h"
#include "Carla/Recorder/CarlaRecorderPhysicsControl.h"
#include "Carla/Recorder/CarlaRecorderPlatformTime.h"
#include "Carla/Recorder/CarlaRecorderBoundingBox.h"
#include "Carla/Recorder/CarlaRecorderKinematics.h"
#include "Carla/Recorder/CarlaRecorderLightScene.h"
#include "Carla/Recorder/CarlaRecorderLightVehicle.h"
#include "Carla/Recorder/CarlaRecorderAnimVehicle.h"
#include "Carla/Recorder/CarlaRecorderAnimWalker.h"
#include "Carla/Recorder/CarlaRecorderAnimBiker.h"
#include "Carla/Recorder/CarlaRecorderCollision.h"
#include "Carla/Recorder/CarlaRecorderEventAdd.h"
#include "Carla/Recorder/CarlaRecorderEventDel.h"
#include "Carla/Recorder/CarlaRecorderEventParent.h"
#include "Carla/Recorder/CarlaRecorderFrames.h"
#include "Carla/Recorder/CarlaRecorderInfo.h"
#include "Carla/Recorder/CarlaRecorderPosition.h"
#include "Carla/Recorder/CarlaRecorderFrameCounter.h"
#include "Carla/Recorder/CarlaRecorderState.h"
#include "Carla/Recorder/CarlaRecorderWeather.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Lights/CarlaLight.h"
#include "Carla/Traffic/TrafficLightBase.h"
#include "Carla/Traffic/TrafficSignBase.h"
#include "Carla/Settings/EpisodeSettings.h"

#include <util/ue-header-guard-begin.h>
#include "Templates/Function.h"
#include <util/ue-header-guard-end.h>

#include <util/disable-ue4-macros.h>
#include "carla/multigpu/frameDataCodec.h"
#include "carla/multigpu/frameTime.h"
#include "carla/multigpu/mirrorPackets.h"
#include "carla/multigpu/sensorSync.h"
#include "carla/multigpu/worldState.h"
#include <util/enable-ue4-macros.h>

#include <optional>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

class UCarlaEpisode;
class FCarlaActor;

/// What a Multi-GPU secondary keeps between the primary frames it plays.
struct FMirrorState
{
  /// Local ids of the mirrors this secondary spawned, as opposed to map actors
  /// it reused (traffic lights, the spectator): only these may be swept.
  std::unordered_set<uint32_t> CreatedIds;

  /// Attachments not replayed yet, as (child, parent) primary ids: one of the
  /// two is not mirrored yet.
  std::vector<std::pair<uint32_t, uint32_t>> PendingParents;

  /// The poses the last frame played set, by local id, to check before the
  /// capture that nothing moved a mirror in between.
  std::vector<std::pair<uint32_t, FTransform>> PlayedPoses;

  /// The environment objects the primary has disabled (sorted), once a frame
  /// said so, and the registration of this server's objects they were last
  /// applied to: one in a map layer still streaming in is disabled once it
  /// is registered.
  std::optional<std::vector<uint64_t>> DisabledObjects;
  uint64 DisabledObjectsRegistrations = 0u;

  void Reset()
  {
    CreatedIds.clear();
    PendingParents.clear();
    PlayedPoses.clear();
    DisabledObjects.reset();
    DisabledObjectsRegistrations = 0u;
  }
};

/// An actor's appearance, as a Multi-GPU primary last sent it.
struct FMirroredAppearance
{
  bool bCollisions = true;
  bool bHidden = false;
  FVector Scale = FVector::OneVector;

  bool operator==(const FMirroredAppearance &Other) const
  {
    return (bCollisions == Other.bCollisions) && (bHidden == Other.bHidden) &&
        Scale.Equals(Other.Scale, 0.0);
  }
};

/// What a Multi-GPU primary remembers between the frames it sends, to send
/// the world state only when it changes.
struct FPrimaryWorldState
{
  /// AWeather::GetApplySerial() of the weather last sent.
  uint64 WeatherSerial = 0u;

  /// ACarlaGameModeBase::GetEnvironmentObjectsSerial() of the set last sent.
  uint64 EnvironmentObjectsSerial = 0u;

  /// The scene lights changed since the last frame sent, by light id.
  TSet<int32> ChangedLights;

  /// The appearance last sent, by actor id.
  std::unordered_map<uint32_t, FMirroredAppearance> Appearances;
};

class FFrameData
{
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
  CarlaRecorderFrameCounter FrameCounter;
  // Multi-GPU: the weather applied in this frame.
  CarlaRecorderWeathers Weathers;
  // Multi-GPU only: the primary's clocks and settings for this frame.
  std::optional<carla::multigpu::FrameTime> MultiGpuFrameTime;
  // Multi-GPU only: every live actor's pose, rotations as quaternions.
  carla::multigpu::MirroredPositions MirrorPositions;
  // Multi-GPU only: the wheels and open doors of every vehicle.
  carla::multigpu::MirroredVehicleParts MirrorVehicleParts;
  // Multi-GPU only: the sensors that tick in this frame, and the state they
  // start from.
  carla::multigpu::SensorTicks MirrorSensorTicks;
  carla::multigpu::SensorStates MirrorSensorStates;
  bool bHasSensorTicks = false;
  // Multi-GPU only: world-wide state, map objects and actor appearance.
  std::optional<carla::multigpu::WorldGlobals> MirrorWorldGlobals;
  carla::multigpu::WorldObjects MirrorWorldObjects;
  carla::multigpu::ActorAppearances MirrorAppearances;

public:

  void SetEpisode(UCarlaEpisode* ThisEpisode) {Episode = ThisEpisode;}

  void GetFrameData(UCarlaEpisode *ThisEpisode, bool bAdditionalData = false, bool bIncludeActorsAgain = false);

  /// @a OnActorAdded receives (primary actor id, local actor id, whether it
  /// was just created rather than reused) for every actor created or reused;
  /// @a OnActorRemoved the primary id of every actor destroyed.
  void PlayFrameData(
      UCarlaEpisode *ThisEpisode,
      std::unordered_map<uint32_t, uint32_t>& MappedId,
      FMirrorState &MirrorState,
      TFunctionRef<void(uint32_t, uint32_t, bool)> OnActorAdded,
      TFunctionRef<void(uint32_t)> OnActorRemoved);

  void Clear();

  void Write(std::ostream& OutStream);

  /// Parses one SEND_FRAME payload. Only a complete frame
  /// (FrameDataReadResult::IsComplete) may be played.
  carla::multigpu::FrameDataReadResult Read(const uint8_t *Data, size_t Size);

  uint64_t GetFrameNumber() const { return FrameCounter.FrameCounter; }

  /// Whether the frame says which sensors tick (only older builds do not).
  bool HasSensorTicks() const { return bHasSensorTicks; }

  void SetFrameTime(const carla::multigpu::FrameTime &Time) { MultiGpuFrameTime = Time; }

  /// The primary's clocks and settings of this frame; null for a frame from
  /// an older primary.
  const carla::multigpu::FrameTime *GetFrameTime() const
  {
    return MultiGpuFrameTime ? &*MultiGpuFrameTime : nullptr;
  }

  // record functions
  void CreateRecorderEventAdd(
      uint32_t DatabaseId,
      uint8_t Type,
      const FTransform &Transform,
      FActorDescription ActorDescription,
      bool bAddOtherRelatedInfo = true);
  void AddEvent(const CarlaRecorderEventAdd &Event);
  void AddEvent(const CarlaRecorderEventDel &Event);
  void AddEvent(const CarlaRecorderEventParent &Event);

private:
  void AddCollision(AActor *Actor1, AActor *Actor2);
  void AddPosition(const CarlaRecorderPosition &Position);
  void AddState(const CarlaRecorderStateTrafficLight &State);
  void AddAnimVehicle(const CarlaRecorderAnimVehicle &Vehicle);
  void AddAnimWalker(const CarlaRecorderAnimWalker &Walker);
  void AddAnimBiker(const CarlaRecorderAnimBiker &Biker);
  void AddLightVehicle(const CarlaRecorderLightVehicle &LightVehicle);
  void AddEventLightSceneChanged(const UCarlaLight* Light);
  void AddKinematics(const CarlaRecorderKinematics &ActorKinematics);
  void AddBoundingBox(const CarlaRecorderActorBoundingBox &ActorBoundingBox);
  void AddTriggerVolume(const ATrafficSignBase &TrafficSign);
  void AddPhysicsControl(const ACarlaWheeledVehicle& Vehicle);
  void AddTrafficLightTime(const ATrafficLightBase& TrafficLight);

  void AddActorPosition(FCarlaActor *CarlaActor);
  void AddWalkerAnimation(FCarlaActor *CarlaActor);
  void AddBikerAnimation(FCarlaActor *CarlaActor);
  void AddVehicleAnimation(FCarlaActor *CarlaActor);
  // packet 32 and, for two-wheeled vehicles, packet 22
  void AddVehicleParts(FCarlaActor *CarlaActor);
  void AddTrafficLightState(FCarlaActor *CarlaActor);
  void AddVehicleLight(FCarlaActor *CarlaActor);
  void AddActorKinematics(FCarlaActor *CarlaActor);
  void AddActorBoundingBox(FCarlaActor *CarlaActor);
  void AddSensorTick(FCarlaActor *CarlaActor);
  // packet 30 for an actor whose appearance changed since the last frame
  void AddActorAppearance(FCarlaActor *CarlaActor, bool bFullResync);
  // the weather, packets 28 and 29, and the scene lights that changed
  void AddWorldState(bool bFullResync);

  void GetFrameCounter();

  std::pair<int, FCarlaActor*> CreateOrReuseActor(
      FVector &Location,
      FVector &Rotation,
      FActorDescription &ActorDesc,
      uint32_t DesiredId,
      bool SpawnSensors,
      std::unordered_map<uint32_t, uint32_t>& MappedId);

    // replay event for creating actor
  std::pair<int, uint32_t> ProcessReplayerEventAdd(
      FVector Location,
      FVector Rotation,
      CarlaRecorderActorDescription Description,
      uint32_t DesiredId,
      bool bIgnoreHero,
      bool ReplaySensors,
      std::unordered_map<uint32_t, uint32_t>& MappedId);

  // replay event for removing actor
  bool ProcessReplayerEventDel(uint32_t DatabaseId);
  // replay event for parenting actors, by primary ids; false while one of
  // the two is not mirrored yet
  bool ProcessReplayerEventParent(
      uint32_t ChildId,
      uint32_t ParentId,
      const std::unordered_map<uint32_t, uint32_t>& MappedId);
  // a mirror never moves or acts on its own: the primary's frames move it
  void PrepareMirroredActor(FCarlaActor *CarlaActor, uint32_t PrimaryActorId);
  // apply packet 34, parents before their children
  void PlayPositions(const std::unordered_map<uint32_t, uint32_t>& MappedId, FMirrorState &MirrorState);
  // move a mirror to the primary's pose, keeping its own scale
  void PlaceMirror(FCarlaActor &CarlaActor, FTransform Transform);
  // apply packet 12: the primary's velocities of every moving mirror
  void PlayKinematics(const std::unordered_map<uint32_t, uint32_t>& MappedId);
  // apply packet 32, once the vehicles are in place
  void PlayVehicleParts(const std::unordered_map<uint32_t, uint32_t>& MappedId);
  // load and unload the map layers the primary did
  void PlayMapLayers();
  // apply packet 30
  void PlayAppearances(const std::unordered_map<uint32_t, uint32_t>& MappedId);
  // apply the weather and packets 28 and 29
  void PlayWorldState(FMirrorState &MirrorState);
  // reposition actors
  bool ProcessReplayerPosition(CarlaRecorderPosition Pos1, 
                               CarlaRecorderPosition Pos2, double Per, double DeltaTime);
  // replay event for traffic light state
  bool ProcessReplayerStateTrafficLight(CarlaRecorderStateTrafficLight State);
  // set the animation for Vehicles
  void ProcessReplayerAnimVehicle(CarlaRecorderAnimVehicle Vehicle);
  // set the animation for walkers
  void ProcessReplayerAnimWalker(CarlaRecorderAnimWalker Walker);
  // set the animation for bikers
  void ProcessReplayerAnimBiker(CarlaRecorderAnimBiker Biker);
  // set the vehicle light
  void ProcessReplayerLightVehicle(CarlaRecorderLightVehicle LightVehicle);
  // set scene lights
  void ProcessReplayerLightScene(CarlaRecorderLightScene LightScene);
  // replay finish
  bool ProcessReplayerFinish(bool bApplyAutopilot, bool bIgnoreHero, 
                             std::unordered_map<uint32_t, bool> &IsHero);
  // set the camera position to follow an actor
  bool SetCameraPosition(uint32_t Id, FVector Offset, FQuat Rotation);
  // set the velocity of the actor
  void SetActorVelocity(FCarlaActor *CarlaActor, FVector Velocity);
  // set the animation speed for walkers
  void SetWalkerSpeed(uint32_t ActorId, float Speed);
  // enable / disable physics for an actor
  bool SetActorSimulatePhysics(FCarlaActor *CarlaActor, bool bEnabled);

  void SetFrameCounter();

  FCarlaActor* FindTrafficSignAt(FVector Location);

  void AddExistingActors(void);

  UCarlaEpisode *Episode = nullptr;
};

/// Copies the episode settings into a FrameTime and back. The secondary keeps
/// its own no-rendering mode.
void FrameTimeFromSettings(const FEpisodeSettings &Settings, carla::multigpu::FrameTime &Time);
void SettingsFromFrameTime(const carla::multigpu::FrameTime &Time, FEpisodeSettings &Settings);
