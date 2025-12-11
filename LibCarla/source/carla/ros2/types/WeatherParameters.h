// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cmath>

#include "carla/rpc/WeatherParameters.h"
#include "carla_msgs/msg/CarlaWeatherParameters.h"

namespace carla {
namespace ros2 {
namespace types {

/**
  Convert a carla rpc WeatherParameters to ROS type
  and holds carla time details
*/
class WeatherParameters {
public:
  explicit WeatherParameters(carla::rpc::WeatherParameters const &weather_parameters) {
    _weather_parameters.cloudiness(weather_parameters.cloudiness);
    _weather_parameters.precipitation(weather_parameters.precipitation);
    _weather_parameters.precipitation_deposits(weather_parameters.precipitation_deposits);
    _weather_parameters.wind_intensity(weather_parameters.wind_intensity);
    _weather_parameters.sun_azimuth_angle(weather_parameters.sun_azimuth_angle);
    _weather_parameters.sun_altitude_angle(weather_parameters.sun_altitude_angle);
    _weather_parameters.fog_density(weather_parameters.fog_density);
    _weather_parameters.fog_distance(weather_parameters.fog_distance);
    _weather_parameters.fog_falloff(weather_parameters.fog_falloff);
    _weather_parameters.wetness(weather_parameters.wetness);
    _weather_parameters.scattering_intensity(weather_parameters.scattering_intensity);
    _weather_parameters.mie_scattering_scale(weather_parameters.mie_scattering_scale);
    _weather_parameters.rayleigh_scattering_scale(weather_parameters.rayleigh_scattering_scale);
    _weather_parameters.dust_storm(weather_parameters.dust_storm);
  }

  explicit WeatherParameters(carla_msgs::msg::CarlaWeatherParameters const &weather_parameters)
    : _weather_parameters(weather_parameters) {}

  ~WeatherParameters() = default;
  WeatherParameters(const WeatherParameters&) = default;
  WeatherParameters& operator=(const WeatherParameters&) = default;
  WeatherParameters(WeatherParameters&&) = default;
  WeatherParameters& operator=(WeatherParameters&&) = default;

  carla::rpc::WeatherParameters weather_parameters_rpc() const {
    carla::rpc::WeatherParameters weather_parameters;
    weather_parameters.cloudiness = _weather_parameters.cloudiness();
    weather_parameters.precipitation = _weather_parameters.precipitation();
    weather_parameters.precipitation_deposits = _weather_parameters.precipitation_deposits();
    weather_parameters.wind_intensity = _weather_parameters.wind_intensity();
    weather_parameters.sun_azimuth_angle = _weather_parameters.sun_azimuth_angle();
    weather_parameters.sun_altitude_angle = _weather_parameters.sun_altitude_angle();
    weather_parameters.fog_density = _weather_parameters.fog_density();
    weather_parameters.fog_distance = _weather_parameters.fog_distance();
    weather_parameters.fog_falloff = _weather_parameters.fog_falloff();
    weather_parameters.wetness = _weather_parameters.wetness();
    weather_parameters.scattering_intensity = _weather_parameters.scattering_intensity();
    weather_parameters.mie_scattering_scale = _weather_parameters.mie_scattering_scale();
    weather_parameters.rayleigh_scattering_scale = _weather_parameters.rayleigh_scattering_scale();
    weather_parameters.dust_storm = _weather_parameters.dust_storm();
    return weather_parameters;
  }  

  /**
   * The resulting ROS carla_msgs::msg::CarlaWeatherParameters
   */
  const carla_msgs::msg::CarlaWeatherParameters& weather_parameters_msg() const {
    return _weather_parameters;
  }

private:
  carla_msgs::msg::CarlaWeatherParameters _weather_parameters;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla