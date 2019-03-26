// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <carla/geom/Location.h>

#include <algorithm>
#include <random>

namespace util {

  class Random {
  public:

    static double Uniform(double min, double max) {
      std::uniform_real_distribution<double> distribution(min, max);
      return distribution(_engine);
    }

    static carla::geom::Location Location(float min, float max) {
      std::uniform_real_distribution<float> distribution(min, max);
      return {distribution(_engine), distribution(_engine), distribution(_engine)};
    }

    template <typename RangeT>
    static void Shuffle(RangeT &range) {
      std::shuffle(std::begin(range), std::end(range), _engine);
    }

  private:

    // Defined in test.cpp.
    static thread_local std::mt19937_64 _engine;
  };

} // namespace util
