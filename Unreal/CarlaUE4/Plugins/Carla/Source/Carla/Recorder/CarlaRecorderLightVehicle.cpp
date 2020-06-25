// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderLightVehicle.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"


void CarlaRecorderLightVehicle::Write(std::ofstream &OutFile)
{
  // database id
  WriteValue<uint32_t>(OutFile, this->DatabaseId);
  WriteValue<VehicleLightStateType>(OutFile, this->State);
}
void CarlaRecorderLightVehicle::Read(std::ifstream &InFile)
{
  // database id
  ReadValue<uint32_t>(InFile, this->DatabaseId);
  ReadValue<VehicleLightStateType>(InFile, this->State);
}

// ---------------------------------------------

void CarlaRecorderLightVehicles::Clear(void)
{
  Vehicles.clear();
}

void CarlaRecorderLightVehicles::Add(const CarlaRecorderLightVehicle &Vehicle)
{
  Vehicles.push_back(Vehicle);
}

void CarlaRecorderLightVehicles::Write(std::ofstream &OutFile)
{
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::VehicleLight));

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
