// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/EpisodeState.h"

namespace carla {
namespace client {
namespace detail {

  static auto DeriveAcceleration(
      double delta_seconds,
      const geom::Vector3D &v0,
      const geom::Vector3D &v1) {
    /// @todo add methods to Vector3D for scalar multiplication.
    auto acc = v1 - v0;
    acc.x /= delta_seconds;
    acc.y /= delta_seconds;
    acc.z /= delta_seconds;
    return acc;
  }

  std::shared_ptr<const EpisodeState> EpisodeState::DeriveNextStep(
      const sensor::data::RawEpisodeState &state) const {
    auto next = std::make_shared<EpisodeState>();
    next->_timestamp.frame_count = state.GetFrameNumber();
    next->_timestamp.elapsed_seconds = state.GetGameTimeStamp();
    next->_timestamp.platform_timestamp = state.GetPlatformTimeStamp();
    next->_timestamp.delta_seconds = next->_timestamp.elapsed_seconds - _timestamp.elapsed_seconds;
    next->_actors.reserve(state.size());
    for (auto &&actor : state) {
      auto acceleration = DeriveAcceleration(
          next->_timestamp.delta_seconds,
          GetActorState(actor.id).velocity,
          actor.velocity);
      DEBUG_ONLY(auto result = )
      next->_actors.emplace(
          actor.id,
          ActorState{actor.transform, actor.velocity, actor.angularVelocity, acceleration, actor.state});
      DEBUG_ASSERT(result.second);
    }
    return next;
  }

} // namespace detail
} // namespace client
} // namespace carla
