#pragma once

#include <memory>
#include <deque>
#include <algorithm>

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

namespace traffic_manager {

  typedef std::vector<std::deque<std::shared_ptr<SimpleWaypoint>>> BufferList;

  class LocalizationStage : PipelineStage {

    /// This class is responsible of maintaining a horizon of waypoints ahead
    /// of the vehicle for it to follow.

  private:

    int planner_messenger_state;
    LocalizationToPlannerFrame planner_frame_a;
    LocalizationToPlannerFrame planner_frame_b;
    std::shared_ptr<LocalizationToPlannerMessenger> planner_messenger;

    InMemoryMap& local_map;
    BufferList buffer_list_a;
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

  public:

    LocalizationStage(
      std::vector<carla::SharedPtr<carla::client::Actor>>& actor_list,
      InMemoryMap& local_map,
      std::shared_ptr<LocalizationToPlannerMessenger> planner_messenger,
      int pool_size,
      int number_of_vehicles
    );

    ~LocalizationStage();

    void DataReceiver() override;
    void Action(int array_index) override;
    void DataSender() override;

    using PipelineStage::Start;

  };

}
