// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/impl/DdsDomainParticipantImpl.h"

#include <cstdlib>
#include <string>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include "carla/Logging.h"

namespace carla {
namespace ros2 {

DdsDomainParticipantImpl::DdsDomainParticipantImpl() {
  _factory = eprosima::fastdds::dds::DomainParticipantFactory::get_shared_instance();
  if (_factory == nullptr) {
    carla::log_error("DdsDomainParticipantImpl(): Failed to acquire DomainParticipantFactory");
    return;
  }

  const char *ros_domain_id_env = std::getenv("ROS_DOMAIN_ID");
  unsigned int ros_domain_id = 0;
  if ( ros_domain_id_env != nullptr ) {
    try {
      ros_domain_id = (unsigned int)(std::atoi(ros_domain_id_env));
    } catch (...) {
      ros_domain_id = 0;
    }
  }
  auto pqos = eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT;
  pqos.name("carla-server");
  _participant = _factory->create_participant(ros_domain_id, pqos);
  if (_participant == nullptr) {
    carla::log_error("DdsDomainParticipantImpl(): Failed to create DomainParticipant");
  }
}

DdsDomainParticipantImpl::~DdsDomainParticipantImpl() {
  carla::log_warning("DdsDomainParticipantImpl::Destructor()");
  if ((_participant != nullptr) && (_factory != nullptr)) {
    _factory->delete_participant(_participant);
    _participant=nullptr;
  }
}

}  // namespace ros2
}  // namespace carla
