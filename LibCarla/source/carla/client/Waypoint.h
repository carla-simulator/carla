// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/geom/Transform.h"
#include "carla/road/element/RoadInfoMarkRecord.h"
#include "carla/road/element/Waypoint.h"

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

    /// Returns an unique Id identifying this waypoint.
    ///
    /// The Id takes into account OpenDrive's road Id, lane Id, and s distance
    /// on its road segment up to half-centimetre precision.
    uint64_t GetId() const {
      return std::hash<road::element::Waypoint>()(_waypoint);
    }

    auto GetRoadId() const {
      return _waypoint.road_id;
    }

    auto GetLaneId() const {
      return _waypoint.lane_id;
    }

    auto GetDistance() const {
      return _waypoint.s;
    }

    const geom::Transform &GetTransform() const {
      return _transform;
    }

    bool IsIntersection() const;

    double GetLaneWidth() const;

    std::string GetType() const;

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

    // Mark record right and left respectively.
    std::pair<
        const road::element::RoadInfoMarkRecord *,
        const road::element::RoadInfoMarkRecord *> _mark_record;
  };

} // namespace client
} // namespace carla
