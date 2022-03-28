
#pragma once

#include <random>
#include <unordered_map>

#include "carla/rpc/ActorId.h"

namespace carla {
namespace traffic_manager {

class RandomGenerator {
public:
    RandomGenerator(const uint64_t seed): mt(std::mt19937(seed)), dist(0.0, 100.0) {}
    double next() { return dist(mt); }
private:
    std::mt19937 mt;
    std::uniform_real_distribution<double> dist;
};

} // namespace traffic_manager
} // namespace carla
