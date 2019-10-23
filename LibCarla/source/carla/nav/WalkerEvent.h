// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <boost/variant.hpp>

#include "carla/rpc/ActorId.h"
// #include "carla/nav/Navigation.h"

namespace carla {
namespace nav {

    class Navigation;

    struct WalkerEventIgnore {
    };

    struct WalkerEventWait {
        double time;
        WalkerEventWait(double duration) : time(duration) {};
    };

    struct WalkerEventStopAndCheck {
        ActorId id;
        Navigation *nav;
        WalkerEventStopAndCheck(ActorId id, Navigation *nav) : id(id), nav(nav) {};
    };

    using WalkerEvent = boost::variant<WalkerEventIgnore, WalkerEventWait, WalkerEventStopAndCheck>;

    struct WalkerEventVisitor {
        bool operator()(WalkerEventIgnore &event, double delta);
        bool operator()(WalkerEventWait &event, double delta);
        bool operator()(WalkerEventStopAndCheck &event, double delta);
    };

} // namespace nav
} // namespace carla
