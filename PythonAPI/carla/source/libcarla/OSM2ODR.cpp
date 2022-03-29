// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <OSM2ODR.h>

// Empty class to emulate the namespace in the PythonAPI
class OSM2ODR {};

namespace osm2odr {
  std::ostream &operator<<(std::ostream &out, const OSM2ODRSettings &settings) {
    out << "Osm2odrSettings(use_offsets=" << (settings.use_offsets ? "true" : "false")
        << ", offset_x=" << settings.offset_x << ", offset_y=" << settings.offset_y
        << ", default_lane_width=" << settings.default_lane_width
        << ", elevation_layer_height=" << settings.elevation_layer_height << ")";
    return out;
  }
  void SetOsmWayTypes(OSM2ODRSettings& self, boost::python::list input) {
    self.osm_highways_types = PythonLitstToVector<std::string>(input);
  }
  void SetTLExcludedWayTypes(OSM2ODRSettings& self, boost::python::list input) {
    self.tl_excluded_highways_types = PythonLitstToVector<std::string>(input);
  }
}

void export_osm2odr() {
  using namespace osm2odr;
  using namespace boost::python;

  class_<OSM2ODRSettings>("Osm2OdrSettings", init<>())
    .add_property("use_offsets", &OSM2ODRSettings::use_offsets, &OSM2ODRSettings::use_offsets)
    .add_property("offset_x", &OSM2ODRSettings::offset_x, &OSM2ODRSettings::offset_x)
    .add_property("offset_y", &OSM2ODRSettings::offset_y, &OSM2ODRSettings::offset_y)
    .add_property("default_lane_width", &OSM2ODRSettings::default_lane_width, &OSM2ODRSettings::default_lane_width)
    .add_property("elevation_layer_height", &OSM2ODRSettings::elevation_layer_height, &OSM2ODRSettings::elevation_layer_height)
    .add_property("proj_string", &OSM2ODRSettings::proj_string, &OSM2ODRSettings::proj_string)
    .add_property("center_map", &OSM2ODRSettings::center_map, &OSM2ODRSettings::center_map)
    .add_property("generate_traffic_lights", &OSM2ODRSettings::generate_traffic_lights, &OSM2ODRSettings::generate_traffic_lights)
    .add_property("all_junctions_with_traffic_lights", &OSM2ODRSettings::all_junctions_traffic_lights, &OSM2ODRSettings::all_junctions_traffic_lights)
    .def("set_osm_way_types", &SetOsmWayTypes, arg("way_types"))
    .def("set_traffic_light_excluded_way_types", &SetTLExcludedWayTypes, arg("way_types"))
    .def(self_ns::str(self_ns::self))
  ;

  class_<OSM2ODR>("Osm2Odr", no_init)
    .def("convert", &ConvertOSMToOpenDRIVE, (arg("osm_file"), arg("settings") = OSM2ODRSettings()))
      .staticmethod("convert")
  ;
}
