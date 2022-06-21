// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include <vector>

#pragma pack(push, 1)
struct CarlaRecorderAnimVehicle
{
  uint32_t DatabaseId;
  float Steering;
  float Throttle;
  float Brake;
  bool bHandbrake;
  int32_t Gear;

  void Read(std::ifstream &InFile);

  void Write(std::ofstream &OutFile);

};
#pragma pack(pop)

class CarlaRecorderAnimVehicles
{
public:

  void Add(const CarlaRecorderAnimVehicle &InObj);

  void Clear(void);

  void Write(std::ofstream &OutFile);

private:

  std::vector<CarlaRecorderAnimVehicle> Vehicles;
};
