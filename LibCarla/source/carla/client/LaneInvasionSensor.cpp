// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/LaneInvasionSensor.h"

#include "carla/Logging.h"
#include "carla/client/Map.h"
#include "carla/client/Vehicle.h"
#include "carla/client/detail/Simulator.h"
#include "carla/geom/Math.h"
#include "carla/sensor/data/LaneInvasionEvent.h"

#include <exception>

namespace carla {
namespace client {

  static geom::Location Rotate(float yaw, const geom::Location &location) {
    yaw *= geom::Math::Pi<float>() / 180.0f;
    const float c = std::cos(yaw);
    const float s = std::sin(yaw);
    return {
        c * location.x - s * location.y,
        s * location.x + c * location.y,
        location.z};
  }

  static std::array<geom::Location, 4u> GetVehicleBounds(const Vehicle &vehicle) {
    const auto &box = vehicle.GetBoundingBox();
    const auto &transform = vehicle.GetTransform();
    const auto location = transform.location + box.location;
    const auto yaw = transform.rotation.yaw;
    return {
        location + Rotate(yaw, geom::Location( box.extent.x,  box.extent.y, 0.0f)),
        location + Rotate(yaw, geom::Location(-box.extent.x,  box.extent.y, 0.0f)),
        location + Rotate(yaw, geom::Location( box.extent.x, -box.extent.y, 0.0f)),
        location + Rotate(yaw, geom::Location(-box.extent.x, -box.extent.y, 0.0f))};
  }

  LaneInvasionSensor::~LaneInvasionSensor() = default;

  void LaneInvasionSensor::Listen(CallbackFunctionType callback) {
    if (_is_listening) {
      log_error(GetDisplayId(), ": already listening");
      return;
    }

    _vehicle = boost::dynamic_pointer_cast<Vehicle>(GetParent());
    if (_vehicle == nullptr) {
      log_error(GetDisplayId(), ": not attached to vehicle");
      return;
    }

    _map = GetWorld().GetMap();
    DEBUG_ASSERT(_map != nullptr);

    auto self = boost::static_pointer_cast<LaneInvasionSensor>(shared_from_this());

    log_debug(GetDisplayId(), ": subscribing to tick event");
    GetEpisode().Lock()->RegisterOnTickEvent([
        cb=std::move(callback),
        weak_self=WeakPtr<LaneInvasionSensor>(self)](const auto &snapshot) {
      auto self = weak_self.lock();
      if (self != nullptr) {
        auto data = self->TickLaneInvasionSensor(snapshot.GetTimestamp());
        if (data != nullptr) {
          cb(std::move(data));
        }
      }
    });
    _is_listening = true;
  }

  void LaneInvasionSensor::Stop() {
    /// @todo We need unsubscribe from the world on tick.
    _is_listening = false;
  }

  SharedPtr<sensor::SensorData> LaneInvasionSensor::TickLaneInvasionSensor(
      const Timestamp &timestamp) {
    try {
      const auto new_bounds = GetVehicleBounds(*_vehicle);
      std::vector<road::element::LaneMarking> crossed_lanes;
      for (auto i = 0u; i < _bounds.size(); ++i) {
        const auto lanes = _map->CalculateCrossedLanes(_bounds[i], new_bounds[i]);
        crossed_lanes.insert(crossed_lanes.end(), lanes.begin(), lanes.end());
      }
      _bounds = new_bounds;
      return crossed_lanes.empty() ?
          nullptr :
          MakeShared<sensor::data::LaneInvasionEvent>(
              timestamp.frame,
              timestamp.elapsed_seconds,
              _vehicle->GetTransform(),
              _vehicle,
              crossed_lanes);
    } catch (const std::exception &e) {
      /// @todo We need to unsubscribe the sensor.
      // log_error("LaneInvasionSensor:", e.what());
      return nullptr;
    }
  }

} // namespace client
} // namespace carla
