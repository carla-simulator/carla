// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include <vector>

#pragma pack(push, 1)
struct CarlaRecorderKinematics
{
  uint32_t DatabaseId;
  FVector LinearVelocity;
  FVector AngularVelocity;

  void Read(std::ifstream &InFile);

  void Write(std::ofstream &OutFile);
};
#pragma pack(pop)

class CarlaRecorderActorsKinematics
{
  public:

  void Add(const CarlaRecorderKinematics &InObj);

  void Clear(void);

  void Write(std::ofstream &OutFile);

private:

  std::vector<CarlaRecorderKinematics> Kinematics;
};
