// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

class UCarlaEpisode;
class FActorView;
struct FActorDescription;

class CarlaReplayerHelper
{

public:

  // set the episode to use
  void SetEpisode(UCarlaEpisode *ThisEpisode)
  {
    Episode = ThisEpisode;
  }

  // replay event for creating actor
  std::pair<int, uint32_t> ProcessReplayerEventAdd(
      FVector Location,
      FVector Rotation,
      CarlaRecorderActorDescription Description,
      uint32_t DesiredId,
      bool bIgnoreHero);

  // replay event for removing actor
  bool ProcessReplayerEventDel(uint32_t DatabaseId);

  // replay event for parenting actors
  bool ProcessReplayerEventParent(uint32_t ChildId, uint32_t ParentId);

  // reposition actors
  bool ProcessReplayerPosition(CarlaRecorderPosition Pos1, CarlaRecorderPosition Pos2, double Per, double DeltaTime);

  // replay event for traffic light state
  bool ProcessReplayerStateTrafficLight(CarlaRecorderStateTrafficLight State);

  // set the animation for Vehicles
  void ProcessReplayerAnimVehicle(CarlaRecorderAnimVehicle Vehicle);

  // set the animation for walkers
  void ProcessReplayerAnimWalker(CarlaRecorderAnimWalker Walker);

  // set the vehicle light
  void ProcessReplayerLightVehicle(CarlaRecorderLightVehicle LightVehicle);

  // set scene lights
  void ProcessReplayerLightScene(CarlaRecorderLightScene LightScene);

  // replay finish
  bool ProcessReplayerFinish(bool bApplyAutopilot, bool bIgnoreHero, std::unordered_map<uint32_t, bool> &IsHero);

  // set the camera position to follow an actor
  bool SetCameraPosition(uint32_t Id, FVector Offset, FQuat Rotation);

  // set the velocity of the actor
  void SetActorVelocity(const FActorView &ActorView, FVector Velocity);

  // set the animation speed for walkers
  void SetWalkerSpeed(uint32_t ActorId, float Speed);

  void RemoveStaticProps();

private:

  UCarlaEpisode *Episode {nullptr};

  std::pair<int, FActorView>TryToCreateReplayerActor(
    FVector &Location,
    FVector &Rotation,
    FActorDescription &ActorDesc,
    uint32_t DesiredId);

  AActor *FindTrafficLightAt(FVector Location);

  // enable / disable physics for an actor
  bool SetActorSimulatePhysics(const FActorView &ActorView, bool bEnabled);
  // enable / disable autopilot for an actor
  bool SetActorAutopilot(const FActorView &ActorView, bool bEnabled, bool bKeepState = false);
};
