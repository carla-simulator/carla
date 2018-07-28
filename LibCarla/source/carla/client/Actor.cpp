// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Actor.h"
#include "carla/client/Client.h"

namespace carla {
namespace client {

  void Actor::Destroy() {
    GetWorld()->GetClient().DestroyActor(*this);
  }

} // namespace client
} // namespace carla
