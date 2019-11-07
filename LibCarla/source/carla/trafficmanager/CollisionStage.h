#pragma once

#include <algorithm>
#include <cmath>
#include <deque>
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

#include "carla/trafficmanager/AtomicActorSet.h"
#include "carla/trafficmanager/AtomicMap.h"
#include "carla/trafficmanager/MessengerAndDataTypes.h"
#include "carla/trafficmanager/PipelineStage.h"
#include "carla/trafficmanager/VicinityGrid.h"

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
    /// Map specifying selective collision avoidance rules between vehicles.
    AtomicMap<ActorId, std::shared_ptr<AtomicActorSet>> &selective_collision;
    /// Map specifying distance to leading vehicle.
    AtomicMap<ActorId, float> &distance_to_leading_vehicle;
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
    chr::time_point<chr::_V2::system_clock, chr::nanoseconds> last_world_actors_pass_instance;
    /// Number of vehicles registered with the traffic manager.
    uint number_of_vehicles;

    /// Stage throughput count.
    uint throughput_count = 0;

    /// Returns true if there is a possible collision detected between the
    /// vehicles passed to the method.
    /// Collision is predicted by extrapolating a boundary around the vehicle
    /// along its trajectory and checking if it overlaps with the extrapolated
    /// boundary of the other vehicle.
    bool CheckOverlap(const LocationList &boundary_a, const LocationList &boundary_b) const;

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

    /// A simple method used to draw bounding boxes around vehicles
    void DrawBoundary(const LocationList &boundary) const;

  public:

    CollisionStage(
        std::shared_ptr<LocalizationToCollisionMessenger> localization_messenger,
        std::shared_ptr<CollisionToPlannerMessenger> planner_messenger,
        cc::World &world,
        AtomicMap<ActorId, std::shared_ptr<AtomicActorSet>> &selective_collision,
        AtomicMap<ActorId, float> &distance_to_leading_vehicle,
        cc::DebugHelper &debug_helper);

    ~CollisionStage();

    void DataReceiver() override;

    void Action() override;

    void DataSender() override;

  };

}
