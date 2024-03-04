// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include <chrono>

#pragma pack(push, 1)
struct CarlaRecorderVisualTime
{
  double Time;

  void SetTime(double ThisTime);

  void Read(std::ifstream &InFile);

  void Write(std::ofstream &OutFile);

};
#pragma pack(pop)
