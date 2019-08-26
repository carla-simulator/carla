#pragma once

#include <cmath>
#include <deque>
#include <string>
#include <vector>
#include <algorithm>

#include "boost/geometry.hpp"
#include "boost/geometry/geometries/point_xy.hpp"
#include "boost/geometry/geometries/polygon.hpp"
#include "boost/foreach.hpp"
#include "boost/pointer_cast.hpp"
#include "carla/client/Vehicle.h"
#include "carla/geom/Location.h"
#include "carla/geom/Vector3D.h"

#include "PipelineCallable.h"

namespace traffic_manager {
  typedef boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double>> polygon;

  class CollisionCallable : public PipelineCallable {
    /// This class is the thread executable for the collission detection stage.
    /// The class is responsible for checking possible collisions with other
    /// vehicles along the vehicle's trajectory.

  private:

    /// Draws a polygon connecting the vector of locations passed to it.
    void drawBoundary(const std::vector<carla::geom::Location> &) const;

    /// Returns true if there is a possible collision detected between the
    /// vehicles passed to the method.
    /// Collision is predicted by extrapolating a boundary around the vehicle
    /// along it's trajectory
    /// and checking if it overlaps with the extrapolated boundary of the other
    /// vehicle.

    bool checkGeodesicCollision(
        carla::SharedPtr<carla::client::Actor> vehicle,
        carla::SharedPtr<carla::client::Actor> ego_vehicle) const;

    /// Returns the bounding box corners of the vehicle passed to the method.
    std::vector<carla::geom::Location> getBoundary(carla::SharedPtr<carla::client::Actor> actor)const;

    /// Returns the extrapolated bounding box of the vehicle along it's
    /// trajectory.
    std::vector<carla::geom::Location> getGeodesicBoundary(
        carla::SharedPtr<carla::client::Actor> actor,
        const std::vector<carla::geom::Location> &bbox)const;

    /// Method to construct a boost polygon object
    polygon getPolygon(const std::vector<carla::geom::Location> &boundary) const;

    /// Method returns true if ego_vehicle should stop and wait for
    /// other_vehicle to pass.
    bool negotiateCollision(
        carla::SharedPtr<carla::client::Actor> ego_vehicle,
        carla::SharedPtr<carla::client::Actor> other_vehicle)const;

    /* Method for getting the nearby vehicles */
    std::map< carla::SharedPtr <carla::client::Actor > , int> getClosestActors(carla::SharedPtr<carla::client::Actor> actor);

  public:

    CollisionCallable(
        SyncQueue<PipelineMessage> *input_queue,
        SyncQueue<PipelineMessage> *output_queue,
        SharedData *shared_data);
    ~CollisionCallable();

    PipelineMessage action(PipelineMessage &message);

  };

}
