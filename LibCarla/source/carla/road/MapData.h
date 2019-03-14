// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/GeoLocation.h"
#include "carla/Iterator.h"
#include "carla/ListView.h"
#include "carla/NonCopyable.h"
#include "carla/road/Road.h"
#include "carla/road/RoadTypes.h"

#include <boost/iterator/transform_iterator.hpp>

#include <unordered_map>

namespace carla {
namespace road {

  class MapData : private MovableNonCopyable {
  public:

    const geom::GeoLocation &GetGeoReference() const {
      return _geo_reference;
    }

  private:

    friend class MapBuilder;

    MapData() = default;

    geom::GeoLocation _geo_reference;

    std::unordered_map<RoadId, Road> _roads;

  };

} // namespace road
} // namespace carla
