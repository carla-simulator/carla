#pragma once

#include <memory>
#include <vector>

#include "carla/client/Actor.h"
#include "carla/Memory.h"

#include "Messenger.h"
#include "SimpleWaypoint.h"

namespace traffic_manager {

  /// Convenience typing

  using Buffer = std::deque<std::shared_ptr<SimpleWaypoint>>;
  using BufferList = std::vector<Buffer>;

  /// Data types

  struct LocalizationToPlannerData {
    carla::SharedPtr<carla::client::Actor> actor;
    float deviation;
    bool approaching_true_junction;
  };

  struct PlannerToControlData {
    int actor_id;
    float throttle;
    float brake;
    float steer;
  };

  struct LocalizationToCollisionData {
    carla::SharedPtr<carla::client::Actor> actor;
    Buffer *buffer;
  };

  struct CollisionToPlannerData {
    bool hazard;
  };

  struct LocalizationToTrafficLightData {
    carla::SharedPtr<carla::client::Actor> actor;
    std::shared_ptr<SimpleWaypoint> closest_waypoint;
    std::shared_ptr<SimpleWaypoint> junction_look_ahead_waypoint;
  };

  struct TrafficLightToPlannerData {
    float traffic_light_hazard;
  };

  /// Data frame types

  using LocalizationToPlannerFrame = std::vector<LocalizationToPlannerData>;
  using PlannerToControlFrame = std::vector<PlannerToControlData>;
  using LocalizationToCollisionFrame = std::vector<LocalizationToCollisionData>;
  using LocalizationToTrafficLightFrame = std::vector<LocalizationToTrafficLightData>;
  using CollisionToPlannerFrame = std::vector<CollisionToPlannerData>;
  using TrafficLightToPlannerFrame = std::vector<TrafficLightToPlannerData>;

  /// Messenger types

  using LocalizationToPlannerMessenger = Messenger<std::shared_ptr<LocalizationToPlannerFrame>>;
  using PlannerToControlMessenger = Messenger<std::shared_ptr<PlannerToControlFrame>>;
  using LocalizationToCollisionMessenger = Messenger<std::shared_ptr<LocalizationToCollisionFrame>>;
  using LocalizationToTrafficLightMessenger = Messenger<std::shared_ptr<LocalizationToTrafficLightFrame>>;
  using CollisionToPlannerMessenger = Messenger<std::shared_ptr<CollisionToPlannerFrame>>;
  using TrafficLightToPlannerMessenger = Messenger<std::shared_ptr<TrafficLightToPlannerFrame>>;
}
