#include "TrafficLightStage.h"
#include "iostream"

namespace traffic_manager {

  static const uint NO_SIGNAL_PASSTHROUGH_INTERVAL = 5u;
  static bool initialized = false;
  TrafficLightStage::TrafficLightStage(
      std::string stage_name,
      std::shared_ptr<LocalizationToTrafficLightMessenger> localization_messenger,
      std::shared_ptr<TrafficLightToPlannerMessenger> planner_messenger,
      cc::DebugHelper &debug_helper,
      cc::World &world)
    : localization_messenger(localization_messenger),
      planner_messenger(planner_messenger),
      debug_helper(debug_helper),
      world(world),
      PipelineStage(stage_name) {

    // Initializing output frame selector.
    frame_selector = true;

    // Initializing messenger state.
    localization_messenger_state = localization_messenger->GetState();

    // Initializing this messenger state to preemptively write
    // since this stage precedes motion planner stage.
    planner_messenger_state = planner_messenger->GetState() - 1;

    // Initializing number of vehicles to zero in the beginning.
    number_of_vehicles = 0u;
  }

  TrafficLightStage::~TrafficLightStage() {}

  void TrafficLightStage::ResetAllTrafficLightGroups() {
    // TO BE FINISHED
    if (!initialized) {
      initialized = true;
      auto world_traffic_lights = world.GetActors()->Filter("*traffic_light*");
      for (auto tl : *world_traffic_lights.get()) {
        auto group = boost::static_pointer_cast<cc::TrafficLight>(tl)->GetGroupTrafficLights();
        for (auto g : group) {
          std::cout << g->GetId() << std::endl;
        }
      }
    }
  }

  void TrafficLightStage::Action() {

    // Selecting the output frame based on the selection key.
    auto current_planner_frame = frame_selector ? planner_frame_a : planner_frame_b;
    // Looping over registered actors.
    for (uint i = 0u; i < number_of_vehicles; ++i) {

      bool traffic_light_hazard = false;
      LocalizationToTrafficLightData &data = localization_frame->at(i);
      Actor ego_actor = data.actor;
      ActorId ego_actor_id = ego_actor->GetId();
      SimpleWaypointPtr closest_waypoint = data.closest_waypoint;
      SimpleWaypointPtr look_ahead_point = data.junction_look_ahead_waypoint;

      JunctionID junction_id = look_ahead_point->GetWaypoint()->GetJunctionId();
      TimeInstance current_time = chr::system_clock::now();

      auto ego_vehicle = boost::static_pointer_cast<cc::Vehicle>(ego_actor);
      TLS traffic_light_state = ego_vehicle->GetTrafficLightState();
      //DrawLight(traffic_light_state, ego_actor);
      ResetAllTrafficLightGroups();
      // We determine to stop if the current position of the vehicle is not a
      // junction,
      // a point on the path beyond a threshold (velocity-dependent) distance
      // is inside the junction and there is a red or yellow light.
      if (ego_vehicle->IsAtTrafficLight() &&
          traffic_light_state != TLS::Green) {

        traffic_light_hazard = true;
      }
      // Handle entry negotiation at non-signalised junction.
      else if (!ego_vehicle->IsAtTrafficLight() &&
          traffic_light_state != TLS::Green) {

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

            TimeInstance &previous_ticket = vehicle_last_ticket.at(ego_actor_id);
            chr::duration<double> diff = current_time - previous_ticket;
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
              chr::duration<double> diff = current_time - last_ticket;
              if (diff.count() > 0) {
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
        TimeInstance &current_ticket = vehicle_last_ticket.at(ego_actor_id);
        chr::duration<double> diff = current_ticket - current_time;
        if (diff.count() > 0) {
          traffic_light_hazard = true;
        }
      }

      TrafficLightToPlannerData &message = current_planner_frame->at(i);
      message.traffic_light_hazard = traffic_light_hazard;
    }

  }

  void TrafficLightStage::DataReceiver() {
    auto packet = localization_messenger->ReceiveData(localization_messenger_state);
    localization_frame = packet.data;
    localization_messenger_state = packet.id;

    // Allocating new containers for the changed number of registered vehicles.
    if (localization_frame != nullptr &&
        number_of_vehicles != (*localization_frame.get()).size()) {

      number_of_vehicles = static_cast<uint>((*localization_frame.get()).size());
      // Allocating output frames.
      planner_frame_a = std::make_shared<TrafficLightToPlannerFrame>(number_of_vehicles);
      planner_frame_b = std::make_shared<TrafficLightToPlannerFrame>(number_of_vehicles);
    }
  }

  void TrafficLightStage::DataSender() {

    DataPacket<std::shared_ptr<TrafficLightToPlannerFrame>> packet{
      planner_messenger_state,
      frame_selector ? planner_frame_a : planner_frame_b
    };
    frame_selector = !frame_selector;
    planner_messenger_state = planner_messenger->SendData(packet);
  }

  void TrafficLightStage::DrawLight(TLS traffic_light_state, const Actor &ego_actor) const {
    std::string str;
    if (traffic_light_state == TLS::Green) {
      str="Green";
      debug_helper.DrawString(
        cg::Location(ego_actor->GetLocation().x, ego_actor->GetLocation().y, ego_actor->GetLocation().z+1),
        str,
        false,
        {0u, 255u, 0u}, 0.1f, true);
    }

    else if (traffic_light_state == TLS::Yellow) {
      str="Yellow";
      debug_helper.DrawString(
        cg::Location(ego_actor->GetLocation().x, ego_actor->GetLocation().y, ego_actor->GetLocation().z+1),
        str,
        false,
        {255u, 255u, 0u}, 0.1f, true);
    }

    else {
      str="Red";
      debug_helper.DrawString(
        cg::Location(ego_actor->GetLocation().x, ego_actor->GetLocation().y, ego_actor->GetLocation().z+1),
        str,
        false,
        {255u, 0u, 0u}, 0.1f, true);
    }
  }
}
