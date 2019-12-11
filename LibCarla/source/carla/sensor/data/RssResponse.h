// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/sensor/SensorData.h"
#include "ad_rss/world/AccelerationRestriction.hpp"
#include "ad_rss/world/Velocity.hpp"

namespace carla {
namespace sensor {
namespace data {

  enum class LateralResponse : int32_t
  {
    /// No action required.
    None = 0,

    /// Vehicle has to decerate at least with brake min laterally
    BrakeMin = 1
  };

  enum class LongitudinalResponse : int32_t
  {
    /// No action required.
    None = 0,

    /// Vehicle has to decerate at least with brake min correct longitudinally
    BrakeMinCorrect = 1,

    /// Vehicle has to decerate at least with brake min longitudinally
    BrakeMin = 2
  };


  /// A RSS Response
  class RssResponse : public SensorData {
  public:

    explicit RssResponse(
        size_t frame_number,
        double timestamp,
        const rpc::Transform &sensor_transform,
        const bool &response_valid,
        const LongitudinalResponse &longitudinal_response,
        const LateralResponse &lateral_response_right,
        const LateralResponse &lateral_response_left,
        const ad_rss::world::AccelerationRestriction &acceleration_restriction,
        const ad_rss::world::Velocity &ego_velocity)
      : SensorData(frame_number, timestamp, sensor_transform),
        _response_valid(response_valid),
        _longitudinal_response(longitudinal_response),
        _lateral_response_right(lateral_response_right),
        _lateral_response_left(lateral_response_left),
        _acceleration_restriction(acceleration_restriction),
        _ego_velocity(ego_velocity){}

    bool GetResponseValid() const {
      return _response_valid;
    }

    const LongitudinalResponse &GetLongitudinalResponse() const {
      return _longitudinal_response;
    }

    const LateralResponse &GetLateralResponseRight() const {
      return _lateral_response_right;
    }

    const LateralResponse &GetLateralResponseLeft() const {
      return _lateral_response_left;
    }

    const ad_rss::world::AccelerationRestriction &GetAccelerationRestriction() const {
      return _acceleration_restriction;
    }

    const ad_rss::world::Velocity &GetEgoVelocity() const {
      return _ego_velocity;
    }

  private:

    /// The validity of RSS calculation.
    bool _response_valid;

    /// The current longitudinal rss response.
    LongitudinalResponse _longitudinal_response;

    /// The current lateral rss response at right side in respect to ego-vehicle driving direction.
    LateralResponse _lateral_response_right;

    /// The current lateral rss state at left side in respect to ego-vehicle driving direction.
    LateralResponse _lateral_response_left;

    ad_rss::world::AccelerationRestriction _acceleration_restriction;

    ad_rss::world::Velocity _ego_velocity;
  };

} // namespace data
} // namespace sensor
} // namespace carla
