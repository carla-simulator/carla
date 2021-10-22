// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <chrono>
#include <memory>

#include "carla/PythonUtil.h"
#include "boost/python/suite/indexing/vector_indexing_suite.hpp"

#include "carla/trafficmanager/TrafficManager.h"
using ActorPtr = carla::SharedPtr<carla::client::Actor>;
using ActorId = carla::ActorId;

void InterSetCustomPath(carla::traffic_manager::TrafficManager& self, const ActorPtr &actor, boost::python::list input, bool empty_buffer) {
  self.SetCustomPath(actor, PythonLitstToVector<carla::geom::Location>(input), empty_buffer);
}

boost::python::list InterGetNextAction(carla::traffic_manager::TrafficManager& self, const ActorId &actor_id) {
  boost::python::list l;
  auto next_action = self.GetNextAction(actor_id);
  l.append(static_cast<uint8_t>(next_action.first));
  l.append(next_action.second);
  return l;
}

boost::python::list InterGetActionBuffer(carla::traffic_manager::TrafficManager& self, const ActorId &actor_id) {
  boost::python::list l;
  auto action_buffer = self.GetActionBuffer(actor_id);
  for (auto &next_action : action_buffer) {
    boost::python::list temp;
    temp.append(static_cast<uint8_t>(next_action.first));
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
    .def("global_percentage_speed_difference", &ctm::TrafficManager::SetGlobalPercentageSpeedDifference)
    .def("collision_detection", &ctm::TrafficManager::SetCollisionDetection)
    .def("force_lane_change", &ctm::TrafficManager::SetForceLaneChange)
    .def("auto_lane_change", &ctm::TrafficManager::SetAutoLaneChange)
    .def("distance_to_leading_vehicle", &ctm::TrafficManager::SetDistanceToLeadingVehicle)
    .def("ignore_walkers_percentage", &ctm::TrafficManager::SetPercentageIgnoreWalkers)
    .def("ignore_vehicles_percentage", &ctm::TrafficManager::SetPercentageIgnoreVehicles)
    .def("ignore_lights_percentage", &ctm::TrafficManager::SetPercentageRunningLight)
    .def("ignore_signs_percentage", &ctm::TrafficManager::SetPercentageRunningSign)
    .def("set_global_distance_to_leading_vehicle", &ctm::TrafficManager::SetGlobalDistanceToLeadingVehicle)
    .def("set_percentage_keep_right_rule", &ctm::TrafficManager::SetKeepRightPercentage)
    .def("set_percentage_random_left_lanechange", &ctm::TrafficManager::SetRandomLeftLaneChangePercentage)
    .def("set_percentage_random_right_lanechange", &ctm::TrafficManager::SetRandomRightLaneChangePercentage)
    .def("set_synchronous_mode", &ctm::TrafficManager::SetSynchronousMode)
    .def("set_hybrid_physics_mode", &ctm::TrafficManager::SetHybridPhysicsMode)
    .def("set_hybrid_physics_radius", &ctm::TrafficManager::SetHybridPhysicsRadius)
    .def("set_random_device_seed", &ctm::TrafficManager::SetRandomDeviceSeed)
    .def("set_osm_mode", &carla::traffic_manager::TrafficManager::SetOSMMode)
    .def("set_custom_path", &InterSetCustomPath, (arg("empty_buffer") = true))
    .def("set_respawn_dormant_vehicles", &carla::traffic_manager::TrafficManager::SetRespawnDormantVehicles)
    .def("set_boundaries_respawn_dormant_vehicles", &carla::traffic_manager::TrafficManager::SetBoundariesRespawnDormantVehicles)
    .def("get_next_action", &InterGetNextAction)
    .def("get_action_buffer", &InterGetActionBuffer)
    .def("shut_down", &ctm::TrafficManager::ShutDown);
}
