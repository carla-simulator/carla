// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/subscribers/CarlaSubscriber.h"
#include "CameraController/ros2/ROS2CameraControlCallbackData.h"

namespace carla {
namespace ros2 {

  struct CarlaCameraControlSubscriberImpl;

  class CarlaCameraControlSubscriber : public CarlaSubscriber {
    public:
      CarlaCameraControlSubscriber(void* camera_control, const char* ros_name = "", const char* parent = "");
      ~CarlaCameraControlSubscriber();
      CarlaCameraControlSubscriber(const CarlaCameraControlSubscriber&);
      CarlaCameraControlSubscriber& operator=(const CarlaCameraControlSubscriber&);
      CarlaCameraControlSubscriber(CarlaCameraControlSubscriber&&);
      CarlaCameraControlSubscriber& operator=(CarlaCameraControlSubscriber&&);

      bool HasNewMessage();
      bool IsAlive();
      CameraControl GetMessage();
      void* GetCameraControl();

      bool Init();
      bool Read();
      const char* type() const override { return "Camera Control "; }

      //Do not call, for internal use only
      void ForwardMessage(CameraControl control);
      void DestroySubscriber();

    private:
      std::shared_ptr<CarlaCameraControlSubscriberImpl> _impl;
  };
}
}
