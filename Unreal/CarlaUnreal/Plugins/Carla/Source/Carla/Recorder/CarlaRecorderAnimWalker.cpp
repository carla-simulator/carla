// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderAnimWalker.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderAnimWalker::Write(std::ostream &OutFile)
{
  // database id
  WriteValue<uint32_t>(OutFile, this->DatabaseId);
  WriteValue<float>(OutFile, this->Speed);
}
void CarlaRecorderAnimWalker::Read(std::istream &InFile)
{
  // database id
  ReadValue<uint32_t>(InFile, this->DatabaseId);
  ReadValue<float>(InFile, this->Speed);
}

// ---------------------------------------------

void CarlaRecorderAnimWalkers::Clear(void)
{
  Walkers.clear();
}

void CarlaRecorderAnimWalkers::Add(const CarlaRecorderAnimWalker &Walker)
{
  Walkers.push_back(Walker);
}

void CarlaRecorderAnimWalkers::Write(std::ostream &OutFile)
{
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::AnimWalker));

  // write the packet size
  uint32_t Total = 2 + Walkers.size() * sizeof(CarlaRecorderAnimWalker);
  WriteValue<uint32_t>(OutFile, Total);

  // write total records
  Total = Walkers.size();
  WriteValue<uint16_t>(OutFile, Total);

  // write records
  if (Total > 0)
  {
    OutFile.write(reinterpret_cast<const char *>(Walkers.data()),
        Walkers.size() * sizeof(CarlaRecorderAnimWalker));
  }
}

void CarlaRecorderAnimWalkers::Read(std::istream &InFile)
{
  uint16_t i, Total;
  CarlaRecorderAnimWalker Walker;

  // read Total walkers
  ReadValue<uint16_t>(InFile, Total);
  for (i = 0; i < Total; ++i)
  {
    Walker.Read(InFile);
    Add(Walker);
  }
}

const std::vector<CarlaRecorderAnimWalker>& CarlaRecorderAnimWalkers::GetWalkers()
{
  return Walkers;
}
