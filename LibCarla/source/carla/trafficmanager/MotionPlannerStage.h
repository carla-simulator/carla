// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <chrono>
#include <memory>
#include <unordered_map>
#include <vector>

#include "carla/client/Vehicle.h"
#include "carla/geom/Math.h"
#include "carla/rpc/Actor.h"

#include "carla/trafficmanager/MessengerAndDataTypes.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/PIDController.h"
#include "carla/trafficmanager/PipelineStage.h"
#include "carla/trafficmanager/SimpleWaypoint.h"

namespace carla {
namespace traffic_manager {

  namespace chr = std::chrono;
  namespace cc = carla::client;

  using Actor = carla::SharedPtr<cc::Actor>;
  using ActorId = carla::rpc::ActorId;
  using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;

  /// The class is responsible for aggregating information from various stages
  /// like the localization stage, traffic light stage, collision detection
  /// stage and actuation signals from the PID controller and makes decisions
  /// on how to move the vehicle to follow it's trajectory safely.
  class MotionPlannerStage : public PipelineStage {

  private:

    /// Selection key to switch between the output frames.
    bool frame_selector;
    /// Pointers to data frames to be shared with the batch control stage
    std::shared_ptr<PlannerToControlFrame> control_frame_a;
    std::shared_ptr<PlannerToControlFrame> control_frame_b;
    /// Pointers to data frames received from various stages.
    std::shared_ptr<LocalizationToPlannerFrame> localization_frame;
    std::shared_ptr<CollisionToPlannerFrame> collision_frame;
    std::shared_ptr<TrafficLightToPlannerFrame> traffic_light_frame;
    /// Pointers to messenger objects connecting to various stages.
    std::shared_ptr<LocalizationToPlannerMessenger> localization_messenger;
    std::shared_ptr<CollisionToPlannerMessenger> collision_messenger;
    std::shared_ptr<TrafficLightToPlannerMessenger> traffic_light_messenger;
    std::shared_ptr<PlannerToControlMessenger> control_messenger;
    /// Map to store states for integral and differential components
    /// of the PID controller for every vehicle
    std::unordered_map<ActorId, StateEntry> pid_state_map;
    /// Run time parameterization object.
    Parameters &parameters;
    /// Configuration parameters for the PID controller.
    std::vector<float> urban_longitudinal_parameters;
    std::vector<float> highway_longitudinal_parameters;
    std::vector<float> urban_lateral_parameters;
    std::vector<float> highway_lateral_parameters;
    std::vector<float> longitudinal_parameters;
    std::vector<float> lateral_parameters;
    /// Controller object.
    PIDController controller;
    /// Number of vehicles registered with the traffic manager.
    uint64_t number_of_vehicles;
    /// Reference to Carla's debug helper object.
    cc::DebugHelper &debug_helper;
    /// Switch indicating hybrid physics mode.
    bool hybrid_physics_mode {false};
    /// Teleportation duration clock;
    std::unordered_map<ActorId, TimePoint> teleportation_instance;

  public:

    MotionPlannerStage(
        std::string stage_name,
        std::shared_ptr<LocalizationToPlannerMessenger> localization_messenger,
        std::shared_ptr<CollisionToPlannerMessenger> collision_messenger,
        std::shared_ptr<TrafficLightToPlannerMessenger> traffic_light_messenger,
        std::shared_ptr<PlannerToControlMessenger> control_messenger,
        Parameters &parameters,
        std::vector<float> longitudinal_parameters,
        std::vector<float> highway_longitudinal_parameters,
        std::vector<float> lateral_parameters,
        std::vector<float> highway_lateral_parameters,
        cc::DebugHelper &debug_helper);

    ~MotionPlannerStage();

    void DataReceiver() override;

    void Action() override;

    void DataSender() override;

    void DrawPIDValues(const boost::shared_ptr<cc::Vehicle> vehicle, const float throttle, const float brake);

  };

} // namespace traffic_manager
} // namespace carla
