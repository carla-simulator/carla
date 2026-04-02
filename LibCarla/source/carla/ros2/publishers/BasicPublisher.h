// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <string>

namespace carla {
namespace ros2 {

  struct BasicPublisherImpl;

  class BasicPublisher {
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
      const char* type() const { return "basic_publisher"; }

      const std::string& name() const { return _name; }
      const std::string& parent() const { return _parent; }

    protected:
      std::string _frame_id = "";
      std::string _name = "";
      std::string _parent = "";

    private:
      std::shared_ptr<BasicPublisherImpl> _impl;
  };
}
}
