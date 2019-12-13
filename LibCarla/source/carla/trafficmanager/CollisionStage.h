// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <algorithm>
#include <cmath>
#include <deque>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "boost/geometry.hpp"
#include "boost/geometry/geometries/point_xy.hpp"
#include "boost/geometry/geometries/polygon.hpp"
#include "boost/pointer_cast.hpp"
#include "carla/client/ActorList.h"
#include "carla/client/Vehicle.h"
#include "carla/client/Walker.h"
#include "carla/client/World.h"
#include "carla/geom/Location.h"
#include "carla/geom/Math.h"
#include "carla/geom/Vector3D.h"
#include "carla/Logging.h"
#include "carla/rpc/ActorId.h"
#include "carla/rpc/TrafficLightState.h"

#include "carla/trafficmanager/MessengerAndDataTypes.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/PipelineStage.h"
#include "carla/trafficmanager/VicinityGrid.h"

namespace carla {
namespace traffic_manager {

  namespace cc = carla::client;
  namespace cg = carla::geom;
  namespace chr = std::chrono;
  namespace bg = boost::geometry;

  using ActorId = carla::ActorId;
  using Actor = carla::SharedPtr<cc::Actor>;
  using Polygon = bg::model::polygon<bg::model::d2::point_xy<double>>;
  using LocationList = std::vector<cg::Location>;
  using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;
  using TLS = carla::rpc::TrafficLightState;

  /// This class is the thread executable for the collision detection stage.
  /// The class is responsible for checking possible collisions with other
  /// vehicles along the vehicle's trajectory.
  class CollisionStage : public PipelineStage {

  private:

    /// Variables to remember messenger states.
    int localization_messenger_state;
    int planner_messenger_state;
    /// Selection key for switching between output frames.
    bool frame_selector;
    /// Pointer to data received from localization stage.
    std::shared_ptr<LocalizationToCollisionFrame> localization_frame;
    /// Pointers to output frames to be shared with motion planner stage.
    std::shared_ptr<CollisionToPlannerFrame> planner_frame_a;
    std::shared_ptr<CollisionToPlannerFrame> planner_frame_b;
    /// Pointers to messenger objects.
    std::shared_ptr<LocalizationToCollisionMessenger> localization_messenger;
    std::shared_ptr<CollisionToPlannerMessenger> planner_messenger;
    /// Reference to Carla's world object.
    cc::World &world;
    /// Runtime parameterization object.
    Parameters &parameters;
    /// Reference to Carla's debug helper object.
    cc::DebugHelper &debug_helper;
    /// An object used for grid binning vehicles for faster proximity detection.
    VicinityGrid vicinity_grid;
    /// The map used to connect actor ids to the array index of data frames.
    std::unordered_map<ActorId, uint> vehicle_id_to_index;
    /// A structure used to keep track of actors spawned outside of traffic
    /// manager.
    std::unordered_map<ActorId, Actor> unregistered_actors;
    /// An object used to keep track of time between checking for all world
    /// actors.
    chr::time_point<chr::system_clock, chr::nanoseconds> last_world_actors_pass_instance;
    /// Number of vehicles registered with the traffic manager.
    uint64_t number_of_vehicles;

    /// Returns the bounding box corners of the vehicle passed to the method.
    LocationList GetBoundary(const Actor &actor) const;

    /// Returns the extrapolated bounding box of the vehicle along its
    /// trajectory.
    LocationList GetGeodesicBoundary(const Actor &actor) const;

    /// Method to construct a boost polygon object.
    Polygon GetPolygon(const LocationList &boundary) const;

    /// The method returns true if ego_vehicle should stop and wait for
    /// other_vehicle to pass.
    bool NegotiateCollision(const Actor &ego_vehicle, const Actor &other_vehicle) const;

    /// Method to calculate the speed dependent bounding box extention for a vehicle.
    float GetBoundingBoxExtention(const Actor &ego_vehicle) const;

    /// Method to retreive the set of vehicles around the path of the given vehicle.
    std::unordered_set<ActorId> GetPotentialVehicleObstacles(const Actor &ego_vehicle);

    /// A simple method used to draw bounding boxes around vehicles
    void DrawBoundary(const LocationList &boundary) const;

  public:

    CollisionStage(
        std::string stage_name,
        std::shared_ptr<LocalizationToCollisionMessenger> localization_messenger,
        std::shared_ptr<CollisionToPlannerMessenger> planner_messenger,
        cc::World &world,
        Parameters &parameters,
        cc::DebugHelper &debug_helper);

    ~CollisionStage();

    void DataReceiver() override;

    void Action() override;

    void DataSender() override;

  };

} // namespace traffic_manager
} // namespace carla
