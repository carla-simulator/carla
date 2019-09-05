#pragma once

#include <vector>
#include <memory>

#include "carla/Memory.h"
#include "carla/client/Actor.h"

#include "Messenger.h"
#include "SimpleWaypoint.h"

namespace traffic_manager {

    /// Convenience typing
    typedef std::deque<std::shared_ptr<SimpleWaypoint>> Buffer;
    typedef std::vector<Buffer> BufferList;

    /// Data types

    struct LocalizationToPlannerData {
        carla::SharedPtr<carla::client::Actor> actor;
        float deviation;
        // bool approaching_true_junction;
    };

    struct PlannerToControlData {
        int actor_id;
        float throttle;
        float brake;
        float steer;
    };

    struct LocalizationToCollisionData {
        carla::SharedPtr<carla::client::Actor> actor;
        Buffer* buffer;
    };

    struct CollisionToPlannerData {
        bool hazard;
    };

    /// Data frame types

    typedef std::vector<LocalizationToPlannerData> LocalizationToPlannerFrame;
    typedef std::vector<PlannerToControlData> PlannerToControlFrame;
    typedef std::vector<LocalizationToCollisionData> LocalizationToCollisionFrame;
    typedef std::vector<CollisionToPlannerData> CollisionToPlannerFrame;

    /// Messenger types

    typedef Messenger<std::shared_ptr<LocalizationToPlannerFrame>> LocalizationToPlannerMessenger;
    typedef Messenger<std::shared_ptr<PlannerToControlFrame>> PlannerToControlMessenger;
    typedef Messenger<std::shared_ptr<LocalizationToCollisionFrame>> LocalizationToCollisionMessenger;
    typedef Messenger<std::shared_ptr<CollisionToPlannerFrame>> CollisionToPlannerMessenger;
}