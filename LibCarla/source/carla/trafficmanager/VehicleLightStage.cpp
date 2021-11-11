
#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/LocalizationUtils.h"

#include "carla/trafficmanager/VehicleLightStage.h"

namespace carla {
namespace traffic_manager {

using namespace constants::VehicleLight;

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

void VehicleLightStage::UpdateWorldInfo() {
  // Get the global weather and all the vehicle light states at once
  all_light_states = world.GetVehiclesLightStates();
  weather = world.GetWeather();
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
  for (auto&& vls : all_light_states) {
    if (vls.first == actor_id) {
      light_states = vls.second;
      break;
    }
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
    if (control_frame[cc].command.type() == typeid(carla::rpc::Command::ApplyVehicleControl)) {
      carla::rpc::Command::ApplyVehicleControl& ctrl = boost::get<carla::rpc::Command::ApplyVehicleControl>(control_frame[cc].command);
      if (ctrl.actor == actor_id) {
        brake_lights = (ctrl.control.brake > 0.5); // hard braking, avoid blinking for throttle control
        break;
      }
    }
  }

  // Determine position, fog and beams

  // Turn on beams & positions from sunset to dawn
  if (weather.sun_altitude_angle < SUN_ALTITUDE_DEGREES_BEFORE_DAWN ||
      weather.sun_altitude_angle >SUN_ALTITUDE_DEGREES_AFTER_SUNSET)
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
  if (new_light_states != light_states)
    control_frame.push_back(carla::rpc::Command::SetVehicleLightState(actor_id, new_light_states));
}

void VehicleLightStage::RemoveActor(const ActorId) {
}

void VehicleLightStage::Reset() {
}

} // namespace traffic_manager
} // namespace carla
