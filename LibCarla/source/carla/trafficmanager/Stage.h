
/// Stage type interface.

#include "carla/rpc/ActorId.h"

namespace carla
{
namespace traffic_manager
{

using ActorId = carla::rpc::ActorId;

class Stage
{
public:
    virtual void Update(const unsigned long index) = 0;
    virtual void RemoveActor(const ActorId actor_id) = 0;
    virtual void Reset() = 0;
};

} // namespace traffic_manager
} // namespace carla
