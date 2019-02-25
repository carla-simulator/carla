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
#include "CarlaRecorderEventAdd.h"
#include "CarlaRecorderEventDel.h"
#include "CarlaRecorderEventParent.h"
#include "CarlaRecorderCollision.h"
#include "CarlaRecorderPosition.h"
#include "CarlaRecorderState.h"
#include "CarlaRecorderHelpers.h"
#include "CarlaReplayerHelper.h"

class UCarlaEpisode;

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

  CarlaReplayer() {};
  ~CarlaReplayer() { Stop(); };

  std::string GetInfo(std::string Filename);
  std::string GetInfoCollisions(std::string Filename, char Category1 = 'a', char Category2 = 'a');
  std::string GetInfoActorsBlocked(std::string Filename, double MinTime = 30, double MinDistance = 10);
  std::string ReplayFile(std::string Filename, double TimeStart = 0.0f, double Duration = 0.0f, uint32_t FollowId = 0);

  // void Start(void);
  void Stop(bool KeepActors = false);

  void Enable(void);

  void Disable(void);

  bool IsEnabled(void)
  {
    return Enabled;
  }

  // set episode
  void SetEpisode(UCarlaEpisode *ThisEpisode)
  {
    Episode = ThisEpisode;
    Helper.SetEpisode(ThisEpisode);
  }

  // tick for the replayer
  void Tick(float Time);

private:

  bool Enabled;
  UCarlaEpisode *Episode = nullptr;
  // binary file reader
  std::ifstream File;
  OHeader Header;
  CarlaRecorderInfo RecInfo;
  CarlaRecorderFrame Frame;
  // positions (to be able to interpolate)
  std::vector<CarlaRecorderPosition> CurrPos;
  std::vector<CarlaRecorderPosition> PrevPos;
  // mapping id
  std::unordered_map<uint32_t, uint32_t> MappedId;
  // times
  double CurrentTime;
  double TimeToStop;
  double TotalTime;
  // helper
  CarlaReplayerHelper Helper;
  // follow camera
  uint32_t FollowId;

  // utils
  bool ReadHeader();

  void SkipPacket();

  double GetTotalTime(void);

  void Rewind(void);

  // processing packets
  void ProcessToTime(double Time);

  void ProcessEventsAdd(void);
  void ProcessEventsDel(void);
  void ProcessEventsParent(void);

  void ProcessPositions(void);

  void ProcessStates(void);

  // positions
  void UpdatePositions(double Per);

  void InterpolatePosition(const CarlaRecorderPosition &Start, const CarlaRecorderPosition &End, double Per);

};
