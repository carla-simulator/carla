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

RssSensor::RssSensor(ActorInitializer init) : Sensor(std::move(init)), _on_tick_register_id(0u), _drop_route(false) {}

RssSensor::~RssSensor() {
  // ensure there is no processing anymore when deleting rss_check object
  const std::lock_guard<std::mutex> lock(_processing_lock);
  _rss_check.reset();
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

  _rss_check = nullptr;
  ::ad::map::access::cleanup();

  ::ad::map::access::initFromOpenDriveContent(open_drive_content, 0.2,
                                              ::ad::map::intersection::IntersectionType::TrafficLight,
                                              ::ad::map::landmark::TrafficLightType::LEFT_STRAIGHT_RED_YELLOW_GREEN);

  if (_rss_actor_constellation_callback == nullptr) {
    _rss_check = std::make_shared<::carla::rss::RssCheck>(max_steering_angle);
  } else {
    _rss_check =
        std::make_shared<::carla::rss::RssCheck>(max_steering_angle, _rss_actor_constellation_callback, GetParent());
  }

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

  log_debug(GetDisplayId(), ": unsubscribing from tick event");
  GetEpisode().Lock()->RemoveOnTickEvent(_on_tick_register_id);
  _on_tick_register_id = 0u;
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

SharedPtr<sensor::SensorData> RssSensor::TickRssSensor(const Timestamp &timestamp) {
  try {
    bool result = false;
    ::ad::rss::state::ProperResponse response;
    ::ad::rss::state::RssStateSnapshot rss_state_snapshot;
    ::ad::rss::situation::SituationSnapshot situation_snapshot;
    ::ad::rss::world::WorldModel world_model;
    carla::rss::EgoDynamicsOnRoute ego_dynamics_on_route;
    if (_processing_lock.try_lock()) {
      spdlog::debug("RssSensor tick: T={}", timestamp.frame);

      if ((timestamp.frame < _last_processed_frame) && ((_last_processed_frame - timestamp.frame) < 0xffffffffu))
      {
        _processing_lock.unlock();
        spdlog::warn("RssSensor tick dropped: T={}", timestamp.frame);
        return nullptr;
      }
      _last_processed_frame = timestamp.frame;

      carla::client::World world = GetWorld();
      SharedPtr<carla::client::ActorList> actors = world.GetActors();

      if (_drop_route) {
        _drop_route = false;
        _rss_check->DropRoute();
      }

      // check all object<->ego pairs with RSS and calculate proper response
      result = _rss_check->CheckObjects(timestamp, actors, GetParent(), response, rss_state_snapshot,
                                        situation_snapshot, world_model, ego_dynamics_on_route);
      _processing_lock.unlock();

      spdlog::debug(
          "RssSensor response: T={} S:{}->E:{} DeltaT:{}", timestamp.frame,
          ego_dynamics_on_route.time_since_epoch_check_start_ms, ego_dynamics_on_route.time_since_epoch_check_end_ms,
          ego_dynamics_on_route.time_since_epoch_check_end_ms - ego_dynamics_on_route.time_since_epoch_check_start_ms);
      return MakeShared<sensor::data::RssResponse>(timestamp.frame, timestamp.elapsed_seconds, GetTransform(), result,
                                                   response, rss_state_snapshot, situation_snapshot, world_model,
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
