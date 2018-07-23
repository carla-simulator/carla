// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <rpc/rpc_error.h>

#include <boost/python.hpp>

#include <sstream>

void translator(rpc::rpc_error e) {
  std::stringstream ss;
  ss << e.what()
     << " in function " << e.get_function_name()
     << ": " << e.get_error().as<std::string>();
  PyErr_SetString(PyExc_RuntimeError, ss.str().c_str());
}

void export_exception() {
  using namespace boost::python;
  namespace cc = carla::client;

  register_exception_translator<rpc::rpc_error>(translator);
}
