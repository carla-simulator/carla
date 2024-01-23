// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <boost/iterator/transform_iterator.hpp>

namespace carla {
namespace iterator {

  /// Creates an iterator over non-const references to the keys of a map.
  template <typename It>
  inline static auto make_map_keys_iterator(It it) {
    using first_value_type = typename It::value_type::first_type;
    using decay_first_value_type = typename std::remove_cv_t<typename std::remove_reference_t<first_value_type>>;
    using ref_to_first = decay_first_value_type &;
    return boost::make_transform_iterator(it, [](auto &pair) -> ref_to_first { return pair.first; });
  }

  /// Creates an iterator over const references to the keys of a map.
  template <typename It>
  inline static auto make_map_keys_const_iterator(It it) {
    using first_value_type = typename It::value_type::first_type;
    using decay_first_value_type = typename std::remove_cv_t<typename std::remove_reference_t<first_value_type>>;
    using const_ref_to_first = const decay_first_value_type &;
    return boost::make_transform_iterator(it, [](const auto &pair) -> const_ref_to_first { return pair.first; });
  }

  /// Creates an iterator over non-const references to the values of a map.
  template <typename It>
  inline static auto make_map_values_iterator(It it) {
    using second_value_type = typename It::value_type::second_type;
    using decay_second_value_type = typename std::remove_cv_t<typename std::remove_reference_t<second_value_type>>;
    using ref_to_second = decay_second_value_type &;
    return boost::make_transform_iterator(it, [](auto &pair) -> ref_to_second { return pair.second; });
  }

  /// Creates an iterator over const references to the values of a map.
  template <typename It>
  inline static auto make_map_values_const_iterator(It it) {
    using second_value_type = typename It::value_type::second_type;
    using decay_second_value_type = typename std::remove_cv_t<typename std::remove_reference_t<second_value_type>>;
    using const_ref_to_second = const decay_second_value_type &;
    return boost::make_transform_iterator(it, [](const auto &pair) -> const_ref_to_second { return pair.second; });
  }

} // namespace iterator
} // namespace carla
