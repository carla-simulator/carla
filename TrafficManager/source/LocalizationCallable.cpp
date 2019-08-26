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

    if (
      shared_data->buffer_map.contains(actor_id)
      and
      !(shared_data->buffer_map.get(actor_id)->empty())
    ) { // Existing actor in buffer map

      auto waypoint_buffer = shared_data->buffer_map.get(actor_id);
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

      /// Re-initialize buffer if empty
      if (waypoint_buffer->empty()) {
        auto closest_waypoint = shared_data->local_map->getWaypoint(vehicle_location);
        waypoint_buffer->push(closest_waypoint);
      }

      /// Re-populate buffer
      while (
        waypoint_buffer->back()->distance(
        waypoint_buffer->front()->getLocation()) <= horizon_size // Make this a constant
      ) {
        auto next_waypoints = waypoint_buffer->back()->getNextWaypoint();
        auto selection_index = next_waypoints.size() > 1 ? rand() % next_waypoints.size() : 0;
        auto feed_waypoint = next_waypoints[selection_index];
        waypoint_buffer->push(feed_waypoint);
      }

    } else {       // New actor to buffer map

      /// Make size of queue a derived or constant
      while (!(shared_data->buffer_map.contains(actor_id))) {
        shared_data->buffer_map.put(
            actor_id, std::make_shared<SyncQueue<std::shared_ptr<SimpleWaypoint>>>(200)
        );
      }
      auto closest_waypoint = shared_data->local_map->getWaypoint(vehicle_location);
      /// Initialize buffer for actor
      auto waypoint_buffer = shared_data->buffer_map.get(actor_id);
      waypoint_buffer->push(closest_waypoint);
      /// Populate buffer
      while (
        waypoint_buffer->back()->distance(
        waypoint_buffer->front()->getLocation()) <= horizon_size // Make this a constant
      ) {
        auto next_waypoints = closest_waypoint->getNextWaypoint();
        auto selection_index = next_waypoints.size() > 1 ? rand() % next_waypoints.size() : 0;
        closest_waypoint = next_waypoints[selection_index];
        waypoint_buffer->push(closest_waypoint);
      }
    }

    /// Generate output message
    PipelineMessage out_message;
    auto horizon_index = static_cast<int>(
      std::max(
      std::ceil(vehicle_velocity * TARGET_WAYPOINT_TIME_HORIZON),
      TARGET_WAYPOINT_HORIZON_LENGTH)
      );
    auto target_waypoint = shared_data->buffer_map.get(actor_id)->get(horizon_index);
    float dot_product = deviationDotProduct(vehicle, target_waypoint->getLocation());
    float cross_product = deviationCrossProduct(vehicle, target_waypoint->getLocation());
    dot_product = 1 - dot_product;
    if (cross_product < 0) {
      dot_product *= -1;
    }
    out_message.setActor(message.getActor());
    out_message.setAttribute("velocity", vehicle_velocity);
    out_message.setAttribute("deviation", dot_product);

    return out_message;
  }

  float LocalizationCallable::deviationDotProduct(
      carla::SharedPtr<carla::client::Actor> actor,
      const carla::geom::Location& target_location) const{
    auto heading_vector = actor->GetTransform().GetForwardVector();
    auto next_vector = target_location - actor->GetLocation();
    next_vector = next_vector.MakeUnitVector();
    auto dot_product = next_vector.x * heading_vector.x +
        next_vector.y * heading_vector.y + next_vector.z * heading_vector.z;
    return dot_product;
  }

  float LocalizationCallable::deviationCrossProduct(
      carla::SharedPtr<carla::client::Actor> actor,
      const carla::geom::Location& target_location) const {
    auto heading_vector = actor->GetTransform().GetForwardVector();
    auto next_vector = target_location - actor->GetLocation();
    next_vector = next_vector.MakeUnitVector();
    float cross_z = heading_vector.x * next_vector.y - heading_vector.y * next_vector.x;
    return cross_z;
  }
}
