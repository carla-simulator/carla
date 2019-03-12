// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/GeoLocation.h"

#include <string>

namespace carla {
namespace opendrive {
namespace parser {

  class GeoReferenceParser {
  public:

    static geom::GeoLocation Parse(const std::string &geo_reference_string);

  };

} // parser
} // opendrive
} // carla
