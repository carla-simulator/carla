#include "LocalizationStage.h"

namespace traffic_manager {

  const float WAYPOINT_TIME_HORIZON = 3.0;
  const float MINIMUM_HORIZON_LENGTH = 25.0;
  const float TARGET_WAYPOINT_TIME_HORIZON = 0.5;
  const float TARGET_WAYPOINT_HORIZON_LENGTH = 2.0;
  const int MINIMUM_JUNCTION_LOOK_AHEAD = 5;
  const float HIGHWAY_SPEED = 50 / 3.6;

  LocalizationStage::LocalizationStage (
    std::vector<carla::SharedPtr<carla::client::Actor>>& actor_list,
    InMemoryMap& local_map,
    std::shared_ptr<LocalizationToPlannerMessenger> planner_messenger,
    int number_of_vehicles,
    int pool_size =1
  ) :
  planner_messenger(planner_messenger),
  actor_list(actor_list),
  local_map(local_map),
  PipelineStage(pool_size, number_of_vehicles)
  {

    /// Initializing buffer lists
    for (int i=0; i < actor_list.size(); i++) {
      buffer_list_a.push_back(std::deque<std::shared_ptr<SimpleWaypoint>>());
      // buffer_list_b.push_back(std::deque<std::shared_ptr<SimpleWaypoint>>());
    }

    planner_frame_a = std::make_shared<LocalizationToPlannerFrame>(actor_list.size());
    planner_frame_b = std::make_shared<LocalizationToPlannerFrame>(actor_list.size());

    frame_selector = true;
    frame_map.insert(std::pair<bool, std::shared_ptr<LocalizationToPlannerFrame>>(true, planner_frame_a));
    frame_map.insert(std::pair<bool, std::shared_ptr<LocalizationToPlannerFrame>>(false, planner_frame_b));

    planner_messenger_state = planner_messenger->GetState() -1;
  }

  LocalizationStage::~LocalizationStage() {}

  void LocalizationStage::Action(int start_index, int end_index) {

    // std::cout 
    // << "Running localizer's action"
    // << " with messenger's state "
    // << planner_messenger->GetState()
    // << " previous state "
    // << planner_messenger_state
    // << std::endl;

    for (int i = start_index; i < end_index; i++) {

      auto vehicle = actor_list.at(i);
      auto actor_id = vehicle->GetId();

      // if (thread_id ==0 and actor_id==first_actor_id)
      //   std::cout << "=======================================start===============================" << std::endl;

      auto vehicle_location = vehicle->GetLocation();
      auto vehicle_velocity = vehicle->GetVelocity().Length();

      float horizon_size = std::max(
          WAYPOINT_TIME_HORIZON * vehicle_velocity,
          MINIMUM_HORIZON_LENGTH);


      auto& waypoint_buffer = buffer_list_a.at(i);
      if (!waypoint_buffer.empty()) {
        auto dot_product = DeviationDotProduct(
          vehicle,
          waypoint_buffer.front()->getLocation());
        while (dot_product <= 0) {
          waypoint_buffer.pop_front();
          if (!waypoint_buffer.empty()) {
            dot_product = DeviationDotProduct(
              vehicle,
              waypoint_buffer.front()->getLocation());
          } else {
            break;
          }
        }
      } else {
        // if (thread_id ==0 and actor_id==first_actor_id)
        //   std::cout << "Buffer empty !" << std::endl;
      }

      /// Initialize buffer if empty
      if (waypoint_buffer.empty()) {
        // if (thread_id ==0 and actor_id==first_actor_id)
        //   std::cout << "Initializing buffer!" << std::endl;
        auto closest_waypoint = local_map.getWaypoint(vehicle_location);
        waypoint_buffer.push_back(closest_waypoint);
      }

      /// Populate buffer
      int populate_count=0;
      while (
        waypoint_buffer.back()->distance(
        waypoint_buffer.front()->getLocation()) <= horizon_size
      ) {

        auto way_front = waypoint_buffer.back();
        auto next_waypoints = way_front->getNextWaypoint();
        auto selection_index = next_waypoints.size() > 1 ? rand() % next_waypoints.size() : 0;
        way_front = next_waypoints[selection_index];
        waypoint_buffer.push_back(way_front);
        populate_count++;
      }
      // if (thread_id ==0 and actor_id==first_actor_id)
      //   std::cout << "Populated buffer with " << populate_count << " entries" << std::endl;

      /// Generate output
      auto horizon_index = static_cast<int>(
        std::max(
          std::ceil(vehicle_velocity * TARGET_WAYPOINT_TIME_HORIZON),
          TARGET_WAYPOINT_HORIZON_LENGTH
        )
      );
      auto target_waypoint = waypoint_buffer.at(horizon_index);
      auto dot_product = DeviationDotProduct(vehicle, target_waypoint->getLocation());
      float cross_product = DeviationCrossProduct(vehicle, target_waypoint->getLocation());
      dot_product = 1 - dot_product;
      if (cross_product < 0) {
        dot_product *= -1;
      }

      auto& message = frame_map.at(frame_selector)->at(i);
      message.actor = vehicle;
      message.deviation = dot_product;

      // auto vehicle_reference = boost::static_pointer_cast<carla::client::Vehicle>(vehicle);
      // auto speed_limit = vehicle_reference->GetSpeedLimit();
      // int look_ahead_index = std::max(
      //   static_cast<int>(std::floor(vehicle_velocity)),
      //   MINIMUM_JUNCTION_LOOK_AHEAD
      // );
      // std::shared_ptr<SimpleWaypoint> look_ahead_point;
      // if (waypoint_buffer.size() > look_ahead_index) {
      //   look_ahead_point = waypoint_buffer.at(look_ahead_index);
      // } else {
      //   look_ahead_point =  waypoint_buffer.back();
      // }
      // auto closest_point = waypoint_buffer.at(1);

      // bool approaching_junction = false;
      // if (
      //   look_ahead_point->checkJunction()
      //   and
      //   !(closest_point->checkJunction())
      // ) {
      //   bool found_true_junction = false;
      //   if (speed_limit > HIGHWAY_SPEED) {
      //     for (int i=0; i<look_ahead_index; i++) {
      //       auto swp = waypoint_buffer.at(i);
      //       if (swp->getNextWaypoint().size() > 1) {
      //         found_true_junction = true;
      //         break;
      //       }
      //     }
      //   } else {
      //     found_true_junction = true;
      //   }
      // }
      // message.approaching_true_junction = approaching_junction;

      // auto current_time = std::chrono::system_clock::now();
      // if (thread_id ==0 and actor_id==first_actor_id) {
      //   std::chrono::duration<double> diff = current_time - last_time;
        // std::cout << "Time for one update : " << diff.count() << std::endl;
        // std::cout << "=======================================start===============================" << std::endl;
    }

    // std::cout 
    // << "Finished localizer's action"
    // << " with messenger's state "
    // << planner_messenger->GetState()
    // << " previous state "
    // << planner_messenger_state
    // << std::endl;

  }

  void LocalizationStage::DataReceiver() {
    // std::cout 
    // << "Ran localizer's receiver"
    // << " with messenger's state "
    // << planner_messenger->GetState()
    // << " previous state "
    // << planner_messenger_state
    // << std::endl;
  }

  void LocalizationStage::DataSender() {
    // std::cout 
    // << "Running localizer's sender"
    // << " with messenger's state "
    // << planner_messenger->GetState()
    // << " previous state "
    // << planner_messenger_state
    // << std::endl;

    DataPacket<std::shared_ptr<LocalizationToPlannerFrame>> data_packet = {
      planner_messenger_state,
      frame_map.at(frame_selector)
    };

    frame_selector = !frame_selector;
    planner_messenger_state = planner_messenger->SendData(data_packet);

    // std::cout 
    // << "Finished localizer's sender"
    // << " with messenger's state "
    // << planner_messenger->GetState()
    // << " previous state "
    // << planner_messenger_state
    // << std::endl;
  }

  float LocalizationStage::DeviationDotProduct(
      carla::SharedPtr<carla::client::Actor> actor,
      const carla::geom::Location &target_location) const {
    auto heading_vector = actor->GetTransform().GetForwardVector();
    auto next_vector = target_location - actor->GetLocation();
    next_vector = next_vector.MakeUnitVector();
    auto dot_product = next_vector.x * heading_vector.x +
        next_vector.y * heading_vector.y + next_vector.z * heading_vector.z;
    return dot_product;
  }

  float LocalizationStage::DeviationCrossProduct(
      carla::SharedPtr<carla::client::Actor> actor,
      const carla::geom::Location &target_location) const {
    auto heading_vector = actor->GetTransform().GetForwardVector();
    auto next_vector = target_location - actor->GetLocation();
    next_vector = next_vector.MakeUnitVector();
    float cross_z = heading_vector.x * next_vector.y - heading_vector.y * next_vector.x;
    return cross_z;
  }
}
