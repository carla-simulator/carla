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
  WP WP::ClearNoon       = {     0.0f,     0.0f,     0.0f,     0.0f,     0.0f,     0.0f};
  WP WP::CloudyNoon      = {     0.0f,     0.0f,     0.0f,     0.0f,     0.0f,     0.0f};
  WP WP::WetNoon         = {     0.0f,     0.0f,     0.0f,     0.0f,     0.0f,     0.0f};
  WP WP::WetCloudyNoon   = {     0.0f,     0.0f,     0.0f,     0.0f,     0.0f,     0.0f};
  WP WP::MidRainyNoon    = {     0.0f,     0.0f,     0.0f,     0.0f,     0.0f,     0.0f};
  WP WP::HardRainNoon    = {     0.0f,     0.0f,     0.0f,     0.0f,     0.0f,     0.0f};
  WP WP::SoftRainNoon    = {     0.0f,     0.0f,     0.0f,     0.0f,     0.0f,     0.0f};
  WP WP::ClearSunset     = {     0.0f,     0.0f,     0.0f,     0.0f,     0.0f,     0.0f};
  WP WP::CloudySunset    = {     0.0f,     0.0f,     0.0f,     0.0f,     0.0f,     0.0f};
  WP WP::WetSunset       = {     0.0f,     0.0f,     0.0f,     0.0f,     0.0f,     0.0f};
  WP WP::WetCloudySunset = {     0.0f,     0.0f,     0.0f,     0.0f,     0.0f,     0.0f};
  WP WP::MidRainSunset   = {     0.0f,     0.0f,     0.0f,     0.0f,     0.0f,     0.0f};
  WP WP::HardRainSunset  = {     0.0f,     0.0f,     0.0f,     0.0f,     0.0f,     0.0f};
  WP WP::SoftRainSunset  = {     0.0f,     0.0f,     0.0f,     0.0f,     0.0f,     0.0f};

} // namespace rpc
} // namespace carla
