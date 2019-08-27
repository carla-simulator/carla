#pragma once

#include <vector>
#include <memory>
#include <map>
#include <mutex>

#include "SimpleWaypoint.h"

namespace traffic_manager {

    struct GeoIds {
        uint32_t road_id = 0;
        uint32_t section_id = 0;
        int lane_id = 0;
    };

    class TrafficDistributor {

    private:

        std::mutex vehicle_add_mutex;

        std::map<
            int, std::map<
                int, std::map<
                    int, std::map<int, int>
                >
            >
        > road_to_vehicle_id_map;

        std::map<
            int, std::map<
                int, std::map<int, int>
            >
        > co_lane_vehicle_count;

        std::map<int, GeoIds> vehicle_id_to_road_map;

        void setVehicleId(int, GeoIds);
        void eraseVehicleId(int, GeoIds);
        void setRoadIds(int, GeoIds);

        std::map<int, std::map<int, int>> getVehicleIds(GeoIds);
        GeoIds getRoadIds(int);

    public:
        TrafficDistributor();
        ~TrafficDistributor();

        std::vector<
            std::shared_ptr<SimpleWaypoint>
        > assignDistribution (
            int actor_id,
            uint32_t road_id,
            uint32_t section_id,
            int lane_id,
            std::shared_ptr<traffic_manager::SimpleWaypoint> waypoint
        );
    };
}
