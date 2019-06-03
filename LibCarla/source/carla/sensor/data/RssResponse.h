// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/sensor/SensorData.h"

namespace carla {
namespace sensor {
namespace data {

  enum class LateralResponse : int32_t
  {
    /*!
     * No action required.
     */
    None = 0,

    /*!
     * Vehicle has to decerate at least with brake min laterally
     */
    BrakeMin = 1
  };

  enum class LongitudinalResponse : int32_t
  {
    /*!
     * No action required.
     */
    None = 0,

    /*!
     * Vehicle has to decerate at least with brake min correct longitudinally
     */
    BrakeMinCorrect = 1,

    /*!
     * Vehicle has to decerate at least with brake min longitudinally
     */
    BrakeMin = 2
  };


  /// A RSS Response
  class RssResponse : public SensorData {
  public:

    explicit RssResponse(
        size_t frame_number,
        double timestamp,
        const rpc::Transform &sensor_transform,
        const LongitudinalResponse &longitudinal_response,
        const LateralResponse &lateral_response_right,
        const LateralResponse &lateral_response_left)
      : SensorData(frame_number, timestamp, sensor_transform),
        _longitudinal_response(longitudinal_response),
        _lateral_response_right(lateral_response_right),
        _lateral_response_left(lateral_response_left) {}

    const LongitudinalResponse &GetLongitudinalResponse() const {
      return _longitudinal_response;
    }

    const LateralResponse &GetLateralResponseRight() const {
      return _lateral_response_right;
    }

    const LateralResponse &GetLateralResponseLeft() const {
      return _lateral_response_left;
    }

  private:

    /*!
     * The current longitudinal rss response.
     */
    LongitudinalResponse _longitudinal_response;

    /*!
     * The current lateral rss response at right side in respect to ego-vehicle driving direction.
     */
    LateralResponse _lateral_response_right;

    /*!
     * The current lateral rss state at left side in respect to ego-vehicle driving direction.
     */
    LateralResponse _lateral_response_left;
  };

} // namespace data
} // namespace sensor
} // namespace carla
