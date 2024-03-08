// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <sstream>
#include <vector>
#include <type_traits>

#pragma pack(push, 1)
struct CarlaRecorderLightVehicle
{
  // Use same type as carla::rpc::VehicleLightState::LightState
  using VehicleLightStateType = uint32_t;

  uint32_t DatabaseId;
  VehicleLightStateType State;

  void Read(std::istream &InFile);

  void Write(std::ostream &OutFile);
};
#pragma pack(pop)

struct CarlaRecorderLightVehicles
{
public:

  void Add(const CarlaRecorderLightVehicle &InObj);

  void Clear(void);

  void Write(std::ostream &OutFile);

  void Read(std::istream &InFile);

  const std::vector<CarlaRecorderLightVehicle>& GetLightVehicles();

private:

  std::vector<CarlaRecorderLightVehicle> Vehicles;
};
