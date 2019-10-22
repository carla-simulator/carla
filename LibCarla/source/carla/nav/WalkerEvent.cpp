// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Logging.h"
#include "carla/nav/WalkerEvent.h"

namespace carla {
namespace nav {

    bool WalkerEventVisitor::operator()(WalkerEventIgnore &, double ) {
        return true;
    }

    bool WalkerEventVisitor::operator()(WalkerEventWait &event, double delta) {
        // refresh time and check
        event.time -= delta;
        return (event.time <= 0.0f);
    }

    bool WalkerEventVisitor::operator()(WalkerEventStopAndCheck &event, double delta) {
        // refresh time and check
        event.time -= delta;
        return (event.time <= 0.0f);
    }

} // namespace nav
} // namespace carla
