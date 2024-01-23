// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>

#include "carla/trafficmanager/SimpleWaypoint.h"

namespace carla {
namespace traffic_manager {

  using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;

  class CachedSimpleWaypoint {
  public:
    uint64_t waypoint_id;
    uint32_t road_id;
    uint32_t section_id;
    int32_t lane_id;
    float s;
    std::vector<uint64_t> next_waypoints;
    std::vector<uint64_t> previous_waypoints;
    uint64_t next_left_waypoint  = 0;
    uint64_t next_right_waypoint = 0;
    int32_t geodesic_grid_id;
    bool is_junction;
    uint8_t road_option;

    CachedSimpleWaypoint() = default;
    CachedSimpleWaypoint(const SimpleWaypointPtr& simple_waypoint);

    void Read(const std::vector<uint8_t>& content, unsigned long& start);

    void Read(std::ifstream &in_file);
    void Write(std::ofstream &out_file);

  private:
    template <typename T>
    void WriteValue(std::ofstream &out_file, const T &in_obj) {
      out_file.write(reinterpret_cast<const char *>(&in_obj), sizeof(T));
    }
    template <typename T>
    void ReadValue(std::ifstream &in_file, T &out_obj) {
      in_file.read(reinterpret_cast<char *>(&out_obj), sizeof(T));
    }
    template <typename T>
    void ReadValue(const std::vector<uint8_t>& content, unsigned long& start, T &out_obj) {
      memcpy(&out_obj, &content[start], sizeof(T));
      start += sizeof(T);
    }
  };

} // namespace traffic_manager
} // namespace carla
