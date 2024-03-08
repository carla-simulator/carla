// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <sstream>
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

  void Read(std::istream &InFile);

  void Write(std::ostream &OutFile);

};
#pragma pack(pop)

class CarlaRecorderAnimVehicles
{
public:

  void Add(const CarlaRecorderAnimVehicle &InObj);

  void Clear(void);

  void Write(std::ostream &OutFile);

  void Read(std::istream &InFile);

  const std::vector<CarlaRecorderAnimVehicle>& GetVehicles();
private:

  std::vector<CarlaRecorderAnimVehicle> Vehicles;
};
