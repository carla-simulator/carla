// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Iterator.h"
#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/client/ActorBlueprint.h"

#include <type_traits>
#include <unordered_map>
#include <vector>

namespace carla {
namespace client {

  /// @todo Works as a list but its actually a map. We should assess the use
  /// cases and reconsider this implementation.
  class BlueprintLibrary
    : public EnableSharedFromThis<BlueprintLibrary>,
      private MovableNonCopyable {
    using map_type = std::unordered_map<std::string, ActorBlueprint>;
  public:

    // Here we force a bit the typedefs to make this class look like a list.
    using key_type = map_type::key_type;
    using value_type = map_type::mapped_type;
    using size_type = map_type::size_type;
    using const_iterator = decltype(carla::iterator::make_map_values_const_iterator<map_type::const_iterator>(map_type::const_iterator{}));
    using const_reference = const value_type &;
    using const_pointer = const value_type *;

    explicit BlueprintLibrary(const std::vector<rpc::ActorDefinition> &blueprints);

    BlueprintLibrary(BlueprintLibrary &&) = default;
    BlueprintLibrary &operator=(BlueprintLibrary &&) = default;

    /// Filters a list of ActorBlueprint with id or tags matching
    /// @a wildcard_pattern.
    SharedPtr<BlueprintLibrary> Filter(const std::string &wildcard_pattern) const;
    SharedPtr<BlueprintLibrary> FilterByAttribute(const std::string &name, const std::string& value) const;

    const_pointer Find(const std::string &key) const;

    /// @throw std::out_of_range if no such element exists.
    const_reference at(const std::string &key) const;

    /// @warning Linear complexity.
    const_reference operator[](size_type pos) const {
      using diff_t = std::iterator_traits<const_iterator>::difference_type;
      return std::next(_blueprints.begin(), static_cast<diff_t>(pos))->second;
    }

    /// @warning Linear complexity.
    /// @throw std::out_of_range if !(pos < size()).
    const_reference at(size_type pos) const;

    const_iterator begin() const /*noexcept*/ {
      return iterator::make_map_values_const_iterator(_blueprints.begin());
    }

    const_iterator end() const /*noexcept*/ {
      return iterator::make_map_values_const_iterator(_blueprints.end());
    }

    bool empty() const /*noexcept*/ {
      return _blueprints.empty();
    }

    size_type size() const /*noexcept*/ {
      return _blueprints.size();
    }

  private:

    BlueprintLibrary(map_type blueprints)
      : _blueprints(std::move(blueprints)) {}

    map_type _blueprints;
  };

} // namespace client
} // namespace carla
