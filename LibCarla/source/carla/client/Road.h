// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/geom/Transform.h"
#include "carla/road/Lane.h"
#include "carla/road/RoadTypes.h"

#include <string>
#include <vector>

namespace carla {
namespace client {

  class Map;
  class Lane;
  class LaneSection;
  class Junction;
  class Waypoint;

  /// An OpenDRIVE road: a run of carriageway divided along s into lane
  /// sections, each holding the lanes.
  ///
  /// Roads connect end to end through GetNextRoads() / GetPreviousRoads(), or
  /// through a junction. There is deliberately no left/right here: OpenDRIVE
  /// gives roads no lateral relation, since a road already contains the lanes
  /// on both sides of its centre line. Lateral neighbours live on Lane.
  class Road
    : public EnableSharedFromThis<Road>,
      private NonCopyable {
  public:

    road::RoadId GetId() const {
      return _road_id;
    }

    std::string GetName() const;

    /// Length of the road, in metres.
    double GetLength() const;

    bool IsJunction() const;

    /// Id of the junction this road belongs to, or -1 when it is not part of
    /// one.
    road::JuncId GetJunctionId() const;

    /// Whether traffic on this road drives on the right.
    bool IsRHT() const;

    /// Id of the road that follows this one, as recorded in OpenDRIVE.
    road::RoadId GetSuccessorId() const;

    /// Id of the road that precedes this one, as recorded in OpenDRIVE.
    road::RoadId GetPredecessorId() const;

    /// Roads reachable by continuing forward.
    std::vector<SharedPtr<Road>> GetNextRoads() const;

    /// Roads that lead into this one.
    std::vector<SharedPtr<Road>> GetPreviousRoads() const;

    /// Every lane of the given type present at road coordinate @a s, across
    /// every lane section covering it.
    std::vector<SharedPtr<Lane>> GetLanes(
        double s,
        road::Lane::LaneType type = road::Lane::LaneType::Driving) const;

    SharedPtr<Lane> GetLane(road::SectionId section_id, road::LaneId lane_id) const;

    /// The lane with this id in whichever section covers @a s.
    SharedPtr<Lane> GetLaneAt(double s, road::LaneId lane_id) const;

    std::vector<SharedPtr<LaneSection>> GetSections() const;

    SharedPtr<LaneSection> GetSection(road::SectionId section_id) const;

    /// The lane section covering road coordinate @a s, or nullptr.
    SharedPtr<LaneSection> GetSectionAt(double s) const;

    /// The junction this road belongs to, or nullptr when it is not part of
    /// one.
    SharedPtr<Junction> GetJunction() const;

    /// Transform at the centre of the road at road coordinate @a s.
    geom::Transform GetTransformAt(double s) const;

    /// A cross-section: one waypoint per lane of the given type at @a s.
    std::vector<SharedPtr<Waypoint>> GetWaypointsAt(
        double s,
        road::Lane::LaneType type = road::Lane::LaneType::Driving) const;

    /// Waypoints sampled *along* the road every @a distance metres, on every
    /// lane of the given type. Not to be confused with GetWaypointsAt(), which
    /// cuts across the road at a single s.
    std::vector<SharedPtr<Waypoint>> GetWaypoints(
        double distance,
        road::Lane::LaneType type = road::Lane::LaneType::Driving) const;

    bool operator==(const Road &rhs) const {
      return _road_id == rhs._road_id;
    }

    bool operator!=(const Road &rhs) const {
      return !(*this == rhs);
    }

  private:

    friend class Map;
    friend class Lane;
    friend class LaneSection;
    friend class Junction;
    friend class Waypoint;

    Road(SharedPtr<const Map> parent, road::RoadId road_id)
      : _parent(std::move(parent)),
        _road_id(road_id) {}

    SharedPtr<const Map> _parent;

    road::RoadId _road_id;
  };

} // namespace client
} // namespace carla
