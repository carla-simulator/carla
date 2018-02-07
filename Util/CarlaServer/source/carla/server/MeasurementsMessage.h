// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/NonCopyable.h"
#include "carla/server/CarlaMeasurements.h"
#include "carla/server/CarlaServerAPI.h"

namespace carla {
namespace server {

  class SensorDataInbox;

  class MeasurementsMessage : private NonCopyable {
  public:

    void Write(
        const carla_measurements &measurements,
        SensorDataInbox &sensor_inbox) {
      _measurements.Write(measurements);
      _sensor_inbox = &sensor_inbox;
    }

    const carla_measurements &measurements() const {
      return _measurements.measurements();
    }

    SensorDataInbox &sensor_inbox() const {
      DEBUG_ASSERT(_sensor_inbox != nullptr);
      return *_sensor_inbox;
    }

  private:

    CarlaMeasurements _measurements;

    SensorDataInbox *_sensor_inbox = nullptr;
  };

} // namespace server
} // namespace carla
