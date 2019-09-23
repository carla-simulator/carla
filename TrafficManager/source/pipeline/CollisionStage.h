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
#include "carla/client/World.h"
#include "carla/geom/Math.h"
#include "carla/geom/Location.h"
#include "carla/geom/Vector3D.h"
#include "carla/Logging.h"
#include "carla/rpc/ActorId.h"

#include "MessengerAndDataTypes.h"
#include "PipelineStage.h"
#include "VicinityGrid.h"

namespace traffic_manager {

namespace cc = carla::client;
namespace cg = carla::geom;
namespace chr = std::chrono;
namespace bg = boost::geometry;

  using ActorId = carla::ActorId;
  using Actor = carla::SharedPtr<cc::Actor>;
  using Polygon = bg::model::polygon<bg::model::d2::point_xy<double>>;
  using LocationList = std::vector<cg::Location>;

  /// This class is the thread executable for the collission detection stage.
  /// The class is responsible for checking possible collisions with other
  /// vehicles along the vehicle's trajectory.
  class CollisionStage : public PipelineStage {

  private:

    /// Reference to carla's world object
    cc::World &world;
    /// Reference to carla's debug helper object
    cc::DebugHelper &debug_helper;
    /// Variables to remember messenger states
    int localization_messenger_state;
    int planner_messenger_state;
    /// Selection key for switching between output frames
    bool frame_selector;
    /// Pointer to data received from localization stage
    std::shared_ptr<LocalizationToCollisionFrame> localization_frame;
    /// Pointers to output frames to be shared with motion planner
    std::shared_ptr<CollisionToPlannerFrame> planner_frame_a;
    std::shared_ptr<CollisionToPlannerFrame> planner_frame_b;
    /// Pointers to messenger objects
    std::shared_ptr<LocalizationToCollisionMessenger> localization_messenger;
    std::shared_ptr<CollisionToPlannerMessenger> planner_messenger;
    /// Object used for grid binning vehicles for faster proximity detection
    VicinityGrid vicinity_grid;
    /// Map used to connect actor ids to array index of data frames
    std::unordered_map<ActorId, uint> id_to_index;
    /// Structure used to keep track of actors spawned outside of traffic
    /// manager
    std::unordered_map<ActorId, Actor> unregistered_actors;
    /// Object used to keep track of time between checking for all world actors
    chr::time_point<chr::_V2::system_clock, chr::nanoseconds> last_world_actors_pass_instance;

    /// Returns true if there is a possible collision detected between the
    /// vehicles passed to the method.
    /// Collision is predicted by extrapolating a boundary around the vehicle
    /// along it's trajectory and checking if it overlaps with the extrapolated
    /// boundary of the other vehicle.
    bool CheckGeodesicCollision(const Actor &vehicle, const Actor &ego_vehicle) const;

    /// Returns the bounding box corners of the vehicle passed to the method.
    LocationList GetBoundary(const Actor &actor) const;

    /// Returns the extrapolated bounding box of the vehicle along it's
    /// trajectory.
    LocationList GetGeodesicBoundary(const Actor &actor) const;

    /// Method to construct a boost polygon object
    Polygon GetPolygon(const LocationList &boundary) const;

    /// Method returns true if ego_vehicle should stop and wait for
    /// other_vehicle to pass.
    bool NegotiateCollision(const Actor &ego_vehicle, const Actor &other_vehicle) const;

    /// Simple method used to draw bounding boxes around vehicles
    void DrawBoundary(const LocationList &boundary) const;

  public:

    CollisionStage(
        std::shared_ptr<LocalizationToCollisionMessenger> localization_messenger,
        std::shared_ptr<CollisionToPlannerMessenger> planner_messenger,
        uint number_of_vehicle,
        uint pool_size,
        cc::World &world,
        cc::DebugHelper &debug_helper);
    ~CollisionStage();

    void DataReceiver() override;

    void Action(const uint start_index, const uint end_index) override;

    void DataSender() override;

  };

}
