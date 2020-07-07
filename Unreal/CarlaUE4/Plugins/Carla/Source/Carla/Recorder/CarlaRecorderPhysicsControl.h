// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include <vector>

#include "Carla/Vehicle/VehiclePhysicsControl.h"

#pragma pack(push, 1)
struct CarlaRecorderPhysicsControl
{
  uint32_t DatabaseId;
  FVehiclePhysicsControl VehiclePhysicsControl;

  void Read(std::ifstream &InFile);

  void Write(std::ofstream &OutFile);
};
#pragma pack(pop)

class CarlaRecorderPhysicsControls
{
  public:

  void Add(const CarlaRecorderPhysicsControl &InObj);

  void Clear(void);

  void Write(std::ofstream &OutFile);

private:

  std::vector<CarlaRecorderPhysicsControl> PhysicsControls;
};
