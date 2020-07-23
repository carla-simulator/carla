// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <Converter.h>

class OSM2ODR {
public:
  // static std::string ConvertOSMToOpenDRIVE(std::string osm_file, osm2odr::) {
  //   return osm2odr::ConvertOSMToOpenDRIVE(osm_file);
  // }
};

void export_converter() {
  using namespace osm2odr;
  using namespace boost::python;

  class_<OSM2ODRSettings>("OSM2ODRSettings", init<>())
    .add_property("use_offsets", &OSM2ODRSettings::use_offsets, &OSM2ODRSettings::use_offsets)
    .add_property("offset_x", &OSM2ODRSettings::offset_x, &OSM2ODRSettings::offset_x)
    .add_property("offset_y", &OSM2ODRSettings::offset_y, &OSM2ODRSettings::offset_y)
    .add_property("default_lane_width", &OSM2ODRSettings::default_lane_width, &OSM2ODRSettings::default_lane_width)
    .add_property("elevation_layer_height", &OSM2ODRSettings::elevation_layer_height, &OSM2ODRSettings::elevation_layer_height)
  ;

  class_<OSM2ODR>("OSM2ODR", no_init)
    .def("convert_to_odr", &ConvertOSMToOpenDRIVE, (arg("osm_file"), arg("OSM2ODRSettings") = OSM2ODRSettings()))
      .staticmethod("convert_to_odr")
  ;
}
