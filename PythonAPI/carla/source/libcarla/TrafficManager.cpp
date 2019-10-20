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

    class_<traffic_manager::TrafficManager, boost::noncopyable>(
        "TrafficManager", init<Parameters, Parameters, Parameters, float, float, cc::Client&>())
        .def("register_vehicles", &traffic_manager::TrafficManager::RegisterVehicles)
        .def("unregister_vehicles", &traffic_manager::TrafficManager::UnregisterVehicles)
        .def("start", &traffic_manager::TrafficManager::Start)
        .def("stop", &traffic_manager::TrafficManager::Stop)
        .def("set_vehicle_target_velocity", &traffic_manager::TrafficManager::SetVehicleTargetVelocity);

}
