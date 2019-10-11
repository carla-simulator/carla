
#include "boost/python.hpp"

#include "Pipeline.h"

using namespace boost::python;

BOOST_PYTHON_MODULE(traffic_manager) {
    class_<traffic_manager::Pipeline>("traffic_manager", init<>());
}