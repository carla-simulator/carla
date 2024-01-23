// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/ActorFactory.h"

#include "carla/Logging.h"
#include "carla/StringUtil.h"
#include "carla/client/Actor.h"
#include "carla/client/LaneInvasionSensor.h"
#include "carla/client/ServerSideSensor.h"
#ifdef RSS_ENABLED
#include "carla/rss/RssSensor.h"
#endif
#include "carla/client/TrafficLight.h"
#include "carla/client/TrafficSign.h"
#include "carla/client/Vehicle.h"
#include "carla/client/Walker.h"
#include "carla/client/WalkerAIController.h"
#include "carla/client/World.h"
#include "carla/client/detail/Client.h"

#include <rpc/config.h>
#include <rpc/rpc_error.h>

#include <exception>

namespace carla {
namespace client {
namespace detail {

  // A deleter cannot throw exceptions; and unlike std::unique_ptr, the deleter
  // of (std|boost)::shared_ptr is invoked even if the managed pointer is null.
  struct GarbageCollector {
    void operator()(::carla::client::Actor *ptr) const noexcept {
      if ((ptr != nullptr) && ptr->IsAlive()) {
        try {
          ptr->Destroy();
          delete ptr;
        } catch (const ::rpc::timeout &timeout) {
          log_error(timeout.what());
          log_error(
              "timeout while trying to garbage collect Actor",
              ptr->GetDisplayId(),
              "actor hasn't been removed from the simulation");
        } catch (const std::exception &e) {
          log_critical(
              "exception thrown while trying to garbage collect Actor",
              ptr->GetDisplayId(),
              e.what());
          std::terminate();
        } catch (...) {
          log_critical(
              "unknown exception thrown while trying to garbage collect an Actor :",
              ptr->GetDisplayId());
          std::terminate();
        }
      }
    }
  };

  template <typename ActorT>
  static auto MakeActorImpl(ActorInitializer init, GarbageCollectionPolicy gc) {
    if (gc == GarbageCollectionPolicy::Enabled) {
      return SharedPtr<ActorT>{new ActorT(std::move(init)), GarbageCollector()};
    }
    DEBUG_ASSERT(gc == GarbageCollectionPolicy::Disabled);
    return SharedPtr<ActorT>{new ActorT(std::move(init))};
  }

  SharedPtr<Actor> ActorFactory::MakeActor(
      EpisodeProxy episode,
      rpc::Actor description,
      GarbageCollectionPolicy gc) {
    auto init = ActorInitializer{description, episode};
    if (description.description.id == "sensor.other.lane_invasion") {
      return MakeActorImpl<LaneInvasionSensor>(std::move(init), gc);
#ifdef RSS_ENABLED
    } else if (description.description.id == "sensor.other.rss") {
      return MakeActorImpl<RssSensor>(std::move(init), gc);
#endif
    } else if (description.HasAStream()) {
      return MakeActorImpl<ServerSideSensor>(std::move(init), gc);
    } else if (StringUtil::StartsWith(description.description.id, "vehicle.")) {
      return MakeActorImpl<Vehicle>(std::move(init), gc);
    } else if (StringUtil::StartsWith(description.description.id, "walker.")) {
      return MakeActorImpl<Walker>(std::move(init), gc);
    } else if (StringUtil::StartsWith(description.description.id, "traffic.traffic_light")) {
      return MakeActorImpl<TrafficLight>(std::move(init), gc);
    } else if (StringUtil::StartsWith(description.description.id, "traffic.")) {
      return MakeActorImpl<TrafficSign>(std::move(init), gc);
    } else if (description.description.id == "controller.ai.walker") {
      return MakeActorImpl<WalkerAIController>(std::move(init), gc);
    }
    return MakeActorImpl<Actor>(std::move(init), gc);
  }

} // namespace detail
} // namespace client
} // namespace carla
