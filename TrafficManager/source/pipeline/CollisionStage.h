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

#include "MessengerAndDataTypes.h"
#include "PipelineStage.h"
#include "VicinityGrid.h"

<<<<<<< HEAD
namespace cc = carla::client;

namespace traffic_manager {
  using polygon = boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double>> ;

=======

namespace traffic_manager {

  namespace cc = carla::client;
  namespace chr = std::chrono;

  using polygon = boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double>>;

>>>>>>> e2c8e19611819ecbb7026355674ba94b985ad488
  /// This class is the thread executable for the collission detection stage.
  /// The class is responsible for checking possible collisions with other
  /// vehicles along the vehicle's trajectory.
  class CollisionStage : public PipelineStage {

  private:

<<<<<<< HEAD
    cc::World &world;
    cc::DebugHelper &debug_helper;

=======
    /// Reference to carla's world object
    cc::World &world;
    /// Reference to carla's debug helper object
    cc::DebugHelper &debug_helper;
    /// Variables to remember messenger states
>>>>>>> e2c8e19611819ecbb7026355674ba94b985ad488
    int localization_messenger_state;
    int planner_messenger_state;
    /// Selection key for switching between output frames
    bool frame_selector;
    /// Pointer to data received from localization stage
    std::shared_ptr<LocalizationToCollisionFrame> localization_frame;
    /// Pointers to output frames to be shared with motion planner
    std::shared_ptr<CollisionToPlannerFrame> planner_frame_a;
    std::shared_ptr<CollisionToPlannerFrame> planner_frame_b;
<<<<<<< HEAD
=======
    /// Pointers to messenger objects
>>>>>>> e2c8e19611819ecbb7026355674ba94b985ad488
    std::shared_ptr<LocalizationToCollisionMessenger> localization_messenger;
    std::shared_ptr<CollisionToPlannerMessenger> planner_messenger;
    /// Object used for grid binning vehicles for faster proximity detection
    VicinityGrid vicinity_grid;
    /// Map used to connect actor ids to array index of data frames
    std::unordered_map<uint, int> id_to_index;
<<<<<<< HEAD
    std::unordered_map<uint, carla::SharedPtr<cc::Actor>> unregistered_actors;
    std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::nanoseconds>
    last_world_actors_pass_instance;
=======
    /// Structure used to keep track of actors spawned outside of traffic manager
    std::unordered_map<uint, carla::SharedPtr<cc::Actor>> unregistered_actors;
    /// Object used to keep track of time between checking for all world actors
    chr::time_point<chr::_V2::system_clock, chr::nanoseconds> last_world_actors_pass_instance;
>>>>>>> e2c8e19611819ecbb7026355674ba94b985ad488

    /// Returns true if there is a possible collision detected between the
    /// vehicles passed to the method.
    /// Collision is predicted by extrapolating a boundary around the vehicle
    /// along it's trajectory and checking if it overlaps with the extrapolated
    /// boundary of the other vehicle.
<<<<<<< HEAD

=======
>>>>>>> e2c8e19611819ecbb7026355674ba94b985ad488
    bool CheckGeodesicCollision(
        carla::SharedPtr<cc::Actor> vehicle,
        carla::SharedPtr<cc::Actor> ego_vehicle) const;

    /// Returns the bounding box corners of the vehicle passed to the method.
    std::vector<carla::geom::Location> GetBoundary(carla::SharedPtr<cc::Actor> actor) const;

    /// Returns the extrapolated bounding box of the vehicle along it's
    /// trajectory.
    std::vector<carla::geom::Location> GetGeodesicBoundary(
        carla::SharedPtr<cc::Actor> actor) const;

    /// Method to construct a boost polygon object
    polygon GetPolygon(const std::vector<carla::geom::Location> &boundary) const;

    /// Method returns true if ego_vehicle should stop and wait for
    /// other_vehicle to pass.
    bool NegotiateCollision(
        carla::SharedPtr<cc::Actor> ego_vehicle,
        carla::SharedPtr<cc::Actor> other_vehicle) const;

    /// Simple method used to draw bounding boxes around vehicles
    void DrawBoundary(const std::vector<carla::geom::Location> &boundary) const;

  public:

    CollisionStage(
        std::shared_ptr<LocalizationToCollisionMessenger> localization_messenger,
        std::shared_ptr<CollisionToPlannerMessenger> planner_messenger,
        int number_of_vehicle,
        int pool_size,
        cc::World &world,
        cc::DebugHelper &debug_helper);
    ~CollisionStage();

    void DataReceiver() override;

    void Action(const int start_index, const int end_index) override;

    void DataSender() override;

  };

}
