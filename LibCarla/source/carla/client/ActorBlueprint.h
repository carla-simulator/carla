// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/rpc/ActorBlueprint.h"

namespace carla {
namespace client {

  class ActorBlueprint {
  public:

    ActorBlueprint(carla::rpc::ActorBlueprint blueprint)
      : _blueprint(std::move(blueprint)) {}

    ActorBlueprint(const ActorBlueprint &) = default;
    ActorBlueprint(ActorBlueprint &&) = default;
    ActorBlueprint &operator=(const ActorBlueprint &) = default;
    ActorBlueprint &operator=(ActorBlueprint &&) = default;

    const std::string &GetTypeId() const {
      return _blueprint.type_id;
    }

    bool StartsWith(const std::string &test) const;

    bool MatchWildcards(const std::string &test) const;

    const auto &Serialize() const {
      return _blueprint;
    }

  private:

    carla::rpc::ActorBlueprint _blueprint;
  };

} // namespace client
} // namespace carla
