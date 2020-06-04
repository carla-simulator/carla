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
        float in_cloudiness,
        float in_precipitation,
        float in_precipitation_deposits,
        float in_wind_intensity,
        float in_sun_azimuth_angle,
        float in_sun_altitude_angle,
        float in_fog_density,
        float in_fog_distance,
        float in_fog_falloff,
        float in_wetness)
      : cloudiness(in_cloudiness),
        precipitation(in_precipitation),
        precipitation_deposits(in_precipitation_deposits),
        wind_intensity(in_wind_intensity),
        sun_azimuth_angle(in_sun_azimuth_angle),
        sun_altitude_angle(in_sun_altitude_angle),
        fog_density(in_fog_density),
        fog_distance(in_fog_distance),
        fog_falloff(in_fog_falloff),
        wetness(in_wetness) {}

    float cloudiness = 0.0f;
    float precipitation = 0.0f;
    float precipitation_deposits = 0.0f;
    float wind_intensity = 0.0f;
    float sun_azimuth_angle = 0.0f;
    float sun_altitude_angle = 0.0f;
    float fog_density = 0.0f;
    float fog_distance = 0.0f;
    float fog_falloff = 0.0f;
    float wetness = 0.0f;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    WeatherParameters(const FWeatherParameters &Weather)
      : cloudiness(Weather.Cloudiness),
        precipitation(Weather.Precipitation),
        precipitation_deposits(Weather.PrecipitationDeposits),
        wind_intensity(Weather.WindIntensity),
        sun_azimuth_angle(Weather.SunAzimuthAngle),
        sun_altitude_angle(Weather.SunAltitudeAngle),
        fog_density(Weather.FogDensity),
        fog_distance(Weather.FogDistance),
        fog_falloff(Weather.FogFalloff),
        wetness(Weather.Wetness) {}

    operator FWeatherParameters() const {
      FWeatherParameters Weather;
      Weather.Cloudiness = cloudiness;
      Weather.Precipitation = precipitation;
      Weather.PrecipitationDeposits = precipitation_deposits;
      Weather.WindIntensity = wind_intensity;
      Weather.SunAzimuthAngle = sun_azimuth_angle;
      Weather.SunAltitudeAngle = sun_altitude_angle;
      Weather.FogDensity = fog_density;
      Weather.FogDistance = fog_distance;
      Weather.FogFalloff = fog_falloff;
      Weather.Wetness = wetness;
      return Weather;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    bool operator!=(const WeatherParameters &rhs) const {
      return
          cloudiness != rhs.cloudiness ||
          precipitation != rhs.precipitation ||
          precipitation_deposits != rhs.precipitation_deposits ||
          wind_intensity != rhs.wind_intensity ||
          sun_azimuth_angle != rhs.sun_azimuth_angle ||
          sun_altitude_angle != rhs.sun_altitude_angle ||
          fog_density != rhs.fog_density ||
          fog_distance != rhs.fog_distance ||
          fog_falloff != rhs.fog_falloff ||
          wetness != rhs.wetness;
    }

    bool operator==(const WeatherParameters &rhs) const {
      return !(*this != rhs);
    }

    MSGPACK_DEFINE_ARRAY(
        cloudiness,
        precipitation,
        precipitation_deposits,
        wind_intensity,
        sun_azimuth_angle,
        sun_altitude_angle,
        fog_density,
        fog_distance,
        fog_falloff,
        wetness);
  };

} // namespace rpc
} // namespace carla
