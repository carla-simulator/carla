// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/road/element/Waypoint.h"

namespace carla {
namespace client {

  class Map;

  class Waypoint
    : public EnableSharedFromThis<Waypoint>,
      private NonCopyable {
  public:

    ~Waypoint();

    const geom::Transform &GetTransform() const {
      return _transform;
    }

    road::element::id_type GetRoadId() const {
      return _waypoint.GetRoadId();
    }

    int GetLaneId() const {
      return _waypoint.GetLaneId();
    }

    std::vector<SharedPtr<Waypoint>> Next(double distance) const;

  private:

    friend class Map;

    Waypoint(SharedPtr<const Map> parent, road::element::Waypoint waypoint);

    SharedPtr<const Map> _parent;

    road::element::Waypoint _waypoint;

    geom::Transform _transform;
  };

} // namespace client
} // namespace carla
