// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/InformationSet.h"
#include "carla/road/RoadTypes.h"

#include <vector>
#include <memory>

namespace carla {
namespace road {

  class LaneSection;
  class MapBuilder;
  class Road;

  class Lane : private MovableNonCopyable {
  public:

    Lane() = default;

    Lane(
      LaneSection *lane_section,
      LaneId id,
      std::vector<std::unique_ptr<element::RoadInfo>> &&info)
      : _lane_section(lane_section),
        _id(id),
        _info(std::move(info)) {}

    Lane(Lane &&) = default;
    Lane &operator=(Lane &&) = default;

    const LaneSection *GetLaneSection() const;

    const Road *GetRoad() const;

    LaneId GetId() const;

    std::string GetType() const;

    bool GetLevel() const;

    template <typename T>
    const T *GetInfo (const float s) const {
      if (_lane_section != nullptr) {
        return _info.GetInfo<T>(s - GetDistance());
      }
      return nullptr;
    }

    const std::vector<Lane *> &GetNextLanes() const {
      return _next_lanes;
    }

    const std::vector<Lane *> &GetPreviousLanes() const {
      return _prev_lanes;
    }

    LaneId GetSuccessor() const {
      return _successor;
    }

    LaneId GetPredecessor() const {
      return _predecessor;
    }

    float GetDistance() const;

    float GetLength() const;

  private:

    friend MapBuilder;

    LaneSection *_lane_section;

    LaneId _id;

    InformationSet _info;

    /// @todo: change to enum, see 6.5 of OpenDRIVEFormatSpecRev1.4H.pdf
    std::string _type;

    bool _level;

    LaneId _successor;
    LaneId _predecessor;

    std::vector<Lane *> _next_lanes;

    std::vector<Lane *> _prev_lanes;

  };

} // road
} // carla
