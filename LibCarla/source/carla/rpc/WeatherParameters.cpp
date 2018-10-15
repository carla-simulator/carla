// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rpc/WeatherParameters.h"

namespace carla {
namespace rpc {

  using WP = WeatherParameters;

  //                       cloudyness   precip.  prec.dep.     wind   azimuth   altitude
  WP WP::ClearNoon       = {     20.0f,     0.0f,     00.0f,     0.35f,     0.0f,     80.0f};
  WP WP::CloudyNoon      = {     90.0f,     0.0f,     00.0f,     0.35f,     0.0f,     80.0f};
  WP WP::WetNoon         = {     20.0f,     0.0f,     50.0f,     0.35f,     0.0f,     80.0f};
  WP WP::WetCloudyNoon   = {     90.0f,     0.0f,     50.0f,     0.35f,     0.0f,     80.0f};
  WP WP::MidRainyNoon    = {     90.0f,    60.0f,     50.0f,     0.40f,     0.0f,     80.0f};
  WP WP::HardRainNoon    = {     90.0f,    90.0f,    100.0f,      1.0f,     0.0f,     80.0f};
  WP WP::SoftRainNoon    = {     70.0f,    30.0f,     50.0f,     0.35f,     0.0f,     80.0f};
  WP WP::ClearSunset     = {     20.0f,     0.0f,     00.0f,     0.35f,     0.0f,     20.0f};
  WP WP::CloudySunset    = {     90.0f,     0.0f,     00.0f,     0.35f,     0.0f,     20.0f};
  WP WP::WetSunset       = {     20.0f,     0.0f,     50.0f,     0.35f,     0.0f,     20.0f};
  WP WP::WetCloudySunset = {     90.0f,     0.0f,     50.0f,     0.35f,     0.0f,     20.0f};
  WP WP::MidRainSunset   = {     90.0f,    60.0f,     50.0f,     0.40f,     0.0f,     20.0f};
  WP WP::HardRainSunset  = {     90.0f,    90.0f,    100.0f,      1.0f,     0.0f,     20.0f};
  WP WP::SoftRainSunset  = {     70.0f,    30.0f,     50.0f,     0.35f,     0.0f,     20.0f};

} // namespace rpc
} // namespace carla
