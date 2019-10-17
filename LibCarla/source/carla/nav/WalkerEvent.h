// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <boost/variant.hpp>

namespace carla {
namespace nav {

    struct WalkerEventIgnore {
    };

    struct WalkerEventWait {
        float time;
        WalkerEventWait(float duration) : time(duration) {};
    };

    struct WalkerEventStopAndCheck {
        float time;
        WalkerEventStopAndCheck(float duration) : time(duration) {};
    };

    using WalkerEvent = boost::variant<WalkerEventIgnore, WalkerEventWait, WalkerEventStopAndCheck>;

    struct WalkerEventVisitor {
        bool operator()(WalkerEventIgnore &event, double delta);
        bool operator()(WalkerEventWait &event, double delta);
        bool operator()(WalkerEventStopAndCheck &event, double delta);
    };

} // namespace nav
} // namespace carla
