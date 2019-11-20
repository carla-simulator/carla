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

    /// result of an event
    enum class EventResult : uint8_t {
        Continue,
        End,
        TimeOut
    };

    /// empty event that just ignores
    struct WalkerEventIgnore {
    };

    /// event to wait for a while
    struct WalkerEventWait {
        double time;
        WalkerEventWait(double duration) : time(duration) {};
    };

    /// event to pause and check for near vehicles
    struct WalkerEventStopAndCheck {
        double time;
        WalkerEventStopAndCheck(double duration) : time(duration) {};
    };

    /// walker event variant
    using WalkerEvent = boost::variant<WalkerEventIgnore, WalkerEventWait, WalkerEventStopAndCheck>;

    /// visitor class
    class WalkerEventVisitor {
        public:
            WalkerEventVisitor(WalkerManager *manager, ActorId id, double delta) : _manager(manager), _id(id), _delta(delta) {};
            EventResult operator()(WalkerEventIgnore &event);
            EventResult operator()(WalkerEventWait &event);
            EventResult operator()(WalkerEventStopAndCheck &event);
        private:
            WalkerManager *_manager { nullptr };
            ActorId _id { 0 };
            double _delta { 0 };
    };

} // namespace nav
} // namespace carla
