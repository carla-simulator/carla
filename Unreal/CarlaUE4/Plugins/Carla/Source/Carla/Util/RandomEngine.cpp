// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "RandomEngine.h"

#include <limits>

int32 URandomEngine::GenerateRandomSeed()
{
  std::random_device RandomDevice;
  std::uniform_int_distribution<int32> Distribution(
      std::numeric_limits<int32>::lowest(),
      std::numeric_limits<int32>::max());
  return Distribution(RandomDevice);
}
