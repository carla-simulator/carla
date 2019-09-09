#pragma once

#include <chrono>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <memory>

#include "carla/client/Vehicle.h"

#include "PipelineStage.h"
#include "PIDController.h"
#include "MessengerAndDataTypes.h"

namespace traffic_manager {

  class MotionPlannerStage : public PipelineStage {

    /// The class is responsible for aggregating information from various stages
    /// like traffic like state stage, collision detection stage and actuation
    /// signals from PID controller and makes decisions on how to move the vehicle to
    /// follow it's trajectory safely.

  private:

    bool frame_selector;
    int localization_messenger_state;
    int control_messenger_state;
    int collision_messenger_state;
    int traffic_light_messenger_state;
    std::shared_ptr<PlannerToControlFrame> control_frame_a;
    std::shared_ptr<PlannerToControlFrame> control_frame_b;
    std::unordered_map<bool, std::shared_ptr<PlannerToControlFrame>> frame_map;
    std::shared_ptr<LocalizationToPlannerFrame> localization_frame;
    std::shared_ptr<CollisionToPlannerFrame> collision_frame;
    std::shared_ptr<TrafficLightToPlannerFrame> traffic_light_frame;
    std::shared_ptr<LocalizationToPlannerMessenger> localization_messenger;
    std::shared_ptr<PlannerToControlMessenger> control_messenger;
    std::shared_ptr<CollisionToPlannerMessenger> collision_messenger;
    std::shared_ptr<TrafficLightToPlannerMessenger> traffic_light_messenger;

    std::shared_ptr<std::vector<StateEntry>> pid_state_vector;
    std::vector<float> longitudinal_parameters;
    std::vector<float> highway_longitudinal_parameters;
    std::vector<float> lateral_parameters;
    float urban_target_velocity;
    float highway_target_velocity;
    PIDController controller;

  public:

    MotionPlannerStage(
        std::shared_ptr<LocalizationToPlannerMessenger> localization_messenger,
        std::shared_ptr<CollisionToPlannerMessenger> collision_messenger,
        std::shared_ptr<TrafficLightToPlannerMessenger> traffic_light_messenger,
        std::shared_ptr<PlannerToControlMessenger> control_messenger,
        int number_of_vehicles,
        int pool_size,
        float urban_target_velocity,
        float highway_target_velocity,
        std::vector<float> longitudinal_parameters,
        std::vector<float> highway_longitudinal_parameters,
        std::vector<float> lateral_parameters
      );
    ~MotionPlannerStage();

    void DataReceiver() override;
    void Action(int start_index, int end_index) override;
    void DataSender() override;

    using PipelineStage::Start;
    using PipelineStage::Stop;

  };

}
