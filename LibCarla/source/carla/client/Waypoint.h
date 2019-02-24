// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/road/element/Waypoint.h"
#include "carla/road/element/RoadInfoMarkRecord.h"

namespace carla {
namespace client {

  class Map;

  class Waypoint
    : public EnableSharedFromThis<Waypoint>,
      private NonCopyable {
  public:

    /// Can be used as flags
    enum class LaneChange : uint8_t {
      None  = 0x00, //00
      Right = 0x01, //01
      Left  = 0x02, //10
      Both  = 0x03  //11
    };

    ~Waypoint();

    const geom::Transform &GetTransform() const {
      return _transform;
    }

    bool IsIntersection() const {
      return _waypoint.IsIntersection();
    }

    double GetLaneWidth() const {
      return _waypoint.GetLaneWidth();
    }

    road::element::id_type GetRoadId() const {
      return _waypoint.GetRoadId();
    }

    int GetLaneId() const {
      return _waypoint.GetLaneId();
    }

    std::string GetType() const {
      return _waypoint.GetType();
    }

    std::vector<SharedPtr<Waypoint>> Next(double distance) const;

    SharedPtr<Waypoint> Right() const;

    SharedPtr<Waypoint> Left() const;

    Waypoint::LaneChange GetLaneChange() const;

  private:

    friend class Map;

    Waypoint(SharedPtr<const Map> parent, road::element::Waypoint waypoint);

    SharedPtr<const Map> _parent;

    road::element::Waypoint _waypoint;

    geom::Transform _transform;

    // Mark record right and left respectively
    std::pair<
        road::element::RoadInfoMarkRecord,
        road::element::RoadInfoMarkRecord> _mark_record;
  };

} // namespace client
} // namespace carla
