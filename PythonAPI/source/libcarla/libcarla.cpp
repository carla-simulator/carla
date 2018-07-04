// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <boost/python.hpp>

void export_actor();
void export_blueprint();
void export_client();
void export_control();
void export_transform();
void export_world();

BOOST_PYTHON_MODULE(libcarla) {
  using namespace boost::python;
  scope().attr("__path__") = "libcarla";
  export_transform();
  export_control();
  export_blueprint();
  export_actor();
  export_world();
  export_client();
}
