// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include <vector>

#pragma pack(push, 1)
struct CarlaRecorderWalkerBone
{
  FString Name;
  FVector Location;
  FVector Rotation;

  CarlaRecorderWalkerBone(FString &InName, FVector &InLocation, FVector &InRotation) : 
    Name(InName), Location(InLocation), Rotation(InRotation) {}
};

struct CarlaRecorderWalkerBones
{
  uint32_t DatabaseId;
  std::vector<CarlaRecorderWalkerBone> Bones;
  
  void Read(std::ifstream &InFile);

  void Write(std::ofstream &OutFile);

  void Clear();

};
#pragma pack(pop)

class CarlaRecorderWalkersBones
{
public:

  void Add(const CarlaRecorderWalkerBones &InObj);

  void Clear(void);

  void Write(std::ofstream &OutFile);

private:

  std::vector<CarlaRecorderWalkerBones> Walkers;
};
