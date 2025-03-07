// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/subscribers/CarlaSubscriber.h"

namespace carla {
namespace ros2 {
  template <class PubSubType, class MessageType>
  struct TopicSubscriptionImpl;

  template<class MessageType>
  using MessageCallback = std::function<void(MessageType data)>;

  class ROSUTILS_API TopicSubscriptionBase : public CarlaSubscriber {
    public:
      virtual bool HasNewMessage() {return false;}
      virtual bool IsAlive() {return false;}
      virtual void Spin() {}
      virtual void* GetOwner() {return nullptr;}
  };

  template <class PubSubType, class MessageType>
  class ROSUTILS_API TopicSubscription : public TopicSubscriptionBase {
    public:
    TopicSubscription(void* owning_actor, const char* ros_name = "", const char* parent = "");
      ~TopicSubscription();
      TopicSubscription(const TopicSubscription&);
      TopicSubscription& operator=(const TopicSubscription&);
      TopicSubscription(TopicSubscription&&);
      TopicSubscription& operator=(TopicSubscription&&);

      virtual bool HasNewMessage();
      virtual bool IsAlive();
      virtual void Spin();
      virtual void* GetOwner();
      MessageType GetMessage();
      
      bool Subscribe(const std::string topic_name, MessageCallback<MessageType> callback);
      bool Read();
      const char* type() const override { return typeid(MessageType).name(); }

      //Do not call, for internal use only
      void ForwardMessage(MessageType message);
      void DestroySubscriber();

    private:
      std::shared_ptr<TopicSubscriptionImpl<PubSubType, MessageType>> _impl;
      std::optional<MessageCallback<MessageType>> callback;
  };
}
}

#include "TopicSubscription.tpp"
  