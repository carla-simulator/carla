#include "LocalizationCallable.h"

namespace traffic_manager {

  const float WAYPOINT_TIME_HORIZON = 3.0;
  const float MINIMUM_HORIZON_LENGTH = 25.0;
  const float TARGET_WAYPOINT_TIME_HORIZON = 0.5;
  const float TARGET_WAYPOINT_HORIZON_LENGTH = 2.0;

  LocalizationCallable::LocalizationCallable(
      SyncQueue<PipelineMessage> *input_queue,
      SyncQueue<PipelineMessage> *output_queue,
      SharedData *shared_data) :  PipelineCallable(input_queue, output_queue, shared_data) {}

  LocalizationCallable::~LocalizationCallable() {}

  PipelineMessage LocalizationCallable::action(PipelineMessage &message) {

    auto vehicle = message.getActor();
    auto actor_id = message.getActorID();

    auto vehicle_location = vehicle->GetLocation();
    auto vehicle_velocity = vehicle->GetVelocity().Length();

    float horizon_size = std::max(
        WAYPOINT_TIME_HORIZON * vehicle_velocity,
        MINIMUM_HORIZON_LENGTH);

    if (shared_data->buffer_map.contains(actor_id)) { // Existing actor in
                                                      // buffer map

      auto waypoint_buffer = shared_data->buffer_map.get(actor_id);
      if (!waypoint_buffer->empty()) {
        /// Purge past waypoints
        auto dot_product = deviationDotProduct(
            vehicle,
            waypoint_buffer->front()->getLocation());
        while (dot_product <= 0) {
          waypoint_buffer->pop();
          if (!waypoint_buffer->empty()) {
            dot_product = deviationDotProduct(
                vehicle,
                waypoint_buffer->front()->getLocation());
          } else {
            break;
          }
        }
      }

      /// Re-initialize buffer if empty
      if (waypoint_buffer->empty()) {
        auto closest_waypoint = shared_data->local_map->getWaypoint(vehicle_location);
        waypoint_buffer->push(closest_waypoint);
      }

      try
      {
        /// Make lane change decisions
        auto simple_way_front = waypoint_buffer->back();
        auto way_front = simple_way_front->getWaypoint();
        auto lane_change_waypoints = shared_data->traffic_distributor.assignDistribution(
            actor_id,
            way_front->GetRoadId(),
            way_front->GetSectionId(),
            way_front->GetLaneId(),
            simple_way_front);
        for (auto wp: lane_change_waypoints) {
          waypoint_buffer->push(wp);
        }        
      }
      catch(const std::exception& e)
      {
        std::cout << "Failed to make lane change decision for actor : " << actor_id << std::endl;
        std::cout << e.what() << '\n';
      }

    } else {       // New actor to buffer map

      /// Make size of queue a derived or constant
      auto waypoint_buffer = std::make_shared<SyncQueue<std::shared_ptr<SimpleWaypoint>>>(200);
      shared_data->buffer_map.put(actor_id, waypoint_buffer);

      /// Initialize buffer for actor
      auto closest_waypoint = shared_data->local_map->getWaypoint(vehicle_location);
      waypoint_buffer->push(closest_waypoint);

    }

    auto waypoint_buffer = shared_data->buffer_map.get(actor_id);
    /// Populate buffer
    while (
      waypoint_buffer->back()->distance(
      waypoint_buffer->front()->getLocation()) <= horizon_size // Make this a
                                                              // constant
    ) {
      
      auto way_front = waypoint_buffer->back();
      auto next_waypoints = way_front->getNextWaypoint();
      auto selection_index = next_waypoints.size() > 1 ? rand() % next_waypoints.size() : 0;
      way_front = next_waypoints[selection_index];
      waypoint_buffer->push(way_front);
    }

    /// Generate output message
    PipelineMessage out_message;
    out_message.setActor(message.getActor());
    out_message.setAttribute("velocity", vehicle_velocity);

    auto horizon_index = static_cast<int>(
      std::max(
      std::ceil(vehicle_velocity * TARGET_WAYPOINT_TIME_HORIZON),
      TARGET_WAYPOINT_HORIZON_LENGTH)
    );
    auto target_waypoint = waypoint_buffer->get(horizon_index);
    auto dot_product = deviationDotProduct(vehicle, target_waypoint->getLocation());
    float cross_product = deviationCrossProduct(vehicle, target_waypoint->getLocation());
    dot_product = 1 - dot_product;
    if (cross_product < 0) {
      dot_product *= -1;
    }
    out_message.setAttribute("deviation", dot_product);

    return out_message;
  }

  float LocalizationCallable::deviationDotProduct(
      carla::SharedPtr<carla::client::Actor> actor,
      const carla::geom::Location &target_location) const {
    auto heading_vector = actor->GetTransform().GetForwardVector();
    auto next_vector = target_location - actor->GetLocation();
    next_vector = next_vector.MakeUnitVector();
    auto dot_product = next_vector.x * heading_vector.x +
        next_vector.y * heading_vector.y + next_vector.z * heading_vector.z;
    return dot_product;
  }

  float LocalizationCallable::deviationCrossProduct(
      carla::SharedPtr<carla::client::Actor> actor,
      const carla::geom::Location &target_location) const {
    auto heading_vector = actor->GetTransform().GetForwardVector();
    auto next_vector = target_location - actor->GetLocation();
    next_vector = next_vector.MakeUnitVector();
    float cross_z = heading_vector.x * next_vector.y - heading_vector.y * next_vector.x;
    return cross_z;
  }
}
