// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderBoundingBox.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderBoundingBox::Write(std::ofstream &OutFile)
{
  WriteFVector(OutFile, this->Origin);
  WriteFVector(OutFile, this->Extension);
}

void CarlaRecorderBoundingBox::Read(std::ifstream &InFile)
{
  ReadFVector(InFile, this->Origin);
  ReadFVector(InFile, this->Extension);
}

void CarlaRecorderActorBoundingBox::Write(std::ofstream &OutFile)
{
  WriteValue<uint32_t>(OutFile, this->DatabaseId);
  BoundingBox.Write(OutFile);
}

void CarlaRecorderActorBoundingBox::Read(std::ifstream &InFile)
{
  ReadValue<uint32_t>(InFile, this->DatabaseId);
  BoundingBox.Read(InFile);
}


// ---------------------------------------------

void CarlaRecorderActorBoundingBoxes::Clear(void)
{
  Boxes.clear();
}

void CarlaRecorderActorBoundingBoxes::Add(const CarlaRecorderActorBoundingBox &InObj)
{
  Boxes.push_back(InObj);
}

void CarlaRecorderActorBoundingBoxes::Write(std::ofstream &OutFile)
{
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::BoundingBox));

  // write the packet size
  uint32_t Total = 2 + Boxes.size() * sizeof(CarlaRecorderActorBoundingBox);
  WriteValue<uint32_t>(OutFile, Total);

  // write total records
  Total = Boxes.size();
  WriteValue<uint16_t>(OutFile, Total);

  // write records
  for(auto& Box : Boxes)
  {
    Box.Write(OutFile);
  }
}

void CarlaRecorderActorTriggerVolumes::Clear(void)
{
  Boxes.clear();
}

void CarlaRecorderActorTriggerVolumes::Add(const CarlaRecorderActorBoundingBox &InObj)
{
  Boxes.push_back(InObj);
}

void CarlaRecorderActorTriggerVolumes::Write(std::ofstream &OutFile)
{
  if (Boxes.size() == 0)
  {
    return;
  }
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::TriggerVolume));

  // write the packet size
  uint32_t Total = 2 + Boxes.size() * sizeof(CarlaRecorderActorBoundingBox);
  WriteValue<uint32_t>(OutFile, Total);

  // write total records
  Total = Boxes.size();
  WriteValue<uint16_t>(OutFile, Total);

  // write records
  for(auto& Box : Boxes)
  {
    Box.Write(OutFile);
  }
}
