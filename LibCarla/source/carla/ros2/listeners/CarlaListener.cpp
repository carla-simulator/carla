#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaListener.h"
#include <iostream>

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
