
#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/LocalizationUtils.h"

#include "carla/trafficmanager/TrafficLightStage.h"

namespace carla {
namespace traffic_manager {

using constants::TrafficLight::DOUBLE_NO_SIGNAL_PASSTHROUGH_INTERVAL;
using constants::WaypointSelection::JUNCTION_LOOK_AHEAD;

TrafficLightStage::TrafficLightStage(
  const std::vector<ActorId> &vehicle_id_list,
  const SimulationState &simulation_state,
  const BufferMap &buffer_map,
  const Parameters &parameters,
  const cc::World &world,
  TLFrame &output_array,
  RandomGeneratorMap &random_devices)
  : vehicle_id_list(vehicle_id_list),
    simulation_state(simulation_state),
    buffer_map(buffer_map),
    parameters(parameters),
    world(world),
    output_array(output_array),
    random_devices(random_devices) {}

void TrafficLightStage::Update(const unsigned long index) {
  bool traffic_light_hazard = false;

  const ActorId ego_actor_id = vehicle_id_list.at(index);
  const Buffer &waypoint_buffer = buffer_map.at(ego_actor_id);
  const SimpleWaypointPtr look_ahead_point = GetTargetWaypoint(waypoint_buffer, JUNCTION_LOOK_AHEAD).first;

  const JunctionID junction_id = look_ahead_point->GetWaypoint()->GetJunctionId();
  current_timestamp = world.GetSnapshot().GetTimestamp();

  const TrafficLightState tl_state = simulation_state.GetTLS(ego_actor_id);
  const TLS traffic_light_state = tl_state.tl_state;
  const bool is_at_traffic_light = tl_state.at_traffic_light;

  // We determine to stop if the current position of the vehicle is not a
  // junction and there is a red or yellow light.
  if (is_at_traffic_light &&
      traffic_light_state != TLS::Green &&
      traffic_light_state != TLS::Off &&
      parameters.GetPercentageRunningLight(ego_actor_id) <= random_devices.at(ego_actor_id).next()) {

    traffic_light_hazard = true;
  }
  // Handle entry negotiation at non-signalised junction.
  else if (look_ahead_point->CheckJunction() &&
          !is_at_traffic_light &&
          traffic_light_state != TLS::Green &&
          traffic_light_state != TLS::Off &&
          parameters.GetPercentageRunningSign(ego_actor_id) <= random_devices.at(ego_actor_id).next()) {

    traffic_light_hazard = HandleNonSignalisedJunction(ego_actor_id, junction_id, current_timestamp);
  }

  output_array.at(index) = traffic_light_hazard;
}

bool TrafficLightStage::HandleNonSignalisedJunction(const ActorId ego_actor_id, const JunctionID junction_id,
                                                    cc::Timestamp timestamp) {

  bool traffic_light_hazard = false;

  if (vehicle_last_junction.find(ego_actor_id) == vehicle_last_junction.end()) {
    // Initializing new map entry for the actor with
    // an arbitrary and different junction id.
    vehicle_last_junction.insert({ego_actor_id, junction_id + 1});
  }

  if (vehicle_last_junction.at(ego_actor_id) != junction_id) {
    vehicle_last_junction.at(ego_actor_id) = junction_id;

    // Check if the vehicle has an outdated ticket or needs a new one.
    bool need_to_issue_new_ticket = false;
    if (vehicle_last_ticket.find(ego_actor_id) == vehicle_last_ticket.end()) {

      need_to_issue_new_ticket = true;
    } else {

      const cc::Timestamp &previous_ticket = vehicle_last_ticket.at(ego_actor_id);
      const double diff = timestamp.elapsed_seconds - previous_ticket.elapsed_seconds;
      if (diff > DOUBLE_NO_SIGNAL_PASSTHROUGH_INTERVAL) {
        need_to_issue_new_ticket = true;
      }
    }

    // If new ticket is needed for the vehicle, then query the junction
    // ticket map
    // and update the map value to the new ticket value.
    if (need_to_issue_new_ticket) {
      if (junction_last_ticket.find(junction_id) != junction_last_ticket.end()) {

        cc::Timestamp &last_ticket = junction_last_ticket.at(junction_id);
        const double diff = timestamp.elapsed_seconds - last_ticket.elapsed_seconds;
        if (diff > 0.0) {
          last_ticket.elapsed_seconds = timestamp.elapsed_seconds + DOUBLE_NO_SIGNAL_PASSTHROUGH_INTERVAL;
        } else {
          last_ticket.elapsed_seconds += DOUBLE_NO_SIGNAL_PASSTHROUGH_INTERVAL;
        }
      } else {
        cc::Timestamp &new_ticket = timestamp;
        new_ticket.elapsed_seconds += DOUBLE_NO_SIGNAL_PASSTHROUGH_INTERVAL;
        junction_last_ticket.insert({junction_id, new_ticket});
      }
      if (vehicle_last_ticket.find(ego_actor_id) != vehicle_last_ticket.end()) {
        vehicle_last_ticket.at(ego_actor_id) = junction_last_ticket.at(junction_id);
      } else {
        vehicle_last_ticket.insert({ego_actor_id, junction_last_ticket.at(junction_id)});
      }
    }
  }

  // If current time is behind ticket time, then do not enter junction.
  const cc::Timestamp current_ticket = vehicle_last_ticket.at(ego_actor_id);
  const double diff = current_ticket.elapsed_seconds - timestamp.elapsed_seconds;
  if (diff > 0.0) {
    traffic_light_hazard = true;
  }

  return traffic_light_hazard;
}

void TrafficLightStage::RemoveActor(const ActorId actor_id) {
  vehicle_last_ticket.erase(actor_id);
  vehicle_last_junction.erase(actor_id);
}

void TrafficLightStage::Reset() {
  vehicle_last_ticket.clear();
  vehicle_last_junction.clear();
  junction_last_ticket.clear();
}

} // namespace traffic_manager
} // namespace carla
