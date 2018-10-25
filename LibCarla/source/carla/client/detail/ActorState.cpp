// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/ActorState.h"

#include <string>

namespace carla {
namespace client {
namespace detail {

  ActorState::ActorState(
      rpc::Actor description,
      EpisodeProxy episode,
      SharedPtr<Actor> parent)
    : _description(std::move(description)),
      _episode(std::move(episode)),
      _parent(std::move(parent)),
      _display_id([](const auto &desc) {
        using namespace std::string_literals;
        return
            "Actor "s +
            std::to_string(desc.id) +
            " (" + desc.description.id + ')';
      }(_description)) {}

} // namespace detail
} // namespace client
} // namespace carla
