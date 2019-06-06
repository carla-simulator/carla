// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/WalkerAIController.h"

#include "carla/client/detail/Simulator.h"

namespace carla {
namespace client {

  WalkerAIController::WalkerAIController(ActorInitializer init)
    : Actor(std::move(init)) {}

  void WalkerAIController::Start(carla::geom::Location location) {
    GetEpisode().Lock()->RegisterAIController(*this);

    // add the walker in the Recast & Detour
    auto walker = GetParent();
    if (walker != nullptr) {
      auto nav = GetEpisode().Lock()->GetNavigation();
      nav->AddWalker(walker->GetId(), location);
    }
  }

  geom::Location WalkerAIController::GetRandomLocation() {
    auto nav = GetEpisode().Lock()->GetNavigation();
    return nav->GetRandomLocation();
  }

  void WalkerAIController::GoToLocation(const carla::geom::Location &destination) {
    auto nav = GetEpisode().Lock()->GetNavigation();
    if (!nav->SetWalkerTarget(GetParent()->GetId(), destination))
      logging::log("NAV: Failed to set request to go to ", destination.x, destination.y, destination.z);
  }

  void WalkerAIController::SetMaxSpeed(const float max_speed) {

    logging::log("SET MAX SPEED: ", max_speed);
  }

} // namespace client
} // namespace carla
