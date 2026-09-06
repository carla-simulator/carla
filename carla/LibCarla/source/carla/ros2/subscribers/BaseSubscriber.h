// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

#include "carla/ros2/ROS2CallbackData.h"

namespace carla {
namespace ros2 {

class BaseSubscriber {
public:
  BaseSubscriber() = default;

  explicit BaseSubscriber(std::string base_topic_name)
    : _base_topic_name(std::move(base_topic_name)) {}

  BaseSubscriber(std::string base_topic_name, std::string frame_id)
    : _base_topic_name(std::move(base_topic_name)),
      _frame_id(std::move(frame_id)) {}

  BaseSubscriber(void *actor, std::string base_topic_name, std::string frame_id)
    : _actor(actor),
      _base_topic_name(std::move(base_topic_name)),
      _frame_id(std::move(frame_id)) {}

  virtual ~BaseSubscriber() = default;

  // Polymorphic base held only via std::shared_ptr<BaseSubscriber>; copying or
  // moving one would slice the derived subscriber, so disable it outright.
  BaseSubscriber(const BaseSubscriber &) = delete;
  BaseSubscriber &operator=(const BaseSubscriber &) = delete;
  BaseSubscriber(BaseSubscriber &&) = delete;
  BaseSubscriber &operator=(BaseSubscriber &&) = delete;

  virtual void ProcessMessages(ActorCallback callback) = 0;

  [[nodiscard]] void *GetActor() const noexcept { return _actor; }
  [[nodiscard]] const std::string &GetBaseTopicName() const noexcept { return _base_topic_name; }
  [[nodiscard]] const std::string &GetFrameId() const noexcept { return _frame_id; }

protected:
  // Decoding the latest sample yields stale data unless gated behind the impl's
  // HasNewMessage(); kept protected so only ProcessMessages can reach it.
  virtual ROS2CallbackData GetMessage() = 0;

  void *_actor{nullptr};
  std::string _base_topic_name;
  std::string _frame_id;
};

}  // namespace ros2
}  // namespace carla
