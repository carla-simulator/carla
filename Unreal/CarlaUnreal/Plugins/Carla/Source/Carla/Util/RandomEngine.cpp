// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Util/RandomEngine.h"
#include "Carla.h"

#include <limits>

uint64 URandomEngine::GenerateRandomId()
{
  // Tuned to be fast as this function might get called relatively often.
  static thread_local std::mt19937_64 Engine((std::random_device())());
  std::uniform_int_distribution<uint64> Distribution(
      std::numeric_limits<uint64>::lowest(),
      std::numeric_limits<uint64>::max());
  return Distribution(Engine);
}

int32 URandomEngine::GenerateRandomSeed()
{
  std::random_device RandomDevice;
  std::uniform_int_distribution<int32> Distribution(
      std::numeric_limits<int32>::lowest(),
      std::numeric_limits<int32>::max());
  return Distribution(RandomDevice);
}

int32 URandomEngine::GenerateSeed()
{
  return GetUniformIntInRange(
      std::numeric_limits<int32>::lowest(),
      std::numeric_limits<int32>::max());
}
