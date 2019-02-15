// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorder.h"
#include "CarlaRecorderFrames.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderFrame::Read(std::ifstream &InFile)
{
  ReadValue<CarlaRecorderFrame>(InFile, *this);
}

void CarlaRecorderFrame::Write(std::ofstream &OutFile)
{
  WriteValue<CarlaRecorderFrame>(OutFile, *this);
}

// ---------------------------------------------

CarlaRecorderFrames::CarlaRecorderFrames(void)
{
  Reset();
}

void CarlaRecorderFrames::Reset(void)
{
  Frame.Id = 0;
  Frame.DurationThis = 0.0f;
  Frame.Elapsed = 0.0f;
  LastTime = std::chrono::high_resolution_clock::now();
  OffsetPreviousFrame = 0;
}

void CarlaRecorderFrames::SetFrame(void)
{
  auto Now = std::chrono::high_resolution_clock::now();

  if (Frame.Id == 0)
  {
    Frame.Elapsed = 0.0f;
    Frame.DurationThis = 0.0f;
  }
  else
  {
    Frame.DurationThis = std::chrono::duration<double>(Now - LastTime).count();
    Frame.Elapsed += Frame.DurationThis;
  }

  LastTime = Now;
  ++Frame.Id;
}

void CarlaRecorderFrames::Write(std::ofstream &OutFile, std::ofstream &OutLog)
{
  std::streampos Pos, Offset;
  double Dummy = -1.0f;

  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::Frame));

  // write the packet size
  uint32_t Total = sizeof(CarlaRecorderFrame);
  WriteValue<uint32_t>(OutFile, Total);

  // write frame record
  WriteValue<uint64_t>(OutFile, Frame.Id);
  Offset = OutFile.tellp();
  WriteValue<double>(OutFile, Dummy);
  WriteValue<double>(OutFile, Frame.Elapsed);

  // we need to write this duration to previous frame
  if (OffsetPreviousFrame > 0)
  {
    Pos = OutFile.tellp();
    OutFile.seekp(OffsetPreviousFrame, std::ios::beg);
    WriteValue<double>(OutFile, Frame.DurationThis);
    OutFile.seekp(Pos, std::ios::beg);
  }

  // save position for next actualization
  OffsetPreviousFrame = Offset;

  OutLog << "frame " << Frame.Id << " elapsed " << Frame.Elapsed << "\n";
}
