// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include "CarlaRecorderInfo.h"
#include "CarlaRecorderFrames.h"
#include "CarlaRecorderEventAdd.h"
#include "CarlaRecorderEventDel.h"
#include "CarlaRecorderEventParent.h"
#include "CarlaRecorderCollision.h"
#include "CarlaRecorderPosition.h"
#include "CarlaRecorderState.h"

class CarlaRecorderQuery
{

  #pragma pack(push, 1)
  struct Header
  {
    char Id;
    uint32_t Size;
  };
  #pragma pack(pop)

public:

  // get general info
  std::string QueryInfo(std::string Filename, bool bShowAll = false);
  // get info about collisions
  std::string QueryCollisions(std::string Filename, char Category1 = 'a', char Category2 = 'a');
  // get info about blocked actors
  std::string QueryBlocked(std::string Filename, double MinTime = 30, double MinDistance = 10);

private:

  std::ifstream File;
  Header Header;
  CarlaRecorderInfo RecInfo;
  CarlaRecorderFrame Frame;
  CarlaRecorderEventAdd EventAdd;
  CarlaRecorderEventDel EventDel;
  CarlaRecorderEventParent EventParent;
  CarlaRecorderPosition Position;
  CarlaRecorderCollision Collision;
  CarlaRecorderStateTrafficLight StateTraffic;

  // read next header packet
  bool ReadHeader(void);

  // skip current packet
  void SkipPacket(void);

  // read the start info structure and check the magic string
  bool CheckFileInfo(std::stringstream &Info);
};
