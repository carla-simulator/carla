// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Logging.h"
#include "carla/nav/Navigation.h"
#include "carla/nav/WalkerManager.h"
#include "carla/nav/WalkerEvent.h"

namespace carla {
namespace nav {

    bool WalkerEventVisitor::operator()(WalkerEventIgnore &, double, WalkerManager &) {
        return true;
    }

    bool WalkerEventVisitor::operator()(WalkerEventWait &event, double delta, WalkerManager &) {
        // refresh time and check
        event.time -= delta;
        return (event.time <= 0.0);
    }

    bool WalkerEventVisitor::operator()(WalkerEventStopAndCheck &event, double delta, WalkerManager &manager) {
        event.time -= delta;
        logging::log("time = ", event.time);
        if (event.time <= 0.0) {
            // assign a new route
            logging::log("event time out");
            manager.SetWalkerRoute(event.id);
            return true;
        } else {
            // check if the agent has any vehicle around
            return (!event.nav->hasVehicleNear(event.id));
        }
    }

} // namespace nav
} // namespace carla
