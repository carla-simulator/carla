// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <boost/iterator/transform_iterator.hpp>

namespace carla {
namespace iterator {

  /// Creates an iterator over the keys of a map.
  template <typename It>
  static auto make_map_keys_iterator(It it) {
    return boost::make_transform_iterator(it, [](auto &pair){ return pair.first; });
  }

  /// Creates an iterator over the values of a map.
  template <typename It>
  static auto make_map_values_iterator(It it) {
    return boost::make_transform_iterator(it, [](auto &pair){ return pair.second; });
  }

} // namespace iterator
} // namespace carla
