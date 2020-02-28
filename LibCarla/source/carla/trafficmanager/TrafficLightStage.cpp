// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <iostream>

#include "carla/trafficmanager/TrafficLightStage.h"

namespace carla {
namespace traffic_manager {

  static const uint64_t NO_SIGNAL_PASSTHROUGH_INTERVAL = 5u;

  TrafficLightStage::TrafficLightStage(
      std::string stage_name,
      std::shared_ptr<LocalizationToTrafficLightMessenger> localization_messenger,
      std::shared_ptr<TrafficLightToPlannerMessenger> planner_messenger,
      Parameters &parameters,
      cc::DebugHelper &debug_helper)
    : PipelineStage(stage_name),
      localization_messenger(localization_messenger),
      planner_messenger(planner_messenger),
      parameters(parameters),
      debug_helper(debug_helper){

    // Initializing output frame selector.
    frame_selector = true;
    // Initializing number of vehicles to zero in the beginning.
    number_of_vehicles = 0u;
    /// @todo: replace with RandomEngine
    // Initializing srand.
    srand(static_cast<unsigned>(time(NULL)));

  }

  TrafficLightStage::~TrafficLightStage() {}

  void TrafficLightStage::Action() {

    // Selecting the output frame based on the selection key.
    const auto current_planner_frame = frame_selector ? planner_frame_a : planner_frame_b;

    // Looping over registered actors.
    for (uint64_t i = 0u; i < number_of_vehicles && localization_frame != nullptr; ++i) {

      bool traffic_light_hazard = false;
      const LocalizationToTrafficLightData &data = localization_frame->at(i);
      if (!data.actor->IsAlive()) {
        continue;
      }

      const Actor ego_actor = data.actor;
      const ActorId ego_actor_id = ego_actor->GetId();
      const SimpleWaypointPtr closest_waypoint = data.closest_waypoint;
      const SimpleWaypointPtr look_ahead_point = data.junction_look_ahead_waypoint;

      const JunctionID junction_id = look_ahead_point->GetWaypoint()->GetJunctionId();
      const TimeInstance current_time = chr::system_clock::now();

      const auto ego_vehicle = boost::static_pointer_cast<cc::Vehicle>(ego_actor);
      TLS traffic_light_state = ego_vehicle->GetTrafficLightState();

      // We determine to stop if the current position of the vehicle is not a
      // junction and there is a red or yellow light.
      if (ego_vehicle->IsAtTrafficLight() &&
          traffic_light_state != TLS::Green &&
          parameters.GetPercentageRunningLight(boost::shared_ptr<cc::Actor>(ego_actor)) <= (rand() % 101)) {

        traffic_light_hazard = true;
      }

      // Handle entry negotiation at non-signalised junction.
      else if (look_ahead_point->CheckJunction() &&
               !ego_vehicle->IsAtTrafficLight() &&
               traffic_light_state != TLS::Green &&
               parameters.GetPercentageRunningSign(boost::shared_ptr<cc::Actor>(ego_actor)) <= (rand() % 101)) {

        std::lock_guard<std::mutex> lock(no_signal_negotiation_mutex);

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

            const TimeInstance &previous_ticket = vehicle_last_ticket.at(ego_actor_id);
            const chr::duration<double> diff = current_time - previous_ticket;
            if (diff.count() > NO_SIGNAL_PASSTHROUGH_INTERVAL) {
              need_to_issue_new_ticket = true;
            }
          }

          // If new ticket is needed for the vehicle, then query the junction
          // ticket map
          // and update the map value to the new ticket value.
          if (need_to_issue_new_ticket) {
            if (junction_last_ticket.find(junction_id) != junction_last_ticket.end()) {

              TimeInstance &last_ticket = junction_last_ticket.at(junction_id);
              const chr::duration<double> diff = current_time - last_ticket;
              if (diff.count() > 0.0) {
                last_ticket = current_time + chr::seconds(NO_SIGNAL_PASSTHROUGH_INTERVAL);
              } else {
                last_ticket += chr::seconds(NO_SIGNAL_PASSTHROUGH_INTERVAL);
              }
            } else {
              junction_last_ticket.insert({junction_id, current_time +
                                           chr::seconds(NO_SIGNAL_PASSTHROUGH_INTERVAL)});
            }
            if (vehicle_last_ticket.find(ego_actor_id) != vehicle_last_ticket.end()) {
              vehicle_last_ticket.at(ego_actor_id) = junction_last_ticket.at(junction_id);
            } else {
              vehicle_last_ticket.insert({ego_actor_id, junction_last_ticket.at(junction_id)});
            }
          }
        }

        // If current time is behind ticket time, then do not enter junction.
        const TimeInstance &current_ticket = vehicle_last_ticket.at(ego_actor_id);
        const chr::duration<double> diff = current_ticket - current_time;
        if (diff.count() > 0.0) {
          traffic_light_hazard = true;
        }
      }

      TrafficLightToPlannerData &message = current_planner_frame->at(i);
      message.traffic_light_hazard = traffic_light_hazard;
    }
  }

  void TrafficLightStage::DataReceiver() {

    localization_frame = localization_messenger->Peek();

    // Allocating new containers for the changed number of registered vehicles.
    if (localization_frame != nullptr && number_of_vehicles != (*localization_frame.get()).size()) {

      number_of_vehicles = static_cast<uint64_t>((*localization_frame.get()).size());

      // Allocating output frames.
      planner_frame_a = std::make_shared<TrafficLightToPlannerFrame>(number_of_vehicles);
      planner_frame_b = std::make_shared<TrafficLightToPlannerFrame>(number_of_vehicles);
    }
  }

  void TrafficLightStage::DataSender() {

    localization_messenger->Pop();

    planner_messenger->Push(frame_selector ? planner_frame_a : planner_frame_b);
    frame_selector = !frame_selector;
  }

  void TrafficLightStage::DrawLight(TLS traffic_light_state, const Actor &ego_actor) const {

    const cg::Location ego_location = ego_actor->GetLocation();
    if (traffic_light_state == TLS::Green) {
      debug_helper.DrawString(
          cg::Location(ego_location.x, ego_location.y, ego_location.z+1.0f),
          "Green",
          false,
          {0u, 255u, 0u}, 0.1f, true);
    } else if (traffic_light_state == TLS::Yellow) {
      debug_helper.DrawString(
          cg::Location(ego_location.x, ego_location.y, ego_location.z+1.0f),
          "Yellow",
          false,
          {255u, 255u, 0u}, 0.1f, true);
    } else {
      debug_helper.DrawString(
          cg::Location(ego_location.x, ego_location.y, ego_location.z+1.0f),
          "Red",
          false,
          {255u, 0u, 0u}, 0.1f, true);
    }
  }

} // namespace traffic_manager
} // namespace carla
