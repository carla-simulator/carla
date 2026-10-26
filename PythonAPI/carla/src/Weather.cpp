// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <PythonAPI.h>

namespace carla {
namespace rpc {

  std::ostream &operator<<(std::ostream &out, const WeatherParameters &weather) {
    out << "WeatherParameters(cloudiness=" << std::to_string(weather.cloudiness)
        << ", precipitation=" << std::to_string(weather.precipitation)
        << ", precipitation_deposits=" << std::to_string(weather.precipitation_deposits)
        << ", wind_intensity=" << std::to_string(weather.wind_intensity)
        << ", sun_azimuth_angle=" << std::to_string(weather.sun_azimuth_angle)
        << ", sun_altitude_angle=" << std::to_string(weather.sun_altitude_angle)
        << ", fog_density=" << std::to_string(weather.fog_density)
        << ", fog_distance=" << std::to_string(weather.fog_distance)
        << ", fog_falloff=" << std::to_string(weather.fog_falloff)
        << ", wetness=" << std::to_string(weather.wetness)
        << ", scattering_intensity=" << std::to_string(weather.scattering_intensity)
        << ", mie_scattering_scale=" << std::to_string(weather.mie_scattering_scale)
        << ", rayleigh_scattering_scale=" << std::to_string(weather.rayleigh_scattering_scale)
        << ", dust_storm=" << std::to_string(weather.dust_storm) << ')';
    return out;
  }

} // namespace rpc
} // namespace carla

void export_weather() {
  using namespace boost::python;
  namespace cr = carla::rpc;

  auto cls = class_<cr::WeatherParameters>("WeatherParameters")
    .def(init<float, float, float, float, float, float, float, float, float, float, float, float, float, float>(
        (arg("cloudiness")=0.0f,
         arg("precipitation")=0.0f,
         arg("precipitation_deposits")=0.0f,
         arg("wind_intensity")=0.0f,
         arg("sun_azimuth_angle")=0.0f,
         arg("sun_altitude_angle")=0.0f,
         arg("fog_density")=0.0f,
         arg("fog_distance")=0.0f,
         arg("fog_falloff")=0.0f,
         arg("wetness")=0.0f,
         arg("scattering_intensity")=0.0f,
         arg("mie_scattering_scale")=0.0f,
         arg("rayleigh_scattering_scale")=0.0331f,
         arg("dust_storm")=0.0f)))
    .def_readwrite("cloudiness", &cr::WeatherParameters::cloudiness)
    .def_readwrite("precipitation", &cr::WeatherParameters::precipitation)
    .def_readwrite("precipitation_deposits", &cr::WeatherParameters::precipitation_deposits)
    .def_readwrite("wind_intensity", &cr::WeatherParameters::wind_intensity)
    .def_readwrite("sun_azimuth_angle", &cr::WeatherParameters::sun_azimuth_angle)
    .def_readwrite("sun_altitude_angle", &cr::WeatherParameters::sun_altitude_angle)
    .def_readwrite("fog_density", &cr::WeatherParameters::fog_density)
    .def_readwrite("fog_distance", &cr::WeatherParameters::fog_distance)
    .def_readwrite("fog_falloff", &cr::WeatherParameters::fog_falloff)
    .def_readwrite("wetness", &cr::WeatherParameters::wetness)
    .def_readwrite("scattering_intensity", &cr::WeatherParameters::scattering_intensity)
    .def_readwrite("mie_scattering_scale", &cr::WeatherParameters::mie_scattering_scale)
    .def_readwrite("rayleigh_scattering_scale", &cr::WeatherParameters::rayleigh_scattering_scale)
    .def_readwrite("dust_storm", &cr::WeatherParameters::dust_storm)
    .def("__eq__", &cr::WeatherParameters::operator==)
    .def("__ne__", &cr::WeatherParameters::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  // Return a fresh editable value without changing the predefined preset.
  cls.add_static_property("Default",
      +[]() { return cr::WeatherParameters::Default; });
  cls.add_static_property("ClearNoon",
      +[]() { return cr::WeatherParameters::ClearNoon; });
  cls.add_static_property("CloudyNoon",
      +[]() { return cr::WeatherParameters::CloudyNoon; });
  cls.add_static_property("WetNoon",
      +[]() { return cr::WeatherParameters::WetNoon; });
  cls.add_static_property("WetCloudyNoon",
      +[]() { return cr::WeatherParameters::WetCloudyNoon; });
  cls.add_static_property("MidRainyNoon",
      +[]() { return cr::WeatherParameters::MidRainyNoon; });
  cls.add_static_property("HardRainNoon",
      +[]() { return cr::WeatherParameters::HardRainNoon; });
  cls.add_static_property("SoftRainNoon",
      +[]() { return cr::WeatherParameters::SoftRainNoon; });
  cls.add_static_property("ClearSunset",
      +[]() { return cr::WeatherParameters::ClearSunset; });
  cls.add_static_property("CloudySunset",
      +[]() { return cr::WeatherParameters::CloudySunset; });
  cls.add_static_property("WetSunset",
      +[]() { return cr::WeatherParameters::WetSunset; });
  cls.add_static_property("WetCloudySunset",
      +[]() { return cr::WeatherParameters::WetCloudySunset; });
  cls.add_static_property("MidRainSunset",
      +[]() { return cr::WeatherParameters::MidRainSunset; });
  cls.add_static_property("HardRainSunset",
      +[]() { return cr::WeatherParameters::HardRainSunset; });
  cls.add_static_property("SoftRainSunset",
      +[]() { return cr::WeatherParameters::SoftRainSunset; });
  cls.add_static_property("ClearNight",
      +[]() { return cr::WeatherParameters::ClearNight; });
  cls.add_static_property("CloudyNight",
      +[]() { return cr::WeatherParameters::CloudyNight; });
  cls.add_static_property("WetNight",
      +[]() { return cr::WeatherParameters::WetNight; });
  cls.add_static_property("WetCloudyNight",
      +[]() { return cr::WeatherParameters::WetCloudyNight; });
  cls.add_static_property("SoftRainNight",
      +[]() { return cr::WeatherParameters::SoftRainNight; });
  cls.add_static_property("MidRainyNight",
      +[]() { return cr::WeatherParameters::MidRainyNight; });
  cls.add_static_property("HardRainNight",
      +[]() { return cr::WeatherParameters::HardRainNight; });
  cls.add_static_property("DustStorm",
      +[]() { return cr::WeatherParameters::DustStorm; });
}
