// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <chrono>
#include <memory>
#include <stdio.h>
#include "carla/PythonUtil.h"
#include "boost/python/suite/indexing/vector_indexing_suite.hpp"

#include "carla/trafficmanager/TrafficManager.h"
#include "carla/trafficmanager/SimpleWaypoint.h"

using ActorPtr = carla::SharedPtr<carla::client::Actor>;
using ActorId = carla::ActorId;

const char* RoadOptionToString(carla::client::RoadOption value) {
  switch (value)
  {
      case carla::client::RoadOption::Void:   return "Void";
      case carla::client::RoadOption::Left:   return "Left";
      case carla::client::RoadOption::Right:   return "Right";
      case carla::client::RoadOption::Straight: return "Straight";
      case carla::client::RoadOption::LaneFollow:   return "LaneFollow";
      case carla::client::RoadOption::ChangeLaneLeft:   return "ChangeLaneLeft";
      case carla::client::RoadOption::ChangeLaneRight: return "ChangeLaneRight";
      case carla::client::RoadOption::RoadEnd:   return "RoadEnd";
      default:      return "[Unknown RoadOption]";
  }
}

std::vector<uint8_t> RoadOptionToUint(boost::python::list input) {
  std::vector<uint8_t> route;
  for (int i = 0; i < len(input); ++i) {
    uint8_t val;
    char* str = boost::python::extract<char*>(input[i]);
    if (strcmp(str,"Void") == 0) val = 0u;
    else if (strcmp(str,"Left") == 0) val = 1u;
    else if (strcmp(str,"Right") == 0) val = 2u;
    else if (strcmp(str,"Straight") == 0) val = 3u;
    else if (strcmp(str,"LaneFollow") == 0) val = 4u;
    else if (strcmp(str,"ChangeLaneLeft") == 0) val = 5u;
    else if (strcmp(str,"ChangeLaneRight") == 0) val = 6u;
    else if (strcmp(str,"RoadEnd") == 0) val = 7u;
    else val = 10u;
    route.push_back(val);
  }
  return route;
}

void InterSetCustomPath(carla::traffic_manager::TrafficManager& self, const ActorPtr &actor, boost::python::list input, bool empty_buffer) {
  self.SetCustomPath(actor, PythonLitstToVector<carla::geom::Location>(input), empty_buffer);
}

void InterSetImportedRoute(carla::traffic_manager::TrafficManager& self, const ActorPtr &actor, boost::python::list input, bool empty_buffer) {
  self.SetImportedRoute(actor, RoadOptionToUint(input), empty_buffer);
}

boost::python::list InterGetNextAction(carla::traffic_manager::TrafficManager& self, const ActorPtr &actor_ptr) {
  boost::python::list l;
  auto next_action = self.GetNextAction(actor_ptr->GetId());
  l.append(RoadOptionToString(next_action.first));
  l.append(next_action.second);
  return l;
}

boost::python::list InterGetActionBuffer(carla::traffic_manager::TrafficManager& self, const ActorPtr &actor_ptr) {
  boost::python::list l;
  auto action_buffer = self.GetActionBuffer(actor_ptr->GetId());
  for (auto &next_action : action_buffer) {
    boost::python::list temp;
    temp.append(RoadOptionToString(next_action.first));
    temp.append(next_action.second);
    l.append(temp);
  }
  return l;
}


void export_trafficmanager() {
  namespace cc = carla::client;
  namespace ctm = carla::traffic_manager;
  using namespace boost::python;

  class_<ctm::TrafficManager>("TrafficManager", no_init)
    .def("get_port", &ctm::TrafficManager::Port)
    .def("vehicle_percentage_speed_difference", &ctm::TrafficManager::SetPercentageSpeedDifference)
    .def("vehicle_lane_offset", &ctm::TrafficManager::SetLaneOffset)
    .def("set_desired_speed", &ctm::TrafficManager::SetDesiredSpeed)
    .def("global_percentage_speed_difference", &ctm::TrafficManager::SetGlobalPercentageSpeedDifference)
    .def("global_lane_offset", &ctm::TrafficManager::SetGlobalLaneOffset)
    .def("update_vehicle_lights", &ctm::TrafficManager::SetUpdateVehicleLights)
    .def("collision_detection", &ctm::TrafficManager::SetCollisionDetection)
    .def("force_lane_change", &ctm::TrafficManager::SetForceLaneChange)
    .def("auto_lane_change", &ctm::TrafficManager::SetAutoLaneChange)
    .def("distance_to_leading_vehicle", &ctm::TrafficManager::SetDistanceToLeadingVehicle)
    .def("ignore_walkers_percentage", &ctm::TrafficManager::SetPercentageIgnoreWalkers)
    .def("ignore_vehicles_percentage", &ctm::TrafficManager::SetPercentageIgnoreVehicles)
    .def("ignore_lights_percentage", &ctm::TrafficManager::SetPercentageRunningLight)
    .def("ignore_signs_percentage", &ctm::TrafficManager::SetPercentageRunningSign)
    .def("set_global_distance_to_leading_vehicle", &ctm::TrafficManager::SetGlobalDistanceToLeadingVehicle)
    .def("keep_right_rule_percentage", &ctm::TrafficManager::SetKeepRightPercentage)
    .def("random_left_lanechange_percentage", &ctm::TrafficManager::SetRandomLeftLaneChangePercentage)
    .def("random_right_lanechange_percentage", &ctm::TrafficManager::SetRandomRightLaneChangePercentage)
    .def("set_synchronous_mode", &ctm::TrafficManager::SetSynchronousMode)
    .def("set_hybrid_physics_mode", &ctm::TrafficManager::SetHybridPhysicsMode)
    .def("set_hybrid_physics_radius", &ctm::TrafficManager::SetHybridPhysicsRadius)
    .def("set_random_device_seed", &ctm::TrafficManager::SetRandomDeviceSeed)
    .def("set_osm_mode", &carla::traffic_manager::TrafficManager::SetOSMMode)
    .def("set_path", &InterSetCustomPath, (arg("empty_buffer") = true))
    .def("set_route", &InterSetImportedRoute, (arg("empty_buffer") = true))
    .def("set_respawn_dormant_vehicles", &carla::traffic_manager::TrafficManager::SetRespawnDormantVehicles)
    .def("set_boundaries_respawn_dormant_vehicles", &carla::traffic_manager::TrafficManager::SetBoundariesRespawnDormantVehicles)
    .def("get_next_action", &InterGetNextAction)
    .def("get_all_actions", &InterGetActionBuffer)
    .def("shut_down", &ctm::TrafficManager::ShutDown);
}
