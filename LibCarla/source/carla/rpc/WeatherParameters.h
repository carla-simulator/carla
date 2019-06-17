// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "Carla/Weather/WeatherParameters.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {

  class WeatherParameters {
  public:

    /// @name Weather presets
    /// @{

    static WeatherParameters Default;
    static WeatherParameters ClearNoon;
    static WeatherParameters CloudyNoon;
    static WeatherParameters WetNoon;
    static WeatherParameters WetCloudyNoon;
    static WeatherParameters MidRainyNoon;
    static WeatherParameters HardRainNoon;
    static WeatherParameters SoftRainNoon;
    static WeatherParameters ClearSunset;
    static WeatherParameters CloudySunset;
    static WeatherParameters WetSunset;
    static WeatherParameters WetCloudySunset;
    static WeatherParameters MidRainSunset;
    static WeatherParameters HardRainSunset;
    static WeatherParameters SoftRainSunset;

    /// @}

    WeatherParameters() = default;

    WeatherParameters(
        float in_cloudyness,
        float in_precipitation,
        float in_precipitation_deposits,
        float in_wind_intensity,
        float in_sun_azimuth_angle,
        float in_sun_altitude_angle)
      : cloudyness(in_cloudyness),
        precipitation(in_precipitation),
        precipitation_deposits(in_precipitation_deposits),
        wind_intensity(in_wind_intensity),
        sun_azimuth_angle(in_sun_azimuth_angle),
        sun_altitude_angle(in_sun_altitude_angle) {}

    float cloudyness = 0.0f;
    float precipitation = 0.0f;
    float precipitation_deposits = 0.0f;
    float wind_intensity = 0.0f;
    float sun_azimuth_angle = 0.0f;
    float sun_altitude_angle = 0.0f;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    WeatherParameters(const FWeatherParameters &Weather)
      : cloudyness(Weather.Cloudyness),
        precipitation(Weather.Precipitation),
        precipitation_deposits(Weather.PrecipitationDeposits),
        wind_intensity(Weather.WindIntensity),
        sun_azimuth_angle(Weather.SunAzimuthAngle),
        sun_altitude_angle(Weather.SunAltitudeAngle) {}

    operator FWeatherParameters() const {
      FWeatherParameters Weather;
      Weather.Cloudyness = cloudyness;
      Weather.Precipitation = precipitation;
      Weather.PrecipitationDeposits = precipitation_deposits;
      Weather.WindIntensity = wind_intensity;
      Weather.SunAzimuthAngle = sun_azimuth_angle;
      Weather.SunAltitudeAngle = sun_altitude_angle;
      return Weather;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    bool operator!=(const WeatherParameters &rhs) const {
      return
          cloudyness != rhs.cloudyness ||
          precipitation != rhs.precipitation ||
          precipitation_deposits != rhs.precipitation_deposits ||
          wind_intensity != rhs.wind_intensity ||
          sun_azimuth_angle != rhs.sun_azimuth_angle ||
          sun_altitude_angle != rhs.sun_altitude_angle;
    }

    bool operator==(const WeatherParameters &rhs) const {
      return !(*this != rhs);
    }

    MSGPACK_DEFINE_ARRAY(
        cloudyness,
        precipitation,
        precipitation_deposits,
        wind_intensity,
        sun_azimuth_angle,
        sun_altitude_angle);
  };

} // namespace rpc
} // namespace carla
