// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rss/RssSensor.h"

#include <ad/map/access/Operation.hpp>
#include <ad/rss/state/ProperResponse.hpp>
#include <ad/rss/world/Velocity.hpp>
#include <exception>
#include <fstream>

#include "carla/Logging.h"
#include "carla/client/Map.h"
#include "carla/client/Sensor.h"
#include "carla/client/Vehicle.h"
#include "carla/client/detail/Simulator.h"
#include "carla/rss/RssCheck.h"
#include "carla/sensor/data/RssResponse.h"

namespace carla {
namespace client {

std::atomic_uint RssSensor::_global_map_initialization_counter_{0u};

RssSensor::RssSensor(ActorInitializer init) : Sensor(std::move(init)), _on_tick_register_id(0u), _drop_route(false) {}

RssSensor::~RssSensor() {
  // ensure there is no processing anymore
  if (IsListening()) {
    Stop();
  }
}

void RssSensor::RegisterActorConstellationCallback(ActorConstellationCallbackFunctionType callback) {
  if (IsListening()) {
    log_error(GetDisplayId(),
              ": registering of the actor constellation callback has to be done before start listening. Register has "
              "no effect.");
    return;
  }
  _rss_actor_constellation_callback = callback;
}

void RssSensor::Listen(CallbackFunctionType callback) {
  if (IsListening()) {
    log_error(GetDisplayId(), ": already listening");
    return;
  }

  if (GetParent() == nullptr) {
    throw_exception(std::runtime_error(GetDisplayId() + ": not attached to actor"));
    return;
  }

  auto vehicle = boost::dynamic_pointer_cast<carla::client::Vehicle>(GetParent());
  if (vehicle == nullptr) {
    throw_exception(std::runtime_error(GetDisplayId() + ": parent is not a vehicle"));
    return;
  }

  // get maximum steering angle
  float max_steer_angle_deg = 0.f;
  for (auto const &wheel : vehicle->GetPhysicsControl().GetWheels()) {
    max_steer_angle_deg = std::max(max_steer_angle_deg, wheel.max_steer_angle);
  }
  auto max_steering_angle = max_steer_angle_deg * static_cast<float>(M_PI) / 180.0f;

  auto map = GetWorld().GetMap();
  DEBUG_ASSERT(map != nullptr);
  std::string const open_drive_content = map->GetOpenDrive();

  auto mapInitializationResult = ::ad::map::access::initFromOpenDriveContent(
      open_drive_content, 0.2, ::ad::map::intersection::IntersectionType::TrafficLight,
      ::ad::map::landmark::TrafficLightType::LEFT_STRAIGHT_RED_YELLOW_GREEN);

  if (!mapInitializationResult) {
    log_error(GetDisplayId(), ": Initialization of map failed");
    return;
  }

  _global_map_initialization_counter_++;

  if (_rss_actor_constellation_callback == nullptr) {
    _rss_check = std::make_shared<::carla::rss::RssCheck>(max_steering_angle);
  } else {
    _rss_check =
        std::make_shared<::carla::rss::RssCheck>(max_steering_angle, _rss_actor_constellation_callback, GetParent());
  }

  auto self = boost::static_pointer_cast<RssSensor>(shared_from_this());

  _last_processed_frame=0u;
  log_debug(GetDisplayId(), ": subscribing to tick event");
  _on_tick_register_id = GetEpisode().Lock()->RegisterOnTickEvent(
      [ cb = std::move(callback), weak_self = WeakPtr<RssSensor>(self) ](const auto &snapshot) {
        auto self = weak_self.lock();
        if (self != nullptr) {
          self->TickRssSensor(snapshot.GetTimestamp(), cb);
        }
      });
}

void RssSensor::Stop() {
  if (!IsListening()) {
    log_error(GetDisplayId(), ": not listening at all");
    return;
  }

  log_debug(GetDisplayId(), ": unsubscribing from tick event");
  GetEpisode().Lock()->RemoveOnTickEvent(_on_tick_register_id);
  _on_tick_register_id = 0u;

  if ( bool(_rss_check) ) {
    _rss_check->GetLogger()->info("RssSensor stopping");
  }
  // don't remove the braces since they protect the lock_guard
  {
     // ensure there is no processing anymore when deleting rss_check object
    const std::lock_guard<std::mutex> lock(_processing_lock);

    if ( bool(_rss_check) ) {
      _rss_check->GetLogger()->info("RssSensor delete checker");
    }
    _rss_check.reset();
    auto const map_initialization_counter_value = _global_map_initialization_counter_--;
    if (map_initialization_counter_value == 0u) {
      // last one stop listening is cleaning up the map
      ::ad::map::access::cleanup();
    }
  }
}

void RssSensor::SetLogLevel(const uint8_t &log_level) {
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. SetLogLevel has no effect.");
    return;
  }

  if (log_level < spdlog::level::n_levels) {
    _rss_check->SetLogLevel(spdlog::level::level_enum(log_level));
  }
}

void RssSensor::SetMapLogLevel(const uint8_t &map_log_level) {
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. SetMapLogLevel has no effect.");
    return;
  }

  if (map_log_level < spdlog::level::n_levels) {
    _rss_check->SetMapLogLevel(spdlog::level::level_enum(map_log_level));
  }
}

const ::ad::rss::world::RssDynamics &RssSensor::GetEgoVehicleDynamics() const {
  static auto default_vehicle_dynamics = rss::RssCheck::GetDefaultVehicleDynamics();
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. GetEgoVehicleDynamics has no effect.");
    return default_vehicle_dynamics;
  }

  if (bool(_rss_actor_constellation_callback)) {
    log_error(GetDisplayId(), ": Actor constellation callback registered. GetEgoVehicleDynamics has no effect.");
    return default_vehicle_dynamics;
  }

  return _rss_check->GetDefaultActorConstellationCallbackEgoVehicleDynamics();
}

void RssSensor::SetEgoVehicleDynamics(const ::ad::rss::world::RssDynamics &ego_dynamics) {
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. SetEgoVehicleDynamics has no effect.");
    return;
  }

  if (bool(_rss_actor_constellation_callback)) {
    log_error(GetDisplayId(), ": Actor constellation callback registered. SetEgoVehicleDynamics has no effect.");
    return;
  }

  _rss_check->SetDefaultActorConstellationCallbackEgoVehicleDynamics(ego_dynamics);
}

const ::ad::rss::world::RssDynamics &RssSensor::GetOtherVehicleDynamics() const {
  static auto default_vehicle_dynamics = rss::RssCheck::GetDefaultVehicleDynamics();
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. GetOtherVehicleDynamics has no effect.");
    return default_vehicle_dynamics;
  }

  if (bool(_rss_actor_constellation_callback)) {
    log_error(GetDisplayId(), ": Actor constellation callback registered. GetOtherVehicleDynamics has no effect.");
    return default_vehicle_dynamics;
  }

  return _rss_check->GetDefaultActorConstellationCallbackOtherVehicleDynamics();
}

void RssSensor::SetOtherVehicleDynamics(const ::ad::rss::world::RssDynamics &other_vehicle_dynamics) {
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. SetOtherVehicleDynamics has no effect.");
    return;
  }

  if (bool(_rss_actor_constellation_callback)) {
    log_error(GetDisplayId(), ": Actor constellation callback registered. SetOtherVehicleDynamics has no effect.");
    return;
  }

  _rss_check->SetDefaultActorConstellationCallbackOtherVehicleDynamics(other_vehicle_dynamics);
}

const ::ad::rss::world::RssDynamics &RssSensor::GetPedestrianDynamics() const {
  static auto default_pedestrian_dynamics = rss::RssCheck::GetDefaultPedestrianDynamics();
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. GetPedestrianDynamics has no effect.");
    return default_pedestrian_dynamics;
  }

  if (bool(_rss_actor_constellation_callback)) {
    log_error(GetDisplayId(), ": Actor constellation callback registered. GetPedestrianDynamics has no effect.");
    return default_pedestrian_dynamics;
  }

  return _rss_check->GetDefaultActorConstellationCallbackPedestrianDynamics();
}

void RssSensor::SetPedestrianDynamics(const ::ad::rss::world::RssDynamics &pedestrian_dynamics) {
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. SetPedestrianDynamics has no effect.");
    return;
  }

  if (bool(_rss_actor_constellation_callback)) {
    log_error(GetDisplayId(), ": Actor constellation callback registered. SetPedestrianDynamics has no effect.");
    return;
  }

  _rss_check->SetDefaultActorConstellationCallbackPedestrianDynamics(pedestrian_dynamics);
}

const ::carla::rss::RoadBoundariesMode &RssSensor::GetRoadBoundariesMode() const {
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. GetRoadBoundariesMode has no effect.");
    static auto default_road_boundaries_mode = rss::RssCheck::GetDefaultRoadBoundariesMode();
    return default_road_boundaries_mode;
  }

  return _rss_check->GetRoadBoundariesMode();
}

void RssSensor::SetRoadBoundariesMode(const ::carla::rss::RoadBoundariesMode &road_boundaries_mode) {
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. SetRoadBoundariesMode has no effect.");
    return;
  }

  _rss_check->SetRoadBoundariesMode(road_boundaries_mode);
}

void RssSensor::AppendRoutingTarget(const ::carla::geom::Transform &routing_target) {
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. AppendRoutingTarget has no effect.");
    return;
  }

  _rss_check->AppendRoutingTarget(routing_target);
}

const std::vector<::carla::geom::Transform> RssSensor::GetRoutingTargets() const {
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. GetRoutingTargets has no effect.");
    return std::vector<::carla::geom::Transform>();
  }

  return _rss_check->GetRoutingTargets();
}

void RssSensor::ResetRoutingTargets() {
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. ResetRoutingTargets has no effect.");
    return;
  }

  _rss_check->ResetRoutingTargets();
}

void RssSensor::DropRoute() {
  // don't execute this immediately as it might break calculations completely
  // postpone to next sensor tick
  _drop_route = true;
}

void RssSensor::TickRssSensor(const client::Timestamp &timestamp, CallbackFunctionType callback) {
  if (_processing_lock.try_lock()) {
    if (!bool(_rss_check)){
      _processing_lock.unlock();
      return;
    }
    if ((timestamp.frame < _last_processed_frame) && ((_last_processed_frame - timestamp.frame) < 0xffffffffu)) {
      _processing_lock.unlock();
      _rss_check->GetLogger()->warn("RssSensor[{}] outdated tick dropped, LastProcessed={}", timestamp.frame, _last_processed_frame);
      return;
    }
    _last_processed_frame = timestamp.frame;
    SharedPtr<carla::client::ActorList> actors = GetWorld().GetActors();

    auto const settings = GetWorld().GetSettings();
    if ( settings.synchronous_mode ) {
      _rss_check->GetLogger()->trace("RssSensor[{}] sync-tick", timestamp.frame);
      TickRssSensorThreadLocked(timestamp, actors, callback);
    }
    else {
      // store the future to prevent the destructor of the future from blocked waiting
      _rss_check->GetLogger()->trace("RssSensor[{}] async-tick", timestamp.frame);
      _tick_future = std::async(&RssSensor::TickRssSensorThreadLocked, this, timestamp, actors, callback);
    }
  } else {
    if (bool(_rss_check)){
      _rss_check->GetLogger()->debug("RssSensor[{}] tick dropped", timestamp.frame);
    }
  }
}

void RssSensor::TickRssSensorThreadLocked(const client::Timestamp &timestamp,
                                          SharedPtr<carla::client::ActorList> actors, CallbackFunctionType callback) {
  try {
    double const time_since_epoch_check_start_ms =
        std::chrono::duration<double, std::milli>(std::chrono::system_clock::now().time_since_epoch()).count();

    if (_drop_route) {
      _drop_route = false;
      _rss_check->DropRoute();
    }

    // check all object<->ego pairs with RSS and calculate proper response
    ::ad::rss::state::ProperResponse response;
    ::ad::rss::state::RssStateSnapshot rss_state_snapshot;
    ::ad::rss::situation::SituationSnapshot situation_snapshot;
    ::ad::rss::world::WorldModel world_model;
    carla::rss::EgoDynamicsOnRoute ego_dynamics_on_route;
    auto const result = _rss_check->CheckObjects(timestamp, actors, GetParent(), response, rss_state_snapshot,
                                                             situation_snapshot, world_model, ego_dynamics_on_route);

    double const time_since_epoch_check_end_ms =
        std::chrono::duration<double, std::milli>(std::chrono::system_clock::now().time_since_epoch()).count();
    auto const delta_time_ms = time_since_epoch_check_end_ms - time_since_epoch_check_start_ms;
    _rss_check->GetLogger()->debug("RssSensor[{}] response: S:{}->E:{} DeltaT:{}", timestamp.frame,
                                  time_since_epoch_check_start_ms, time_since_epoch_check_end_ms,
                                  delta_time_ms);
    _rss_check_timings.push_back(delta_time_ms);
    while (_rss_check_timings.size() > 10u) {
      _rss_check_timings.pop_front();
    }
    double agv_time=0.;
    for (auto run_time: _rss_check_timings) {
      agv_time += run_time;
    }
    agv_time /= _rss_check_timings.size();
    _rss_check->GetLogger()->info("RssSensor[{}] runtime {} avg {}", timestamp.frame, delta_time_ms, agv_time);
    _processing_lock.unlock();

    callback(MakeShared<sensor::data::RssResponse>(timestamp.frame, timestamp.elapsed_seconds, GetTransform(), result,
                                                 response, rss_state_snapshot, situation_snapshot, world_model,
                                                   ego_dynamics_on_route));
  } catch (const std::exception &e) {
    _rss_check->GetLogger()->error("RssSensor[{}] tick exception", timestamp.frame);
    _processing_lock.unlock();
  } catch (...) {
    _rss_check->GetLogger()->error("RssSensor[{}] tick exception", timestamp.frame);
    _processing_lock.unlock();
  }
}

}  // namespace client
}  // namespace carla
