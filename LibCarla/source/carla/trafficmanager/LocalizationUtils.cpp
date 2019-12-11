
#include "carla/trafficmanager/LocalizationUtils.h"

namespace traffic_manager {

    float DeviationCrossProduct(Actor actor, const cg::Location &target_location) {

        cg::Vector3D heading_vector = actor->GetTransform().GetForwardVector();
        heading_vector.z = 0;
        heading_vector = heading_vector.MakeUnitVector();
        cg::Location next_vector = target_location - actor->GetLocation();
        next_vector.z = 0;
        if (next_vector.Length() > 2.0f * std::numeric_limits<float>::epsilon()) {
        next_vector = next_vector.MakeUnitVector();
        float cross_z = heading_vector.x * next_vector.y - heading_vector.y * next_vector.x;
        return cross_z;
        } else {
        return 0;
        }
    }

    float DeviationDotProduct(Actor actor, const cg::Location &target_location, bool rear_offset) {

        cg::Vector3D heading_vector = actor->GetTransform().GetForwardVector();
        heading_vector.z = 0;
        heading_vector = heading_vector.MakeUnitVector();
        cg::Location next_vector;

        if (!rear_offset) {
          next_vector = target_location - actor->GetLocation();
        } else {
          auto vehicle_ptr = boost::static_pointer_cast<cc::Vehicle>(actor);
          float vehicle_half_length = vehicle_ptr->GetBoundingBox().extent.x;
          next_vector = target_location - (cg::Location(-1* vehicle_half_length * heading_vector)
                                           + vehicle_ptr->GetLocation());
        }

        next_vector.z = 0;
        if (next_vector.Length() > 2.0f * std::numeric_limits<float>::epsilon()) {
          next_vector = next_vector.MakeUnitVector();
          float dot_product = cg::Math::Dot(next_vector, heading_vector);
          return dot_product;
        } else {
          return 0;
        }
    }

    TrackTraffic::TrackTraffic() {}

    void TrackTraffic::UpdateOverlappingVehicle(ActorId actor_id, ActorId other_id) {

      if (overlapping_vehicles.find(actor_id) != overlapping_vehicles.end()) {
        ActorIdSet& actor_set = overlapping_vehicles.at(actor_id);
        if (actor_set.find(other_id) == actor_set.end()) {
          actor_set.insert(other_id);
        }
      } else {
        overlapping_vehicles.insert({actor_id, {other_id}});
      }
    }

    void TrackTraffic::RemoveOverlappingVehicle(ActorId actor_id, ActorId other_id) {

      if (overlapping_vehicles.find(actor_id) != overlapping_vehicles.end()) {
        ActorIdSet& actor_set = overlapping_vehicles.at(actor_id);
        if (actor_set.find(actor_id) != actor_set.end()) {
          actor_set.erase(other_id);
        }
      }
    }

    ActorIdSet TrackTraffic::GetOverlappingVehicles(ActorId actor_id) {

      if (overlapping_vehicles.find(actor_id) != overlapping_vehicles.end()) {
        return overlapping_vehicles.at(actor_id);
      } else {
        return ActorIdSet();
      }
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

}