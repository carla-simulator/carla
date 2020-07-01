// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include <vector>
#include <type_traits>

#pragma pack(push, 1)
struct CarlaRecorderLightVehicle
{
  // Use same type as carla::rpc::VehicleLightState::LightState
  using VehicleLightStateType = uint32_t;

  uint32_t DatabaseId;
  VehicleLightStateType State;

  void Read(std::ifstream &InFile);

  void Write(std::ofstream &OutFile);
};
#pragma pack(pop)

struct CarlaRecorderLightVehicles
{
public:

  void Add(const CarlaRecorderLightVehicle &InObj);

  void Clear(void);

  void Write(std::ofstream &OutFile);

private:

  std::vector<CarlaRecorderLightVehicle> Vehicles;
};
