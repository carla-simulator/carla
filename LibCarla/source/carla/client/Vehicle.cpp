// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Vehicle.h"

#include "carla/client/detail/Simulator.h"
#include "carla/client/ActorList.h"
#include "carla/client/TrafficLight.h"
#include "carla/rpc/TrafficLightState.h"

namespace carla {
namespace client {

  void Vehicle::SetAutopilot(bool enabled) {
    GetEpisode().Lock()->SetVehicleAutopilot(*this, enabled);
  }

  void Vehicle::ApplyControl(const Control &control) {
    if (control != _control) {
      GetEpisode().Lock()->ApplyControlToVehicle(*this, control);
      _control = control;
    }
  }

  Vehicle::Control Vehicle::GetControl() const {
    return GetEpisode().Lock()->GetActorDynamicState(*this).state.vehicle_data.control;
  }

  float Vehicle::GetSpeedLimit() const {
    return GetEpisode().Lock()->GetActorDynamicState(*this).state.vehicle_data.speed_limit;
  }

  rpc::TrafficLightState Vehicle::GetTrafficLightState() const {
    return GetEpisode().Lock()->GetActorDynamicState(*this).state.vehicle_data.traffic_light_state;
  }

  bool Vehicle::IsAtTrafficLight() {
    return GetEpisode().Lock()->GetActorDynamicState(*this).state.vehicle_data.has_traffic_light;
  }

  SharedPtr<TrafficLight> Vehicle::GetTrafficLight() const {
    auto id = GetEpisode().Lock()->GetActorDynamicState(*this).state.vehicle_data.traffic_light_id;
    SharedPtr<Actor> actor = GetWorld().GetActors()->Find(id);
    return boost::static_pointer_cast<TrafficLight>(actor);
  }

  rpc::VehiclePhysicsControl Vehicle::GetPhysicsControl() const {
    return GetEpisode().Lock()->GetVehiclePhysicsControl(GetId());
  }

  void Vehicle::SetPhysicsControl(const rpc::VehiclePhysicsControl &physics_control) {
    return GetEpisode().Lock()->SetVehiclePhysicsControl(GetId(), physics_control);
  }
} // namespace client
} // namespace carla
