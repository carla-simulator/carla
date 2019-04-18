// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorder.h"
#include "CarlaRecorderAnimWalker.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderAnimWalker::Write(std::ofstream &OutFile)
{
  // database id
  WriteValue<uint32_t>(OutFile, this->DatabaseId);
  WriteValue<float>(OutFile, this->Speed);
}
void CarlaRecorderAnimWalker::Read(std::ifstream &InFile)
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

void CarlaRecorderAnimWalkers::Write(std::ofstream &OutFile)
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
