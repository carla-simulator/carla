// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/rpc/WeatherParameters.h>

#include <ostream>

namespace carla {
namespace rpc {

  std::ostream &operator<<(std::ostream &out, const WeatherParameters &weather) {
    out << "WeatherParameters(cloudyness=" << weather.cloudyness
        << ", precipitation=" << weather.precipitation
        << ", precipitation_deposits=" << weather.precipitation_deposits
        << ", wind_intensity=" << weather.wind_intensity
        << ", sun_azimuth_angle=" << weather.sun_azimuth_angle
        << ", sun_altitude_angle=" << weather.sun_altitude_angle << ')';
    return out;
  }

} // namespace rpc
} // namespace carla

void export_weather() {
  using namespace boost::python;
  namespace cr = carla::rpc;

  auto cls = class_<cr::WeatherParameters>("WeatherParameters")
    .def(init<float, float, float, float, float, float>(
        (arg("cloudyness")=0.0f,
         arg("precipitation")=0.0f,
         arg("precipitation_deposits")=0.0f,
         arg("wind_intensity")=0.0f,
         arg("sun_azimuth_angle")=0.0f,
         arg("sun_altitude_angle")=0.0f)))
    .def_readwrite("cloudyness", &cr::WeatherParameters::cloudyness)
    .def_readwrite("precipitation", &cr::WeatherParameters::precipitation)
    .def_readwrite("precipitation_deposits", &cr::WeatherParameters::precipitation_deposits)
    .def_readwrite("wind_intensity", &cr::WeatherParameters::wind_intensity)
    .def_readwrite("sun_azimuth_angle", &cr::WeatherParameters::sun_azimuth_angle)
    .def_readwrite("sun_altitude_angle", &cr::WeatherParameters::sun_altitude_angle)
    .def("__eq__", &cr::WeatherParameters::operator==)
    .def("__ne__", &cr::WeatherParameters::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  cls.attr("ClearNoon") = cr::WeatherParameters::ClearNoon;
  cls.attr("CloudyNoon") = cr::WeatherParameters::CloudyNoon;
  cls.attr("WetNoon") = cr::WeatherParameters::WetNoon;
  cls.attr("WetCloudyNoon") = cr::WeatherParameters::WetCloudyNoon;
  cls.attr("MidRainyNoon") = cr::WeatherParameters::MidRainyNoon;
  cls.attr("HardRainNoon") = cr::WeatherParameters::HardRainNoon;
  cls.attr("SoftRainNoon") = cr::WeatherParameters::SoftRainNoon;
  cls.attr("ClearSunset") = cr::WeatherParameters::ClearSunset;
  cls.attr("CloudySunset") = cr::WeatherParameters::CloudySunset;
  cls.attr("WetSunset") = cr::WeatherParameters::WetSunset;
  cls.attr("WetCloudySunset") = cr::WeatherParameters::WetCloudySunset;
  cls.attr("MidRainSunset") = cr::WeatherParameters::MidRainSunset;
  cls.attr("HardRainSunset") = cr::WeatherParameters::HardRainSunset;
  cls.attr("SoftRainSunset") = cr::WeatherParameters::SoftRainSunset;
}
