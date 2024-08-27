
// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

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
  // keep also a copy of the factory that the underlying DDS is keeping their stuff up
  std::shared_ptr<eprosima::fastdds::dds::DomainParticipantFactory> _factory;
};

}  // namespace ros2
}  // namespace carla