// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>
#include <vector>

namespace carla {
namespace sensor {

namespace s11n {
  class BenchmarkSerializer;
}

namespace data {

  struct BenchmarkResult {
    BenchmarkResult(std::string group, std::string stat, float value) :
      _group(group), _stat(stat), _value(value) {}

    std::string _group;
    std::string _stat;
    float _value;
  };

  class BenchmarkData {
  public:
    explicit BenchmarkData() = default;

    BenchmarkData &operator=(BenchmarkData &&) = default;

  void AddBenchmarkResult(std::string group, std::string stat, float value) {
    BenchmarkResult result{group, stat, value};
    _benchmark_result.emplace_back(result);
  }

  private:
    std::vector<BenchmarkResult> _benchmark_result;

    friend class s11n::BenchmarkSerializer;

  };

} // namespace data
} // namespace sensor
} // namespace carla