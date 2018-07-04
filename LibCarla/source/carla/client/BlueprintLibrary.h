// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/NonCopyable.h"
#include "carla/client/ActorBlueprint.h"

#include <algorithm>
#include <vector>

namespace carla {
namespace client {

  class Client;

  class BlueprintLibrary /*: private NonCopyable*/ {
    using list_type = std::vector<ActorBlueprint>;
  public:

    // BlueprintLibrary() = default;

    // BlueprintLibrary(BlueprintLibrary &&) = default;
    // BlueprintLibrary &operator=(BlueprintLibrary &&) = default;

    using value_type = list_type::value_type;
    using size_type = list_type::size_type;
    using const_iterator = list_type::const_iterator;
    using const_reference = list_type::const_reference;

    BlueprintLibrary Filter(const std::string &wildcard_pattern) const {
      list_type result;
      std::copy_if(begin(), end(), std::back_inserter(result), [&](const auto &x) {
        return x.MatchWildcards(wildcard_pattern);
      });
      return result;
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

    friend class Client;

    BlueprintLibrary(list_type blueprints)
      : _blueprints(std::move(blueprints)) {}

    BlueprintLibrary(const std::vector<carla::rpc::ActorBlueprint> &blueprints)
      : _blueprints(blueprints.begin(), blueprints.end()) {}

    list_type _blueprints;
  };

} // namespace client
} // namespace carla
