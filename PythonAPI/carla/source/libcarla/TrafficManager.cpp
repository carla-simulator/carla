#include <chrono>
#include <memory>

#include "boost/python.hpp"
#include "boost/python/suite/indexing/vector_indexing_suite.hpp"

#include "Pipeline.h"

namespace carla {
namespace trafficmanager {
void export_trafficmanager() {
    using namespace boost::python;
    namespace cc = carla::client;
    using ActorList = std::vector<carla::SharedPtr<cc::Actor>>;
    using Parameters = std::vector<float>;
    class_<ActorList>("actor_list").def(vector_indexing_suite<ActorList>());

    class_<Parameters>("parameters").def(vector_indexing_suite<Parameters>());

    class_<traffic_manager::Pipeline, boost::noncopyable>(
        "traffic_manager",
        .def(init<Parameters, Parameters, Parameters, float, float, cc::Client&>()))
        .def("register_vehicles", &traffic_manager::Pipeline::RegisterVehicles)
        .def("unregister_vehicles", &traffic_manager::Pipeline::UnregisterVehicles)
        .def("start", &traffic_manager::Pipeline::Start)
        .def("stop", &traffic_manager::Pipeline::Stop);

}
}
}