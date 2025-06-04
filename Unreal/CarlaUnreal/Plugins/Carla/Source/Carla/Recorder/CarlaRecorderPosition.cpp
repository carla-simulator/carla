// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderPosition.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderPosition::Write(std::ostream &OutFile)
{
  // database id
  WriteValue<uint32_t>(OutFile, this->DatabaseId);
  // transform
  WriteFVector(OutFile, this->Location);
  WriteFVector(OutFile, this->Rotation);
}
void CarlaRecorderPosition::Read(std::istream &InFile)
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

void CarlaRecorderPositions::Write(std::ostream &OutFile)
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

void CarlaRecorderPositions::Read(std::istream &InFile)
{
  uint16_t i, Total;

  // read all positions
  ReadValue<uint16_t>(InFile, Total);
  for (i = 0; i < Total; ++i)
  {
    CarlaRecorderPosition Pos;
    Pos.Read(InFile);
    Add(Pos);
  }
}

const std::vector<CarlaRecorderPosition>& CarlaRecorderPositions::GetPositions()
{
  return Positions;
}
