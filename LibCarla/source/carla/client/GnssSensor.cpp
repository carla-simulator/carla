// Copyright (c) 2019 Intel Labs.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/GnssSensor.h"

#include "carla/Logging.h"
#include "carla/client/Map.h"
#include "carla/client/detail/Simulator.h"
#include "carla/geom/GeoLocation.h"
#include "carla/sensor/data/GnssEvent.h"

#include <exception>

namespace carla {
namespace client {

  // ===========================================================================
  // -- GnssCallback -----------------------------------------------------------
  // ===========================================================================

  class GnssCallback {
  public:

    GnssCallback(
        ActorId sensor_id,
        geom::GeoLocation geo_reference,
        Sensor::CallbackFunctionType &&user_callback)
      : _sensor_id(sensor_id),
        _geo_reference(geo_reference),
        _callback(std::move(user_callback)) {}

    void operator()(const WorldSnapshot &snapshot) const;

  private:

    ActorId _sensor_id;

    geom::GeoLocation _geo_reference;

    Sensor::CallbackFunctionType _callback;
  };

  void GnssCallback::operator()(const WorldSnapshot &snapshot) const {
    try {
      auto actor_snapshot = snapshot.Find(_sensor_id);
      if (actor_snapshot.has_value()) {
        auto transform = actor_snapshot->transform;
        _callback(MakeShared<sensor::data::GnssEvent>(
            snapshot.GetTimestamp().frame,
            snapshot.GetTimestamp().elapsed_seconds,
            transform,
            _geo_reference.Transform(transform.location)));
      }
    } catch (const std::exception &e) {
      log_error("GnssSensor:", e.what());
    }
  }

  // ===========================================================================
  // -- GnssSensor -------------------------------------------------------------
  // ===========================================================================

  GnssSensor::~GnssSensor() {
    Stop();
  }

  void GnssSensor::Listen(CallbackFunctionType callback) {
    auto episode = GetEpisode().Lock();
    SharedPtr<Map> map = episode->GetCurrentMap();
    DEBUG_ASSERT(map != nullptr);

    const size_t callback_id = episode->RegisterOnTickEvent(GnssCallback(
        GetId(),
        map->GetGeoReference(),
        std::move(callback)));

    const size_t previous = _callback_id.exchange(callback_id);
    if (previous != 0u) {
      episode->RemoveOnTickEvent(previous);
    }
  }

  void GnssSensor::Stop() {
    const size_t previous = _callback_id.exchange(0u);
    auto episode = GetEpisode().TryLock();
    if ((previous != 0u) && (episode != nullptr)) {
      episode->RemoveOnTickEvent(previous);
    }
  }

} // namespace client
} // namespace carla
