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
            // calculate the direction to look for vehicles
            carla::geom::Location currentUnrealPos;
            carla::geom::Location crosswalkEnd;
            carla::geom::Location direction;
            _manager->GetNavigation()->GetWalkerPosition(_id, currentUnrealPos);
            _manager->GetWalkerCrosswalkEnd(_id, crosswalkEnd);
            direction.x = crosswalkEnd.x - currentUnrealPos.x;
            direction.y = crosswalkEnd.y - currentUnrealPos.y;
            direction.z = crosswalkEnd.z - currentUnrealPos.z;
            // check if the agent has any vehicle around
            if (_manager && !(_manager->GetNavigation()->HasVehicleNear(_id, 6.0f, direction))) {
                return EventResult::End;
            } else {
                return EventResult::Continue;
            }
        }
    }

} // namespace nav
} // namespace carla
