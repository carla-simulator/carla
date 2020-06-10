
#pragma once

#include <random>
#include <unordered_map>

#include "carla/rpc/ActorId.h"

namespace carla {
namespace traffic_manager {

class RandomGenerator {
public:
    RandomGenerator(): mt(std::mt19937(0)), dist(0.0, 100.0) {}
    double next() { return dist(mt); }
private:
    std::mt19937 mt;
    std::uniform_real_distribution<double> dist;
};

using RandomGeneratorMap = std::unordered_map<carla::rpc::ActorId, RandomGenerator>;

} // namespace traffic_manager
} // namespace carla
