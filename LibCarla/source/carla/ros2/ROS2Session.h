// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/detail/Session.h"

namespace carla {
namespace ros2 {

/// A ROS2 streaming session to be able to (re-)use the standard server tcp buffers to receive the data
// to be published via ROS2
class ROS2Session : public carla::streaming::detail::Session {
public:
  ROS2Session(carla::streaming::detail::stream_id_type stream_id) : carla::streaming::detail::Session(stream_id) {}

  /// Writes a message to the ROS2 publisher.
  void WriteMessage(std::shared_ptr<const carla::streaming::detail::Message> message) override;

  /// Post a job to close the session.
  virtual void Close() override {
    // ROS2 session is closed in case there are no subscribers anymore
    // this is handled directly within ROS2 class
    // nothing to be done here
  }

  void EnableForROS() override;
  void DisableForROS()override;  
  bool IsEnabledForROS() override;
};

}  // namespace ros2
}  // namespace carla
