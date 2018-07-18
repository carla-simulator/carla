// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/NonCopyable.h"
#include "carla/client/ActorBlueprint.h"
#include "carla/client/Memory.h"

#include <algorithm>
#include <vector>

namespace carla {
namespace client {

  class BlueprintLibrary
    : public EnableSharedFromThis<BlueprintLibrary>,
      private NonCopyable {
    using list_type = std::vector<ActorBlueprint>;
  public:

    using value_type = list_type::value_type;
    using size_type = list_type::size_type;
    using const_iterator = list_type::const_iterator;
    using const_reference = list_type::const_reference;

    explicit BlueprintLibrary(const std::vector<rpc::ActorDefinition> &blueprints)
      : _blueprints(blueprints.begin(), blueprints.end()) {}

    BlueprintLibrary(BlueprintLibrary &&) = default;
    BlueprintLibrary &operator=(BlueprintLibrary &&) = default;

    /// Filters a list of ActorBlueprint with tags matching @a wildcard_pattern.
    SharedPtr<BlueprintLibrary> Filter(const std::string &wildcard_pattern) const {
      list_type result;
      std::copy_if(begin(), end(), std::back_inserter(result), [&](const auto &x) {
        return x.MatchTags(wildcard_pattern);
      });
      return SharedPtr<BlueprintLibrary>{new BlueprintLibrary(result)};
    }

    const_reference operator[](size_type pos) const {
      return _blueprints[pos];
    }

    const_iterator begin() const /*noexcept*/ {
      return _blueprints.begin();
    }

    const_iterator end() const /*noexcept*/ {
      return _blueprints.end();
    }

    bool empty() const /*noexcept*/ {
      return _blueprints.empty();
    }

    size_type size() const /*noexcept*/ {
      return _blueprints.size();
    }

  private:

    BlueprintLibrary(list_type blueprints)
      : _blueprints(std::move(blueprints)) {}

    list_type _blueprints;
  };

} // namespace client
} // namespace carla
