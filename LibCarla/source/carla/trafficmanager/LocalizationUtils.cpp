// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/LocalizationUtils.h"
#include "carla/trafficmanager/Constants.h"


namespace carla {
namespace traffic_manager {

using constants::Collision::EPSILON;

float DeviationCrossProduct(const cg::Location &reference_location,
                            const cg::Vector3D &heading_vector,
                            const cg::Location &target_location) {
  cg::Vector3D next_vector = target_location - reference_location;
  next_vector = next_vector.MakeSafeUnitVector(EPSILON);
  const float cross_z = heading_vector.x * next_vector.y - heading_vector.y * next_vector.x;
  return cross_z;
}

float DeviationDotProduct(const cg::Location &reference_location,
                          const cg::Vector3D &heading_vector,
                          const cg::Location &target_location) {
  cg::Vector3D next_vector = target_location - reference_location;
  next_vector.z = 0.0f;
  next_vector = next_vector.MakeSafeUnitVector(EPSILON);
  cg::Vector3D heading_vector_flat(heading_vector.x, heading_vector.y, 0);
  heading_vector_flat = heading_vector_flat.MakeSafeUnitVector(EPSILON);
  float dot_product = cg::Math::Dot(next_vector, heading_vector_flat);
  dot_product = std::max(0.0f, std::min(dot_product, 1.0f));
  return dot_product;
}

void PushWaypoint(ActorId actor_id, TrackTraffic &track_traffic,
                  Buffer &buffer, SimpleWaypointPtr &waypoint) {

  const uint64_t waypoint_id = waypoint->GetId();
  buffer.push_back(waypoint);
  track_traffic.UpdatePassingVehicle(waypoint_id, actor_id);
}

void PopWaypoint(ActorId actor_id, TrackTraffic &track_traffic,
                 Buffer &buffer, bool front_or_back) {

  SimpleWaypointPtr removed_waypoint = front_or_back ? buffer.front() : buffer.back();
  const uint64_t removed_waypoint_id = removed_waypoint->GetId();
  if (front_or_back) {
    buffer.pop_front();
  } else {
    buffer.pop_back();
  }
  track_traffic.RemovePassingVehicle(removed_waypoint_id, actor_id);
}

TargetWPInfo GetTargetWaypoint(const Buffer &waypoint_buffer, const float &target_point_distance) {

  SimpleWaypointPtr target_waypoint = waypoint_buffer.front();
  const SimpleWaypointPtr &buffer_front = waypoint_buffer.front();
  uint64_t startPosn = static_cast<uint64_t>(std::fabs(target_point_distance * INV_MAP_RESOLUTION));
  uint64_t index = startPosn;
  /// Condition to determine forward or backward scanning of waypoint buffer.

  if (startPosn < waypoint_buffer.size()) {
    bool mScanForward = false;
    const float target_point_dist_power = target_point_distance * target_point_distance;
    if (buffer_front->DistanceSquared(target_waypoint) < target_point_dist_power) {
      mScanForward = true;
    }

    if (mScanForward) {
      for (uint64_t i = startPosn;
           (i < waypoint_buffer.size()) && (buffer_front->DistanceSquared(target_waypoint) < target_point_dist_power);
           ++i) {
        target_waypoint = waypoint_buffer.at(i);
        index = i;
      }
    } else {
      for (uint64_t i = startPosn;
           (buffer_front->DistanceSquared(target_waypoint) > target_point_dist_power);
           --i) {
        target_waypoint = waypoint_buffer.at(i);
        index = i;
      }
    }
  } else {
    target_waypoint = waypoint_buffer.back();
    index = waypoint_buffer.size() - 1;
  }
  return std::make_pair(target_waypoint, index);
}

} // namespace traffic_manager
} // namespace carla
