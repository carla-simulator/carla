// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderFrameCounter.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderFrameCounter::Read(std::istream &InFile)
{
  ReadValue<uint64_t>(InFile, this->FrameCounter);
}

void CarlaRecorderFrameCounter::Write(std::ostream &OutFile)
{
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::FrameCounter));

  // write packet size
  uint32_t Total = sizeof(uint64_t);
  WriteValue<uint32_t>(OutFile, Total);

  WriteValue<uint64_t>(OutFile, this->FrameCounter);
}
