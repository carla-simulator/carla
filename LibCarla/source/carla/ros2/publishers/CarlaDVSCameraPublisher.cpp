#include "CarlaDVSCameraPublisher.h"

#include <string>

#include "carla/sensor/data/DVSEvent.h"

#include "carla/ros2/types/ImagePubSubTypes.h"
#include "carla/ros2/types/CameraInfoPubSubTypes.h"
#include "carla/ros2/types/PointCloud2PubSubTypes.h"
#include "carla/ros2/listeners/CarlaListener.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>


namespace carla {
namespace ros2 {

  namespace efd = eprosima::fastdds::dds;
  using erc = eprosima::fastrtps::types::ReturnCode_t;

  struct CarlaDVSCameraPublisherImpl {
    efd::DomainParticipant* _participant { nullptr };
    efd::Publisher* _publisher { nullptr };
    efd::Topic* _topic { nullptr };
    efd::DataWriter* _datawriter { nullptr };
    efd::TypeSupport _type { new sensor_msgs::msg::ImagePubSubType() };
    CarlaListener _listener {};
    sensor_msgs::msg::Image _image {};
  };

  struct CarlaCameraInfoPublisherImpl {
    efd::DomainParticipant* _participant { nullptr };
    efd::Publisher* _publisher { nullptr };
    efd::Topic* _topic { nullptr };
    efd::DataWriter* _datawriter { nullptr };
    efd::TypeSupport _type { new sensor_msgs::msg::CameraInfoPubSubType() };
    CarlaListener _listener {};
    bool _init {false};
    sensor_msgs::msg::CameraInfo _ci {};
  };

  struct CarlaPointCloudPublisherImpl {
    efd::DomainParticipant* _participant { nullptr };
    efd::Publisher* _publisher { nullptr };
    efd::Topic* _topic { nullptr };
    efd::DataWriter* _datawriter { nullptr };
    efd::TypeSupport _type { new sensor_msgs::msg::PointCloud2PubSubType() };
    CarlaListener _listener {};
    sensor_msgs::msg::PointCloud2 _pc {};
  };

  bool CarlaDVSCameraPublisher::HasBeenInitialized() const {
    return _info->_init;
  }

  void CarlaDVSCameraPublisher::InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify) {
    _info->_ci = std::move(sensor_msgs::msg::CameraInfo(height, width, fov));
    SetInfoRegionOfInterest(x_offset, y_offset, height, width, do_rectify);
    _info->_init = true;
  }

  bool CarlaDVSCameraPublisher::Init() {
    return InitImage() && InitInfo() && InitPointCloud();
  }

  bool CarlaDVSCameraPublisher::InitImage() {
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }

    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    _impl->_type.register_type(_impl->_participant);

    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }

    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string publisher_type {"/image"};
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    topic_name += publisher_type;
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }

    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    _frame_id = _name;
    return true;
  }

  bool CarlaDVSCameraPublisher::InitInfo() {
    if (_info->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }

    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    auto factory = efd::DomainParticipantFactory::get_instance();
    _info->_participant = factory->create_participant(0, pqos);
    if (_info->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    _info->_type.register_type(_info->_participant);

    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _info->_publisher = _info->_participant->create_publisher(pubqos, nullptr);
    if (_info->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }

    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string publisher_type {"/camera_info"};
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    topic_name += publisher_type;
    _info->_topic = _info->_participant->create_topic(topic_name, _info->_type->getName(), tqos);
    if (_info->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_info->_listener._impl.get();
    _info->_datawriter = _info->_publisher->create_datawriter(_info->_topic, wqos, listener);
    if (_info->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }

    _frame_id = _name;
    return true;
  }

  bool CarlaDVSCameraPublisher::InitPointCloud() {
    if (_point_cloud->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }

    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    auto factory = efd::DomainParticipantFactory::get_instance();
    _point_cloud->_participant = factory->create_participant(0, pqos);
    if (_point_cloud->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    _point_cloud->_type.register_type(_point_cloud->_participant);

    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _point_cloud->_publisher = _point_cloud->_participant->create_publisher(pubqos, nullptr);
    if (_point_cloud->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }

    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string publisher_type {"/point_cloud"};
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    topic_name += publisher_type;
    _point_cloud->_topic = _point_cloud->_participant->create_topic(topic_name, _point_cloud->_type->getName(), tqos);
    if (_point_cloud->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }

    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_point_cloud->_listener._impl.get();
    _point_cloud->_datawriter = _point_cloud->_publisher->create_datawriter(_point_cloud->_topic, wqos, listener);
    if (_point_cloud->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    _frame_id = _name;
    return true;
  }

  bool CarlaDVSCameraPublisher::Publish() {
    return PublishImage() && PublishInfo() && PublishPointCloud();
  }

  bool CarlaDVSCameraPublisher::PublishImage() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(& _impl->_image, instance_handle);
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        return true;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
        std::cerr << "RETCODE_ERROR" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
        std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_BAD_PARAMETER) {
        std::cerr << "RETCODE_BAD_PARAMETER" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET) {
        std::cerr << "RETCODE_PRECONDITION_NOT_MET" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES) {
        std::cerr << "RETCODE_OUT_OF_RESOURCES" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ENABLED) {
        std::cerr << "RETCODE_NOT_ENABLED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY) {
        std::cerr << "RETCODE_IMMUTABLE_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY) {
        std::cerr << "RETCODE_INCONSISTENT_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ALREADY_DELETED) {
        std::cerr << "RETCODE_ALREADY_DELETED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_TIMEOUT) {
        std::cerr << "RETCODE_TIMEOUT" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NO_DATA) {
        std::cerr << "RETCODE_NO_DATA" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION) {
        std::cerr << "RETCODE_ILLEGAL_OPERATION" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY) {
        std::cerr << "RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl;
        return false;
    }
    std::cerr << "UNKNOWN" << std::endl;
    return false;
  }

  bool CarlaDVSCameraPublisher::PublishInfo() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastrtps::types::ReturnCode_t rcode = _info->_datawriter->write(& _info->_ci, instance_handle);
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        return true;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
        std::cerr << "RETCODE_ERROR" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
        std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_BAD_PARAMETER) {
        std::cerr << "RETCODE_BAD_PARAMETER" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET) {
        std::cerr << "RETCODE_PRECONDITION_NOT_MET" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES) {
        std::cerr << "RETCODE_OUT_OF_RESOURCES" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ENABLED) {
        std::cerr << "RETCODE_NOT_ENABLED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY) {
        std::cerr << "RETCODE_IMMUTABLE_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY) {
        std::cerr << "RETCODE_INCONSISTENT_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ALREADY_DELETED) {
        std::cerr << "RETCODE_ALREADY_DELETED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_TIMEOUT) {
        std::cerr << "RETCODE_TIMEOUT" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NO_DATA) {
        std::cerr << "RETCODE_NO_DATA" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION) {
        std::cerr << "RETCODE_ILLEGAL_OPERATION" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY) {
        std::cerr << "RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl;
        return false;
    }
    std::cerr << "UNKNOWN" << std::endl;
    return false;
  }

  bool CarlaDVSCameraPublisher::PublishPointCloud() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastrtps::types::ReturnCode_t rcode = _point_cloud->_datawriter->write(&_point_cloud->_pc, instance_handle);
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        return true;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
        std::cerr << "RETCODE_ERROR" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
        std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_BAD_PARAMETER) {
        std::cerr << "RETCODE_BAD_PARAMETER" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET) {
        std::cerr << "RETCODE_PRECONDITION_NOT_MET" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES) {
        std::cerr << "RETCODE_OUT_OF_RESOURCES" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ENABLED) {
        std::cerr << "RETCODE_NOT_ENABLED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY) {
        std::cerr << "RETCODE_IMMUTABLE_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY) {
        std::cerr << "RETCODE_INCONSISTENT_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ALREADY_DELETED) {
        std::cerr << "RETCODE_ALREADY_DELETED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_TIMEOUT) {
        std::cerr << "RETCODE_TIMEOUT" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NO_DATA) {
        std::cerr << "RETCODE_NO_DATA" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION) {
        std::cerr << "RETCODE_ILLEGAL_OPERATION" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY) {
        std::cerr << "RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl;
        return false;
    }
    std::cerr << "UNKNOWN" << std::endl;
    return false;
  }

  void CarlaDVSCameraPublisher::SetImageData(int32_t seconds, uint32_t nanoseconds, size_t elements, size_t height, size_t width, const uint8_t* data) {
    std::vector<uint8_t> im_data;
    const size_t im_size = width * height * 3;
    im_data.resize(im_size);
    carla::sensor::data::DVSEvent* vec_event = (carla::sensor::data::DVSEvent*)&data[0];
    for (size_t i = 0; i < elements; ++i, ++vec_event) {
        size_t index = (vec_event->y * width + vec_event->x) * 3 + (static_cast<int>(vec_event->pol) * 2);
        im_data[index] = 255;
    }

    SetData(seconds, nanoseconds, height, width, std::move(im_data));
  }

  void CarlaDVSCameraPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data) {
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);
    _impl->_image.header(header);
    _info->_ci.header(header);
    _point_cloud->_pc.header(header);

    _impl->_image.width(width);
    _impl->_image.height(height);
    _impl->_image.encoding("bgr8"); //taken from the list of strings in include/sensor_msgs/image_encodings.h
    _impl->_image.is_bigendian(0);
    _impl->_image.step(_impl->_image.width() * sizeof(uint8_t) * 3);
    _impl->_image.data(std::move(data)); //https://github.com/eProsima/Fast-DDS/issues/2330
  }

  void CarlaDVSCameraPublisher::SetCameraInfoData(int32_t seconds, uint32_t nanoseconds) {
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);
  }

  void CarlaDVSCameraPublisher::SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify) {
    sensor_msgs::msg::RegionOfInterest roi;
    roi.x_offset(x_offset);
    roi.y_offset(y_offset);
    roi.height(height);
    roi.width(width);
    roi.do_rectify(do_rectify);
    _info->_ci.roi(roi);
  }

  void CarlaDVSCameraPublisher::SetPointCloudData(size_t height, size_t width, size_t elements, const uint8_t* data) {

    std::vector<uint8_t> vector_data;
    const size_t size = height * width;
    vector_data.resize(size);
    std::memcpy(&vector_data[0], &data[0], size);

    sensor_msgs::msg::PointField descriptor1;
    descriptor1.name("x");
    descriptor1.offset(0);
    descriptor1.datatype(sensor_msgs::msg::PointField__UINT16);
    descriptor1.count(1);
    sensor_msgs::msg::PointField descriptor2;
    descriptor2.name("y");
    descriptor2.offset(2);
    descriptor2.datatype(sensor_msgs::msg::PointField__UINT16);
    descriptor2.count(1);
    sensor_msgs::msg::PointField descriptor3;
    descriptor3.name("t");
    descriptor3.offset(4);
    descriptor3.datatype(sensor_msgs::msg::PointField__FLOAT64);
    descriptor3.count(1);
    sensor_msgs::msg::PointField descriptor4;
    descriptor3.name("pol");
    descriptor3.offset(12);
    descriptor3.datatype(sensor_msgs::msg::PointField__INT8);
    descriptor3.count(1);

    const size_t point_size = sizeof(carla::sensor::data::DVSEvent);
    _point_cloud->_pc.width(width);
    _point_cloud->_pc.height(height);
    _point_cloud->_pc.is_bigendian(false);
    _point_cloud->_pc.fields({descriptor1, descriptor2, descriptor3, descriptor4});
    _point_cloud->_pc.point_step(point_size);
    _point_cloud->_pc.row_step(width * point_size);
    _point_cloud->_pc.is_dense(false); //True if there are not invalid points
    _point_cloud->_pc.data(std::move(vector_data));
  }

  CarlaDVSCameraPublisher::CarlaDVSCameraPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaDVSCameraPublisherImpl>()),
  _info(std::make_shared<CarlaCameraInfoPublisherImpl>()),
  _point_cloud(std::make_shared<CarlaPointCloudPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaDVSCameraPublisher::~CarlaDVSCameraPublisher() {
      if (!_impl)
          return;

      if (_impl->_datawriter)
          _impl->_publisher->delete_datawriter(_impl->_datawriter);

      if (_impl->_publisher)
          _impl->_participant->delete_publisher(_impl->_publisher);

      if (_impl->_topic)
          _impl->_participant->delete_topic(_impl->_topic);

      if (_impl->_participant)
          efd::DomainParticipantFactory::get_instance()->delete_participant(_impl->_participant);

      if (!_info)
          return;

      if (_info->_datawriter)
          _info->_publisher->delete_datawriter(_info->_datawriter);

      if (_info->_publisher)
          _info->_participant->delete_publisher(_info->_publisher);

      if (_info->_topic)
          _info->_participant->delete_topic(_info->_topic);

      if (_info->_participant)
          efd::DomainParticipantFactory::get_instance()->delete_participant(_info->_participant);

      if (!_point_cloud)
          return;

      if (_point_cloud->_datawriter)
          _point_cloud->_publisher->delete_datawriter(_point_cloud->_datawriter);

      if (_point_cloud->_publisher)
          _point_cloud->_participant->delete_publisher(_point_cloud->_publisher);

      if (_point_cloud->_topic)
          _point_cloud->_participant->delete_topic(_point_cloud->_topic);

      if (_point_cloud->_participant)
          efd::DomainParticipantFactory::get_instance()->delete_participant(_point_cloud->_participant);
  }

  CarlaDVSCameraPublisher::CarlaDVSCameraPublisher(const CarlaDVSCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _info = other._info;
    _point_cloud = other._point_cloud;
  }

  CarlaDVSCameraPublisher& CarlaDVSCameraPublisher::operator=(const CarlaDVSCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _info = other._info;
    _point_cloud = other._point_cloud;

    return *this;
  }

  CarlaDVSCameraPublisher::CarlaDVSCameraPublisher(CarlaDVSCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _info = std::move(other._info);
    _point_cloud = std::move(other._point_cloud);
  }

  CarlaDVSCameraPublisher& CarlaDVSCameraPublisher::operator=(CarlaDVSCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _info = std::move(other._info);
    _point_cloud = std::move(other._point_cloud);

    return *this;
  }
}
}
