// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/BlueprintLibrary.h"

#include "carla/Exception.h"

#include <algorithm>
#include <iterator>

namespace carla {
namespace client {

  BlueprintLibrary::BlueprintLibrary(
      const std::vector<rpc::ActorDefinition> &blueprints) {
    _blueprints.reserve(blueprints.size());
    for (auto &definition : blueprints) {
      _blueprints.emplace(definition.id, ActorBlueprint{definition});
    }
  }

  SharedPtr<BlueprintLibrary> BlueprintLibrary::Filter(
      const std::string &wildcard_pattern) const {
    map_type result;
    for (auto &pair : _blueprints) {
      if (pair.second.MatchTags(wildcard_pattern)) {
        result.emplace(pair);
      }
    }
    return SharedPtr<BlueprintLibrary>{new BlueprintLibrary(result)};
  }

  BlueprintLibrary::const_pointer BlueprintLibrary::Find(const std::string &key) const {
    auto it = _blueprints.find(key);
    return it != _blueprints.end() ? &it->second : nullptr;
  }

  BlueprintLibrary::const_reference BlueprintLibrary::at(const std::string &key) const {
    auto it = _blueprints.find(key);
    if (it == _blueprints.end()) {
      using namespace std::string_literals;
      throw_exception(std::out_of_range("blueprint '"s + key + "' not found"));
    }
    return it->second;
  }

  BlueprintLibrary::const_reference BlueprintLibrary::at(size_type pos) const {
    if (pos >= size()) {
      throw_exception(std::out_of_range("index out of range"));
    }
    return operator[](pos);
  }

} // namespace client
} // namespace carla
