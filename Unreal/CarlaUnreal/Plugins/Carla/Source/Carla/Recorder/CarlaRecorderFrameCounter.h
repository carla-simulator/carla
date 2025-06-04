// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <sstream>

#pragma pack(push, 1)
struct CarlaRecorderFrameCounter
{

  uint64_t FrameCounter;

  void Read(std::istream &InFile);

  void Write(std::ostream &OutFile);

};
#pragma pack(pop)
