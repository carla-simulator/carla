// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/publishers/CarlaPublisher.h"

namespace carla {
namespace ros2 {

  class ROSUTILS_API TopicPublisherBase : public CarlaPublisher {
    public:
      virtual bool Init(std::string topic_name) {return false;}
  };

  template<class PubSubTypes, class MessageType>
  struct TopicPublisherImpl;

  template<class PubSubTypes, class MessageType>
  class TopicPublisher : public TopicPublisherBase {
    public:
      TopicPublisher(const char* ros_name = "", const char* parent = "");
      ~TopicPublisher();
      TopicPublisher(const TopicPublisher&);
      TopicPublisher& operator=(const TopicPublisher&);
      TopicPublisher(TopicPublisher&&);
      TopicPublisher& operator=(TopicPublisher&&);

      virtual bool Init(std::string topic_name) override;
      virtual const char* type() const override { return "template"; }

      void MakeHeader(MessageType& Message, int32_t seconds, uint32_t nanoseconds);
      bool Publish(MessageType Message);
      

    private:
      std::shared_ptr<TopicPublisherImpl<PubSubTypes, MessageType>> _impl;
  };
}
}

#include "TopicPublisher.tpp"
