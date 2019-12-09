// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rss/RssRestrictor.h"
#include "carla/rpc/VehicleControl.h"
#include "carla/rpc/VehiclePhysicsControl.h"

#include "ad_rss/world/AccelerationRestriction.hpp"
#include "ad_rss/world/Velocity.hpp"

namespace carla {
namespace rss {

  RssRestrictor::RssRestrictor() {}

  RssRestrictor::~RssRestrictor() = default;

  carla::rpc::VehicleControl RssRestrictor::restrictVehicleControl(
  const carla::rpc::VehicleControl &vehicleControl, const ad_rss::world::AccelerationRestriction &restriction,
  const ad_rss::world::Velocity &egoVelocity, const carla::rpc::VehiclePhysicsControl &vehiclePhysics) {
    carla::rpc::VehicleControl restrictedVehicleControl(vehicleControl);

    // Pretty basic implementation of a RSS restrictor modifying the
    // VehicleControl according to the given
    // restrictions. Basic braking and countersteering actions are applied.

    ad_rss::physics::Acceleration zeroAccel(0.0);

    float mass =  vehiclePhysics.mass;
    float sumMaxBrakeTorque = 0.f;
    float radius = 1.f;
    for (auto const &wheel : vehiclePhysics.wheels) {
      sumMaxBrakeTorque += wheel.max_brake_torque;
      radius = wheel.radius;
    }

    // do not apply any restrictions when in reverse gear
    if (!vehicleControl.reverse) {

      // decelerate
      if (restriction.longitudinalRange.maximum < zeroAccel) {
        restrictedVehicleControl.throttle = 0.0f;

        double sumBrakeTorque = mass * std::fabs(static_cast<double>(restriction.longitudinalRange.minimum)) *
        radius / 100.0;
        restrictedVehicleControl.brake =
        std::min(static_cast<float>(sumBrakeTorque /   sumMaxBrakeTorque), 1.0f);
      }

      if (restriction.lateralLeftRange.maximum <= ad_rss::physics::Acceleration(0.0)) {
        if (egoVelocity.speedLat < ad_rss::physics::Speed(0.0)) {
          // driving to the left
          if (egoVelocity.speedLon != ad_rss::physics::Speed(0.0)) {
            double angle = std::atan(egoVelocity.speedLat / egoVelocity.speedLon);
            restrictedVehicleControl.steer = -1.f * static_cast<float>(angle);
          }
        }
      }
      if (restriction.lateralRightRange.maximum <= ad_rss::physics::Acceleration(0.0)) {
        if (egoVelocity.speedLat > ad_rss::physics::Speed(0.0)) {
          // driving to the right
          if (egoVelocity.speedLon != ad_rss::physics::Speed(0.0)) {
            double angle = std::atan(egoVelocity.speedLat / egoVelocity.speedLon);
            restrictedVehicleControl.steer = -1.f * static_cast<float>(angle);
          }
        }
      }
    }
    return restrictedVehicleControl;
  }

}  // namespace rss
}  // namespace carla
