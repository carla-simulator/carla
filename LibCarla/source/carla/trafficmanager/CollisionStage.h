
#pragma once

#include <memory>

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wshadow"
#endif
#include "boost/geometry.hpp"
#include "boost/geometry/geometries/geometries.hpp"
#include "boost/geometry/geometries/point_xy.hpp"
#include "boost/geometry/geometries/polygon.hpp"
#if defined(__clang__)
#  pragma clang diagnostic pop
#endif

#include "carla/trafficmanager/DataStructures.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/RandomGenerator.h"
#include "carla/trafficmanager/SimulationState.h"
#include "carla/trafficmanager/Stage.h"

namespace carla {
namespace traffic_manager {

struct GeometryComparison {
  double reference_vehicle_to_other_geodesic;
  double other_vehicle_to_reference_geodesic;
  double inter_geodesic_distance;
  double inter_bbox_distance;
};

struct CollisionLock {
  double distance_to_lead_vehicle;
  double initial_lock_distance;
  ActorId lead_vehicle_id;
};
using CollisionLockMap = std::unordered_map<ActorId, CollisionLock>;

namespace cc = carla::client;
namespace bg = boost::geometry;

using Buffer = std::deque<std::shared_ptr<SimpleWaypoint>>;
using BufferMap = std::unordered_map<carla::ActorId, Buffer>;
using LocationVector = std::vector<cg::Location>;
using GeodesicBoundaryMap = std::unordered_map<ActorId, LocationVector>;
using GeometryComparisonMap = std::unordered_map<uint64_t, GeometryComparison>;
using Polygon = bg::model::polygon<bg::model::d2::point_xy<double>>;

/// This class has functionality to detect potential collision with a nearby actor.
class CollisionStage : Stage {
private:
  const std::vector<ActorId> &vehicle_id_list;
  const SimulationState &simulation_state;
  const BufferMap &buffer_map;
  const TrackTraffic &track_traffic;
  const Parameters &parameters;
  CollisionFrame &output_array;
  // Structure keeping track of blocking lead vehicles.
  CollisionLockMap collision_locks;
  // Structures to cache geodesic boundaries of vehicle and
  // comparision between vehicle boundaries
  // to avoid repeated computation within a cycle.
  GeometryComparisonMap geometry_cache;
  GeodesicBoundaryMap geodesic_boundary_map;
  RandomGenerator &random_device;

  // Method to determine if a vehicle is on a collision path to another.
  std::pair<bool, float> NegotiateCollision(const ActorId reference_vehicle_id,
                                            const ActorId other_actor_id,
                                            const uint64_t reference_junction_look_ahead_index);

  // Method to calculate bounding box extention length ahead of the vehicle.
  float GetBoundingBoxExtention(const ActorId actor_id);

  // Method to calculate polygon points around the vehicle's bounding box.
  LocationVector GetBoundary(const ActorId actor_id);

  // Method to construct polygon points around the path boundary of the vehicle.
  LocationVector GetGeodesicBoundary(const ActorId actor_id);

  Polygon GetPolygon(const LocationVector &boundary);

  // Method to compare path boundaries, bounding boxes of vehicles
  // and cache the results for reuse in current update cycle.
  GeometryComparison GetGeometryBetweenActors(const ActorId reference_vehicle_id,
                                              const ActorId other_actor_id);

  // Method to draw path boundary.
  void DrawBoundary(const LocationVector &boundary);

public:
  CollisionStage(const std::vector<ActorId> &vehicle_id_list,
                 const SimulationState &simulation_state,
                 const BufferMap &buffer_map,
                 const TrackTraffic &track_traffic,
                 const Parameters &parameters,
                 CollisionFrame &output_array,
                 RandomGenerator &random_device);

  void Update (const unsigned long index) override;

  void RemoveActor(const ActorId actor_id) override;

  void Reset() override;

  // Method to flush cache for current update cycle.
  void ClearCycleCache();
};

} // namespace traffic_manager
} // namespace carla
