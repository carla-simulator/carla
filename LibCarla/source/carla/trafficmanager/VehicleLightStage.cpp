
#include <algorithm>

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/LocalizationUtils.h"

#include "carla/trafficmanager/VehicleLightStage.h"

namespace carla {
namespace traffic_manager {

using namespace constants::VehicleLight;
using namespace constants::WorldInfoRefresh;

VehicleLightStage::VehicleLightStage(
  const std::vector<ActorId> &vehicle_id_list,
  const BufferMap &buffer_map,
  const Parameters &parameters,
  const cc::World &world,
  ControlFrame& control_frame)
  : vehicle_id_list(vehicle_id_list),
    buffer_map(buffer_map),
    parameters(parameters),
    world(world),
    control_frame(control_frame) {}

void VehicleLightStage::UpdateWorldInfo(const double current_time, const bool synchronous_mode) {
  // Nothing has to be read while no vehicle is set to update its lights.
  const bool any_vehicle_updates_lights = std::any_of(
      vehicle_id_list.begin(),
      vehicle_id_list.end(),
      [this](const ActorId actor_id) { return parameters.GetUpdateVehicleLights(actor_id); });
  if (!any_vehicle_updates_lights) {
    all_light_states.clear();
    last_light_states_update = -std::numeric_limits<double>::infinity();
    light_states_refreshed = false;
    return;
  }

  light_states_refreshed = synchronous_mode ||
      IsRefreshDue(current_time, last_light_states_update, VEHICLE_LIGHT_STATES_REFRESH_PERIOD);
  if (light_states_refreshed) {
    all_light_states = world.GetVehiclesLightStates();
    last_light_states_update = current_time;
  }

  if (synchronous_mode ||
      IsRefreshDue(current_time, last_weather_update, WEATHER_REFRESH_PERIOD)) {
    is_weather_enabled = world.IsWeatherEnabled();
    if (is_weather_enabled) {
      weather = world.GetWeather();
    }
    last_weather_update = current_time;
  }
}

void VehicleLightStage::SetCachedLightState(
    const ActorId actor_id,
    const rpc::VehicleLightState::flag_type light_state) {
  for (auto &vls : all_light_states) {
    if (vls.first == actor_id) {
      vls.second = light_state;
      return;
    }
  }
  all_light_states.emplace_back(actor_id, light_state);
}

void VehicleLightStage::Update(const unsigned long index) {
  ActorId actor_id = vehicle_id_list.at(index);

  if (!parameters.GetUpdateVehicleLights(actor_id))
    return; // this vehicle is not set to have automatic lights update

  rpc::VehicleLightState::flag_type light_states = uint32_t(-1);
  bool brake_lights = false;
  bool left_turn_indicator = false;
  bool right_turn_indicator = false;
  bool position = false;
  bool low_beam = false;
  bool high_beam = false;
  bool fog_lights = false;

  // search the current light state of the vehicle
  bool found_light_state = false;
  for (auto&& vls : all_light_states) {
    if (vls.first == actor_id) {
      light_states = vls.second;
      found_light_state = true;
      break;
    }
  }

  if (!found_light_state && !light_states_refreshed) {
    // The cached list predates this vehicle. Deriving a command from the
    // sentinel above would switch on every bit this stage does not manage
    // (reverse, interior, special), and the write-back below would then keep
    // them on for good, so wait one step for a refreshed list instead.
    last_light_states_update = -std::numeric_limits<double>::infinity();
    return;
  }

  // Determine if the vehicle is truning left or right by checking the close waypoints

  const Buffer& waypoint_buffer = buffer_map.at(actor_id);
  cg::Location front_location = waypoint_buffer.front()->GetLocation();

  for (const SimpleWaypointPtr& waypoint : waypoint_buffer) {
    if (waypoint->CheckJunction()) {
      RoadOption target_ro = waypoint->GetRoadOption();
      if (target_ro == RoadOption::Left) left_turn_indicator = true;
      else if (target_ro == RoadOption::Right) right_turn_indicator = true;
      break;
    }
    if (cg::Math::DistanceSquared(front_location, waypoint->GetLocation()) > MAX_DISTANCE_LIGHT_CHECK) {
      break;
    }
  }

  // Determine brake light state
  for (size_t cc = 0; cc < control_frame.size(); cc++) {
    if (auto* maybe_ctrl = std::get_if<carla::rpc::Command::ApplyVehicleControl>(&control_frame[cc].command)) {
      carla::rpc::Command::ApplyVehicleControl& ctrl = *maybe_ctrl;
      if (ctrl.actor == actor_id) {
        brake_lights = (ctrl.control.brake > 0.5); // hard braking, avoid blinking for throttle control
        break;
      }
    }
  }

  // Determine position, fog and beams. Do nothing if the weather is disabled.
  if (is_weather_enabled) {

    // Turn on beams & positions from sunset to dawn
    if (weather.sun_altitude_angle < SUN_ALTITUDE_DEGREES_BEFORE_DAWN ||
        weather.sun_altitude_angle > SUN_ALTITUDE_DEGREES_AFTER_SUNSET)
    {
      position = true;
      low_beam = true;
    }
    else if (weather.sun_altitude_angle < SUN_ALTITUDE_DEGREES_JUST_AFTER_DAWN ||
            weather.sun_altitude_angle > SUN_ALTITUDE_DEGREES_JUST_BEFORE_SUNSET)
    {
      position = true;
    }
    // Turn on lights under heavy rain
    if (weather.precipitation > HEAVY_PRECIPITATION_THRESHOLD) {
      position = true;
      low_beam = true;
    }
    // Turn on fog lights
    if (weather.fog_density > FOG_DENSITY_THRESHOLD) {
      position = true;
      low_beam = true;
      fog_lights = true;
    }
  }

  // Determine the new vehicle light state
  rpc::VehicleLightState::flag_type new_light_states = light_states;
  if (brake_lights)
    new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Brake);
  else
    new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Brake);

  if (left_turn_indicator)
    new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::LeftBlinker);
  else
    new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::LeftBlinker);

  if (right_turn_indicator)
    new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::RightBlinker);
  else
    new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::RightBlinker);

  if (position)
    new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Position);
  else
    new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Position);

  if (low_beam)
    new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::LowBeam);
  else
    new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::LowBeam);

  if (high_beam)
    new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::HighBeam);
  else
    new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::HighBeam);

  if (fog_lights)
    new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Fog);
  else
    new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Fog);

  // Update the vehicle light state if it has changed
  if (new_light_states != light_states) {
    control_frame.push_back(carla::rpc::Command::SetVehicleLightState(actor_id, new_light_states));
    // Otherwise the same command is queued again on every step until the list
    // is refreshed.
    SetCachedLightState(actor_id, new_light_states);
  }
}

void VehicleLightStage::RemoveActor(const ActorId) {
}

void VehicleLightStage::Reset() {
}

} // namespace traffic_manager
} // namespace carla
