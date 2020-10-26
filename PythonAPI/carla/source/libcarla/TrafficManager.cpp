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
    .def("set_synchronous_mode", &ctm::TrafficManager::SetSynchronousMode)
    .def("set_hybrid_physics_mode", &ctm::TrafficManager::SetHybridPhysicsMode)
    .def("set_hybrid_physics_radius", &ctm::TrafficManager::SetHybridPhysicsRadius)
    .def("set_random_device_seed", &ctm::TrafficManager::SetRandomDeviceSeed)
    .def("set_osm_mode", &carla::traffic_manager::TrafficManager::SetOSMMode);
}
