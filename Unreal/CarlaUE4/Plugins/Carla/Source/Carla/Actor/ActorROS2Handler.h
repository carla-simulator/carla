// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <compiler/disable-ue4-macros.h>
#include "carla/ros2/ROS2.h"
#include <compiler/enable-ue4-macros.h>

/// visitor class
class ActorROS2Handler
{
  public:
    ActorROS2Handler() = delete;
    ActorROS2Handler(AActor *Actor) : _Actor(Actor) {};

    // =========================================================================
    // -- Control interface ----------------------------------------------------
    // =========================================================================
    void operator()(carla::ros2::VehicleControl &Source);
    void operator()(carla::ros2::AckermannControl &Source);

    // =========================================================================
    // -- Pseudo publishers ----------------------------------------------------
    // =========================================================================
    void operator()(carla::ros2::VehicleOdometryReport &Target);
    void operator()(carla::ros2::VehicleChassisReport &Target);

  private:
    AActor *_Actor {nullptr};
};
