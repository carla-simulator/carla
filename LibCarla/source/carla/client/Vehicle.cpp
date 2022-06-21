// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Vehicle.h"

#include "carla/client/ActorList.h"
#include "carla/client/detail/Simulator.h"
#include "carla/client/TrafficLight.h"
#include "carla/Memory.h"
#include "carla/rpc/TrafficLightState.h"

#include "carla/trafficmanager/TrafficManager.h"

namespace carla {

using TM = traffic_manager::TrafficManager;

namespace client {


  template <typename AttributesT>
  static bool GetControlIsSticky(const AttributesT &attributes) {
    for (auto &&attribute : attributes) {
      if (attribute.GetId() == "sticky_control") {
        return attribute.template As<bool>();
      }
    }
    return true;
  }

  Vehicle::Vehicle(ActorInitializer init)
    : Actor(std::move(init)),
      _is_control_sticky(GetControlIsSticky(GetAttributes())) {}

  void Vehicle::SetAutopilot(bool enabled, uint16_t tm_port) {
    TM tm(GetEpisode(), tm_port);
    if (enabled) {
      tm.RegisterVehicles({shared_from_this()});
    } else {
      tm.UnregisterVehicles({shared_from_this()});
    }
  }

  void Vehicle::ShowDebugTelemetry(bool enabled) {
    GetEpisode().Lock()->ShowVehicleDebugTelemetry(*this, enabled);
  }

  void Vehicle::ApplyControl(const Control &control) {
    if (!_is_control_sticky || (control != _control)) {
      GetEpisode().Lock()->ApplyControlToVehicle(*this, control);
      _control = control;
    }
  }

  void Vehicle::ApplyPhysicsControl(const PhysicsControl &physics_control) {
    GetEpisode().Lock()->ApplyPhysicsControlToVehicle(*this, physics_control);
  }

  void Vehicle::OpenDoor(const VehicleDoor door_idx) {
    GetEpisode().Lock()->OpenVehicleDoor(*this, rpc::VehicleDoor(door_idx));
  }

  void Vehicle::CloseDoor(const VehicleDoor door_idx) {
    GetEpisode().Lock()->CloseVehicleDoor(*this, rpc::VehicleDoor(door_idx));
  }

  void Vehicle::SetLightState(const LightState &light_state) {
    GetEpisode().Lock()->SetLightStateToVehicle(*this, rpc::VehicleLightState(light_state));
  }

  void Vehicle::SetWheelSteerDirection(WheelLocation wheel_location, float angle_in_deg) {
    GetEpisode().Lock()->SetWheelSteerDirection(*this, wheel_location, angle_in_deg);
  }

  float Vehicle::GetWheelSteerAngle(WheelLocation wheel_location) {
    return GetEpisode().Lock()->GetWheelSteerAngle(*this, wheel_location);
  }

  Vehicle::Control Vehicle::GetControl() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.vehicle_data.control;
  }

  Vehicle::PhysicsControl Vehicle::GetPhysicsControl() const {
    return GetEpisode().Lock()->GetVehiclePhysicsControl(*this);
  }

  Vehicle::LightState Vehicle::GetLightState() const {
    return GetEpisode().Lock()->GetVehicleLightState(*this).GetLightStateEnum();
  }

  float Vehicle::GetSpeedLimit() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.vehicle_data.speed_limit;
  }

  rpc::TrafficLightState Vehicle::GetTrafficLightState() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.vehicle_data.traffic_light_state;
  }

  bool Vehicle::IsAtTrafficLight() {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.vehicle_data.has_traffic_light;
  }

  SharedPtr<TrafficLight> Vehicle::GetTrafficLight() const {
    auto id = GetEpisode().Lock()->GetActorSnapshot(*this).state.vehicle_data.traffic_light_id;
    return boost::static_pointer_cast<TrafficLight>(GetWorld().GetActor(id));
  }

  void Vehicle::EnableCarSim(std::string simfile_path) {
    GetEpisode().Lock()->EnableCarSim(*this, simfile_path);
  }

  void Vehicle::UseCarSimRoad(bool enabled) {
    GetEpisode().Lock()->UseCarSimRoad(*this, enabled);
  }

  void Vehicle::EnableChronoPhysics(
      uint64_t MaxSubsteps,
      float MaxSubstepDeltaTime,
      std::string VehicleJSON,
      std::string PowertrainJSON,
      std::string TireJSON,
      std::string BaseJSONPath) {
    GetEpisode().Lock()->EnableChronoPhysics(*this,
        MaxSubsteps,
        MaxSubstepDeltaTime,
        VehicleJSON,
        PowertrainJSON,
        TireJSON,
        BaseJSONPath);
  }

} // namespace client
} // namespace carla
