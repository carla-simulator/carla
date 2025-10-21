
#pragma once

#include <random>
#include <unordered_map>

#include "carla/rpc/ActorId.h"

namespace carla {
namespace traffic_manager {

class UniformPRNG {

    using UnderlyingPRNG = std::mt19937;

    UnderlyingPRNG state;

public:

    UniformPRNG(uint64_t seed) :
        state(UnderlyingPRNG(seed)) {
    }
    
    template <
        typename T,
        typename = std::enable_if<std::is_scalar<T>::value>>
    T next(T begin, T end) {

        using Distribution = typename std::conditional<
            std::is_floating_point<T>::value,
            std::uniform_real_distribution<T>,
            std::uniform_int_distribution<T>>::type;

        return Distribution(begin, end)(state);
    }

    template <
        typename T = double,
        typename = std::enable_if<std::is_scalar<T>::value>>
    T next() {
        return next(0.0, 100.0);
    }

};

} // namespace traffic_manager
} // namespace carla
