// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

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
