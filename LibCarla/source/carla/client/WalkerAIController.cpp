// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/WalkerAIController.h"

#include "carla/Exception.h"
#include "carla/Logging.h"
#include "carla/client/detail/Simulator.h"
#include "carla/client/detail/WalkerNavigation.h"

#include <stdexcept>

namespace carla {
namespace client {

  WalkerAIController::WalkerAIController(ActorInitializer init)
    : Actor(std::move(init)) {}

  void WalkerAIController::Start() {
    auto simulator = GetEpisode().Lock();
    if (simulator->IsNavigationServerSide()) {
      // Server-side navigation: the walker keeps physics and collisions on;
      // the server's crowd controller drives its CharacterMovement directly.
      auto walker = GetParent();
      if (walker == nullptr) {
        throw_exception(std::runtime_error(GetDisplayId() + ": not attached to walker"));
        return;
      }
      if (!simulator->WalkerStartNavigation(walker->GetId())) {
        log_warning("NAV: server failed to start navigation for walker", walker->GetId());
      }
      return;
    }

    simulator->RegisterAIController(*this);

    // add the walker in the Recast & Detour
    auto walker = GetParent();
    if (walker != nullptr) {
      auto nav = simulator->GetNavigation();
      if (nav != nullptr) {
        nav->AddWalker(walker->GetId(), walker->GetLocation());
        // disable physics and collision of walker actor
        simulator->SetActorSimulatePhysics(*walker, false);
        simulator->SetActorCollisions(*walker, false);
      }
    }
  }

  void WalkerAIController::Stop() {
    auto simulator = GetEpisode().Lock();
    if (simulator->IsNavigationServerSide()) {
      auto walker = GetParent();
      if (walker == nullptr) {
        throw_exception(std::runtime_error(GetDisplayId() + ": not attached to walker"));
        return;
      }
      if (!simulator->WalkerStopNavigation(walker->GetId())) {
        log_warning("NAV: server failed to stop navigation for walker", walker->GetId());
      }
      return;
    }

    simulator->UnregisterAIController(*this);

    // remove the walker from the Recast & Detour
    auto walker = GetParent();
    if (walker != nullptr) {
      auto nav = simulator->GetNavigation();
      if (nav != nullptr) {
        nav->RemoveWalker(walker->GetId());
      }
    }
  }

  std::optional<geom::Location> WalkerAIController::GetRandomLocation() {
    auto simulator = GetEpisode().Lock();
    if (simulator->IsNavigationServerSide()) {
      return simulator->GetRandomLocationFromNavigation();
    }

    auto nav = simulator->GetNavigation();
    if (nav != nullptr) {
      return nav->GetRandomLocation();
    }
    return {};
  }

  void WalkerAIController::GoToLocation(const carla::geom::Location &destination) {
    auto simulator = GetEpisode().Lock();
    if (simulator->IsNavigationServerSide()) {
      auto walker = GetParent();
      if (walker != nullptr) {
        if (!simulator->WalkerGoToLocation(walker->GetId(), destination)) {
          log_warning("NAV: Failed to set request to go to ", destination.x, destination.y, destination.z);
        }
      } else {
        log_warning("NAV: Failed to set request to go to ", destination.x, destination.y, destination.z, "(parent does not exist)");
      }
      return;
    }

    auto nav = simulator->GetNavigation();
    if (nav != nullptr) {
      auto walker = GetParent();
      if (walker != nullptr) {
        if (!nav->SetWalkerTarget(walker->GetId(), destination)) {
          log_warning("NAV: Failed to set request to go to ", destination.x, destination.y, destination.z);
        }
      } else {
        log_warning("NAV: Failed to set request to go to ", destination.x, destination.y, destination.z, "(parent does not exist)");
      }
    }
  }

  void WalkerAIController::SetMaxSpeed(const float max_speed) {
    auto simulator = GetEpisode().Lock();
    if (simulator->IsNavigationServerSide()) {
      auto walker = GetParent();
      if (walker != nullptr) {
        if (!simulator->WalkerSetMaxSpeed(walker->GetId(), max_speed)) {
          log_warning("NAV: failed to set max speed");
        }
      } else {
        log_warning("NAV: failed to set max speed (parent does not exist)");
      }
      return;
    }

    auto nav = simulator->GetNavigation();
    if (nav != nullptr) {
      auto walker = GetParent();
      if (walker != nullptr) {
        if (!nav->SetWalkerMaxSpeed(walker->GetId(), max_speed)) {
          log_warning("NAV: failed to set max speed");
        }
      } else {
        log_warning("NAV: failed to set max speed (parent does not exist)");
      }
    }
  }

} // namespace client
} // namespace carla
