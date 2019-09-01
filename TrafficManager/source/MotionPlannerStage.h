#pragma once

#include <chrono>
#include <cmath>
#include <vector>
#include <unordered_map>

#include "carla/client/Vehicle.h"

#include "PipelineStage.h"
#include "PIDController.h"
#include "MessengerAndDataTypes.h"

namespace traffic_manager {

  typedef std::chrono::time_point<
      std::chrono::_V2::system_clock,
      std::chrono::nanoseconds
      > TimeInstance;

  struct StateEntry {
    float deviation;
    float velocity;
    TimeInstance time_instance;
    float deviation_integral;
    float velocity_integral;
  };

  class MotionPlannerStage : public PipelineStage {

    /// The class is responsible for aggregating information from various stages
    /// like traffic like state stage, collision detection stage and actuation
    /// signals from PID controller and makes decisions on how to move the vehicle to
    /// follow it's trajectory safely.

  private:

    int localization_messenger_state;
    int control_messenger_state;
    std::shared_ptr<LocalizationToPlannerMessenger> localization_messenger;
    std::shared_ptr<PlannerToControlMessenger> control_messenger;
    PlannerToControlFrame control_frame_a;
    // PlannerToControlFrame control_frame_b;
    LocalizationToPlannerFrame* localization_frame;

    std::unordered_map<int, StateEntry> pid_state_map;
    std::vector<float> longitudinal_parameters;
    std::vector<float> highway_longitudinal_parameters;
    std::vector<float> lateral_parameters;
    float urban_target_velocity;
    float highway_target_velocity;
    PIDController controller;

  public:

    MotionPlannerStage(
        float urban_target_velocity,
        float highway_target_velocity,
        std::vector<float> longitudinal_parameters,
        std::vector<float> highway_longitudinal_parameters,
        std::vector<float> lateral_parameters,
        std::shared_ptr<LocalizationToPlannerMessenger> localization_messenger,
        std::shared_ptr<PlannerToControlMessenger> control_messenger,
        int pool_size,
        int number_of_vehicles
      );
    ~MotionPlannerStage();

    void DataReceiver() override;
    void Action(int thread_id) override;
    void DataSender() override;

    using PipelineStage::Start;

  };

}
