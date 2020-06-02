#pragma once

#include <random>

namespace carla {
namespace traffic_manager {

template<class T = double,
    class = std::enable_if_t<std::is_floating_point<T>::value>
> class RandomGenerator {
public:
    RandomGenerator(): mt{std::random_device{}()}, dist(0.0, 100.0) {}
    T next() { return dist(mt); }
private:
    std::mt19937 mt;
    std::uniform_real_distribution<T> dist;
};

} // namespace traffic_manager
} // namespace carla
