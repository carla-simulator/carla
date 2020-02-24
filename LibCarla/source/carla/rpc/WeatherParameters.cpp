// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rpc/WeatherParameters.h"

namespace carla {
namespace rpc {

  using WP = WeatherParameters;

  //                        cloudiness   precip.  prec.dep.     wind   azimuth   altitude  fog dens.  fog dist  wetness
  WP WP::Default         = {     -1.0f,    -1.0f,     -1.0f,  -1.00f,    -1.0f,     -1.0f,    -1.0f,    -1.0f,   -1.0f};
  WP WP::ClearNoon       = {     15.0f,     0.0f,      0.0f,   0.35f,     0.0f,     75.0f,     0.0f,     0.0f,    0.0f};
  WP WP::CloudyNoon      = {     80.0f,     0.0f,      0.0f,   0.35f,     0.0f,     75.0f,     0.0f,     0.0f,    0.0f};
  WP WP::WetNoon         = {     20.0f,     0.0f,     50.0f,   0.35f,     0.0f,     75.0f,     0.0f,     0.0f,    0.0f};
  WP WP::WetCloudyNoon   = {     80.0f,     0.0f,     50.0f,   0.35f,     0.0f,     75.0f,     0.0f,     0.0f,    0.0f};
  WP WP::MidRainyNoon    = {     80.0f,    30.0f,     50.0f,   0.40f,     0.0f,     75.0f,     0.0f,     0.0f,    0.0f};
  WP WP::HardRainNoon    = {     90.0f,    60.0f,    100.0f,   1.00f,     0.0f,     75.0f,     0.0f,     0.0f,    0.0f};
  WP WP::SoftRainNoon    = {     70.0f,    15.0f,     50.0f,   0.35f,     0.0f,     75.0f,     0.0f,     0.0f,    0.0f};
  WP WP::ClearSunset     = {     15.0f,     0.0f,      0.0f,   0.35f,     0.0f,     15.0f,     0.0f,     0.0f,    0.0f};
  WP WP::CloudySunset    = {     80.0f,     0.0f,      0.0f,   0.35f,     0.0f,     15.0f,     0.0f,     0.0f,    0.0f};
  WP WP::WetSunset       = {     20.0f,     0.0f,     50.0f,   0.35f,     0.0f,     15.0f,     0.0f,     0.0f,    0.0f};
  WP WP::WetCloudySunset = {     90.0f,     0.0f,     50.0f,   0.35f,     0.0f,     15.0f,     0.0f,     0.0f,    0.0f};
  WP WP::MidRainSunset   = {     80.0f,    30.0f,     50.0f,   0.40f,     0.0f,     15.0f,     0.0f,     0.0f,    0.0f};
  WP WP::HardRainSunset  = {     80.0f,    60.0f,    100.0f,   1.00f,     0.0f,     15.0f,     0.0f,     0.0f,    0.0f};
  WP WP::SoftRainSunset  = {     90.0f,    15.0f,     50.0f,   0.35f,     0.0f,     15.0f,     0.0f,     0.0f,    0.0f};

} // namespace rpc
} // namespace carla
