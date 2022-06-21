// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderTraficLightTime.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"


void CarlaRecorderTrafficLightTime::Write(std::ofstream &OutFile)
{
  WriteValue<uint32_t>(OutFile, this->DatabaseId);
  WriteValue(OutFile, this->GreenTime);
  WriteValue(OutFile, this->YellowTime);
  WriteValue(OutFile, this->RedTime);
}

void CarlaRecorderTrafficLightTime::Read(std::ifstream &InFile)
{
  ReadValue<uint32_t>(InFile, this->DatabaseId);
  ReadValue(InFile, this->GreenTime);
  ReadValue(InFile, this->YellowTime);
  ReadValue(InFile, this->RedTime);
}

// ---------------------------------------------

void CarlaRecorderTrafficLightTimes::Clear(void)
{
  TrafficLightTimes.clear();
}

void CarlaRecorderTrafficLightTimes::Add(const CarlaRecorderTrafficLightTime &InObj)
{
  TrafficLightTimes.push_back(InObj);
}

void CarlaRecorderTrafficLightTimes::Write(std::ofstream &OutFile)
{
  if (TrafficLightTimes.size() == 0)
  {
    return;
  }
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::TrafficLightTime));

  uint32_t Total = sizeof(uint16_t) + TrafficLightTimes.size() * sizeof(CarlaRecorderTrafficLightTime);
  WriteValue<uint32_t>(OutFile, Total);

  Total = TrafficLightTimes.size();
  WriteValue<uint16_t>(OutFile, Total);

  for (auto& TrafficLightTime : TrafficLightTimes)
  {
    TrafficLightTime.Write(OutFile);
  }
}
