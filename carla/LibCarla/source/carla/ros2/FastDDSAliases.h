// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastrtps/types/TypesBase.h>

namespace carla {
namespace ros2 {

// Shared FastDDS spelling shortcuts used by PublisherImpl/SubscriberImpl. Defined
// once here so the two impl headers do not each re-declare them at namespace scope.
namespace efd = eprosima::fastdds::dds;
using erc = eprosima::fastrtps::types::ReturnCode_t;

}  // namespace ros2
}  // namespace carla
