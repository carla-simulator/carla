// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include <chrono>

#pragma pack(push, 1)
struct CarlaRecorderPlatformTime
{

  std::chrono::time_point<std::chrono::system_clock>
      RecorderStartTime;

  double Time;

  void SetStartTime();
  void UpdateTime();

  void Read(std::ifstream &InFile);

  void Write(std::ofstream &OutFile);

};
#pragma pack(pop)
