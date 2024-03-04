// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderPlatformTime.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderPlatformTime::SetStartTime()
{
  RecorderStartTime = std::chrono::system_clock::now();
}
void CarlaRecorderPlatformTime::UpdateTime()
{
  auto CurrentTime = std::chrono::system_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::microseconds>
      (CurrentTime-RecorderStartTime).count();
  Time = diff/1000000.0;
}

void CarlaRecorderPlatformTime::Read(std::istream &InFile)
{
  ReadValue<double>(InFile, this->Time);
}

void CarlaRecorderPlatformTime::Write(std::ostream &OutFile)
{
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::PlatformTime));

  // write packet size
  uint32_t Total = sizeof(double);
  WriteValue<uint32_t>(OutFile, Total);

  WriteValue<double>(OutFile, this->Time);
}
