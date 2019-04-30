// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorder.h"
#include "CarlaRecorderAnimVehicle.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderAnimVehicle::Write(std::ofstream &OutFile)
{
  // database id
  WriteValue<uint32_t>(OutFile, this->DatabaseId);
  WriteValue<float>(OutFile, this->Steering);
  WriteValue<float>(OutFile, this->Throttle);
  WriteValue<float>(OutFile, this->Brake);
  WriteValue<bool>(OutFile, this->bHandbrake);
  WriteValue<int32_t>(OutFile, this->Gear);
}
void CarlaRecorderAnimVehicle::Read(std::ifstream &InFile)
{
  // database id
  ReadValue<uint32_t>(InFile, this->DatabaseId);
  ReadValue<float>(InFile, this->Steering);
  ReadValue<float>(InFile, this->Throttle);
  ReadValue<float>(InFile, this->Brake);
  ReadValue<bool>(InFile, this->bHandbrake);
  ReadValue<int32_t>(InFile, this->Gear);
}

// ---------------------------------------------

void CarlaRecorderAnimVehicles::Clear(void)
{
  Vehicles.clear();
}

void CarlaRecorderAnimVehicles::Add(const CarlaRecorderAnimVehicle &Vehicle)
{
  Vehicles.push_back(Vehicle);
}

void CarlaRecorderAnimVehicles::Write(std::ofstream &OutFile)
{
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::AnimVehicle));

  std::streampos PosStart = OutFile.tellp();

  // write a dummy packet size
  uint32_t Total = 0;
  WriteValue<uint32_t>(OutFile, Total);

  // write total records
  Total = Vehicles.size();
  WriteValue<uint16_t>(OutFile, Total);

  for (uint16_t i=0; i<Total; ++i)
    Vehicles[i].Write(OutFile);

  // write the real packet size
  std::streampos PosEnd = OutFile.tellp();
  Total = PosEnd - PosStart - sizeof(uint32_t);
  OutFile.seekp(PosStart, std::ios::beg);
  WriteValue<uint32_t>(OutFile, Total);
  OutFile.seekp(PosEnd, std::ios::beg);
}
