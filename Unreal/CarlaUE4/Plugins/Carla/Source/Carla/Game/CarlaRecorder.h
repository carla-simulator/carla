// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// #include "GameFramework/Actor.h"
#include <fstream>
#include "Carla/Game/CarlaRecorderInfo.h"
#include "Carla/Game/CarlaRecorderFrames.h"
#include "Carla/Game/CarlaRecorderEvent.h"
#include "Carla/Game/CarlaRecorderPosition.h"
#include "Carla/Game/CarlaRecorderState.h"
#include "Carla/Game/CarlaReplayer.h"
#include "Carla/Actor/ActorDescription.h"

#include "CarlaRecorder.generated.h"

class AActor;
class UCarlaEpisode;

enum class CarlaRecorderPacketId : uint8_t
{
  Frame = 0,
  Event,
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

  void Write(void);

  // events
  void AddEvent(const CarlaRecorderEventAdd &Event);

  void AddEvent(const CarlaRecorderEventDel &Event);

  void AddEvent(const CarlaRecorderEventParent &Event);

  void AddPosition(const CarlaRecorderPosition &Position);

  void AddState(const CarlaRecorderStateTrafficLight &State);

  // set episode
  void SetEpisode(UCarlaEpisode *ThisEpisode)
  {
    Episode = ThisEpisode;
  }

  void CreateRecorderEventAdd(
      unsigned int DatabaseId,
      const FTransform &Transform,
      FActorDescription ActorDescription);

  // replayer
  CarlaReplayer *GetReplayer(void)
  {
    return &Replayer;
  }

  std::string ShowFileInfo(std::string Path, std::string Name)
  {
    return Replayer.GetInfo(Path + Name);
  }
  std::string ReplayFile(std::string Path, std::string Name, double TimeStart, double Duration)
  {
    Stop();
    return Replayer.ReplayFile(Path + Name, TimeStart, Duration);
  }

// protected:

  void Tick(float DeltaSeconds) final;

private:

  bool Enabled;   // enabled or not

  // files
  std::ofstream File;
  std::ofstream Log;

  UCarlaEpisode *Episode = nullptr;

  // structures
  CarlaRecorderInfo Info;
  CarlaRecorderFrames Frames;
  CarlaRecorderEvents Events;
  CarlaRecorderPositions Positions;
  CarlaRecorderStates States;

  // replayer
  CarlaReplayer Replayer;

  void AddExistingActors(void);

};
