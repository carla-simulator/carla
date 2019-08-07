// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorder.h"
#include "CarlaRecorderPosition.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderPosition::Write(std::ofstream &OutFile)
{
  // database id
  WriteValue<uint32_t>(OutFile, this->DatabaseId);
  // transform
  WriteFVector(OutFile, this->Location);
  WriteFVector(OutFile, this->Rotation);
}
void CarlaRecorderPosition::Read(std::ifstream &InFile)
{
  // database id
  ReadValue<uint32_t>(InFile, this->DatabaseId);
  // transform
  ReadFVector(InFile, this->Location);
  ReadFVector(InFile, this->Rotation);
}

// ---------------------------------------------

void CarlaRecorderPositions::Clear(void)
{
  Positions.clear();
}

void CarlaRecorderPositions::Add(const CarlaRecorderPosition &Position)
{
  Positions.push_back(Position);
}

void CarlaRecorderPositions::Write(std::ofstream &OutFile)
{
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::Position));

  // write the packet size
  uint32_t Total = 2 + Positions.size() * sizeof(CarlaRecorderPosition);
  WriteValue<uint32_t>(OutFile, Total);

  // write total records
  Total = Positions.size();
  WriteValue<uint16_t>(OutFile, Total);

  // write records
  if (Total > 0)
  {
    OutFile.write(reinterpret_cast<const char *>(Positions.data()),
        Positions.size() * sizeof(CarlaRecorderPosition));
  }
}
