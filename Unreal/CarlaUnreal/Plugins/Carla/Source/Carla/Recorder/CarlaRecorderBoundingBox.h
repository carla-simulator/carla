// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <sstream>
#include <vector>

#pragma pack(push, 1)
struct CarlaRecorderBoundingBox
{
  FVector Origin;
  FVector Extension;

  void Read(std::istream &InFile);

  void Write(std::ostream &OutFile);
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CarlaRecorderActorBoundingBox
{
  uint32_t DatabaseId;
  CarlaRecorderBoundingBox BoundingBox;

  void Read(std::istream &InFile);

  void Write(std::ostream &OutFile);
};
#pragma pack(pop)

class CarlaRecorderActorBoundingBoxes
{
  public:

  void Add(const CarlaRecorderActorBoundingBox &InObj);

  void Clear(void);

  void Write(std::ostream &OutFile);

private:

  std::vector<CarlaRecorderActorBoundingBox> Boxes;
};

class CarlaRecorderActorTriggerVolumes
{
  public:

  void Add(const CarlaRecorderActorBoundingBox &InObj);

  void Clear(void);

  void Write(std::ostream &OutFile);

private:

  std::vector<CarlaRecorderActorBoundingBox> Boxes;
};
