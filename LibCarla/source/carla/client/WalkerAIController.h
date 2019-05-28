// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"

namespace carla {
namespace client {

  class WalkerAIController : public Actor {
  public:

    explicit WalkerAIController(ActorInitializer init);

    void Start();
  };

} // namespace client
} // namespace carla
