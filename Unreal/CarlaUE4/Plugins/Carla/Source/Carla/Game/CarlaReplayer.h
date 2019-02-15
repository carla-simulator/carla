// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include <sstream>
#include <unordered_map>

#include <functional>
#include "CarlaRecorderInfo.h"
#include "CarlaRecorderFrames.h"
#include "CarlaRecorderEvent.h"
#include "CarlaRecorderPosition.h"
#include "CarlaRecorderState.h"
#include "CarlaRecorderHelpers.h"

// callback prototypes
typedef std::function<std::pair<int, uint32_t>(FVector Location, FVector Rotation,
    CarlaRecorderActorDescription Desc, uint32_t UId)> CarlaRecorderCallbackEventAdd;
typedef std::function<bool (uint32_t UId)> CarlaRecorderCallbackEventDel;
typedef std::function<bool (uint32_t ChildId, uint32_t ParentId)> CarlaRecorderCallbackEventParent;
typedef std::function<bool (CarlaRecorderPosition Pos1, CarlaRecorderPosition Pos2,
    double Per)> CarlaRecorderCallbackPosition;
typedef std::function<bool (bool ApplyAutopilot)> CarlaRecorderCallbackFinish;
typedef std::function<bool (CarlaRecorderStateTrafficLight State)> CarlaRecorderCallbackStateTrafficLight;

#pragma pack(push, 1)
struct OHeader
{
  char Id;
  uint32_t Size;
};
#pragma pack(pop)

class CarlaReplayer
{
public:

  CarlaReplayer();
  ~CarlaReplayer();

  std::string GetInfo(std::string Filename);

  std::string ReplayFile(std::string Filename, double TimeStart = 0.0f, double Duration = 0.0f);

  // void Start(void);
  void Stop(bool KeepActors = false);

  void Enable(void);

  void Disable(void);

  bool IsEnabled(void)
  {
    return Enabled;
  }

  // callbacks
  void SetCallbackEventAdd(CarlaRecorderCallbackEventAdd f);

  void SetCallbackEventDel(CarlaRecorderCallbackEventDel f);

  void SetCallbackEventParent(CarlaRecorderCallbackEventParent f);

  void SetCallbackEventPosition(CarlaRecorderCallbackPosition f);

  void SetCallbackEventFinish(CarlaRecorderCallbackFinish f);

  void SetCallbackStateTrafficLight(CarlaRecorderCallbackStateTrafficLight f);

  // tick for the replayer
  void Tick(float Time);

private:

  bool Enabled;
  // binary file reader
  std::ifstream File;
  OHeader Header;
  CarlaRecorderInfo RecInfo;
  CarlaRecorderFrame Frame;
  // callbacks
  CarlaRecorderCallbackEventAdd CallbackEventAdd;
  CarlaRecorderCallbackEventDel CallbackEventDel;
  CarlaRecorderCallbackEventParent CallbackEventParent;
  CarlaRecorderCallbackPosition CallbackPosition;
  CarlaRecorderCallbackFinish CallbackFinish;
  CarlaRecorderCallbackStateTrafficLight CallbackStateTrafficLight;
  // positions (to be able to interpolate)
  std::vector<CarlaRecorderPosition> CurrPos;
  std::vector<CarlaRecorderPosition> PrevPos;
  // mapping id
  std::unordered_map<uint32_t, uint32_t> MappedId;
  // times
  double CurrentTime;
  double TimeToStop;
  double TotalTime;

  // utils
  bool ReadHeader();

  void SkipPacket();

  double GetTotalTime(void);

  void Rewind(void);

  // processing packets
  void ProcessToTime(double Time);

  void ProcessEvents(void);

  void ProcessPositions(void);

  void ProcessStates(void);

  // positions
  void UpdatePositions(double Per);

  void InterpolatePosition(const CarlaRecorderPosition &Start, const CarlaRecorderPosition &End, double Per);

};
