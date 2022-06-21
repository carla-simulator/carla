// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rpc/WeatherParameters.h"

namespace carla {
namespace rpc {

  using WP = WeatherParameters;

//                        cloudiness   precip.  prec.dep.     wind   azimuth   altitude  fog dens  fog dist  fog fall  wetness  scat.i  mie.scat.s rayleigh.scat.scale
  WP WP::Default         = {    -1.0f,   -1.0f,    -1.0f,   -1.00f,    -1.0f,     -1.0f,    -1.0f,     -1.0f,    -1.0f,   -1.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::ClearNoon       = {     5.0f,    0.0f,     0.0f,    10.0f,    -1.0f,     45.0f,     2.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::CloudyNoon      = {    60.0f,    0.0f,     0.0f,    10.0f,    -1.0f,     45.0f,     3.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::WetNoon         = {     5.0f,    0.0f,    50.0f,    10.0f,    -1.0f,     45.0f,     3.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::WetCloudyNoon   = {    60.0f,    0.0f,    50.0f,    10.0f,    -1.0f,     45.0f,     3.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::MidRainyNoon    = {    60.0f,   60.0f,    60.0f,    60.0f,    -1.0f,     45.0f,     3.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::HardRainNoon    = {   100.0f,  100.0f,    90.0f,    100.0f,   -1.0f,     45.0f,     7.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::SoftRainNoon    = {    20.0f,   30.0f,    50.0f,    30.0f,    -1.0f,     45.0f,     3.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::ClearSunset     = {     5.0f,    0.0f,     0.0f,    10.0f,    -1.0f,     15.0f,     2.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::CloudySunset    = {    60.0f,    0.0f,     0.0f,    10.0f,    -1.0f,     15.0f,     3.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::WetSunset       = {     5.0f,    0.0f,    50.0f,    10.0f,    -1.0f,     15.0f,     2.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::WetCloudySunset = {    60.0f,    0.0f,    50.0f,    10.0f,    -1.0f,     15.0f,     2.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::MidRainSunset   = {    60.0f,   60.0f,    60.0f,    60.0f,    -1.0f,     15.0f,     3.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::HardRainSunset  = {   100.0f,  100.0f,    90.0f,    100.0f,   -1.0f,     15.0f,     7.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::SoftRainSunset  = {    20.0f,   30.0f,    50.0f,    30.0f,    -1.0f,     15.0f,     2.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::ClearNight      = {     5.0f,    0.0f,     0.0f,    10.0f,    -1.0f,    -90.0f,    60.0f,     75.0f,     1.0f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::CloudyNight     = {    60.0f,    0.0f,     0.0f,    10.0f,    -1.0f,    -90.0f,    60.0f,     0.75f,     0.1f,    0.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::WetNight        = {     5.0f,    0.0f,    50.0f,    10.0f,    -1.0f,    -90.0f,    60.0f,     75.0f,     1.0f,   60.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::WetCloudyNight  = {    60.0f,    0.0f,    50.0f,    10.0f,    -1.0f,    -90.0f,    60.0f,     0.75f,     0.1f,   60.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::SoftRainNight   = {    60.0f,   30.0f,    50.0f,    30.0f,    -1.0f,    -90.0f,    60.0f,     0.75f,     0.1f,   60.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::MidRainyNight   = {    80.0f,   60.0f,    60.0f,    60.0f,    -1.0f,    -90.0f,    60.0f,     0.75f,     0.1f,   80.0f,   1.0f,   0.03f,   0.0331f};
  WP WP::HardRainNight   = {   100.0f,  100.0f,    90.0f,    100.0f,   -1.0f,    -90.0f,   100.0f,     0.75f,     0.1f,  100.0f,   1.0f,   0.03f,   0.0331f};

} // namespace rpc
} // namespace carla
