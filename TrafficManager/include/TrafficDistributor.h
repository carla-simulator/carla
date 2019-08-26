#include <vector>
#include <memory>
#include <map>
#include <mutex>

#include "SimpleWaypoint.h"

namespace traffic_manager {

    class TrafficDistributor {

    private:

        std::mutex distributor_update_mutex;

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

        std::map<int, std::vector<int>> vehicle_id_to_road_map;

        void setVehicleId(int, int, int, int);
        void eraseVehicleId(int, int, int, int);
        void setCoLaneCount(int, int, int, int);
        void setRoadIds(int, int, int, int);

        std::map<int, int> getVehicleIds(int, int, int);
        int getCoLaneCount(int, int, int);
        std::vector<int> getRoadIds(int);

    public:
        TrafficDistributor();
        ~TrafficDistributor();

        std::vector<
            std::shared_ptr<SimpleWaypoint>
        > assignDistribution (
            int actor_id,
            int road_id,
            int section_id,
            int lane_id
        );
    };
}
