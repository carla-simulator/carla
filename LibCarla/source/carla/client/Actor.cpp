// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Actor.h"

#include "carla/Logging.h"
#include "carla/client/detail/Simulator.h"

namespace carla {
namespace client {

  geom::Location Actor::GetLocation() const {
    return GetEpisode().Lock()->GetActorLocation(*this);
  }

  geom::Transform Actor::GetTransform() const {
    return GetEpisode().Lock()->GetActorTransform(*this);
  }

  geom::Vector3D Actor::GetVelocity() const {
    return GetEpisode().Lock()->GetActorVelocity(*this);
  }

  geom::Vector3D Actor::GetAcceleration() const {
    return GetEpisode().Lock()->GetActorAcceleration(*this);
  }

  void Actor::SetLocation(const geom::Location &location) {
    GetEpisode().Lock()->SetActorLocation(*this, location);
  }

  void Actor::SetTransform(const geom::Transform &transform) {
    GetEpisode().Lock()->SetActorTransform(*this, transform);
  }

  void Actor::SetSimulatePhysics(const bool enabled) {
    GetEpisode().Lock()->SetActorSimulatePhysics(*this, enabled);
  }

  void Actor::Destroy() {
    if (_is_alive) {
      // Let the exceptions leave the function, IsAlive() will still be true.
      _is_alive = !GetEpisode().Lock()->DestroyActor(*this);
    } else {
      log_warning(
          "attempting to destroy an actor that is already dead:",
          GetDisplayId());
    }
  }

} // namespace client
} // namespace carla
