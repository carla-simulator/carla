// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>

#pragma pack(push, 1)

struct CarlaRecorderStateTrafficLight
{
  uint32_t DatabaseId;
  bool IsFrozen;
  float ElapsedTime;
  char State;

  void Read(std::ifstream &InFile);

  void Write(std::ofstream &OutFile);

};

#pragma pack(pop)

class CarlaRecorderStates
{

public:

  void AddState(const CarlaRecorderStateTrafficLight &State);

  void Clear(void);

  void Write(std::ofstream &OutFile, std::ofstream &OutLog);

private:

  std::vector<CarlaRecorderStateTrafficLight> StatesTrafficLights;

  void WriteStatesTrafficLight(std::ofstream &OutFile);

};
