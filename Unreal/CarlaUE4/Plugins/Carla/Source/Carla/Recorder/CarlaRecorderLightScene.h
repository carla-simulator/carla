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
struct CarlaRecorderLightScene
{

  int LightId;
  float Intensity;
  FLinearColor Color;
  bool bOn;
  uint8 Type;

  void Read(std::ifstream &InFile);

  void Write(std::ofstream &OutFile);
};
#pragma pack(pop)

struct CarlaRecorderLightScenes
{
public:

  void Add(const CarlaRecorderLightScene &InObj);

  void Clear(void);

  void Write(std::ofstream &OutFile);

private:

  std::vector<CarlaRecorderLightScene> Lights;
};
