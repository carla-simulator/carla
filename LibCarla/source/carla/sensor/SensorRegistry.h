// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/sensor/CompositeSerializer.h"

#include "carla/sensor/s11n/ImageSerializer.h"

class ASceneCaptureCamera;

namespace carla {
namespace sensor {

  using SensorRegistry = CompositeSerializer<
    std::pair<ASceneCaptureCamera *, s11n::ImageSerializer>
  >;

} // namespace sensor
} // namespace carla
