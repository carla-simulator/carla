// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <sstream>
#include <vector>

#pragma pack(push, 1)
struct CarlaRecorderTrafficLightTime
{
  uint32_t DatabaseId;
  float GreenTime = 0;
  float YellowTime = 0;
  float RedTime = 0;

  void Read(std::istream &InFile);

  void Write(std::ostream &OutFile);
};
#pragma pack(pop)

class CarlaRecorderTrafficLightTimes
{
  public:

  void Add(const CarlaRecorderTrafficLightTime &InObj);

  void Clear(void);

  void Write(std::ostream &OutFile);

private:

  std::vector<CarlaRecorderTrafficLightTime> TrafficLightTimes;
};
