// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// #include "GameFramework/Actor.h"
#include <fstream>
#include "CarlaRecorderInfo.h"
#include "CarlaRecorderFrames.h"
#include "CarlaRecorderEventAdd.h"
#include "CarlaRecorderEventDel.h"
#include "CarlaRecorderEventParent.h"
#include "CarlaRecorderCollision.h"
#include "CarlaRecorderPosition.h"
#include "CarlaRecorderState.h"
#include "CarlaRecorderQuery.h"
#include "CarlaReplayer.h"
#include "Carla/Actor/ActorDescription.h"

#include "CarlaRecorder.generated.h"

class AActor;
class UCarlaEpisode;

enum class CarlaRecorderPacketId : uint8_t
{
  FrameStart = 0,
  FrameEnd,
  EventAdd,
  EventDel,
  EventParent,
  Collision,
  Position,
  State
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
  std::string Start(FString Path, FString Name, FString MapName);

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

  // forwarded to replayer
  std::string ShowFileInfo(std::string Path, std::string Name);
  std::string ShowFileCollisions(std::string Path, std::string Name, char Type1, char Type2);
  std::string ShowFileActorsBlocked(std::string Path, std::string Name, double MinTime = 30, double MinDistance = 10);
  std::string ReplayFile(std::string Path, std::string Name, double TimeStart, double Duration, uint32_t FollowId);

  void Tick(float DeltaSeconds) final;

private:

  bool Enabled;   // enabled or not

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

  // replayer
  CarlaReplayer Replayer;

  // query tools
  CarlaRecorderQuery Query;

  void AddExistingActors(void);

};
