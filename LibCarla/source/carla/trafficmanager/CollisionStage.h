
/// This file has functionality to detect potential collision with a nearby actor.

#include <memory>

#include "carla/Memory.h"

#include "boost/geometry.hpp"
#include "boost/geometry/geometries/geometries.hpp"
#include "boost/geometry/geometries/point_xy.hpp"
#include "boost/geometry/geometries/polygon.hpp"
#include "carla/geom/Math.h"

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/DataStructures.h"
#include "carla/trafficmanager/LocalizationUtils.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/SimpleWaypoint.h"
#include "carla/trafficmanager/SimulationState.h"
#include "carla/trafficmanager/Stage.h"

namespace carla
{
namespace traffic_manager
{

struct GeometryComparison
{
  double reference_vehicle_to_other_geodesic;
  double other_vehicle_to_reference_geodesic;
  double inter_geodesic_distance;
  double inter_bbox_distance;
};

namespace bg = boost::geometry;

using Buffer = std::deque<std::shared_ptr<SimpleWaypoint>>;
using BufferMap = std::unordered_map<carla::ActorId, Buffer>;
using BufferMapPtr = std::shared_ptr<BufferMap>;
using LocationList = std::vector<cg::Location>;
using GeodesicBoundaryMap = std::unordered_map<ActorId, LocationList>;
using GeometryComparisonMap = std::unordered_map<std::string, GeometryComparison>;
using Point2D = bg::model::point<double, 2, bg::cs::cartesian>;
using Polygon = bg::model::polygon<bg::model::d2::point_xy<double>>;
using TLS = carla::rpc::TrafficLightState;

using namespace constants::Collision;
using constants::WaypointSelection::JUNCTION_LOOK_AHEAD;

class CollisionStage : Stage
{
private:
  const std::vector<ActorId> &vehicle_id_list;
  const SimulationState &simulation_state;
  const BufferMapPtr &buffer_map;
  const TrackTraffic &track_traffic;
  const Parameters &parameters;
  CollisionFramePtr &output_array;
  CollisionLockMap collision_locks;
  GeometryComparisonMap geometry_cache;
  GeodesicBoundaryMap geodesic_boundary_map;

public:
  CollisionStage(const std::vector<ActorId> &vehicle_id_list,
                 const SimulationState &simulation_state,
                 const BufferMapPtr &buffer_map,
                 const TrackTraffic &track_traffic,
                 const Parameters &parameters,
                 CollisionFramePtr &output_array);

  void Update (const unsigned long index) override;

  void RemoveActor(const ActorId actor_id) override;

  void Reset() override;

  std::pair<bool, float> NegotiateCollision(const ActorId reference_vehicle_id,
                                            const ActorId other_actor_id,
                                            const uint64_t reference_junction_look_ahead_index,
                                            const float reference_lead_distance,
                                            const float other_lead_distance);

  float GetBoundingBoxExtention(const ActorId actor_id);

  LocationList GetBoundary(const ActorId actor_id);

  LocationList GetGeodesicBoundary(const ActorId actor_id,
                                   const float specific_lead_distance);

  Polygon GetPolygon(const LocationList &boundary);

  GeometryComparison GetGeometryBetweenActors(const ActorId reference_vehicle_id,
                                              const ActorId other_actor_id,
                                              const float reference_lead_distance,
                                              const float other_lead_distance);
};

} // namespace traffic_manager
} // namespace carla
