#include "TrafficDistributor.h"

namespace traffic_manager {
    TrafficDistributor::TrafficDistributor(){}
    TrafficDistributor::~TrafficDistributor() {}

    void TrafficDistributor::setVehicleId(
        int vehicle_id,
        int road_id,
        int section_id,
        int lane_id
    ) {
        if (road_to_vehicle_id_map.find(road_id) != road_to_vehicle_id_map.end()) {
            if (
                road_to_vehicle_id_map[road_id].find(section_id)
                !=
                road_to_vehicle_id_map[section_id].end()
            ) {
                if (
                    road_to_vehicle_id_map[road_id][section_id].find(lane_id)
                    !=
                    road_to_vehicle_id_map[road_id][section_id].end()
                ) {
                    road_to_vehicle_id_map
                        [road_id]
                        [section_id]
                        [lane_id]
                        [vehicle_id]
                    = 0;
                } else {
                    road_to_vehicle_id_map[road_id][section_id][lane_id] = std::map<int, int>();
                    road_to_vehicle_id_map[road_id][section_id][lane_id][vehicle_id] = 0;
                }
            } else {
                road_to_vehicle_id_map[road_id][section_id] = std::map<int, std::map<int, int>>();
                road_to_vehicle_id_map[road_id][section_id][lane_id] = std::map<int, int>();
                road_to_vehicle_id_map[road_id][section_id][lane_id][vehicle_id] = 0;
            }
        } else {
            road_to_vehicle_id_map[road_id] = std::map<int, std::map<int, std::map<int, int>>>();
            road_to_vehicle_id_map[road_id][section_id] = std::map<int, std::map<int, int>>();
            road_to_vehicle_id_map[road_id][section_id][lane_id] = std::map<int, int>();
            road_to_vehicle_id_map[road_id][section_id][lane_id][vehicle_id] = 0;
        }
    }

    void TrafficDistributor::eraseVehicleId(
        int vehicle_id,
        int road_id,
        int section_id,
        int lane_id
    ) {
        road_to_vehicle_id_map[road_id][section_id][lane_id].erase(vehicle_id);
    }

    void TrafficDistributor::setCoLaneCount(
        int count,
        int road_id,
        int section_id,
        int road_side
    ) {

        if (co_lane_vehicle_count.find(road_id) != co_lane_vehicle_count.end()) {
            if (
                co_lane_vehicle_count[road_id].find(section_id)
                !=
                co_lane_vehicle_count[section_id].end()
            ) {
                if (
                    co_lane_vehicle_count[road_id][section_id].find(road_side)
                    !=
                    co_lane_vehicle_count[road_id][section_id].end()
                ) {
                    co_lane_vehicle_count
                        [road_id]
                        [section_id]
                        [road_side]
                    = count;
                } else {
                    co_lane_vehicle_count[road_id][section_id][road_side] = count;
                }
            } else {
                co_lane_vehicle_count[road_id][section_id] = std::map<int, int>();
                co_lane_vehicle_count[road_id][section_id][road_side] = count;
            }
        } else {
            co_lane_vehicle_count[road_id] = std::map<int, std::map<int, int>>();
            co_lane_vehicle_count[road_id][section_id] = std::map<int, int>();
            co_lane_vehicle_count[road_id][section_id][road_side] = count;
        }
    }

    void TrafficDistributor::setRoadIds(
        int vehicle_id,
        int road_id,
        int section_id,
        int lane_id
    ) {
        if (vehicle_id_to_road_map.find(vehicle_id) != vehicle_id_to_road_map.end()) {
                vehicle_id_to_road_map[vehicle_id] = {road_id, section_id, lane_id};
        } else {
            vehicle_id_to_road_map[vehicle_id] = {road_id, section_id, lane_id};
        }
    }

    std::vector<int> TrafficDistributor::getRoadIds(int vehicle_id) {
        if (vehicle_id_to_road_map.find(vehicle_id) != vehicle_id_to_road_map.end()) {
            return vehicle_id_to_road_map[vehicle_id];
        } else {
            return {};
        }
    }

    std::map<int, int> TrafficDistributor::getVehicleIds(
        int road_id,
        int section_id,
        int lane_id
    ) {
        bool entry_found = false;
        if (road_to_vehicle_id_map.find(road_id) != road_to_vehicle_id_map.end()) {
            if (
                road_to_vehicle_id_map[road_id].find(section_id)
                !=
                road_to_vehicle_id_map[section_id].end()
            ) {
                if (
                    road_to_vehicle_id_map[road_id][section_id].find(lane_id)
                    !=
                    road_to_vehicle_id_map[road_id][section_id].end()
                ) {
                    entry_found = true;
                }
            }
        }

        if (entry_found) {
            return road_to_vehicle_id_map
                [road_id]
                [section_id]
                [lane_id];
        } else {
            return std::map<int, int>();
        }
    }

    int TrafficDistributor::getCoLaneCount(
        int road_id,
        int section_id,
        int road_side
    ) {
        bool entry_found = false;
        if (co_lane_vehicle_count.find(road_id) != co_lane_vehicle_count.end()) {
            if (
                co_lane_vehicle_count[road_id].find(section_id)
                !=
                co_lane_vehicle_count[section_id].end()
            ) {
                if (
                    co_lane_vehicle_count[road_id][section_id].find(road_side)
                    !=
                    co_lane_vehicle_count[road_id][section_id].end()
                ) {
                    entry_found = true;
                }
            }
        }

        if (entry_found) {
            return co_lane_vehicle_count
                [road_id]
                [section_id]
                [road_side];
        } else {
            return -1;
        }
    }

}
