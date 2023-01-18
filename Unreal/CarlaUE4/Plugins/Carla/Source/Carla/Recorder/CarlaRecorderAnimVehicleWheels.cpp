// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"
#include "CarlaRecorderAnimVehicleWheels.h"

void WheelInfo::Write(std::ostream &OutFile) const
{
  WriteValue<EVehicleWheelLocation>(OutFile, Location);
  WriteValue<float>(OutFile, SteeringAngle);
  WriteValue<float>(OutFile, TireRotation);
}

void WheelInfo::Read(std::istream &InFile)
{
  ReadValue<EVehicleWheelLocation>(InFile, Location);
  ReadValue<float>(InFile, SteeringAngle);
  ReadValue<float>(InFile, TireRotation);
}

void CarlaRecorderAnimWheels::Write(std::ostream &OutFile)
{
  WriteValue<uint32_t>(OutFile, DatabaseId);
  WriteValue<uint32_t>(OutFile, WheelValues.size());
  for (const WheelInfo& Wheel : WheelValues)
  {
    Wheel.Write(OutFile);
  }
}

void CarlaRecorderAnimWheels::Read(std::istream &InFile)
{
  ReadValue<uint32_t>(InFile, DatabaseId);
  uint32_t NumWheels = 0;
  ReadValue<uint32_t>(InFile, NumWheels);
  WheelValues.reserve(NumWheels);
  for (size_t i = 0; i < NumWheels; ++i)
  {
    WheelInfo Wheel;
    Wheel.Read(InFile);
    WheelValues.push_back(Wheel);
  }
}

// ---------------------------------------------

void CarlaRecorderAnimVehicleWheels::Clear(void)
{
  VehicleWheels.clear();
}

void CarlaRecorderAnimVehicleWheels::Add(const CarlaRecorderAnimWheels &Vehicle)
{
  VehicleWheels.push_back(Vehicle);
}

void CarlaRecorderAnimVehicleWheels::Write(std::ostream &OutFile)
{
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::AnimVehicleWheels));

  std::streampos PosStart = OutFile.tellp();

  // write a dummy packet size
  uint32_t Total = 0;
  WriteValue<uint32_t>(OutFile, Total);

  // write total records
  Total = VehicleWheels.size();
  WriteValue<uint16_t>(OutFile, Total);

  for (uint16_t i=0; i<Total; ++i)
    VehicleWheels[i].Write(OutFile);

  // write the real packet size
  std::streampos PosEnd = OutFile.tellp();
  Total = PosEnd - PosStart - sizeof(uint32_t);
  OutFile.seekp(PosStart, std::ios::beg);
  WriteValue<uint32_t>(OutFile, Total);
  OutFile.seekp(PosEnd, std::ios::beg);
}

void CarlaRecorderAnimVehicleWheels::Read(std::istream &InFile)
{
  uint16_t i, Total;
  CarlaRecorderAnimWheels Wheels;

  // read Total Vehicles
  ReadValue<uint16_t>(InFile, Total);
  for (i = 0; i < Total; ++i)
  {
    Wheels.Read(InFile);
    Add(Wheels);
  }
}

const std::vector<CarlaRecorderAnimWheels>& CarlaRecorderAnimVehicleWheels::GetVehicleWheels()
{
  return VehicleWheels;
}
