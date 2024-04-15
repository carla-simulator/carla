// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <compiler/disable-ue4-macros.h>
#if defined(WITH_ROS2)
#  include <carla/ros2/ROS2.h>
#endif
#include <compiler/enable-ue4-macros.h>

/// visitor class
class ActorROS2Handler
{
    public:
        ActorROS2Handler() = delete;
        ActorROS2Handler(AActor *Actor, std::string RosName) : _Actor(Actor), _RosName(RosName) {};

#if defined(WITH_ROS2)
        void operator()(carla::ros2::VehicleControl &Source);
#endif

    private:
        AActor *_Actor {nullptr};
        std::string _RosName;
};
