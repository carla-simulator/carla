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
    using ActorList = std::vector<carla::SharedPtr<cc::Actor>>;
    using Parameters = std::vector<float>;

    class_<ActorList>("TM_ActorList").def(vector_indexing_suite<ActorList>());

    class_<Parameters>("TM_Parameters").def(vector_indexing_suite<Parameters>());

    class_<traffic_manager::TrafficManager, boost::noncopyable>("TrafficManager", no_init)
        .def("register_vehicles", &traffic_manager::TrafficManager::RegisterVehicles)
        .def("unregister_vehicles", &traffic_manager::TrafficManager::UnregisterVehicles)
        .def("set_vehicle_max_speed_difference", &traffic_manager::TrafficManager::SetPercentageSpeedDifference)
        .def("set_global_max_speed_difference", &traffic_manager::TrafficManager::SetGlobalPercentageSpeedDifference)
        .def("set_collision_detection", &traffic_manager::TrafficManager::SetCollisionDetection)
        .def("force_lane_change", &traffic_manager::TrafficManager::SetForceLaneChange)
        .def("set_auto_lane_change", &traffic_manager::TrafficManager::SetAutoLaneChange)
        .def("set_distance_to_leading_vehicle", &traffic_manager::TrafficManager::SetDistanceToLeadingVehicle)
        .def("reset_traffic_lights", &traffic_manager::TrafficManager::ResetAllTrafficLights)
        .def("destroy_vehicle", &traffic_manager::TrafficManager::DestroyVehicle)
        .def("ignore_actors_percentage", &traffic_manager::TrafficManager::SetPercentageIgnoreActors)
        .def("ignore_lights_percentage", &traffic_manager::TrafficManager::SetPercentageRunningLight);

    def("GetTrafficManager", &traffic_manager::TrafficManager::GetInstance, return_value_policy<reference_existing_object>());

}
