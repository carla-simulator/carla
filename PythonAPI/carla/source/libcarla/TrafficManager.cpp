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
      .def("get_port", &carla::traffic_manager::TrafficManager::Port, "@DocString(TrafficManager.get_port)")
      .def("vehicle_percentage_speed_difference", &carla::traffic_manager::TrafficManager::SetPercentageSpeedDifference, "@DocString(TrafficManager.vehicle_percentage_speed_difference)")
      .def("global_percentage_speed_difference", &carla::traffic_manager::TrafficManager::SetGlobalPercentageSpeedDifference, "@DocString(TrafficManager.global_percentage_speed_difference)")
      .def("collision_detection", &carla::traffic_manager::TrafficManager::SetCollisionDetection, "@DocString(TrafficManager.collision_detection)")
      .def("force_lane_change", &carla::traffic_manager::TrafficManager::SetForceLaneChange, "@DocString(TrafficManager.force_lane_change)")
      .def("auto_lane_change", &carla::traffic_manager::TrafficManager::SetAutoLaneChange, "@DocString(TrafficManager.auto_lane_change)")
      .def("distance_to_leading_vehicle", &carla::traffic_manager::TrafficManager::SetDistanceToLeadingVehicle, "@DocString(TrafficManager.distance_to_leading_vehicle)")
      .def("reset_traffic_lights", &carla::traffic_manager::TrafficManager::ResetAllTrafficLights, "@DocString(TrafficManager.reset_traffic_lights)")
      .def("ignore_walkers_percentage", &carla::traffic_manager::TrafficManager::SetPercentageIgnoreWalkers, "@DocString(TrafficManager.ignore_walkers_percentage)")
      .def("ignore_vehicles_percentage", &carla::traffic_manager::TrafficManager::SetPercentageIgnoreVehicles, "@DocString(TrafficManager.ignore_vehicles_percentage)")
      .def("ignore_lights_percentage", &carla::traffic_manager::TrafficManager::SetPercentageRunningLight, "@DocString(TrafficManager.ignore_lights_percentage)")
      .def("ignore_signs_percentage", &carla::traffic_manager::TrafficManager::SetPercentageRunningSign, "@DocString(TrafficManager.ignore_signs_percentage)")
      .def("set_global_distance_to_leading_vehicle", &carla::traffic_manager::TrafficManager::SetGlobalDistanceToLeadingVehicle, "@DocString(TrafficManager.set_global_distance_to_leading_vehicle)")
      .def("set_percentage_keep_right_rule", &carla::traffic_manager::TrafficManager::SetKeepRightPercentage, "@DocString(TrafficManager.set_percentage_keep_right_rule)")
      .def("set_synchronous_mode", &carla::traffic_manager::TrafficManager::SetSynchronousMode, "@DocString(TrafficManager.set_synchronous_mode)")
      .def("set_hybrid_physics_mode", &carla::traffic_manager::TrafficManager::SetHybridPhysicsMode, "@DocString(TrafficManager.set_hybrid_physics_mode)")
      .def("set_hybrid_physics_radius", &carla::traffic_manager::TrafficManager::SetHybridPhysicsRadius, "@DocString(TrafficManager.set_hybrid_physics_radius)")
    ;
}
