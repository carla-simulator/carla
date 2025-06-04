// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderVisualTime.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderVisualTime::SetTime(double ThisTime)
{
  Time = ThisTime;
}

void CarlaRecorderVisualTime::Read(std::ifstream &InFile)
{
  ReadValue<double>(InFile, this->Time);
}

void CarlaRecorderVisualTime::Write(std::ofstream &OutFile)
{
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::VisualTime));

  // write packet size
  uint32_t Total = sizeof(double);
  WriteValue<uint32_t>(OutFile, Total);

  WriteValue<double>(OutFile, this->Time);
}
