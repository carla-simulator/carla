#define _GLIBCXX_USE_CXX11_ABI 0

#include "ImageListener.h"
#include <iostream>

#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>

namespace carla {
namespace ros2 {

    class CarlaImageListenerImpl : public eprosima::fastdds::dds::DataWriterListener
    {
    public:
        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;
                    

        int m_Matched {0};
        bool m_FirstConnected {false};
    };

    void CarlaImageListenerImpl::on_publication_matched(eprosima::fastdds::dds::DataWriter* writer, const eprosima::fastdds::dds::PublicationMatchedStatus& info)
    {
        if (info.current_count_change == 1)
        {
            m_Matched = info.total_count;
            m_FirstConnected = true;
            std::cout << "Publisher matched." << std::endl;
        }
        else if (info.current_count_change == -1)
        {
            m_Matched = info.total_count;
            std::cout << "Publisher unmatched." << std::endl;
        }
        else
        {
            std::cerr << info.current_count_change
                    << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
        }
    }

    CarlaImageListener::CarlaImageListener() : m_Impl(new CarlaImageListenerImpl())
    {

    }

    CarlaImageListener::~CarlaImageListener()
    {
        delete m_Impl;
    }

}}
