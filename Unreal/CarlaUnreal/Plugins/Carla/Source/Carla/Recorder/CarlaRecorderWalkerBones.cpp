// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderWalkerBones.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderWalkerBones::Write(std::ofstream &OutFile)
{
  // database id
  WriteValue<uint32_t>(OutFile, this->DatabaseId);
  
  // write all bones
  WriteValue<uint16_t>(OutFile, this->Bones.size());
  for (const auto& Obj : this->Bones)
  {
    // name & transform
    WriteFString(OutFile, Obj.Name);
    WriteFVector(OutFile, Obj.Location);
    WriteFVector(OutFile, Obj.Rotation);
  }
}

void CarlaRecorderWalkerBones::Read(std::ifstream &InFile)
{
  // database id
  ReadValue<uint32_t>(InFile, this->DatabaseId);

  // read all bones
  uint16_t Total;
  ReadValue<uint16_t>(InFile, Total);
  this->Bones.reserve(Total);
  FString Name;
  FVector Location, Rotation;
  for (int i=0; i<Total; ++i)
  {
    // name & transform
    ReadFString(InFile, Name);
    ReadFVector(InFile, Location);
    ReadFVector(InFile, Rotation);
    // add to the vector of bones
    this->Bones.emplace_back(Name, Location, Rotation);
  }
}

void CarlaRecorderWalkerBones::Clear(void)
{
  Bones.clear();
}

// ---------------------------------------------

void CarlaRecorderWalkersBones::Clear(void)
{
  Walkers.clear();
}

void CarlaRecorderWalkersBones::Add(const CarlaRecorderWalkerBones &Walker)
{
  Walkers.push_back(Walker);
}

void CarlaRecorderWalkersBones::Write(std::ofstream &OutFile)
{
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::WalkerBones));

  std::streampos PosStart = OutFile.tellp();

  // write a dummy packet size
  uint32_t Total = 0;
  WriteValue<uint32_t>(OutFile, Total);

  // write total records
  Total = Walkers.size();
  WriteValue<uint16_t>(OutFile, Total);

  // write records
  for (uint16_t i=0; i<Total; ++i)
    Walkers[i].Write(OutFile);

  // write the real packet size
  std::streampos PosEnd = OutFile.tellp();
  Total = PosEnd - PosStart - sizeof(uint32_t);
  OutFile.seekp(PosStart, std::ios::beg);
  WriteValue<uint32_t>(OutFile, Total);
  OutFile.seekp(PosEnd, std::ios::beg);
}
