// Copyright (c) 2018-2020 CVC.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/agents/navigation/ConstantVelocityAgent.h"

#include "carla/agents/navigation/LocalPlanner.h"

#include "carla/client/Actor.h"
#include "carla/client/ActorBlueprint.h"
#include "carla/client/ActorList.h"
#include "carla/client/BlueprintLibrary.h"
#include "carla/client/Sensor.h"
#include "carla/client/TrafficLight.h"
#include "carla/client/Vehicle.h"
#include "carla/client/World.h"
#include "carla/client/WorldSnapshot.h"
#include "carla/client/Timestamp.h"
#include "carla/geom/Transform.h"
#include "carla/geom/Vector3D.h"
#include "carla/rpc/VehicleControl.h"

#include <utility>

namespace carla {
namespace agents {
namespace navigation {

ConstantVelocityAgent::ConstantVelocityAgent(SharedPtr<client::Vehicle> vehicle,
                                             float target_speed_kmh,
                                             const ConstantOptions &opts,
                                             SharedPtr<client::Map> map,
                                             SharedPtr<GlobalRoutePlanner> grp)
    : BasicAgent(std::move(vehicle), target_speed_kmh,
                 static_cast<const Options &>(opts), std::move(map), std::move(grp)),
      _use_basic_behavior(opts.use_basic_behavior),
      _target_speed_ms(target_speed_kmh / 3.6f),
      _restart_time(opts.restart_time),
      _is_constant_velocity_active(true),
      _constant_velocity_stop_time(0.0),
      _has_stop_timestamp(false),
      _collision_sensor(nullptr) {

  SetCollisionSensor();
  SetConstantVelocity(_target_speed_ms);
}

ConstantVelocityAgent::~ConstantVelocityAgent() {
  DestroySensor();
}

void ConstantVelocityAgent::SetTargetSpeed(float speed_kmh) {
  _target_speed_ms = speed_kmh / 3.6f;
  _local_planner->SetSpeed(speed_kmh);
}

void ConstantVelocityAgent::StopConstantVelocity() {
  _is_constant_velocity_active = false;
  _vehicle->DisableConstantVelocity();
  _constant_velocity_stop_time =
      _world->GetSnapshot().GetTimestamp().elapsed_seconds;
  _has_stop_timestamp = true;
}

void ConstantVelocityAgent::RestartConstantVelocity() {
  _is_constant_velocity_active = true;
  SetConstantVelocity(_target_speed_ms);
}

void ConstantVelocityAgent::SetConstantVelocity(float speed_ms) {
  _vehicle->EnableConstantVelocity(geom::Vector3D(speed_ms, 0.0f, 0.0f));
}

rpc::VehicleControl ConstantVelocityAgent::RunStep() {
  if (!_is_constant_velocity_active) {
    const double elapsed =
        _world->GetSnapshot().GetTimestamp().elapsed_seconds;
    if (_has_stop_timestamp &&
        (elapsed - _constant_velocity_stop_time) >
            static_cast<double>(_restart_time)) {
      RestartConstantVelocity();
      _is_constant_velocity_active = true;
    } else if (_use_basic_behavior) {
      return BasicAgent::RunStep();
    } else {
      return rpc::VehicleControl{};
    }
  }

  bool hazard_detected = false;
  float hazard_speed = 0.0f;

  auto actor_list   = _world->GetActors();
  auto vehicle_list = actor_list->Filter("*vehicle*");
  auto lights_list  = actor_list->Filter("*traffic_light*");

  const float vehicle_speed = _vehicle->GetVelocity().Length();

  // Vehicle obstacle check.
  const float max_vehicle_distance = _base_vehicle_threshold + vehicle_speed;
  auto v_obs = VehicleObstacleDetected(vehicle_list, max_vehicle_distance);
  if (v_obs.affected && v_obs.actor) {
    const auto vehicle_velocity = _vehicle->GetVelocity();
    const float v_len = vehicle_velocity.Length();
    if (v_len == 0.0f) {
      hazard_speed = 0.0f;
    } else {
      const auto adv_velocity = v_obs.actor->GetVelocity();
      const float dot = vehicle_velocity.x * adv_velocity.x +
                        vehicle_velocity.y * adv_velocity.y +
                        vehicle_velocity.z * adv_velocity.z;
      hazard_speed = dot / v_len;
    }
    hazard_detected = true;
  }

  // Red-light check (uses 0.3 * speed factor instead of _speed_ratio — match
  // the Python source verbatim).
  const float max_tlight_distance = _base_tlight_threshold + 0.3f * vehicle_speed;
  auto tl_pair = AffectedByTrafficLight(lights_list, max_tlight_distance);
  if (tl_pair.first) {
    hazard_speed = 0.0f;
    hazard_detected = true;
  }

  // Run the local planner so the wheels keep spinning even though the actual
  // forward motion comes from EnableConstantVelocity.
  rpc::VehicleControl control = _local_planner->RunStep();
  if (hazard_detected) {
    SetConstantVelocity(hazard_speed);
  } else {
    SetConstantVelocity(_target_speed_ms);
  }
  return control;
}

void ConstantVelocityAgent::SetCollisionSensor() {
  auto blueprint_lib = _world->GetBlueprintLibrary();
  const auto *blueprint = blueprint_lib->Find("sensor.other.collision");
  if (!blueprint) {
    return;
  }

  auto actor = _world->SpawnActor(*blueprint, geom::Transform{}, _vehicle.get());
  _collision_sensor = std::dynamic_pointer_cast<client::Sensor>(actor);
  if (!_collision_sensor) {
    return;
  }

  // Capture `this` raw — the agent owns the sensor and tears it down in the
  // destructor, so the callback can never fire on a destroyed agent.
  _collision_sensor->Listen(
      [this](SharedPtr<sensor::SensorData>) {
        this->StopConstantVelocity();
      });
}

void ConstantVelocityAgent::DestroySensor() {
  if (_collision_sensor) {
    if (_collision_sensor->IsListening()) {
      _collision_sensor->Stop();
    }
    _collision_sensor->Destroy();
    _collision_sensor.reset();
  }
}

} // namespace navigation
} // namespace agents
} // namespace carla
