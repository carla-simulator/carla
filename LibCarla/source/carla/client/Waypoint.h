// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/geom/Transform.h"
#include "carla/road/element/LaneMarking.h"
#include "carla/road/element/RoadInfoMarkRecord.h"
#include "carla/road/element/Waypoint.h"
#include "carla/road/Lane.h"
#include "carla/road/RoadTypes.h"

#include <boost/optional.hpp>

namespace carla {
namespace client {

  class Map;
  class Junction;
  class Landmark;

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

    road::JuncId GetJunctionId() const;

    bool IsJunction() const;

    SharedPtr<Junction> GetJunction() const;

    double GetLaneWidth() const;

    road::Lane::LaneType GetType() const;

    std::vector<SharedPtr<Waypoint>> GetNext(double distance) const;

    std::vector<SharedPtr<Waypoint>> GetPrevious(double distance) const;

    /// Returns a list of waypoints separated by distance from the current waypoint
    /// to the end of the lane
    std::vector<SharedPtr<Waypoint>> GetNextUntilLaneEnd(double distance) const;

    /// Returns a list of waypoints separated by distance from the current waypoint
    /// to the start of the lane
    std::vector<SharedPtr<Waypoint>> GetPreviousUntilLaneStart(double distance) const;

    SharedPtr<Waypoint> GetRight() const;

    SharedPtr<Waypoint> GetLeft() const;

    boost::optional<road::element::LaneMarking> GetRightLaneMarking() const;

    boost::optional<road::element::LaneMarking> GetLeftLaneMarking() const;

    road::element::LaneMarking::LaneChange GetLaneChange() const;

    /// Returns a list of landmarks from the current position to a certain distance
    std::vector<SharedPtr<Landmark>> GetAllLandmakrsInDistance(
        double distance, bool stop_at_junction = false) const;

    /// Returns a list of landmarks from the current position to a certain distance
    /// Filters by specified type
    std::vector<SharedPtr<Landmark>> GetLandmakrsOfTypeInDistance(
        double distance, std::string filter_type, bool stop_at_junction = false) const;

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
