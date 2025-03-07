// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

namespace carla {
namespace ros2 {

  template<class PubSubTypes, class MessageType>
  class SubscriberListenerImpl;

  template<class PubSubTypes, class MessageType>
  class TopicSubscription;

  template<class PubSubTypes, class MessageType>
  class ROSUTILS_API SubscriberListener {
    public:
    SubscriberListener(TopicSubscription<PubSubTypes, MessageType>* owner);
      ~SubscriberListener();
      SubscriberListener(const SubscriberListener&) = delete;
      SubscriberListener& operator=(const SubscriberListener&) = delete;
      SubscriberListener(SubscriberListener&&) = delete;
      SubscriberListener& operator=(SubscriberListener&&) = delete;

      void SetOwner(TopicSubscription<PubSubTypes, MessageType>* owner);

      std::unique_ptr<SubscriberListenerImpl<PubSubTypes, MessageType>> _impl;
  };
}
}

#include "SubscriberListener.tpp"