// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Transform.h"
#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/road/element/RoadInfoMarkRecord.h"
#include "carla/road/element/Waypoint.h"
#include "carla/road/element/WaypointInformationTypes.h"
#include "carla/road/Lane.h"

#include <boost/optional.hpp>

namespace carla {
namespace client {

  class Map;

  class Waypoint
    : public EnableSharedFromThis<Waypoint>,
      private NonCopyable {
  public:

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

    auto GetSectionId() const {
      return _waypoint.section_id;
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

    road::Lane::LaneType GetType() const;

    std::vector<SharedPtr<Waypoint>> Next(double distance) const;

    SharedPtr<Waypoint> Right() const;

    SharedPtr<Waypoint> Left() const;

    boost::optional<road::element::WaypointInfoRoadMark> GetRightRoadMark() const;

    boost::optional<road::element::WaypointInfoRoadMark> GetLeftRoadMark() const;

    road::element::WaypointInfoRoadMark::LaneChange GetLaneChange() const;

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
