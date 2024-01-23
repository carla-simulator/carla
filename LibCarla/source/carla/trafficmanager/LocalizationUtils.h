// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"
#include "carla/client/ActorList.h"
#include "carla/client/Vehicle.h"
#include "carla/client/World.h"
#include "carla/geom/Location.h"
#include "carla/road/RoadTypes.h"
#include "carla/rpc/ActorId.h"

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/SimpleWaypoint.h"
#include "carla/trafficmanager/TrackTraffic.h"

namespace carla {
namespace traffic_manager {

  namespace cc = carla::client;
  namespace cg = carla::geom;
  using Actor = carla::SharedPtr<cc::Actor>;
  using ActorId = carla::ActorId;
  using ActorIdSet = std::unordered_set<ActorId>;
  using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;
  using Buffer = std::deque<SimpleWaypointPtr>;
  using GeoGridId = carla::road::JuncId;
  using constants::Map::MAP_RESOLUTION;
  using constants::Map::INV_MAP_RESOLUTION;

  /// Returns the cross product (z component value) between the vehicle's
  /// heading vector and the vector along the direction to the next
  /// target waypoint on the horizon.
  float DeviationCrossProduct(const cg::Location &reference_location,
                              const cg::Vector3D &heading_vector,
                              const cg::Location &target_location);
  /// Returns the dot product between the vehicle's heading vector and
  /// the vector along the direction to the next target waypoint on the horizon.
  float DeviationDotProduct(const cg::Location &reference_location,
                            const cg::Vector3D &heading_vector,
                            const cg::Location &target_location);

  // Function to add a waypoint to a path buffer and update waypoint tracking.
  void PushWaypoint(ActorId actor_id, TrackTraffic& track_traffic,
                    Buffer& buffer, SimpleWaypointPtr& waypoint);

  // Function to remove a waypoint from a path buffer and update waypoint tracking.
  void PopWaypoint(ActorId actor_id, TrackTraffic& track_traffic,
                   Buffer& buffer, bool front_or_back=true);

  /// Method to return the wayPoints from the waypoint Buffer by using target point distance
  using TargetWPInfo = std::pair<SimpleWaypointPtr,uint64_t>;
  TargetWPInfo GetTargetWaypoint(const Buffer& waypoint_buffer, const float& target_point_distance);

} // namespace traffic_manager
} // namespace carla
