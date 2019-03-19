// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Iterator.h"
#include "carla/client/ActorAttribute.h"
#include "carla/rpc/ActorDefinition.h"
#include "carla/rpc/ActorDescription.h"

#include <exception>
#include <unordered_map>
#include <unordered_set>

namespace carla {
namespace client {

  /// Contains all the necessary information for spawning an Actor.
  class ActorBlueprint {
  public:

    // =========================================================================
    /// @name Constructor
    // =========================================================================
    /// @{

    explicit ActorBlueprint(rpc::ActorDefinition actor_definition);

    /// @}
    // =========================================================================
    /// @name Id
    // =========================================================================
    /// @{

  public:

    const std::string &GetId() const {
      return _id;
    }

    /// @}
    // =========================================================================
    /// @name Tags
    // =========================================================================
    /// @{

  public:

    bool ContainsTag(const std::string &tag) const {
      return _tags.find(tag) != _tags.end();
    }

    /// Test if any of the flags matches @a wildcard_pattern.
    ///
    /// @a wildcard_pattern follows Unix shell-style wildcards.
    bool MatchTags(const std::string &wildcard_pattern) const;

    std::vector<std::string> GetTags() const {
      return {_tags.begin(), _tags.end()};
    }

    /// @}
    // =========================================================================
    /// @name Attributes
    // =========================================================================
    /// @{

  public:

    bool ContainsAttribute(const std::string &id) const {
      return _attributes.find(id) != _attributes.end();
    }

    /// @throw std::out_of_range if no such element exists.
    const ActorAttribute &GetAttribute(const std::string &id) const;

    /// Set the value of the attribute given by @a id.
    ///
    /// @throw std::out_of_range if no such element exists.
    /// @throw InvalidAttributeValue if attribute is not modifiable.
    /// @throw InvalidAttributeValue if format does not match the attribute type.
    void SetAttribute(const std::string &id, std::string value);

    size_t size() const {
      return _attributes.size();
    }

    auto begin() const {
      return iterator::make_map_values_const_iterator(_attributes.begin());
    }

    auto end() const {
      return iterator::make_map_values_const_iterator(_attributes.end());
    }

    /// @}
    // =========================================================================
    /// @name ActorDescription
    // =========================================================================
    /// @{

  public:

    rpc::ActorDescription MakeActorDescription() const;

    /// @}

  private:

    uint32_t _uid = 0u;

    std::string _id;

    std::unordered_set<std::string> _tags;

    std::unordered_map<std::string, ActorAttribute> _attributes;
  };

} // namespace client
} // namespace carla
