// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
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

    /// Bytes of a record with empty next/previous lists: id (8), road/section/lane/s
    /// (16), two u16 list sizes (4), left/right ids (16), grid id (4), junction (1),
    /// road option (1). Lets InMemoryMap::Load bounds-check a cache before reading it.
    static constexpr unsigned long MinimumSize() { return 50; }

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
