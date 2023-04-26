#define _GLIBCXX_USE_CXX11_ABI 0

#include "ImagePublisher.h"

#include "types/ImagePubSubTypes.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include <string>
#include <chrono>



namespace carla {
namespace ros2 {

struct CarlaImagePublisherImpl
{
    eprosima::fastdds::dds::DomainParticipant* m_Participant { nullptr };
    eprosima::fastdds::dds::Publisher* m_Publisher { nullptr };
    eprosima::fastdds::dds::Topic* m_Topic { nullptr };
    eprosima::fastdds::dds::DataWriter* m_DataWriter { nullptr };
    eprosima::fastdds::dds::TypeSupport m_Type { nullptr };
    sensor_msgs::msg::Image m_Image {};
};


bool CarlaImagePublisher::Init(const char* topic)
{
    if (m_Impl->m_Type == nullptr)
        return false;

    eprosima::fastdds::dds::DomainParticipantQos pqos = eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT;
    pqos.name("CarlaPublisherParticipant");
    auto factory = eprosima::fastdds::dds::DomainParticipantFactory::get_instance();
    m_Impl->m_Participant = factory->create_participant(0, pqos);
    if (m_Impl->m_Participant == nullptr)
        return false;

    m_Impl->m_Type.register_type(m_Impl->m_Participant);
    eprosima::fastdds::dds::PublisherQos pubqos = eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT;
    m_Impl->m_Publisher = m_Impl->m_Participant->create_publisher(pubqos, nullptr);
    if (m_Impl->m_Publisher == nullptr)
        return false;

    eprosima::fastdds::dds::TopicQos tqos = eprosima::fastdds::dds::TOPIC_QOS_DEFAULT;
    std::string TopicName {"rt/"};
    TopicName += topic;
    m_Impl->m_Topic = m_Impl->m_Participant->create_topic(TopicName, m_Impl->m_Type->getName(), tqos);
    if (m_Impl->m_Topic == nullptr)
        return false;

    eprosima::fastrtps::ReliabilityQosPolicy rqos;
    rqos.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
    eprosima::fastdds::dds::DataWriterQos wqos = eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    m_Impl->m_DataWriter = m_Impl->m_Publisher->create_datawriter(m_Impl->m_Topic, wqos, nullptr);
    if (m_Impl->m_DataWriter == nullptr)
        return false;

    return true;
}

bool CarlaImagePublisher::Publish()
{
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastdds::dds::TypeSupport::ReturnCode_t rcode = m_Impl->m_DataWriter->write(&m_Impl->m_Image, instance_handle);
    if (rcode == eprosima::fastdds::dds::TypeSupport::ReturnCode_t::RETCODE_OK)
        return true;
    return false;
}

void CarlaImagePublisher::SetImage(uint32_t height, uint32_t width, const std::vector<uint8_t>& data)
{
    auto epoch = std::chrono::system_clock::now().time_since_epoch();
    auto secons = std::chrono::duration_cast<std::chrono::seconds>(epoch);
    epoch -= secons;
    auto secs = static_cast<int32_t>(secons.count());
    auto nanoseconds = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count());

    builtin_interfaces::msg::Time time;
    time.sec(secs);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(time);
    header.frame_id("1");

    sensor_msgs::msg::Image image;
    image.header(header);
    image.width(width);
    image.height(height);
    image.encoding("bgr8"); //taken from the list of strings in include/sensor_msgs/image_encodings.h
    image.is_bigendian(0);
    image.step(width * sizeof(uint8_t) * 3);
    image.data(data); //https://github.com/eProsima/Fast-DDS/issues/2330
    m_Impl->m_Image = image;
}

CarlaImagePublisher::CarlaImagePublisher() :
m_Impl(std::make_unique<CarlaImagePublisherImpl>())
{
}

CarlaImagePublisher::~CarlaImagePublisher()
{
    if (!m_Impl)
        return;

    if (m_Impl->m_DataWriter)
        m_Impl->m_Publisher->delete_datawriter(m_Impl->m_DataWriter);

    if (m_Impl->m_Publisher)
        m_Impl->m_Participant->delete_publisher(m_Impl->m_Publisher);

    if (m_Impl->m_Topic)
        m_Impl->m_Participant->delete_topic(m_Impl->m_Topic);
    
    if (m_Impl->m_Participant)
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(m_Impl->m_Participant);
}

}
}