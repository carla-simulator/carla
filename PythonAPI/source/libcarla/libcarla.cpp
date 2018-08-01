// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <boost/python.hpp>

#include "Actor.cpp"
#include "Blueprint.cpp"
#include "Client.cpp"
#include "Control.cpp"
#include "Exception.cpp"
#include "Transform.cpp"
#include "World.cpp"

BOOST_PYTHON_MODULE(libcarla) {
  using namespace boost::python;
  PyEval_InitThreads();
  scope().attr("__path__") = "libcarla";
  export_transform();
  export_control();
  export_blueprint();
  export_actor();
  export_world();
  export_client();
  export_exception();
}
