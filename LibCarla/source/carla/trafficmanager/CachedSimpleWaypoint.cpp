// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/CachedSimpleWaypoint.h"

namespace carla {
namespace traffic_manager {

  using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;

  CachedSimpleWaypoint::CachedSimpleWaypoint(const SimpleWaypointPtr& simple_waypoint) {
    this->waypoint_id = simple_waypoint->GetId();

    this->road_id = simple_waypoint->GetWaypoint()->GetRoadId();
    this->section_id = simple_waypoint->GetWaypoint()->GetSectionId();
    this->lane_id = simple_waypoint->GetWaypoint()->GetLaneId();
    this->s = static_cast<float>(simple_waypoint->GetWaypoint()->GetDistance());

    for (auto &wp : simple_waypoint->GetNextWaypoint()) {
      this->next_waypoints.push_back(wp->GetId());
    }
    for (auto &wp : simple_waypoint->GetPreviousWaypoint()) {
      this->previous_waypoints.push_back(wp->GetId());
    }

    if (simple_waypoint->GetLeftWaypoint() != nullptr) {
      this->next_left_waypoint = simple_waypoint->GetLeftWaypoint()->GetId();
    }
    if (simple_waypoint->GetRightWaypoint() != nullptr) {
      this->next_right_waypoint = simple_waypoint->GetRightWaypoint()->GetId();
    }

    this->geodesic_grid_id = simple_waypoint->GetGeodesicGridId();
    this->is_junction = simple_waypoint->CheckJunction();
    this->road_option = static_cast<uint8_t>(simple_waypoint->GetRoadOption());
  }

  void CachedSimpleWaypoint::Write(std::ofstream &out_file) {

    WriteValue<uint64_t>(out_file, this->waypoint_id);

    // road_id, section_id, lane_id, s
    WriteValue<uint32_t>(out_file, this->road_id);
    WriteValue<uint32_t>(out_file, this->section_id);
    WriteValue<int32_t>(out_file, this->lane_id);
    WriteValue<float>(out_file, this->s);

    // list_of_next
    uint16_t total_next = static_cast<uint16_t>(this->next_waypoints.size());
    WriteValue<uint16_t>(out_file, total_next);
    for (auto &id : this->next_waypoints) {
      WriteValue<uint64_t>(out_file, id);
    }

    // list_of_previous
    uint16_t total_previous = static_cast<uint16_t>(this->previous_waypoints.size());
    WriteValue<uint16_t>(out_file, total_previous);
    for (auto &id : this->previous_waypoints) {
      WriteValue<uint64_t>(out_file, id);
    }

    // left, right
    WriteValue<uint64_t>(out_file, this->next_left_waypoint);
    WriteValue<uint64_t>(out_file, this->next_right_waypoint);

    // geo_grid_id
    WriteValue<int32_t>(out_file, this->geodesic_grid_id);

    // is_junction
    WriteValue<bool>(out_file, this->is_junction);

    // road_option
    WriteValue<uint8_t>(out_file, this->road_option);
  }

  void CachedSimpleWaypoint::Read(std::ifstream &in_file) {

    ReadValue<uint64_t>(in_file, this->waypoint_id);

    // road_id, section_id, lane_id, s
    ReadValue<uint32_t>(in_file, this->road_id);
    ReadValue<uint32_t>(in_file, this->section_id);
    ReadValue<int32_t>(in_file, this->lane_id);
    ReadValue<float>(in_file, this->s);

    // list_of_next
    uint16_t total_next;
    ReadValue<uint16_t>(in_file, total_next);
    for (uint16_t i = 0; i < total_next; i++) {
      uint64_t id;
      ReadValue<uint64_t>(in_file, id);
      this->next_waypoints.push_back(id);
    }

    // list_of_previous
    uint16_t total_previous;
    ReadValue<uint16_t>(in_file, total_previous);
    for (uint16_t i = 0; i < total_previous; i++) {
      uint64_t id;
      ReadValue<uint64_t>(in_file, id);
      this->previous_waypoints.push_back(id);
    }

    // left, right
    ReadValue<uint64_t>(in_file, this->next_left_waypoint);
    ReadValue<uint64_t>(in_file, this->next_right_waypoint);

    // geo_grid_id
    ReadValue<int32_t>(in_file, this->geodesic_grid_id);

    // is_junction
    ReadValue<bool>(in_file, this->is_junction);

    // road_option
    ReadValue<uint8_t>(in_file, this->road_option);
  }

  void CachedSimpleWaypoint::Read(const std::vector<uint8_t>& content, unsigned long& start) {
    ReadValue<uint64_t>(content, start, this->waypoint_id);

    // road_id, section_id, lane_id, s
    ReadValue<uint32_t>(content, start, this->road_id);
    ReadValue<uint32_t>(content, start, this->section_id);
    ReadValue<int32_t>(content, start, this->lane_id);
    ReadValue<float>(content, start, this->s);

    // list_of_next
    uint16_t total_next;
    ReadValue<uint16_t>(content, start, total_next);
    for (uint16_t i = 0; i < total_next; i++) {
      uint64_t id;
      ReadValue<uint64_t>(content, start, id);
      this->next_waypoints.push_back(id);
    }

    // list_of_previous
    uint16_t total_previous;
    ReadValue<uint16_t>(content, start, total_previous);
    for (uint16_t i = 0; i < total_previous; i++) {
      uint64_t id;
      ReadValue<uint64_t>(content, start, id);
      this->previous_waypoints.push_back(id);
    }

    // left, right
    ReadValue<uint64_t>(content, start, this->next_left_waypoint);
    ReadValue<uint64_t>(content, start, this->next_right_waypoint);

    // geo_grid_id
    ReadValue<int32_t>(content, start, this->geodesic_grid_id);

    // is_junction
    ReadValue<bool>(content, start, this->is_junction);

    // road_option
    ReadValue<uint8_t>(content, start, this->road_option);
  }

} // namespace traffic_manager
} // namespace carla
