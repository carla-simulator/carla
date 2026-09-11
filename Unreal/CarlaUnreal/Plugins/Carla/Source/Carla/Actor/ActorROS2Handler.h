// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/disable-ue4-macros.h>
#include "carla/ros2/ROS2.h"
#include <util/enable-ue4-macros.h>

/// visitor class
class ActorROS2Handler
{
    public:
        ActorROS2Handler() = delete;
        ActorROS2Handler(AActor *Actor, std::string RosName) : _Actor(Actor), _RosName(RosName) {};

        void operator()(carla::ros2::VehicleControl &Source);
        void operator()(carla::ros2::AckermannControl &Source);
        void operator()(carla::ros2::VehicleAccelerationControl &Source);
        void operator()(carla::ros2::MessageControl Message);

        /// Flatten the vehicle's steering curve to 1.0 at all speeds so the
        /// Autoware steering command maps 1:1 to wheel angle (tier4 port).
        static bool FlattenSteeringCurve(AActor *Actor);

    private:
        AActor *_Actor {nullptr};
        std::string _RosName;
};
