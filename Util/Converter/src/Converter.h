// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

namespace osm2odr {

  struct OSM2ODRSettings {
    double offsetX = 0;
    double offsetY = 0;
  };

  std::string ConvertOSMToOpenDRIVE(std::string osm_file, OSM2ODRSettings settings = OSM2ODRSettings());

} // namespace osm2odr
