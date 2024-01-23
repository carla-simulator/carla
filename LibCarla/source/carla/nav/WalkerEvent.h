// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4583)
#pragma warning(disable:4582)
#include <variant>
#pragma warning(pop)
#else
#include <variant>
#endif

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
        bool check_for_trafficlight;
        SharedPtr<carla::client::TrafficLight> actor;
        WalkerEventStopAndCheck(double duration) : time(duration),
                                                   check_for_trafficlight(true)
                                                   {};
    };

    /// walker event variant
    using WalkerEvent = std::variant<WalkerEventIgnore, WalkerEventWait, WalkerEventStopAndCheck>;

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
