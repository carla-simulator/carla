// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/LaneSection.h"

#include "carla/client/Lane.h"
#include "carla/client/Map.h"
#include "carla/client/Road.h"
#include "carla/road/LaneSection.h"
#include "carla/road/Road.h"

#include <algorithm>
#include <vector>

namespace carla {
namespace client {

  // Sections ordered by their s coordinate, so "next" and "previous" mean
  // along the road rather than by id.
  static std::vector<const road::LaneSection *> SectionsByDistance(const road::Road &road) {
    std::vector<const road::LaneSection *> sections;
    for (const road::LaneSection &section : road.GetLaneSections()) {
      sections.emplace_back(&section);
    }
    std::sort(sections.begin(), sections.end(),
        [](const road::LaneSection *lhs, const road::LaneSection *rhs) {
          return lhs->GetDistance() < rhs->GetDistance();
        });
    return sections;
  }

  uint64_t LaneSection::GetUniqueId() const {
    return (static_cast<uint64_t>(_road_id) << 32) ^ static_cast<uint64_t>(_section_id);
  }

  SharedPtr<Road> LaneSection::GetRoad() const {
    return _parent->GetRoad(_road_id);
  }

  double LaneSection::GetDistance() const {
    return _parent->GetMap().GetData().GetRoad(_road_id)
        .GetLaneSectionById(_section_id).GetDistance();
  }

  double LaneSection::GetLength() const {
    return _parent->GetMap().GetData().GetRoad(_road_id)
        .GetLaneSectionById(_section_id).GetLength();
  }

  std::vector<SharedPtr<Lane>> LaneSection::GetLanes(road::Lane::LaneType type) const {
    std::vector<SharedPtr<Lane>> result;
    const road::LaneSection &section =
        _parent->GetMap().GetData().GetRoad(_road_id).GetLaneSectionById(_section_id);
    for (const auto &pair : section.GetLanes()) {
      const road::Lane &lane = pair.second;
      // The centre lane has no width and cannot be driven or walked on.
      if (lane.GetId() == 0) {
        continue;
      }
      if ((static_cast<int32_t>(lane.GetType()) & static_cast<int32_t>(type)) == 0) {
        continue;
      }
      result.emplace_back(_parent->GetLane(_road_id, _section_id, lane.GetId()));
    }
    return result;
  }

  SharedPtr<Lane> LaneSection::GetLane(road::LaneId lane_id) const {
    return _parent->GetLane(_road_id, _section_id, lane_id);
  }

  bool LaneSection::ContainsLane(road::LaneId lane_id) const {
    return _parent->GetMap().GetData().GetRoad(_road_id)
        .GetLaneSectionById(_section_id).ContainsLane(lane_id);
  }

  SharedPtr<LaneSection> LaneSection::GetNextSection() const {
    const road::Road &road = _parent->GetMap().GetData().GetRoad(_road_id);
    const auto sections = SectionsByDistance(road);
    for (size_t i = 0u; i < sections.size(); ++i) {
      if (sections[i]->GetId() == _section_id) {
        if (i + 1u < sections.size()) {
          return _parent->GetLaneSection(_road_id, sections[i + 1u]->GetId());
        }
        break;
      }
    }
    return nullptr;
  }

  SharedPtr<LaneSection> LaneSection::GetPreviousSection() const {
    const road::Road &road = _parent->GetMap().GetData().GetRoad(_road_id);
    const auto sections = SectionsByDistance(road);
    for (size_t i = 0u; i < sections.size(); ++i) {
      if (sections[i]->GetId() == _section_id) {
        if (i > 0u) {
          return _parent->GetLaneSection(_road_id, sections[i - 1u]->GetId());
        }
        break;
      }
    }
    return nullptr;
  }

} // namespace client
} // namespace carla
