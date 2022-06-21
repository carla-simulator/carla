
#pragma once

#include "carla/rpc/ActorId.h"

namespace carla {
namespace traffic_manager {

using ActorId = carla::rpc::ActorId;

/// Stage type interface.
class Stage {

public:
    Stage() {};
    Stage(const Stage&) {};
    virtual ~Stage() {};

    // This method should be called every update cycle
    // and represents the core operation of the stage.
    virtual void Update(const unsigned long index) = 0;
    // This method should remove an actor from the internal state of the stage type.
    virtual void RemoveActor(const ActorId actor_id) = 0;
    // This method should flush all internal state of the state type.
    virtual void Reset() = 0;
};

} // namespace traffic_manager
} // namespace carla
