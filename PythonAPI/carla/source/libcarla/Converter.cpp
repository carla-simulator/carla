// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <Converter.h>

class Converter {
public:
  std::string ConvertOSMToOpenDRIVE(std::string osm_file) {
    return converter::ConvertOSMToOpenDRIVE(osm_file);
  }
};

void export_converter() {
  using namespace converter;
  using namespace boost::python;

  def("convert_to_odr", &ConvertOSMToOpenDRIVE, (arg("osm_file"), arg("offsetX") = 0, arg("offsetY") = 0));

  // class_<Converter>("Converter", init<>())
  //   .def("convert_to_odr", &Converter::ConvertOSMToOpenDRIVE, (arg("osm_file")))
  //     .staticmethod("convert_to_odr")
  // ;
}
