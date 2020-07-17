// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderPhysicsControl.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/rpc/VehiclePhysicsControl.h"
#include <compiler/enable-ue4-macros.h>


void CarlaRecorderPhysicsControl::Write(std::ofstream &OutFile)
{
  carla::rpc::VehiclePhysicsControl RPCPhysicsControl(VehiclePhysicsControl);
  WriteValue<uint32_t>(OutFile, this->DatabaseId);
  WriteValue(OutFile, RPCPhysicsControl.max_rpm);
  WriteValue(OutFile, RPCPhysicsControl.moi);
  WriteValue(OutFile, RPCPhysicsControl.damping_rate_full_throttle);
  WriteValue(OutFile, RPCPhysicsControl.damping_rate_zero_throttle_clutch_engaged);
  WriteValue(OutFile, RPCPhysicsControl.damping_rate_zero_throttle_clutch_disengaged);
  WriteValue(OutFile, RPCPhysicsControl.use_gear_autobox);
  WriteValue(OutFile, RPCPhysicsControl.clutch_strength);
  WriteValue(OutFile, RPCPhysicsControl.final_ratio);
  WriteValue(OutFile, RPCPhysicsControl.mass);
  WriteValue(OutFile, RPCPhysicsControl.drag_coefficient);
  WriteValue(OutFile, RPCPhysicsControl.center_of_mass);

  // torque curve
  WriteStdVector(OutFile, RPCPhysicsControl.torque_curve);

  // forward gears
  WriteStdVector(OutFile, RPCPhysicsControl.forward_gears);

  // steering curve
  WriteStdVector(OutFile, RPCPhysicsControl.steering_curve);

  // wheels
  WriteStdVector(OutFile, RPCPhysicsControl.wheels);
}

void CarlaRecorderPhysicsControl::Read(std::ifstream &InFile)
{
  carla::rpc::VehiclePhysicsControl RPCPhysicsControl;
  ReadValue<uint32_t>(InFile, this->DatabaseId);
  ReadValue(InFile, RPCPhysicsControl.max_rpm);
  ReadValue(InFile, RPCPhysicsControl.moi);
  ReadValue(InFile, RPCPhysicsControl.damping_rate_full_throttle);
  ReadValue(InFile, RPCPhysicsControl.damping_rate_zero_throttle_clutch_engaged);
  ReadValue(InFile, RPCPhysicsControl.damping_rate_zero_throttle_clutch_disengaged);
  ReadValue(InFile, RPCPhysicsControl.use_gear_autobox);
  ReadValue(InFile, RPCPhysicsControl.clutch_strength);
  ReadValue(InFile, RPCPhysicsControl.final_ratio);
  ReadValue(InFile, RPCPhysicsControl.mass);
  ReadValue(InFile, RPCPhysicsControl.drag_coefficient);
  ReadValue(InFile, RPCPhysicsControl.center_of_mass);

  // torque curve
  ReadStdVector(InFile, RPCPhysicsControl.torque_curve);

  // forward gears
  ReadStdVector(InFile, RPCPhysicsControl.forward_gears);

  // steering curve
  ReadStdVector(InFile, RPCPhysicsControl.steering_curve);

  // wheels
  ReadStdVector(InFile, RPCPhysicsControl.wheels);

  VehiclePhysicsControl = FVehiclePhysicsControl(RPCPhysicsControl);
}

// ---------------------------------------------

void CarlaRecorderPhysicsControls::Clear(void)
{
  PhysicsControls.clear();
}

void CarlaRecorderPhysicsControls::Add(const CarlaRecorderPhysicsControl &InObj)
{
  PhysicsControls.push_back(InObj);
}

void CarlaRecorderPhysicsControls::Write(std::ofstream &OutFile)
{
  if (PhysicsControls.size() == 0)
  {
    return;
  }
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::PhysicsControl));

  std::streampos PosStart = OutFile.tellp();
  // write dummy packet size
  uint32_t Total = 0;
  WriteValue<uint32_t>(OutFile, Total);

  // write total records
  Total = PhysicsControls.size();
  WriteValue<uint16_t>(OutFile, Total);

  // write records
  for (auto& PhysicsControl : PhysicsControls)
  {
    PhysicsControl.Write(OutFile);
  }

  // write the real packet size
  std::streampos PosEnd = OutFile.tellp();
  Total = PosEnd - PosStart - sizeof(uint32_t);
  OutFile.seekp(PosStart, std::ios::beg);
  WriteValue<uint32_t>(OutFile, Total);
  OutFile.seekp(PosEnd, std::ios::beg);
}
