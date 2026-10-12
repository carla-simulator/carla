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

#include <vector>

namespace carla {
namespace client {

  class Map;
  class Road;
  class LaneSection;
  class Waypoint;

  /// A single OpenDRIVE lane, identified by (road id, lane section id, lane id).
  ///
  /// A lane lives inside exactly one lane section, which is why its identity
  /// carries the section: the same lane id on the same road is a different lane
  /// in a different section. Following GetNextLanes() can therefore cross into
  /// another section, or return nothing where the lane simply ends.
  ///
  /// Like carla::client::Junction, this holds the parent map alive and resolves
  /// its ids on each call rather than caching a road::Lane pointer, which would
  /// dangle once the map is released.
  class Lane
    : public EnableSharedFromThis<Lane>,
      private NonCopyable {
  public:

    road::LaneId GetId() const {
      return _lane_id;
    }

    road::RoadId GetRoadId() const {
      return _road_id;
    }

    road::SectionId GetSectionId() const {
      return _section_id;
    }

    /// Unique id of this lane within its map, for use as a dictionary key.
    uint64_t GetUniqueId() const;

    road::Lane::LaneType GetType() const;

    SharedPtr<Road> GetRoad() const;

    SharedPtr<LaneSection> GetSection() const;

    /// Length of the lane, in metres.
    double GetLength() const;

    /// The s coordinate, along the road, at which this lane's section begins.
    double GetDistance() const;

    bool GetLevel() const;

    bool IsStraight() const;

    /// Whether the lane runs along increasing s (a right-hand lane, id < 0).
    bool IsPositiveDirection() const;

    /// Lane width at the given s, in metres. @a s is a road coordinate, not an
    /// offset from the start of the lane.
    double GetWidth(double s) const;

    geom::Transform GetTransform(double s) const;

    /// The lane to the right, or nullptr at the edge of the road. Crossing the
    /// centre line flips the sign of the lane id, so the neighbour returned may
    /// run in the opposite direction; this matches Waypoint::GetRight().
    SharedPtr<Lane> GetRightLane() const;

    /// The lane to the left, or nullptr. See GetRightLane() on direction.
    SharedPtr<Lane> GetLeftLane() const;

    /// Lanes this one leads into. Empty where the lane ends.
    std::vector<SharedPtr<Lane>> GetNextLanes() const;

    /// Lanes leading into this one.
    std::vector<SharedPtr<Lane>> GetPreviousLanes() const;

    /// The waypoint on this lane at road coordinate @a s, or nullptr when @a s
    /// falls outside the lane.
    SharedPtr<Waypoint> GetWaypoint(double s) const;

    /// Waypoints along this lane, every @a distance metres.
    std::vector<SharedPtr<Waypoint>> GetWaypoints(double distance) const;

    bool operator==(const Lane &rhs) const {
      return (_road_id == rhs._road_id) &&
             (_section_id == rhs._section_id) &&
             (_lane_id == rhs._lane_id);
    }

    bool operator!=(const Lane &rhs) const {
      return !(*this == rhs);
    }

  private:

    friend class Map;
    friend class Road;
    friend class LaneSection;
    friend class Junction;
    friend class Waypoint;

    Lane(
        SharedPtr<const Map> parent,
        road::RoadId road_id,
        road::SectionId section_id,
        road::LaneId lane_id)
      : _parent(std::move(parent)),
        _road_id(road_id),
        _section_id(section_id),
        _lane_id(lane_id) {}

    /// The underlying OpenDRIVE lane. Valid only while _parent is alive.
    const road::Lane &GetLane() const;

    SharedPtr<const Map> _parent;

    road::RoadId _road_id;

    road::SectionId _section_id;

    road::LaneId _lane_id;
  };

} // namespace client
} // namespace carla
