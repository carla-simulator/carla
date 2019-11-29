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

    EventResult WalkerEventVisitor::operator()(WalkerEventIgnore &) {
        return EventResult::End;
    }

    EventResult WalkerEventVisitor::operator()(WalkerEventWait &event) {
        // refresh time and check
        event.time -= _delta;
        if (event.time <= 0.0)
            return EventResult::End;
        else
            return EventResult::Continue;
    }

    EventResult WalkerEventVisitor::operator()(WalkerEventStopAndCheck &event) {
        event.time -= _delta;
        if (event.time <= 0.0) {
            return EventResult::TimeOut;
        } else {
            // check if the agent has any vehicle around
            if (_manager && !(_manager->GetNavigation()->HasVehicleNear(_id, 6.0f)))
                return EventResult::End;
            else
            {
                // make look at the next point to go
                carla::geom::Location location;
                if (_manager->GetWalkerNextPoint(_id, location))
                    _manager->GetNavigation()->SetWalkerLookAt(_id, location);
                return EventResult::Continue;
            }
        }
    }

} // namespace nav
} // namespace carla
