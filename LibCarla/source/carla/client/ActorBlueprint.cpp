// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/ActorBlueprint.h"

#include "carla/Exception.h"
#include "carla/StringUtil.h"

#include <algorithm>

namespace carla {
namespace client {

  template <typename Map, typename Container>
  static void FillMap(Map &destination, Container &source) {
    destination.reserve(source.size());
    for (auto &item : source) {
      auto id = item.id;
      destination.emplace(id, std::move(item));
    }
  }

  ActorBlueprint::ActorBlueprint(rpc::ActorDefinition definition)
    : _uid(definition.uid),
      _id(std::move(definition.id)) {
    StringUtil::Split(_tags, definition.tags, ",");
    FillMap(_attributes, definition.attributes);
  }

  bool ActorBlueprint::MatchTags(const std::string &wildcard_pattern) const {
    return
        StringUtil::Match(_id, wildcard_pattern) ||
        std::any_of(_tags.begin(), _tags.end(), [&](const auto &tag) {
          return StringUtil::Match(tag, wildcard_pattern);
        });
  }

  const ActorAttribute &ActorBlueprint::GetAttribute(const std::string &id) const {
    auto it = _attributes.find(id);
    if (it == _attributes.end()) {
      using namespace std::string_literals;
      throw_exception(std::out_of_range("attribute '"s + id + "' not found"));
    }
    return it->second;
  }

  void ActorBlueprint::SetAttribute(const std::string &id, std::string value) {
    const_cast<ActorAttribute &>(GetAttribute(id)).Set(std::move(value));
  }

  rpc::ActorDescription ActorBlueprint::MakeActorDescription() const {
    rpc::ActorDescription description;
    description.uid = _uid;
    description.id = _id;
    description.attributes.reserve(_attributes.size());
    for (const auto &attribute : *this) {
      description.attributes.push_back(attribute);
    }
    return description;
  }

} // namespace client
} // namespace carla
