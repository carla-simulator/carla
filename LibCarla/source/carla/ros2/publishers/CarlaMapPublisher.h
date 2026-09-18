// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/BasePublisher.h"

#include <memory>
#include <string>

namespace carla {
namespace ros2 {

template <typename Traits> class PublisherImpl;
struct CarlaMapMsgTraits;

/// Publishes the current OpenDRIVE description on rt/carla/map.
///
/// The topic is a transient-local std_msgs/String with one cached sample, so a
/// late subscriber receives the current map without needing the simulator to
/// reload it. std_msgs/String deliberately has no Header: the topic remains
/// wire-compatible with carla-ros-bridge's /carla/map endpoint. Every episode
/// start overwrites the single cached sample with that episode's OpenDRIVE.
class CarlaMapPublisher final : public BasePublisher {
public:
  CarlaMapPublisher();
  ~CarlaMapPublisher() override;

  CarlaMapPublisher(const CarlaMapPublisher &) = delete;
  CarlaMapPublisher &operator=(const CarlaMapPublisher &) = delete;

  bool Publish() override;
  bool Write(const std::string &open_drive);

private:
  std::shared_ptr<PublisherImpl<CarlaMapMsgTraits>> _impl;
  bool _initialized{false};
};

}  // namespace ros2
}  // namespace carla
