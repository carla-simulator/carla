#pragma once

#include <memory>
#include <map>

#include "carla/geom/Vector3D.h"
#include "carla/geom/Location.h"
#include "carla/geom/Transform.h"

#include "PipelineCallable.h"
#include "SimpleWaypoint.h"
#include "TrafficDistributor.h"

namespace traffic_manager {

  class LocalizationCallable : public PipelineCallable {

    /// This class is the thread executable for the localization stage.
    /// This class is responsible of maintaining a horizon of waypoints ahead
    /// of the vehicle for it to follow.

  private:

    /// Returns the dot product between vehicle's heading vector and
    /// the vector along the direction to the next target waypoint in the
    /// horizon.
    float deviationDotProduct(
        carla::SharedPtr<carla::client::Actor>,
        const carla::geom::Location&
        ) const;

    /// Returns the cross product (z component value) between vehicle's heading
    /// vector and
    /// the vector along the direction to the next target waypoint in the
    /// horizon.
    float deviationCrossProduct(
        carla::SharedPtr<carla::client::Actor>,
        const carla::geom::Location&
        ) const;

  public:

    LocalizationCallable(
        SyncQueue<PipelineMessage> *input_queue,
        SyncQueue<PipelineMessage> *output_queue,
        SharedData *shared_data);
    ~LocalizationCallable();

    PipelineMessage action(PipelineMessage &message);

  };

}
