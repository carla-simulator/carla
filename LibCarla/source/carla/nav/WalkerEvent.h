// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <boost/variant.hpp>

#include "carla/rpc/ActorId.h"

namespace carla {
namespace nav {

    class Navigation;
    class WalkerManager;

    struct WalkerEventIgnore {
    };

    struct WalkerEventWait {
        double time;
        WalkerEventWait(double duration) : time(duration) {};
    };

    struct WalkerEventStopAndCheck {
        ActorId id;
        Navigation *nav;
        double time;
        WalkerEventStopAndCheck(ActorId id, Navigation *nav, double duration) : id(id), nav(nav), time(duration) {};
    };

    using WalkerEvent = boost::variant<WalkerEventIgnore, WalkerEventWait, WalkerEventStopAndCheck>;

    struct WalkerEventVisitor {
        bool operator()(WalkerEventIgnore &event, double delta, WalkerManager &manager);
        bool operator()(WalkerEventWait &event, double delta, WalkerManager &manager);
        bool operator()(WalkerEventStopAndCheck &event, double delta, WalkerManager &manager);
    };

} // namespace nav
} // namespace carla
