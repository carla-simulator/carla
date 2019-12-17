// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/LocalizationUtils.h"

namespace carla {
namespace traffic_manager {

  float DeviationCrossProduct(Actor actor, const cg::Location &target_location) {

    cg::Vector3D heading_vector = actor->GetTransform().GetForwardVector();
    heading_vector.z = 0.0f;
    heading_vector = heading_vector.MakeUnitVector();
    cg::Location next_vector = target_location - actor->GetLocation();
    next_vector.z = 0.0f;
    if (next_vector.Length() > 2.0f * std::numeric_limits<float>::epsilon()) {
      next_vector = next_vector.MakeUnitVector();
      const float cross_z = heading_vector.x * next_vector.y - heading_vector.y * next_vector.x;
      return cross_z;
    } else {
      return 0.0f;
    }
  }

  float DeviationDotProduct(Actor actor, const cg::Location &target_location, bool rear_offset) {

    cg::Vector3D heading_vector = actor->GetTransform().GetForwardVector();
    heading_vector.z = 0.0f;
    heading_vector = heading_vector.MakeUnitVector();
    cg::Location next_vector;

    if (!rear_offset) {
      next_vector = target_location - actor->GetLocation();
    } else {
      const auto vehicle_ptr = boost::static_pointer_cast<cc::Vehicle>(actor);
      const float vehicle_half_length = vehicle_ptr->GetBoundingBox().extent.x;
      next_vector = target_location - (cg::Location(-1* vehicle_half_length * heading_vector)
                                        + vehicle_ptr->GetLocation());
    }

    next_vector.z = 0.0f;
    if (next_vector.Length() > 2.0f * std::numeric_limits<float>::epsilon()) {
      next_vector = next_vector.MakeUnitVector();
      const float dot_product = cg::Math::Dot(next_vector, heading_vector);
      return dot_product;
    } else {
      return 0.0f;
    }
  }

  TrackTraffic::TrackTraffic() {}

  void TrackTraffic::UpdateGridPosition(ActorId actor_id, SimpleWaypointPtr waypoint) {

    GeoGridId new_geo_grid_id = waypoint->GetGeodesicGridId();

    if (actor_to_grids.find(actor_id) != actor_to_grids.end()) {
      auto& grid_ids = actor_to_grids.at(actor_id);
      if (grid_ids.find(new_geo_grid_id) == grid_ids.end()) {
        grid_ids.insert(new_geo_grid_id);
      }
    } else {
      actor_to_grids.insert({actor_id, {new_geo_grid_id}});
    }

    if (grid_to_actors.find(new_geo_grid_id) != grid_to_actors.end()) {
      ActorIdSet& actor_set = grid_to_actors.at(new_geo_grid_id);
      if (actor_set.find(actor_id) == actor_set.end()) {
        actor_set.insert(actor_id);
      }
    } else {
      grid_to_actors.insert({new_geo_grid_id, {actor_id}});
    }
  }

  void TrackTraffic::RemoveGridPosition(ActorId actor_id, SimpleWaypointPtr removed_waypoint,
                                        SimpleWaypointPtr remaining_waypoint) {

    if (removed_waypoint != nullptr) {

      GeoGridId removed_grid_id = removed_waypoint->GetGeodesicGridId();
      GeoGridId remaining_grid_id;
      if (remaining_waypoint != nullptr) {
        remaining_grid_id = remaining_waypoint->GetGeodesicGridId();
      }

      if (remaining_waypoint == nullptr || removed_grid_id != remaining_grid_id) {
        if (actor_to_grids.find(actor_id) != actor_to_grids.end()) {
          auto& grid_ids = actor_to_grids.at(actor_id);
          if (grid_ids.find(removed_grid_id) != grid_ids.end()) {
            grid_ids.erase(removed_grid_id);
          }
        }

        if (grid_to_actors.find(removed_grid_id) != grid_to_actors.end()) {
          ActorIdSet& actor_set = grid_to_actors.at(removed_grid_id);
          if (actor_set.find(actor_id) != actor_set.end()) {
            actor_set.erase(actor_id);
          }
        }
      }
    }
  }

  ActorIdSet TrackTraffic::GetOverlappingVehicles(ActorId actor_id) {

    ActorIdSet actor_id_set;

    if (actor_to_grids.find(actor_id) != actor_to_grids.end()) {
      std::unordered_set<GeoGridId>& grid_ids = actor_to_grids.at(actor_id);
      for (auto& grid_id: grid_ids) {
        if (grid_to_actors.find(grid_id) != grid_to_actors.end()) {
          ActorIdSet& actor_ids = grid_to_actors.at(grid_id);
          actor_id_set.insert(actor_ids.begin(), actor_ids.end());
        }
      }
    }

    return actor_id_set;
  }

  void TrackTraffic::DeleteActor(ActorId actor_id) {

    if (actor_to_grids.find(actor_id) != actor_to_grids.end()) {
      std::unordered_set<GeoGridId>& grid_ids = actor_to_grids.at(actor_id);
      for (auto& grid_id: grid_ids) {
        if (grid_to_actors.find(grid_id) != grid_to_actors.end()) {
          ActorIdSet& actor_ids = grid_to_actors.at(grid_id);
          if (actor_ids.find(actor_id) != actor_ids.end()) {
            actor_ids.erase(actor_id);
          }
        }
      }
      actor_to_grids.erase(actor_id);
    }
  }

  std::unordered_set<GeoGridId> TrackTraffic::GetGridIds(ActorId actor_id) {

    std::unordered_set<GeoGridId> grid_ids;

    if (actor_to_grids.find(actor_id) != actor_to_grids.end()) {
      grid_ids = actor_to_grids.at(actor_id);
    }

    return grid_ids;
  }

  std::unordered_map<GeoGridId, ActorIdSet>& TrackTraffic::GetGridActors() {
    return grid_to_actors;
  }

  void TrackTraffic::UpdatePassingVehicle(uint64_t waypoint_id, ActorId actor_id) {

    if (waypoint_overlap_tracker.find(waypoint_id) != waypoint_overlap_tracker.end()) {

      ActorIdSet& actor_id_set = waypoint_overlap_tracker.at(waypoint_id);
      if (actor_id_set.find(actor_id) == actor_id_set.end()) {
        actor_id_set.insert(actor_id);
      }
    } else {

      waypoint_overlap_tracker.insert({waypoint_id, {actor_id}});
    }
  }

  void TrackTraffic::RemovePassingVehicle(uint64_t waypoint_id, ActorId actor_id) {

    if (waypoint_overlap_tracker.find(waypoint_id) != waypoint_overlap_tracker.end()) {

      auto& actor_id_set = waypoint_overlap_tracker.at(waypoint_id);
      if (actor_id_set.find(actor_id) != actor_id_set.end()) {
        actor_id_set.erase(actor_id);
      }

      if (actor_id_set.size() == 0) {
        waypoint_overlap_tracker.erase(waypoint_id);
      }
    }
  }

  ActorIdSet TrackTraffic::GetPassingVehicles(uint64_t waypoint_id) {

    if (waypoint_overlap_tracker.find(waypoint_id) != waypoint_overlap_tracker.end()) {
      return waypoint_overlap_tracker.at(waypoint_id);
    } else {
      return ActorIdSet();
    }

  }

} // namespace traffic_manager
} // namespace carla
