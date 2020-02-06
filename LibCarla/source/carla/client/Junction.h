// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/road/Junction.h"
#include "carla/road/RoadTypes.h"
#include "carla/geom/BoundingBox.h"
#include "carla/client/Waypoint.h"

#include <vector>

namespace carla {
namespace client {

  class Map;

  class Junction
    : public EnableSharedFromThis<Junction>,
    private NonCopyable
  {
  public:

    carla::road::JuncId GetId() const {
      return _id;
    }

    std::vector<std::pair<SharedPtr<Waypoint>,SharedPtr<Waypoint>>> GetWaypoints(
        road::Lane::LaneType type = road::Lane::LaneType::Driving) const;

    geom::BoundingBox GetBoundingBox() const;

  private:

    friend class Map;

    Junction(SharedPtr<const Map> parent, const road::Junction *junction);

    SharedPtr<const Map> _parent;

    geom::BoundingBox _bounding_box;

    road::JuncId _id;
  };

} // namespace client
} // namespace carla
