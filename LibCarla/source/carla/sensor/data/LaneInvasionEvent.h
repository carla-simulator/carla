// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/LaneMarking.h"
#include "carla/sensor/SensorData.h"

#include <vector>

namespace carla {
namespace sensor {
namespace data {

  /// A change of lane event.
  class LaneInvasionEvent : public SensorData {
  public:

    using LaneMarking = road::element::LaneMarking;

    explicit LaneInvasionEvent(
        size_t frame,
        double timestamp,
        const rpc::Transform &sensor_transform,
        SharedPtr<client::Actor> self_actor,
        std::vector<LaneMarking> crossed_lane_markings)
      : SensorData(frame, timestamp, sensor_transform),
        _self_actor(std::move(self_actor)),
        _crossed_lane_markings(std::move(crossed_lane_markings)) {}

    /// Get "self" actor. Actor that invaded another lane.
    SharedPtr<client::Actor> GetActor() const {
      return _self_actor;
    }

    /// List of lane markings that have been crossed.
    const std::vector<LaneMarking> &GetCrossedLaneMarkings() const {
      return _crossed_lane_markings;
    }

  private:

    SharedPtr<client::Actor> _self_actor;

    std::vector<LaneMarking> _crossed_lane_markings;
  };

} // namespace data
} // namespace sensor
} // namespace carla
