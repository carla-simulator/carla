#pragma once

#include <memory>

#include "carla/client/Client.h"
#include "boost/python.hpp"

#include "Pipeline.h"

namespace cc = carla::client;
using namespace boost::python;

namespace traffic_manager {

    class TrafficManager {

    private:
        std::shared_ptr<Pipeline> pipeline;
        cc::Client &client;

    public:
        TrafficManager (list &longitudinal_parameters,
                        list &longitudinal_highway_parameters,
                        list &lateral_parameters,
                        float urban_target_velocity,
                        float highway_target_velocity,
                        cc::Client &client):
                        client(client) {

            std::vector<float> longitudinal_parameters_vector;
            for (uint i = 0u; i < len(longitudinal_parameters); ++i) {
                longitudinal_parameters_vector.push_back(
                    extract<double>(longitudinal_parameters[i]));
            }

            std::vector<float> longitudinal_highway_parameters_vector;
            for (uint i = 0u; i < len(longitudinal_highway_parameters); ++i) {
                longitudinal_highway_parameters_vector.push_back(
                    extract<double>(longitudinal_highway_parameters[i]));
            }

            std::vector<float> lateral_parameters_vector;
            for (uint i = 0u; i < len(lateral_parameters); ++i) {
                lateral_parameters_vector.push_back(
                    extract<double>(lateral_parameters[i]));
            }

            pipeline = std::make_shared<Pipeline>(longitudinal_parameters_vector,
                                                  longitudinal_highway_parameters_vector,
                                                  lateral_parameters_vector,
                                                  urban_target_velocity,
                                                  highway_target_velocity,
                                                  this->client);
        };
    };

}

BOOST_PYTHON_MODULE(traffic_manager) {
    class_<traffic_manager::TrafficManager, boost::noncopyable>(
        "traffic_manager",
        init<list&, list&, list&, float, float, cc::Client&>())
        .def("register_vehicles", &traffic_manager::Pipeline::RegisterVehicles)
        .def("unregister_vehicles", &traffic_manager::Pipeline::UnregisterVehicles)
        .def("start", &traffic_manager::Pipeline::Start)
        .def("stop", &traffic_manager::Pipeline::Stop);
}
