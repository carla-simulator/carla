// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/ActorState.h"

#include <string>
#include <iterator>

namespace carla {
namespace client {
namespace detail {

  ActorState::ActorState(
      rpc::Actor description,
      EpisodeProxy episode)
    : _description(std::move(description)),
      _episode(std::move(episode)),
      _display_id([](const auto &desc) {
        using namespace std::string_literals;
        return
            "Actor "s +
            std::to_string(desc.id) +
            " (" + desc.description.id + ')';
      }(_description)),
      _attributes(_description.description.attributes.begin(), _description.description.attributes.end())
  {}

  SharedPtr<Actor> ActorState::GetParent() const {
    auto parent_id = GetParentId();
    return parent_id != 0u ? GetWorld().GetActor(parent_id) : nullptr;
  }

} // namespace detail
} // namespace client
} // namespace carla
