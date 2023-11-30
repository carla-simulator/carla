// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CarlaListener.h"

#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>

namespace carla {
namespace ros2 {

  namespace efd = eprosima::fastdds::dds;

    class CarlaListenerImpl : public efd::DataWriterListener {
      public:
      void on_publication_matched(
              efd::DataWriter* writer,
              const efd::PublicationMatchedStatus& info) override;


      int _matched {0};
      bool _first_connected {false};
    };

}}
