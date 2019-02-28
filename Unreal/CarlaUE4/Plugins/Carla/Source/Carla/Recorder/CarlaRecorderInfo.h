// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CarlaRecorderHelpers.h"

#include <fstream>
#include <ctime>

struct CarlaRecorderInfo
{
  uint16_t Version;
  FString Magic;
  std::time_t Date;
  FString Mapfile;

  void Read(std::ifstream &File)
  {
    ReadValue<uint16_t>(File, Version);
    ReadFString(File, Magic);
    ReadValue<std::time_t>(File, Date);
    ReadFString(File, Mapfile);
  }

  void Write(std::ofstream &File)
  {
    WriteValue<uint16_t>(File, Version);
    WriteFString(File, Magic);
    WriteValue<std::time_t>(File, Date);
    WriteFString(File, Mapfile);
  }
};
