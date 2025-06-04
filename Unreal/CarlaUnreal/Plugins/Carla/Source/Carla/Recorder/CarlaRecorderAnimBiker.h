// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <sstream>
#include <vector>

#pragma pack(push, 1)
struct CarlaRecorderAnimBiker
{
  uint32_t DatabaseId;
  float ForwardSpeed { 0.0f };
  float EngineRotation { 0.0f };

  void Read(std::istream &InFile);

  void Write(std::ostream &OutFile) const;

};
#pragma pack(pop)

class CarlaRecorderAnimBikers
{
public:

  void Add(const CarlaRecorderAnimBiker &InObj);

  void Clear(void);

  void Write(std::ostream &OutFile) const;

  void Read(std::istream &InFile);

  const std::vector<CarlaRecorderAnimBiker>& GetBikers();

private:

  std::vector<CarlaRecorderAnimBiker> Bikers;
};
