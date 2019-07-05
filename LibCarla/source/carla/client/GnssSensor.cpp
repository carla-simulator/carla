// Copyright (c) 2019 Intel Labs.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/GnssSensor.h"

#include "carla/Logging.h"
#include "carla/client/Map.h"
#include "carla/client/detail/Simulator.h"
#include "carla/sensor/data/GnssEvent.h"

#include <exception>

namespace carla {
namespace client {

  GnssSensor::~GnssSensor() = default;

  void GnssSensor::Listen(CallbackFunctionType callback) {
    if (_is_listening) {
      log_error(GetDisplayId(), ": already listening");
      return;
    }

    if (GetParent() == nullptr) {
      throw_exception(std::runtime_error(GetDisplayId() + ": not attached to vehicle"));
      return;
    }

    SharedPtr<Map> map = GetWorld().GetMap();
    DEBUG_ASSERT(map != nullptr);
    _geo_reference = map->GetGeoReference();

    auto self = boost::static_pointer_cast<GnssSensor>(shared_from_this());

    log_debug(GetDisplayId(), ": subscribing to tick event");
    GetEpisode().Lock()->RegisterOnTickEvent([
        cb=std::move(callback),
        weak_self=WeakPtr<GnssSensor>(self)](const auto &snapshot) {
      auto self = weak_self.lock();
      if (self != nullptr) {
        auto data = self->TickGnssSensor(snapshot.GetTimestamp());
        if (data != nullptr) {
          cb(std::move(data));
        }
      }
    });

    _is_listening = true;
  }

  SharedPtr<sensor::SensorData> GnssSensor::TickGnssSensor(
      const Timestamp &timestamp) {
    try {
      return MakeShared<sensor::data::GnssEvent>(
               timestamp.frame,
               timestamp.elapsed_seconds,
               GetTransform(),
               _geo_reference.Transform(GetLocation()));
    } catch (const std::exception &e) {
      /// @todo We need to unsubscribe the sensor.
      // log_error("GnssSensor:", e.what());
      return nullptr;
    }
  }

  void GnssSensor::Stop() {
    /// @todo We need unsubscribe from the world on tick.
    _is_listening = false;
  }

} // namespace client
} // namespace carla
