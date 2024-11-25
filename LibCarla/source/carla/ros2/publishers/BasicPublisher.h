// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

  struct BasicPublisherImpl;

  class BasicPublisher : public CarlaPublisher {
    public:
      BasicPublisher(const char* ros_name = "", const char* parent = "");
      ~BasicPublisher();
      BasicPublisher(const BasicPublisher&);
      BasicPublisher& operator=(const BasicPublisher&);
      BasicPublisher(BasicPublisher&&);
      BasicPublisher& operator=(BasicPublisher&&);

      bool Init();
      bool Publish();
      void SetData(const char* msg);
      const char* type() const override { return "basic_publisher"; }

    private:
      std::shared_ptr<BasicPublisherImpl> _impl;
  };
}
}
