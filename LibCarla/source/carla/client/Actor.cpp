// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Actor.h"

#include "carla/Logging.h"
#include "carla/client/detail/Client.h"

namespace carla {
namespace client {

  geom::Location Actor::GetLocation() const {
    return GetClientImplementation()->GetActorLocation(*this);
  }

  geom::Transform Actor::GetTransform() const {
    return GetClientImplementation()->GetActorTransform(*this);
  }

  void Actor::SetLocation(const geom::Location &location) {
    GetClientImplementation()->SetActorLocation(*this, location);
  }

  void Actor::SetTransform(const geom::Transform &transform) {
    GetClientImplementation()->SetActorTransform(*this, transform);
  }

  void Actor::Destroy() {
    if (_is_alive) {
      // Let the exceptions leave the function, IsAlive() will still be true.
      _is_alive = !GetClientImplementation()->DestroyActor(*this);
    } else {
      log_warning(
          "attempting to destroy an actor that is already dead:",
          GetDisplayId());
    }
  }

} // namespace client
} // namespace carla
