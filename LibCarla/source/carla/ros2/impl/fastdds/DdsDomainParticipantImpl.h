
// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fastdds/dds/domain/DomainParticipant.hpp>

namespace carla {
namespace ros2 {

class DdsDomainParticipantImpl {
public:
  DdsDomainParticipantImpl();
  ~DdsDomainParticipantImpl();

  eprosima::fastdds::dds::DomainParticipant* GetDomainParticipant() {
    return _participant;
  }

private:
  eprosima::fastdds::dds::DomainParticipant* _participant{nullptr};
};

}  // namespace ros2
}  // namespace carla