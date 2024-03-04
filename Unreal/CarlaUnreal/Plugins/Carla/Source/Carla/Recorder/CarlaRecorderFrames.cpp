// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderFrames.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderFrame::Read(std::istream &InFile)
{
  ReadValue<CarlaRecorderFrame>(InFile, *this);
}

void CarlaRecorderFrame::Write(std::ostream &OutFile)
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
  OffsetPreviousFrame = 0;
}

void CarlaRecorderFrames::SetFrame(double DeltaSeconds)
{
  if (Frame.Id == 0)
  {
    Frame.Elapsed = 0.0f;
    Frame.DurationThis = 0.0f;
  }
  else
  {
    Frame.DurationThis = DeltaSeconds;
    Frame.Elapsed += DeltaSeconds;
  }

  ++Frame.Id;
}

void CarlaRecorderFrames::WriteStart(std::ostream &OutFile)
{
  std::streampos Pos, Offset;
  double Dummy = -1.0f;

  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::FrameStart));

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
}

void CarlaRecorderFrames::WriteEnd(std::ostream &OutFile)
{
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::FrameEnd));

  // write the packet size (0)
  uint32_t Total = 0;
  WriteValue<uint32_t>(OutFile, Total);
}
