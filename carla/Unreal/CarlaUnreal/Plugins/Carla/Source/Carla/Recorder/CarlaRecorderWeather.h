// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <sstream>
#include <vector>

#include "Carla/Weather/WeatherParameters.h"

#pragma pack(push, 1)
struct CarlaRecorderWeather
{
  float Cloudiness;
  float Precipitation;
  float PrecipitationDeposits;
  float WindIntensity;
  float SunAzimuthAngle;
  float SunAltitudeAngle;
  float FogDensity;
  float FogDistance;
  float FogFalloff;
  float Wetness;
  float ScatteringIntensity;
  float MieScatteringScale;
  float RayleighScatteringScale;
  float DustStorm;

  void Read(std::istream &InFile);

  void Write(std::ostream &OutFile);
};
#pragma pack(pop)

class CarlaRecorderWeathers
{
public:

  void Add(const CarlaRecorderWeather &InObj);

  void Clear(void);

  void Write(std::ostream &OutFile);

private:

  std::vector<CarlaRecorderWeather> Weathers;
};
