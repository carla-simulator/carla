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
        .def("set_vehicle_target_velocity", &traffic_manager::TrafficManager::SetVehicleTargetVelocity)
        .def("set_collision_detection", &traffic_manager::TrafficManager::SetCollisionDetection)
        .def("force_lane_change", &traffic_manager::TrafficManager::ForceLaneChange)
        .def("set_auto_lane_change", &traffic_manager::TrafficManager::AutoLaneChange)
        .def("set_distance_to_leading_vehicle", &traffic_manager::TrafficManager::SetDistanceToLeadingVehicle);

    def("GetTrafficManager", &traffic_manager::TrafficManager::GetInstance, return_value_policy<reference_existing_object>());

}
