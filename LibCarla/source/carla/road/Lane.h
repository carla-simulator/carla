// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Mesh.h"
#include "carla/geom/Transform.h"
#include "carla/road/InformationSet.h"
#include "carla/road/RoadTypes.h"

#include <vector>
#include <iostream>
#include <memory>

namespace carla {
namespace road {

  class LaneSection;
  class MapBuilder;
  class Road;

  class Lane : private MovableNonCopyable {
  public:

    /// Can be used as flags
    enum class LaneType : int32_t {
      None          = 0x1,
      Driving       = 0x1 << 1,
      Stop          = 0x1 << 2,
      Shoulder      = 0x1 << 3,
      Biking        = 0x1 << 4,
      Sidewalk      = 0x1 << 5,
      Border        = 0x1 << 6,
      Restricted    = 0x1 << 7,
      Parking       = 0x1 << 8,
      Bidirectional = 0x1 << 9,
      Median        = 0x1 << 10,
      Special1      = 0x1 << 11,
      Special2      = 0x1 << 12,
      Special3      = 0x1 << 13,
      RoadWorks     = 0x1 << 14,
      Tram          = 0x1 << 15,
      Rail          = 0x1 << 16,
      Entry         = 0x1 << 17,
      Exit          = 0x1 << 18,
      OffRamp       = 0x1 << 19,
      OnRamp        = 0x1 << 20,
      Any           = -2 // 0xFFFFFFFE
    };

  public:

    Lane() = default;

    Lane(
        LaneSection *lane_section,
        LaneId id,
        std::vector<std::unique_ptr<element::RoadInfo>> &&info)
      : _lane_section(lane_section),
        _id(id),
        _info(std::move(info)) {
      DEBUG_ASSERT(lane_section != nullptr);
    }

    const LaneSection *GetLaneSection() const;

    Road *GetRoad() const;

    LaneId GetId() const;

    LaneType GetType() const;

    bool GetLevel() const;

    template <typename T>
    const T *GetInfo(const double s) const {
      DEBUG_ASSERT(_lane_section != nullptr);
      return _info.GetInfo<T>(s);
    }

    template <typename T>
    std::vector<const T*> GetInfos() const {
      DEBUG_ASSERT(_lane_section != nullptr);
      return _info.GetInfos<T>();
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

    double GetDistance() const;

    double GetLength() const;

    /// Returns the total lane width given a s
    double GetWidth(const double s) const;

    /// Checks whether the geometry is straight or not
    bool IsStraight() const;

    geom::Transform ComputeTransform(const double s) const;

    /// Computes the location of the edges given a s
    std::pair<geom::Vector3D, geom::Vector3D> GetCornerPositions(
      const double s, const float extra_width = 0.f) const;

  private:

    friend MapBuilder;

    LaneSection *_lane_section = nullptr;

    LaneId _id = 0;

    InformationSet _info;

    LaneType _type = LaneType::None;

    bool _level = false;

    LaneId _successor = 0;

    LaneId _predecessor = 0;

    std::vector<Lane *> _next_lanes;

    std::vector<Lane *> _prev_lanes;
  };

} // road
} // carla
