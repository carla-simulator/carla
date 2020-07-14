// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rss/RssRestrictor.h"
#include "carla/rpc/VehicleControl.h"
#include "carla/rpc/VehiclePhysicsControl.h"
#include "carla/rss/RssCheck.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <ad/rss/state/ProperResponse.hpp>
#include <ad/rss/unstructured/Geometry.hpp>
#include <ad/rss/world/Velocity.hpp>

namespace carla {
namespace rss {

RssRestrictor::RssRestrictor() {
  std::string logger_name = "RssRestrictor";
  _logger = spdlog::get(logger_name);
  if (!_logger) {
    _logger = spdlog::create<spdlog::sinks::stdout_color_sink_st>(logger_name);
  }
  //_logger->set_level(spdlog::level::debug);
}

RssRestrictor::~RssRestrictor() = default;

carla::rpc::VehicleControl RssRestrictor::RestrictVehicleControl(
    const carla::rpc::VehicleControl &vehicle_control, const ::ad::rss::state::ProperResponse &proper_response,
    const carla::rss::EgoDynamicsOnRoute &ego_dynamics_on_route,
    const carla::rpc::VehiclePhysicsControl &vehicle_physics) {
  carla::rpc::VehicleControl restricted_vehicle_control(vehicle_control);

  // Pretty basic implementation of a RSS restrictor modifying the
  // VehicleControl according to the given
  // restrictions. Basic braking and countersteering actions are applied.
  // In case countersteering is not possible anymore (i.e. the lateral velocity
  // reached zero),
  // as a fallback longitudinal braking is applied instead (escalation
  // strategy).

  ::ad::physics::Acceleration zero_accel(0.0);

  float mass = vehicle_physics.mass;
  float max_steer_angle_deg = 0.f;
  float sum_max_brake_torque = 0.f;
  float radius = 1.f;
  for (auto const &wheel : vehicle_physics.wheels) {
    sum_max_brake_torque += wheel.max_brake_torque;
    radius = wheel.radius;
    max_steer_angle_deg = std::max(max_steer_angle_deg, wheel.max_steer_angle);
  }

  // do not apply any restrictions when in reverse gear
  if (!vehicle_control.reverse) {
    _logger->debug("Lon {}, L {}, R {}; LatSpeed {}, Accel {}, Avg {}, Hdg {}, AllowedHeadingRanges {}",
                   proper_response.accelerationRestrictions.longitudinalRange,
                   proper_response.accelerationRestrictions.lateralLeftRange,
                   proper_response.accelerationRestrictions.lateralRightRange, ego_dynamics_on_route.route_speed_lat,
                   ego_dynamics_on_route.route_accel_lat, ego_dynamics_on_route.avg_route_accel_lat,
                   ego_dynamics_on_route.ego_heading, proper_response.headingRanges);
    if (proper_response.accelerationRestrictions.lateralLeftRange.maximum <= ::ad::physics::Acceleration(0.0)) {
      if (ego_dynamics_on_route.route_speed_lat < ::ad::physics::Speed(0.0)) {
        // driving to the left
        if (ego_dynamics_on_route.route_speed_lon != ::ad::physics::Speed(0.0)) {
          double angle_rad = std::atan(ego_dynamics_on_route.route_speed_lat / ego_dynamics_on_route.route_speed_lon);
          float desired_steer_ratio = -180.f * static_cast<float>(angle_rad / M_PI) / max_steer_angle_deg;
          if (ego_dynamics_on_route.crossing_border) {
            desired_steer_ratio += 0.1f;
          }
          float orig_steer = restricted_vehicle_control.steer;
          restricted_vehicle_control.steer = std::max(restricted_vehicle_control.steer, desired_steer_ratio);
          restricted_vehicle_control.steer = std::min(restricted_vehicle_control.steer, 1.0f);
          _logger->debug("EgoVelocity: {}", ego_dynamics_on_route);
          _logger->debug("Countersteer left to right: {} -> {}", orig_steer, restricted_vehicle_control.steer);
        }
      }
    }

    if (proper_response.accelerationRestrictions.lateralRightRange.maximum <= ::ad::physics::Acceleration(0.0)) {
      if (ego_dynamics_on_route.route_speed_lat > ::ad::physics::Speed(0.0)) {
        // driving to the right
        if (ego_dynamics_on_route.route_speed_lon != ::ad::physics::Speed(0.0)) {
          double angle_rad = std::atan(ego_dynamics_on_route.route_speed_lat / ego_dynamics_on_route.route_speed_lon);
          float desired_steer_ratio = -180.f * static_cast<float>(angle_rad / M_PI) / max_steer_angle_deg;
          if (ego_dynamics_on_route.crossing_border) {
            desired_steer_ratio -= 0.1f;
          }
          float orig_steer = restricted_vehicle_control.steer;
          restricted_vehicle_control.steer = std::min(restricted_vehicle_control.steer, desired_steer_ratio);
          restricted_vehicle_control.steer = std::max(restricted_vehicle_control.steer, -1.0f);
          _logger->debug("EgoVelocity: {}", ego_dynamics_on_route);
          _logger->debug("Countersteer right to left: {} -> {}", orig_steer, restricted_vehicle_control.steer);
        }
      }
    }

    // restrict acceleration
    if (proper_response.accelerationRestrictions.longitudinalRange.maximum > zero_accel) {
      // TODO: determine acceleration and limit throttle
    }

    // decelerate
    if (proper_response.accelerationRestrictions.longitudinalRange.maximum < zero_accel) {
      restricted_vehicle_control.throttle = 0.0f;

      double brake_acceleration =
          std::fabs(static_cast<double>(proper_response.accelerationRestrictions.longitudinalRange.minimum));
      double sum_brake_torque = mass * brake_acceleration * radius / 100.0;
      restricted_vehicle_control.brake = std::min(static_cast<float>(sum_brake_torque / sum_max_brake_torque), 1.0f);
    }
  }
  if (restricted_vehicle_control != vehicle_control) {
    _logger->debug(
        "Restrictor active: throttle({} -> {}), brake ({} -> {}). steer ({} -> "
        "{})",
        vehicle_control.throttle, restricted_vehicle_control.throttle, vehicle_control.brake,
        restricted_vehicle_control.brake, vehicle_control.steer, restricted_vehicle_control.steer);
  }
  return restricted_vehicle_control;
}

}  // namespace rss
}  // namespace carla
