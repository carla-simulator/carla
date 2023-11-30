// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaListener.h"
#include "CarlaListenerImpl.h"
#include <iostream>

#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>

namespace carla {
namespace ros2 {

  namespace efd = eprosima::fastdds::dds;

    void CarlaListenerImpl::on_publication_matched(efd::DataWriter* writer, const efd::PublicationMatchedStatus& info)
    {
      if (info.current_count_change == 1) {
          _matched = info.total_count;
          _first_connected = true;
      } else if (info.current_count_change == -1) {
          _matched = info.total_count;
      } else {
          std::cerr << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
      }
    }

    CarlaListener::CarlaListener() :
    _impl(std::make_unique<CarlaListenerImpl>()) { }

    CarlaListener::~CarlaListener() {}

}}
