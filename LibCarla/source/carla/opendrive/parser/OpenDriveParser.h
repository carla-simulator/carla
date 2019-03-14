// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/opendrive/types.h"

enum class XmlInputType : int {
  FILE,
  CONTENT
};

struct OpenDriveParser {
  static bool Parse(
      const char *xml,
      carla::opendrive::types::OpenDriveData &out_open_drive_data,
      XmlInputType inputType,
      std::string *out_error = nullptr);
};
