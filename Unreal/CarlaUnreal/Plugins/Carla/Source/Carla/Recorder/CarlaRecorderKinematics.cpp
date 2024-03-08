// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderKinematics.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderKinematics::Write(std::ostream &OutFile)
{
  WriteValue<uint32_t>(OutFile, this->DatabaseId);
  WriteFVector(OutFile, this->LinearVelocity);
  WriteFVector(OutFile, this->AngularVelocity);
}

void CarlaRecorderKinematics::Read(std::istream &InFile)
{
  ReadValue<uint32_t>(InFile, this->DatabaseId);
  ReadFVector(InFile, this->LinearVelocity);
  ReadFVector(InFile, this->AngularVelocity);
}

// ---------------------------------------------

void CarlaRecorderActorsKinematics::Clear(void)
{
  Kinematics.clear();
}

void CarlaRecorderActorsKinematics::Add(const CarlaRecorderKinematics &InObj)
{
  Kinematics.push_back(InObj);
}

void CarlaRecorderActorsKinematics::Write(std::ostream &OutFile)
{
  if (Kinematics.size() == 0)
  {
    return;
  }
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::Kinematics));

  // write the packet size
  uint32_t Total = 2 + Kinematics.size() * sizeof(CarlaRecorderKinematics);
  WriteValue<uint32_t>(OutFile, Total);

  // write total records
  Total = Kinematics.size();
  WriteValue<uint16_t>(OutFile, Total);

  // write records
  for (auto& Kinematic : Kinematics)
  {
    Kinematic.Write(OutFile);
  }
}
