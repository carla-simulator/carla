// Copyright (c) 2020 Robotics and Perception Group (GPR)
// University of Zurich and ETH Zurich
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

namespace carla {
namespace sensor {
namespace data {

#pragma pack(push, 1)
struct DVSEvent {
    /// Default constructor
    DVSEvent() = default;

    /// Copy Constructor
    DVSEvent(const DVSEvent &arg)
      : x(arg.x), y(arg.y), t(arg.t), pol(arg.pol) {}

    /// Moving constructor
    DVSEvent(const DVSEvent &&arg)
      : x(std::move(arg.x)),
        y(std::move(arg.y)),
        t(std::move(arg.t)),
        pol(std::move(arg.pol)) {}

    /// Constructor
    DVSEvent(std::uint16_t x, std::uint16_t y, std::int64_t t, bool pol)
      : x(x), y(y), t(t), pol(pol) {}

    /// Assignement operator
    DVSEvent &operator=(const DVSEvent &other) {
      x = other.x;
      y = other.y;
      t = other.t;
      pol = other.pol;
      return *this;
    }

    /// Move Assignement operator
    DVSEvent &operator=(const DVSEvent &&other) {
      x = std::move(other.x);
      y = std::move(other.y);
      t = std::move(other.t);
      pol = std::move(other.pol);
      return *this;
    }

    bool operator==(const DVSEvent &rhs) const {
      return (x == rhs.x) && (y == rhs.y) && (t == rhs.t) && (pol == rhs.pol);
    }

    bool operator!=(const DVSEvent &rhs) const {
      return !(*this == rhs);
    }

    std::uint16_t x;
    std::uint16_t y;
    std::int64_t t;
    bool pol;

  };
#pragma pack(pop)
} // namespace data
} // namespace sensor
} // namespace carla
