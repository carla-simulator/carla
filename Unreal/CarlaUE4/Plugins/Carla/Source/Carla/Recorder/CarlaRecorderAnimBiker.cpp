// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderAnimBiker.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderAnimBiker::Write(std::ostream &OutFile) const
{
  WriteValue<uint32_t>(OutFile, DatabaseId);
  WriteValue<float>(OutFile, ForwardSpeed);
  WriteValue<float>(OutFile, EngineRotation);
}

void CarlaRecorderAnimBiker::Read(std::istream &InFile)
{
  ReadValue<uint32_t>(InFile, DatabaseId);
  ReadValue<float>(InFile, ForwardSpeed);
  ReadValue<float>(InFile, EngineRotation);
}

// ---------------------------------------------

void CarlaRecorderAnimBikers::Clear(void)
{
  Bikers.clear();
}

void CarlaRecorderAnimBikers::Add(const CarlaRecorderAnimBiker &Biker)
{
  Bikers.push_back(Biker);
}

void CarlaRecorderAnimBikers::Write(std::ostream &OutFile) const
{
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::AnimBiker));

  // write the packet size
  uint32_t Total = 2 + Bikers.size() * sizeof(CarlaRecorderAnimBiker);
  WriteValue<uint32_t>(OutFile, Total);

  // write total records
  Total = Bikers.size();
  WriteValue<uint16_t>(OutFile, Total);

  // write records
  if (Total > 0)
  {
    OutFile.write(reinterpret_cast<const char *>(Bikers.data()),
        Bikers.size() * sizeof(CarlaRecorderAnimBiker));
  }
}

void CarlaRecorderAnimBikers::Read(std::istream &InFile)
{
  uint16_t i, Total;
  CarlaRecorderAnimBiker Biker;

  // read Total Bikers
  ReadValue<uint16_t>(InFile, Total);
  for (i = 0; i < Total; ++i)
  {
    Biker.Read(InFile);
    Add(Biker);
  }
}

const std::vector<CarlaRecorderAnimBiker>& CarlaRecorderAnimBikers::GetBikers()
{
  return Bikers;
}
