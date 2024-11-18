// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/LaneInvasionSensor.h"

#include "carla/Logging.h"
#include "carla/client/Map.h"
#include "carla/client/Vehicle.h"
#include "carla/client/detail/Simulator.h"
#include "carla/geom/Location.h"
#include "carla/geom/Math.h"
#include "carla/sensor/data/LaneInvasionEvent.h"

#include <exception>

namespace carla {
namespace client {

  // ===========================================================================
  // -- Static local methods ---------------------------------------------------
  // ===========================================================================

  static geom::Location Rotate(float yaw, const geom::Location &location) {
    yaw *= geom::Math::Pi<float>() / 180.0f;
    const float c = std::cos(yaw);
    const float s = std::sin(yaw);
    return {
        c * location.x - s * location.y,
        s * location.x + c * location.y,
        location.z};
  }

  // ===========================================================================
  // -- LaneInvasionCallback ---------------------------------------------------
  // ===========================================================================

  class LaneInvasionCallback {
  public:

    LaneInvasionCallback(
        const Vehicle &vehicle,
        SharedPtr<Map> &&map,
        Sensor::CallbackFunctionType &&user_callback)
      : _parent(vehicle.GetId()),
        _parent_bounding_box(vehicle.GetBoundingBox()),
        _map(std::move(map)),
        _callback(std::move(user_callback)) {
      DEBUG_ASSERT(_map != nullptr);
    }

    void Tick(const WorldSnapshot &snapshot) const;

  private:

    struct Bounds {
      size_t frame;
      std::array<geom::Location, 4u> corners;
    };

    std::shared_ptr<const Bounds> MakeBounds(
        size_t frame,
        const geom::Transform &vehicle_transform) const;

    ActorId _parent;

    geom::BoundingBox _parent_bounding_box;

    SharedPtr<const Map> _map;

    Sensor::CallbackFunctionType _callback;

    mutable AtomicSharedPtr<const Bounds> _bounds;
  };

  void LaneInvasionCallback::Tick(const WorldSnapshot &snapshot) const {
    // Make sure the parent is alive.
    auto parent = snapshot.Find(_parent);
    if (!parent) {
      return;
    }

    auto next = MakeBounds(snapshot.GetFrame(), parent->transform);
    auto prev = _bounds.load();

    // First frame it'll be null.
    if ((prev == nullptr) && _bounds.compare_exchange(&prev, next)) {
      return;
    }

    // Make sure the distance is long enough.
    constexpr float distance_threshold = 10.0f * std::numeric_limits<float>::epsilon();
    for (auto i = 0u; i < 4u; ++i) {
      if ((next->corners[i] - prev->corners[i]).Length() < distance_threshold) {
        return;
      }
    }

    // Make sure the current frame is up-to-date.
    do {
      if (prev->frame >= next->frame) {
        return;
      }
    } while (!_bounds.compare_exchange(&prev, next));

    // Finally it's safe to compute the crossed lanes.
    std::vector<road::element::LaneMarking> crossed_lanes;
    for (auto i = 0u; i < 4u; ++i) {
      const auto lanes = _map->CalculateCrossedLanes(prev->corners[i], next->corners[i]);
      crossed_lanes.insert(crossed_lanes.end(), lanes.begin(), lanes.end());
    }

    if (!crossed_lanes.empty()) {
      _callback(MakeShared<sensor::data::LaneInvasionEvent>(
          snapshot.GetTimestamp().frame,
          snapshot.GetTimestamp().elapsed_seconds,
          parent->transform,
          _parent,
          std::move(crossed_lanes)));
    }
  }

  std::shared_ptr<const LaneInvasionCallback::Bounds> LaneInvasionCallback::MakeBounds(
      const size_t frame,
      const geom::Transform &transform) const {
    const auto &box = _parent_bounding_box;
    const auto location = transform.location + box.location;
    const auto yaw = transform.rotation.yaw;
    return std::make_shared<Bounds>(Bounds{frame, {
        location + Rotate(yaw, geom::Location( box.extent.x,  box.extent.y, 0.0f)),
        location + Rotate(yaw, geom::Location(-box.extent.x,  box.extent.y, 0.0f)),
        location + Rotate(yaw, geom::Location( box.extent.x, -box.extent.y, 0.0f)),
        location + Rotate(yaw, geom::Location(-box.extent.x, -box.extent.y, 0.0f))}});
  }

  // ===========================================================================
  // -- LaneInvasionSensor -----------------------------------------------------
  // ===========================================================================

  LaneInvasionSensor::~LaneInvasionSensor() {
    Stop();
  }

  void LaneInvasionSensor::Listen(CallbackFunctionType callback) {
    auto vehicle = std::dynamic_pointer_cast<Vehicle>(GetParent());
    if (vehicle == nullptr) {
      log_error(GetDisplayId(), ": not attached to a vehicle");
      return;
    }

    auto episode = GetEpisode().Lock();
    
    auto cb = std::make_shared<LaneInvasionCallback>(
        *vehicle,
        episode->GetCurrentMap(),
        std::move(callback));

    const size_t callback_id = episode->RegisterOnTickEvent([cb=std::move(cb)](const auto &snapshot) {
      try {
        cb->Tick(snapshot);
      } catch (const std::exception &e) {
        log_error("LaneInvasionSensor:", e.what());
      }
    });

    const size_t previous = _callback_id.exchange(callback_id);
    if (previous != 0u) {
      episode->RemoveOnTickEvent(previous);
    }
  }

  void LaneInvasionSensor::Stop() {
    const size_t previous = _callback_id.exchange(0u);
    auto episode = GetEpisode().TryLock();
    if ((previous != 0u) && (episode != nullptr)) {
      episode->RemoveOnTickEvent(previous);
    }
  }

} // namespace client
} // namespace carla
