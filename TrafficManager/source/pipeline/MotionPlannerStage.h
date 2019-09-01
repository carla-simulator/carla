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

    int localization_messenger_state;
    int control_messenger_state;
    std::shared_ptr<PlannerToControlFrame> control_frame_a;
    std::shared_ptr<PlannerToControlFrame> control_frame_b;
    bool frame_selector;
    std::unordered_map<bool, std::shared_ptr<PlannerToControlFrame>> frame_map;
    std::shared_ptr<LocalizationToPlannerFrame> localization_frame;
    std::shared_ptr<LocalizationToPlannerMessenger> localization_messenger;
    std::shared_ptr<PlannerToControlMessenger> control_messenger;

    std::unordered_map<int, StateEntry> pid_state_map;
    std::vector<float> longitudinal_parameters;
    std::vector<float> highway_longitudinal_parameters;
    std::vector<float> lateral_parameters;
    float urban_target_velocity;
    float highway_target_velocity;
    PIDController controller;

  public:

    MotionPlannerStage(
        std::shared_ptr<LocalizationToPlannerMessenger> localization_messenger,
        std::shared_ptr<PlannerToControlMessenger> control_messenger,
        int number_of_vehicles,
        float urban_target_velocity,
        float highway_target_velocity,
        int pool_size,
        std::vector<float> longitudinal_parameters,
        std::vector<float> highway_longitudinal_parameters,
        std::vector<float> lateral_parameters
      );
    ~MotionPlannerStage();

    void DataReceiver() override;
    void Action(int array_index) override;
    void DataSender() override;

    using PipelineStage::Start;

  };

}
