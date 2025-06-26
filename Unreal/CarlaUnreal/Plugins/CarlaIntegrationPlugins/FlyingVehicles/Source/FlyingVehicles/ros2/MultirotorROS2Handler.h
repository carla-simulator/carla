// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/disable-ue4-macros.h>
#include "carla/ros2/ROS2.h"
#include <util/enable-ue4-macros.h>

/// visitor class
class MultirotorROS2Handler
{
    public:
        MultirotorROS2Handler() = delete;
        MultirotorROS2Handler(AActor *Actor, std::string RosName) : _Actor(Actor), _RosName(RosName) {};

        void operator()(carla::ros2::MultirotorControl &Source);

    private:
        AActor *_Actor {nullptr};
        std::string _RosName;
};
