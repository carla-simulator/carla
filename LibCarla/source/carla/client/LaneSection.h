// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/road/Lane.h"
#include "carla/road/RoadTypes.h"

#include <vector>

namespace carla {
namespace client {

  class Map;
  class Road;
  class Lane;

  /// A stretch of road over which the lane layout does not change.
  ///
  /// An OpenDRIVE road is divided along s into lane sections; lanes are added
  /// or dropped only at a section boundary. This is why a lane's identity
  /// carries a section id, and why Lane::GetNextLanes() can step into a
  /// different section.
  class LaneSection
    : public EnableSharedFromThis<LaneSection>,
      private NonCopyable {
  public:

    road::SectionId GetId() const {
      return _section_id;
    }

    road::RoadId GetRoadId() const {
      return _road_id;
    }

    /// Unique id of this section within its map, for use as a dictionary key.
    uint64_t GetUniqueId() const;

    SharedPtr<Road> GetRoad() const;

    /// The s coordinate, along the road, at which this section begins.
    double GetDistance() const;

    /// Length of the section, in metres.
    double GetLength() const;

    /// Every lane in this section of the given type. The centre lane (id 0)
    /// carries no width and is never returned.
    std::vector<SharedPtr<Lane>> GetLanes(
        road::Lane::LaneType type = road::Lane::LaneType::Driving) const;

    /// The lane with this id, or nullptr when the section has no such lane.
    SharedPtr<Lane> GetLane(road::LaneId lane_id) const;

    bool ContainsLane(road::LaneId lane_id) const;

    /// The next section along the road, or nullptr at the end.
    SharedPtr<LaneSection> GetNextSection() const;

    /// The previous section along the road, or nullptr at the start.
    SharedPtr<LaneSection> GetPreviousSection() const;

    bool operator==(const LaneSection &rhs) const {
      return (_road_id == rhs._road_id) && (_section_id == rhs._section_id);
    }

    bool operator!=(const LaneSection &rhs) const {
      return !(*this == rhs);
    }

  private:

    friend class Map;
    friend class Road;
    friend class Lane;

    LaneSection(SharedPtr<const Map> parent, road::RoadId road_id, road::SectionId section_id)
      : _parent(std::move(parent)),
        _road_id(road_id),
        _section_id(section_id) {}

    SharedPtr<const Map> _parent;

    road::RoadId _road_id;

    road::SectionId _section_id;
  };

} // namespace client
} // namespace carla
