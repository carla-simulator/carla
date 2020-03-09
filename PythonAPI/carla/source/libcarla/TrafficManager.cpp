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
    using namespace boost::python;

    class_<carla::traffic_manager::TrafficManager>("TrafficManager", no_init)
      .def("vehicle_percentage_speed_difference", &carla::traffic_manager::TrafficManager::SetPercentageSpeedDifference)
      .def("global_percentage_speed_difference", &carla::traffic_manager::TrafficManager::SetGlobalPercentageSpeedDifference)
      .def("collision_detection", &carla::traffic_manager::TrafficManager::SetCollisionDetection)
      .def("force_lane_change", &carla::traffic_manager::TrafficManager::SetForceLaneChange)
      .def("auto_lane_change", &carla::traffic_manager::TrafficManager::SetAutoLaneChange)
      .def("distance_to_leading_vehicle", &carla::traffic_manager::TrafficManager::SetDistanceToLeadingVehicle)
      .def("reset_traffic_lights", &carla::traffic_manager::TrafficManager::ResetAllTrafficLights)
      .def("ignore_walkers_percentage", &carla::traffic_manager::TrafficManager::SetPercentageIgnoreWalkers)
      .def("ignore_vehicles_percentage", &carla::traffic_manager::TrafficManager::SetPercentageIgnoreVehicles)
      .def("ignore_lights_percentage", &carla::traffic_manager::TrafficManager::SetPercentageRunningLight)
      .def("ignore_signs_percentage", &carla::traffic_manager::TrafficManager::SetPercentageRunningSign)
      .def("set_global_distance_to_leading_vehicle", &carla::traffic_manager::TrafficManager::SetGlobalDistanceToLeadingVehicle)
      .def("set_percentage_keep_right_rule", &carla::traffic_manager::TrafficManager::SetKeepRightPercentage);
}
