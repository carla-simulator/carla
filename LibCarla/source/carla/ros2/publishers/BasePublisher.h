// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

namespace carla {
namespace ros2 {

class BasePublisher {
public:
  BasePublisher() = default;

  explicit BasePublisher(std::string base_topic_name)
    : _base_topic_name(std::move(base_topic_name)) {}

  BasePublisher(std::string base_topic_name, std::string frame_id)
    : _base_topic_name(std::move(base_topic_name)),
      _frame_id(std::move(frame_id)) {}

  BasePublisher(void *actor, std::string base_topic_name, std::string frame_id)
    : _actor(actor),
      _base_topic_name(std::move(base_topic_name)),
      _frame_id(std::move(frame_id)) {}

  virtual ~BasePublisher() = default;

  // Polymorphic base held only via std::shared_ptr<BasePublisher>; copying or
  // moving one would slice the derived publisher, so disable it outright.
  BasePublisher(const BasePublisher &) = delete;
  BasePublisher &operator=(const BasePublisher &) = delete;
  BasePublisher(BasePublisher &&) = delete;
  BasePublisher &operator=(BasePublisher &&) = delete;

  virtual bool Publish() = 0;

  [[nodiscard]] void *GetActor() const noexcept { return _actor; }
  [[nodiscard]] const std::string &GetBaseTopicName() const noexcept { return _base_topic_name; }
  [[nodiscard]] const std::string &GetFrameId() const noexcept { return _frame_id; }

protected:
  void *_actor{nullptr};
  std::string _base_topic_name;
  std::string _frame_id;
};

}  // namespace ros2
}  // namespace carla
