// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Logging.h"
#include "carla/client/Actor.h"

namespace carla {
namespace client {

  class Sensor : public Actor {
  public:

    template <typename Functor>
    void Listen(Functor callback) {
      /// @todo should we check if we are already listening?
      log_debug("sensor", GetId(), "type", GetTypeId(), ": subscribing to stream");
      GetWorld()->GetClient().SubscribeToStream(_stream_token, std::forward<Functor>(callback));
    }

  private:

    friend class Client;

    Sensor(carla::rpc::Actor actor, SharedPtr<World> world)
      : Actor(actor, std::move(world)),
        _stream_token(actor.GetStreamToken()) {}

    streaming::Token _stream_token;
  };

} // namespace client
} // namespace carla
