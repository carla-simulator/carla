// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <Converter.h>

class OSM2ODR {
public:
  static std::string ConvertOSMToOpenDRIVE(std::string osm_file) {
    return osm2odr::ConvertOSMToOpenDRIVE(osm_file);
  }
};

void export_converter() {
  using namespace osm2odr;
  using namespace boost::python;

  // def("convert_to_odr", &ConvertOSMToOpenDRIVE, (arg("osm_file"), arg("offsetX") = 0, arg("offsetY") = 0));

  class_<OSM2ODR>("OSM2ODR", init<>())
    .def("convert_to_odr", &OSM2ODR::ConvertOSMToOpenDRIVE, (arg("osm_file")))
      .staticmethod("convert_to_odr")
  ;
}
