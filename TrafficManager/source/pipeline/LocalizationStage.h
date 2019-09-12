#pragma once

#include <memory>
#include <deque>
#include <unordered_map>
#include <algorithm>
#include <mutex>
#include <chrono>

#include "carla/client/Actor.h"
#include "carla/geom/Vector3D.h"
#include "carla/geom/Location.h"
#include "carla/geom/Transform.h"
#include "carla/Memory.h"
#include "carla/client/Vehicle.h"

#include "PipelineStage.h"
#include "SimpleWaypoint.h"
#include "MessengerAndDataTypes.h"
#include "InMemoryMap.h"
#include "TrafficDistribution.h"

namespace traffic_manager {

  typedef std::chrono::time_point<
    std::chrono::_V2::system_clock,
    std::chrono::nanoseconds
    > TimeInstance;

  class LocalizationStage : PipelineStage {

    /// This class is responsible of maintaining a horizon of waypoints ahead
    /// of the vehicle for it to follow.

  private:

    carla::client::DebugHelper& debug_helper;

    int planner_messenger_state;
    int collision_messenger_state;
    int traffic_light_messenger_state;
    bool planner_frame_selector;
    bool collision_frame_selector;
    bool traffic_light_frame_selector;

    std::shared_ptr<LocalizationToPlannerFrame> planner_frame_a;
    std::shared_ptr<LocalizationToPlannerFrame> planner_frame_b;

    std::shared_ptr<LocalizationToCollisionFrame> collision_frame_a;
    std::shared_ptr<LocalizationToCollisionFrame> collision_frame_b;

    std::shared_ptr<LocalizationToTrafficLightFrame> traffic_light_frame_a;
    std::shared_ptr<LocalizationToTrafficLightFrame> traffic_light_frame_b;

    std::unordered_map<bool, std::shared_ptr<LocalizationToPlannerFrame>> planner_frame_map;
    std::unordered_map<bool, std::shared_ptr<LocalizationToCollisionFrame>> collision_frame_map;
    std::unordered_map<bool, std::shared_ptr<LocalizationToTrafficLightFrame>> traffic_light_frame_map;

    std::shared_ptr<LocalizationToPlannerMessenger> planner_messenger;
    std::shared_ptr<LocalizationToCollisionMessenger> collision_messenger;
    std::shared_ptr<LocalizationToTrafficLightMessenger> traffic_light_messenger;

    InMemoryMap& local_map;
    std::vector<int> divergence_choice;
    std::shared_ptr<BufferList> buffer_list_a;
    std::shared_ptr<BufferList> buffer_list_b;
    std::unordered_map<bool, std::shared_ptr<BufferList>> buffer_map;
    TrafficDistribution traffic_distribution;
    std::unordered_map<uint, int> vehicle_id_to_index;
    std::vector<carla::geom::Location> last_lane_change_location;
    std::unordered_map<bool, std::vector<carla::geom::Location>*> last_lane_change_map;
    std::vector<carla::SharedPtr<carla::client::Actor>>& actor_list;

    /// Returns the dot product between vehicle's heading vector and
    /// the vector along the direction to the next target waypoint in the
    /// horizon.
    float DeviationDotProduct(
        carla::SharedPtr<carla::client::Actor>,
        const carla::geom::Location &) const;

    /// Returns the cross product (z component value) between vehicle's heading
    /// vector and the vector along the direction to the next target waypoint in the
    /// horizon.
    float DeviationCrossProduct(
        carla::SharedPtr<carla::client::Actor>,
        const carla::geom::Location &) const;

    void drawBuffer(Buffer& buffer);

  public:

    LocalizationStage(
      std::shared_ptr<LocalizationToPlannerMessenger> planner_messenger,
      std::shared_ptr<LocalizationToCollisionMessenger> collision_messenger,
      std::shared_ptr<LocalizationToTrafficLightMessenger> traffic_light_messenger,
      int number_of_vehicles,
      int pool_size,
      std::vector<carla::SharedPtr<carla::client::Actor>>& actor_list,
      InMemoryMap& local_map,
      carla::client::DebugHelper& debug_helper
    );

    ~LocalizationStage();

    void DataReceiver() override;
    void Action(int start_index, int end_index) override;
    void DataSender() override;

    using PipelineStage::Start;
    using PipelineStage::Stop;

  };

}
