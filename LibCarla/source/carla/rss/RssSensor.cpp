// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rss/RssSensor.h"

#include <ad/map/access/Operation.hpp>
#include <ad/rss/state/ProperResponse.hpp>
#include <ad/rss/world/AccelerationRestriction.hpp>
#include <ad/rss/world/Velocity.hpp>
#include <exception>
#include <fstream>

#include "carla/Logging.h"
#include "carla/client/Map.h"
#include "carla/client/Sensor.h"
#include "carla/client/detail/Simulator.h"
#include "carla/rss/RssCheck.h"
#include "carla/sensor/data/RssResponse.h"

namespace carla {
namespace client {

RssSensor::RssSensor(ActorInitializer init) : Sensor(std::move(init)), _on_tick_register_id(0u), _drop_route(false) {}

RssSensor::~RssSensor() = default;

void RssSensor::Listen(CallbackFunctionType callback) {
  if (IsListening()) {
    log_error(GetDisplayId(), ": already listening");
    return;
  }

  if (GetParent() == nullptr) {
    throw_exception(std::runtime_error(GetDisplayId() + ": not attached to vehicle"));
    return;
  }

  _rss_check = std::make_shared<::carla::rss::RssCheck>();

  auto map = GetWorld().GetMap();
  DEBUG_ASSERT(map != nullptr);
  std::string const open_drive_content = map->GetOpenDrive();

  ::ad::map::access::cleanup();
  ::ad::map::access::initFromOpenDriveContent(open_drive_content, 0.2,
                                              ::ad::map::intersection::IntersectionType::TrafficLight,
                                              ::ad::map::landmark::TrafficLightType::LEFT_STRAIGHT_RED_YELLOW_GREEN);

  auto self = boost::static_pointer_cast<RssSensor>(shared_from_this());

  log_debug(GetDisplayId(), ": subscribing to tick event");
  _on_tick_register_id = GetEpisode().Lock()->RegisterOnTickEvent(
      [ cb = std::move(callback), weak_self = WeakPtr<RssSensor>(self) ](const auto &snapshot) {
        auto self = weak_self.lock();
        if (self != nullptr) {
          auto data = self->TickRssSensor(snapshot.GetTimestamp());
          if (data != nullptr) {
            cb(std::move(data));
          }
        }
      });
}

void RssSensor::Stop() {
  if (!IsListening()) {
    log_error(GetDisplayId(), ": not listening at all");
    return;
  }

  // ensure there is no processing anymore
  const std::lock_guard<std::mutex> lock(_processing_lock);

  log_debug(GetDisplayId(), ": unsubscribing from tick event");
  GetEpisode().Lock()->RemoveOnTickEvent(_on_tick_register_id);
  _on_tick_register_id = 0u;

  ::ad::map::access::cleanup();
}

const ::ad::rss::world::RssDynamics &RssSensor::GetEgoVehicleDynamics() const {
  if (bool(_rss_check)) {
    return _rss_check->GetEgoVehicleDynamics();
  } else {
    static auto default_vehicle_dynamics = rss::RssCheck::GetDefaultVehicleDynamics();
    return default_vehicle_dynamics;
  }
}

void RssSensor::SetEgoVehicleDynamics(const ::ad::rss::world::RssDynamics &ego_dynamics) {
  if (bool(_rss_check)) {
    _rss_check->SetEgoVehicleDynamics(ego_dynamics);
  }
}

const ::ad::rss::world::RssDynamics &RssSensor::GetOtherVehicleDynamics() const {
  if (bool(_rss_check)) {
    return _rss_check->GetOtherVehicleDynamics();
  } else {
    static auto default_vehicle_dynamics = rss::RssCheck::GetDefaultVehicleDynamics();
    return default_vehicle_dynamics;
  }
}

void RssSensor::SetOtherVehicleDynamics(const ::ad::rss::world::RssDynamics &other_vehicle_dynamics) {
  if (bool(_rss_check)) {
    _rss_check->SetOtherVehicleDynamics(other_vehicle_dynamics);
  }
}

const ::carla::rss::RoadBoundariesMode &RssSensor::GetRoadBoundariesMode() const {
  if (bool(_rss_check)) {
    return _rss_check->GetRoadBoundariesMode();
  } else {
    static auto default_road_boundaries_mode = rss::RssCheck::GetDefaultRoadBoundariesMode();
    return default_road_boundaries_mode;
  }
}

void RssSensor::SetRoadBoundariesMode(const ::carla::rss::RoadBoundariesMode &road_boundaries_mode) {
  if (bool(_rss_check)) {
    _rss_check->SetRoadBoundariesMode(road_boundaries_mode);
  }
}

void RssSensor::AppendRoutingTarget(const ::carla::geom::Transform &routing_target) {
  if (bool(_rss_check)) {
    _rss_check->AppendRoutingTarget(routing_target);
  }
}

const std::vector<::carla::geom::Transform> RssSensor::GetRoutingTargets() const {
  if (bool(_rss_check)) {
    return _rss_check->GetRoutingTargets();
  } else {
    return std::vector<::carla::geom::Transform>();
  }
}

void RssSensor::ResetRoutingTargets() {
  if (bool(_rss_check)) {
    _rss_check->ResetRoutingTargets();
  }
}

void RssSensor::SetVisualizationMode(const ::carla::rss::VisualizationMode &visualization_mode) {
  if (bool(_rss_check)) {
    _rss_check->SetVisualizationMode(visualization_mode);
  }
}

const ::carla::rss::VisualizationMode &RssSensor::GetVisualizationMode() const {
  if (bool(_rss_check)) {
    return _rss_check->GetVisualizationMode();
  } else {
    static auto default_visualization_mode = rss::RssCheck::GetDefaultVisualizationMode();
    return default_visualization_mode;
  }
}

void RssSensor::DropRoute() {
  // don't execute this immediately as it might break calculations completely
  // postpone to next sensor tick
  _drop_route = true;
}

SharedPtr<sensor::SensorData> RssSensor::TickRssSensor(const Timestamp &timestamp) {
  try {
    bool result = false;
    ::ad::rss::state::ProperResponse response;
    ::ad::rss::world::AccelerationRestriction acceleration_restriction;
    ::ad::rss::state::RssStateSnapshot rss_state_snapshot;
    carla::rss::EgoDynamicsOnRoute ego_dynamics_on_route;
    if (_processing_lock.try_lock()) {
      spdlog::debug("RssSensor tick: T={}", timestamp.frame);
      carla::client::World world = GetWorld();
      SharedPtr<carla::client::ActorList> actors = world.GetActors();

      if (_drop_route) {
        _drop_route = false;
        _rss_check->DropRoute();
      }

      // check all object<->ego pairs with RSS and calculate proper response
      result = _rss_check->CheckObjects(timestamp, actors, GetParent(), response, acceleration_restriction,
                                        rss_state_snapshot, ego_dynamics_on_route);
      _processing_lock.unlock();

      _rss_check->VisualizeResults(world, GetParent());

      spdlog::debug(
          "RssSensor response: T={} S:{}->E:{} DeltaT:{}", timestamp.frame,
          ego_dynamics_on_route.time_since_epoch_check_start_ms, ego_dynamics_on_route.time_since_epoch_check_end_ms,
          ego_dynamics_on_route.time_since_epoch_check_end_ms - ego_dynamics_on_route.time_since_epoch_check_start_ms);
      return MakeShared<sensor::data::RssResponse>(timestamp.frame, timestamp.elapsed_seconds, GetTransform(), result,
                                                   response, acceleration_restriction, rss_state_snapshot,
                                                   ego_dynamics_on_route);
    } else {
      spdlog::debug("RssSensor tick dropped: T={}", timestamp.frame);
      return nullptr;
    }
  } catch (const std::exception &e) {
    /// @todo do we need to unsubscribe the sensor here?
    std::cout << e.what() << std::endl;
    _processing_lock.unlock();
    spdlog::error("RssSensor tick exception");
    return nullptr;
  } catch (...) {
    _processing_lock.unlock();
    spdlog::error("RssSensor tick exception");
    return nullptr;
  }
}

}  // namespace client
}  // namespace carla
