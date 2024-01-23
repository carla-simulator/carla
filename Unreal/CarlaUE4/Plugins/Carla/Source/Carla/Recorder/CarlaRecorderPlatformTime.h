// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <sstream>
#include <chrono>

#pragma pack(push, 1)
struct CarlaRecorderPlatformTime
{

  std::chrono::time_point<std::chrono::system_clock>
      RecorderStartTime;

  double Time;

  void SetStartTime();
  void UpdateTime();

  void Read(std::istream &InFile);

  void Write(std::ostream &OutFile);

};
#pragma pack(pop)
