#pragma once

#include <algorithm>
#include <cmath>
#include <deque>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "carla/client/Actor.h"
#include "carla/client/Vehicle.h"
#include "carla/geom/Math.h"
#include "carla/geom/Location.h"
#include "carla/geom/Transform.h"
#include "carla/geom/Vector3D.h"
#include "carla/Memory.h"

#include "InMemoryMap.h"
#include "MessengerAndDataTypes.h"
#include "PipelineStage.h"
#include "SimpleWaypoint.h"
#include "TrafficDistributor.h"

namespace traffic_manager {

  /// This class is responsible of maintaining a horizon of waypoints ahead
  /// of the vehicle for it to follow.
  /// The class is also responsible for managing lane change decisions and
  /// modify the waypoints trajectory appropriately
  class LocalizationStage : public PipelineStage {

  private:

    carla::client::DebugHelper &debug_helper;

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

    std::shared_ptr<LocalizationToPlannerMessenger> planner_messenger;
    std::shared_ptr<LocalizationToCollisionMessenger> collision_messenger;
    std::shared_ptr<LocalizationToTrafficLightMessenger> traffic_light_messenger;

    InMemoryMap &local_map;
    std::vector<int> divergence_choice;
    std::shared_ptr<BufferList> buffer_list_a;
    std::shared_ptr<BufferList> buffer_list_b;
    TrafficDistributor traffic_distributor;
    std::unordered_map<uint, int> vehicle_id_to_index;
    std::vector<carla::geom::Location> last_lane_change_location;
    std::unordered_map<bool, std::vector<carla::geom::Location> *> last_lane_change_map;
    std::vector<carla::SharedPtr<carla::client::Actor>> &actor_list;

    void drawBuffer(Buffer &buffer);

  public:

    LocalizationStage(
        std::shared_ptr<LocalizationToPlannerMessenger> planner_messenger,
        std::shared_ptr<LocalizationToCollisionMessenger> collision_messenger,
        std::shared_ptr<LocalizationToTrafficLightMessenger> traffic_light_messenger,
        int number_of_vehicles,
        int pool_size,
        std::vector<carla::SharedPtr<carla::client::Actor>> &actor_list,
        InMemoryMap &local_map,
        carla::client::DebugHelper &debug_helper);

    ~LocalizationStage();

    void DataReceiver() override;

    void Action(const int start_index, const int end_index) override;

    void DataSender() override;

  };

}
