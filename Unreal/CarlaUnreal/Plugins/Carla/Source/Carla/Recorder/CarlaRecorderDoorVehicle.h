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
struct CarlaRecorderDoorVehicle
{
  // Use same type as carla::vehicle::CarlaWheeledVehicle::EVehicleDoor
  using VehicleDoorType = uint8_t;

  uint32_t DatabaseId;
  VehicleDoorType Doors;
  bool bIsOpen;

  void Read(std::istream &InFile);

  void Write(std::ostream &OutFile);
};
#pragma pack(pop)

struct CarlaRecorderDoorVehicles
{
public:

  void Add(const CarlaRecorderDoorVehicle &InObj);

  void Clear(void);

  void Write(std::ostream &OutFile);

  void Read(std::istream &InFile);

  const std::vector<CarlaRecorderDoorVehicle>& GetDoorVehicles();

private:
  std::vector<CarlaRecorderDoorVehicle> Vehicles;
};
