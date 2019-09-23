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
#include "carla/rpc/ActorId.h"

#include "InMemoryMap.h"
#include "MessengerAndDataTypes.h"
#include "PipelineStage.h"
#include "SimpleWaypoint.h"
#include "TrafficDistributor.h"

namespace traffic_manager {

  using Actor = carla::SharedPtr<carla::client::Actor>;
namespace cc = carla::client;

  /// This class is responsible of maintaining a horizon of waypoints ahead
  /// of the vehicle for it to follow.
  /// The class is also responsible for managing lane change decisions and
  /// modify the waypoints trajectory appropriately
  class LocalizationStage : public PipelineStage {

  private:

    /// Reference to carla's debug helper object
    carla::client::DebugHelper &debug_helper;
    /// Variables to remember messenger states
    int planner_messenger_state;
    int collision_messenger_state;
    int traffic_light_messenger_state;
    /// Section keys to switch between output data frames
    bool planner_frame_selector;
    bool collision_frame_selector;
    bool traffic_light_frame_selector;
    /// Output data frames to be shared with motion planner stage
    std::shared_ptr<LocalizationToPlannerFrame> planner_frame_a;
    std::shared_ptr<LocalizationToPlannerFrame> planner_frame_b;
    /// Output data frames to be shared with collision stage
    std::shared_ptr<LocalizationToCollisionFrame> collision_frame_a;
    std::shared_ptr<LocalizationToCollisionFrame> collision_frame_b;
    /// Output data frames to be shared with traffic light stage
    std::shared_ptr<LocalizationToTrafficLightFrame> traffic_light_frame_a;
    std::shared_ptr<LocalizationToTrafficLightFrame> traffic_light_frame_b;
    /// Pointer to messenger to motion planner stage
    std::shared_ptr<LocalizationToPlannerMessenger> planner_messenger;
    /// Pointer to messenger to collision stage
    std::shared_ptr<LocalizationToCollisionMessenger> collision_messenger;
    /// Pointer to messenger to traffic light stage
    std::shared_ptr<LocalizationToTrafficLightMessenger> traffic_light_messenger;
    /// Reference to local map cache object
    InMemoryMap &local_map;
    /// Random seed array for turn decisions
    std::vector<uint> divergence_choice;
    /// Structures to hold waypoint buffers for all vehicles
    /// These are shared with collisions stage
    std::shared_ptr<BufferList> buffer_list_a;
    std::shared_ptr<BufferList> buffer_list_b;
    /// Object used to keep track of vehicles according to their map position,
    /// determine and execute lane changes
    TrafficDistributor traffic_distributor;
    /// Map connecting actor ids to index of data arrays
    std::unordered_map<carla::ActorId, uint> vehicle_id_to_index;
    /// Reference to list of all the actors registered with traffic manager
    std::vector<Actor> &actor_list;

    /// Simple method used to draw waypoint buffer ahead of a vehicle
    void DrawBuffer(Buffer &buffer);

  public:

    LocalizationStage(
        std::shared_ptr<LocalizationToPlannerMessenger> planner_messenger,
        std::shared_ptr<LocalizationToCollisionMessenger> collision_messenger,
        std::shared_ptr<LocalizationToTrafficLightMessenger> traffic_light_messenger,
        uint number_of_vehicles,
        uint pool_size,
        std::vector<Actor> &actor_list,
        InMemoryMap &local_map,
        cc::DebugHelper &debug_helper);

    ~LocalizationStage();

    void DataReceiver() override;

    void Action(const uint start_index, const uint end_index) override;

    void DataSender() override;

  };

}
