// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderLightVehicle.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"


void CarlaRecorderLightVehicle::Write(std::ostream &OutFile)
{
  // database id
  WriteValue<uint32_t>(OutFile, this->DatabaseId);
  WriteValue<VehicleLightStateType>(OutFile, this->State);
}
void CarlaRecorderLightVehicle::Read(std::istream &InFile)
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

void CarlaRecorderLightVehicles::Write(std::ostream &OutFile)
{
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::VehicleLight));

  // write a dummy packet size
  uint32_t Total = 2 + Vehicles.size() * sizeof(CarlaRecorderLightVehicle);
  WriteValue<uint32_t>(OutFile, Total);

  // write total records
  Total = Vehicles.size();
  WriteValue<uint16_t>(OutFile, Total);

  for (auto& Vehicle : Vehicles)
  {
    Vehicle.Write(OutFile);
  }
}

void CarlaRecorderLightVehicles::Read(std::istream &InFile)
{
  uint16_t Total;
  CarlaRecorderLightVehicle LightVehicle;

  // read Total walkers
  ReadValue<uint16_t>(InFile, Total);
  for (uint16_t i = 0; i < Total; ++i)
  {
    LightVehicle.Read(InFile);
    Add(LightVehicle);
  }
}

const std::vector<CarlaRecorderLightVehicle>& CarlaRecorderLightVehicles::GetLightVehicles()
{
  return Vehicles;
}
