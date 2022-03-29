
#include "carla/trafficmanager/Constants.h"

#include "carla/trafficmanager/TrackTraffic.h"

namespace carla {
namespace traffic_manager {

using constants::TrackTraffic::BUFFER_STEP_THROUGH;
using constants::TrackTraffic::INV_BUFFER_STEP_THROUGH;

TrackTraffic::TrackTraffic() {}

void TrackTraffic::UpdateUnregisteredGridPosition(const ActorId actor_id,
                                                  const std::vector<SimpleWaypointPtr> waypoints) {

    DeleteActor(actor_id);

    std::unordered_set<GeoGridId> current_grids;
    // Step through waypoints and update grid list for actor and actor list for grids.
    for (auto &waypoint : waypoints) {
        UpdatePassingVehicle(waypoint->GetId(), actor_id);

        GeoGridId ggid = waypoint->GetGeodesicGridId();
        current_grids.insert(ggid);

        if (grid_to_actors.find(ggid) != grid_to_actors.end()) {
            ActorIdSet &actor_ids = grid_to_actors.at(ggid);
            if (actor_ids.find(actor_id) == actor_ids.end()) {
                actor_ids.insert(actor_id);
            }
        } else {
            grid_to_actors.insert({ggid, {actor_id}});
        }
    }

    actor_to_grids.insert({actor_id, current_grids});
}

void TrackTraffic::UpdateGridPosition(const ActorId actor_id, const Buffer &buffer) {
    if (!buffer.empty()) {

        // Clear current actor from all grids containing itself.
        if (actor_to_grids.find(actor_id) != actor_to_grids.end()) {
            std::unordered_set<GeoGridId> &current_grids = actor_to_grids.at(actor_id);
            for (auto &grid_id : current_grids) {
                if (grid_to_actors.find(grid_id) != grid_to_actors.end()) {
                    ActorIdSet &actor_ids = grid_to_actors.at(grid_id);
                    actor_ids.erase(actor_id);
                }
            }

            actor_to_grids.erase(actor_id);
        }

        // Step through buffer and update grid list for actor and actor list for grids.
        std::unordered_set<GeoGridId> current_grids;
        uint64_t buffer_size = buffer.size();
        for (uint64_t i = 0u; i <= buffer_size - 1u; ++i) {
            auto waypoint = buffer.at(i);
            GeoGridId ggid = waypoint->GetGeodesicGridId();
            current_grids.insert(ggid);
            // Add grid entry if not present.
            if (grid_to_actors.find(ggid) == grid_to_actors.end()) {
                grid_to_actors.insert({ggid, {}});
            }

            ActorIdSet &actor_ids = grid_to_actors.at(ggid);
            if (actor_ids.find(actor_id) == actor_ids.end()) {
                actor_ids.insert(actor_id);
            }
        }

        actor_to_grids.insert({actor_id, current_grids});
    }
}


bool TrackTraffic::IsGeoGridFree(const GeoGridId geogrid_id) const {
    if (grid_to_actors.find(geogrid_id) != grid_to_actors.end()) {
        return grid_to_actors.at(geogrid_id).empty();
    }
    return true;
}

void TrackTraffic::AddTakenGrid(const GeoGridId geogrid_id, const ActorId actor_id) {
    if (grid_to_actors.find(geogrid_id) == grid_to_actors.end()) {
        grid_to_actors.insert({geogrid_id, {actor_id}});
    }
}


void TrackTraffic::SetHeroLocation(const cg::Location _location) {
    hero_location = _location;
}

cg::Location TrackTraffic::GetHeroLocation() const {
    return hero_location;
}

ActorIdSet TrackTraffic::GetOverlappingVehicles(ActorId actor_id) const {
    ActorIdSet actor_id_set;

    if (actor_to_grids.find(actor_id) != actor_to_grids.end()) {
        const std::unordered_set<GeoGridId> &grid_ids = actor_to_grids.at(actor_id);
        for (auto &grid_id : grid_ids) {
            if (grid_to_actors.find(grid_id) != grid_to_actors.end()) {
                const ActorIdSet &actor_ids = grid_to_actors.at(grid_id);
                actor_id_set.insert(actor_ids.begin(), actor_ids.end());
            }
        }
    }

    return actor_id_set;
}

void TrackTraffic::DeleteActor(ActorId actor_id) {
    if (actor_to_grids.find(actor_id) != actor_to_grids.end()) {
        std::unordered_set<GeoGridId> &grid_ids = actor_to_grids.at(actor_id);
        for (auto &grid_id : grid_ids) {
            if (grid_to_actors.find(grid_id) != grid_to_actors.end()) {
                ActorIdSet &actor_ids = grid_to_actors.at(grid_id);
                actor_ids.erase(actor_id);
                }
            }
        actor_to_grids.erase(actor_id);
    }

    if (waypoint_occupied.find(actor_id) != waypoint_occupied.end()) {
        WaypointIdSet waypoint_id_set = waypoint_occupied.at(actor_id);
        for (const uint64_t &waypoint_id : waypoint_id_set) {
            RemovePassingVehicle(waypoint_id, actor_id);
        }
    }
}

void TrackTraffic::UpdatePassingVehicle(uint64_t waypoint_id, ActorId actor_id) {
    if (waypoint_overlap_tracker.find(waypoint_id) != waypoint_overlap_tracker.end()) {
        ActorIdSet &actor_id_set = waypoint_overlap_tracker.at(waypoint_id);
        if (actor_id_set.find(actor_id) == actor_id_set.end()) {
            actor_id_set.insert(actor_id);
        }
    } else {
        waypoint_overlap_tracker.insert({waypoint_id, {actor_id}});
    }

    if (waypoint_occupied.find(actor_id) != waypoint_occupied.end()) {
        WaypointIdSet &waypoint_id_set = waypoint_occupied.at(actor_id);
        if (waypoint_id_set.find(waypoint_id) == waypoint_id_set.end()) {
            waypoint_id_set.insert(waypoint_id);
        }
    } else {
        waypoint_occupied.insert({actor_id, {waypoint_id}});
    }
}

void TrackTraffic::RemovePassingVehicle(uint64_t waypoint_id, ActorId actor_id) {
    if (waypoint_overlap_tracker.find(waypoint_id) != waypoint_overlap_tracker.end()) {
        ActorIdSet &actor_id_set = waypoint_overlap_tracker.at(waypoint_id);
        actor_id_set.erase(actor_id);

        if (actor_id_set.size() == 0) {
            waypoint_overlap_tracker.erase(waypoint_id);
        }
    }

    if (waypoint_occupied.find(actor_id) != waypoint_occupied.end()) {
        WaypointIdSet &waypoint_id_set = waypoint_occupied.at(actor_id);
        waypoint_id_set.erase(waypoint_id);

        if (waypoint_id_set.size() == 0) {
            waypoint_occupied.erase(actor_id);
        }
    }
}

ActorIdSet TrackTraffic::GetPassingVehicles(uint64_t waypoint_id) const {

    if (waypoint_overlap_tracker.find(waypoint_id) != waypoint_overlap_tracker.end()) {
        return waypoint_overlap_tracker.at(waypoint_id);
    } else {
        return ActorIdSet();
    }
}

void TrackTraffic::Clear() {
    waypoint_overlap_tracker.clear();
    waypoint_occupied.clear();
    actor_to_grids.clear();
    grid_to_actors.clear();
}

} // namespace traffic_manager
} // namespace carla
