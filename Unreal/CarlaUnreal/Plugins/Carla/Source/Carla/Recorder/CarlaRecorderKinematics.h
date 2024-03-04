// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <sstream>
#include <vector>

#pragma pack(push, 1)
struct CarlaRecorderKinematics
{
  uint32_t DatabaseId;
  FVector LinearVelocity;
  FVector AngularVelocity;

  void Read(std::istream &InFile);

  void Write(std::ostream &OutFile);
};
#pragma pack(pop)

class CarlaRecorderActorsKinematics
{
  public:

  void Add(const CarlaRecorderKinematics &InObj);

  void Clear(void);

  void Write(std::ostream &OutFile);

private:

  std::vector<CarlaRecorderKinematics> Kinematics;
};
