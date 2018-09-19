// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <boost/optional.hpp>

namespace carla {

  template <typename T>
  class Optional : private boost::optional<T> {
    using parent_type = boost::optional<T>;
  public:

    template <typename... Args>
    explicit Optional(Args &&... args)
      : parent_type(std::forward<Args>(args)...) {}

    using parent_type::operator*;
    using parent_type::operator->;
    using parent_type::operator=;
    using parent_type::emplace;
    using parent_type::reset;
    using parent_type::swap;
    // using parent_type::value; disabled to avoid exceptions.
    using parent_type::value_or;

    bool has_value() const {
      return parent_type::is_initialized();
    }
  };

} // namespace carla
