#include "CarlaOdometryPublisher.h"

#include <cmath>
#include <iostream>
#include <string>
#include <utility>

#include "carla/ros2/types/OdometryPubSubTypes.h"
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

  struct CarlaOdometryPublisherImpl {
    efd::DomainParticipant* _participant { nullptr };
    efd::Publisher* _publisher { nullptr };
    efd::Topic* _topic { nullptr };
    efd::DataWriter* _datawriter { nullptr };
    efd::TypeSupport _type { new nav_msgs::msg::OdometryPubSubType() };
    CarlaListener _listener {};
    nav_msgs::msg::Odometry _odometry {};
  };

  static bool CheckWriteResult(erc rcode) {
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
      return true;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
      std::cerr << "RETCODE_ERROR" << std::endl;
    } else if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
      std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
    } else if (rcode == erc::ReturnCodeValue::RETCODE_BAD_PARAMETER) {
      std::cerr << "RETCODE_BAD_PARAMETER" << std::endl;
    } else if (rcode == erc::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET) {
      std::cerr << "RETCODE_PRECONDITION_NOT_MET" << std::endl;
    } else if (rcode == erc::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES) {
      std::cerr << "RETCODE_OUT_OF_RESOURCES" << std::endl;
    } else if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ENABLED) {
      std::cerr << "RETCODE_NOT_ENABLED" << std::endl;
    } else if (rcode == erc::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY) {
      std::cerr << "RETCODE_IMMUTABLE_POLICY" << std::endl;
    } else if (rcode == erc::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY) {
      std::cerr << "RETCODE_INCONSISTENT_POLICY" << std::endl;
    } else if (rcode == erc::ReturnCodeValue::RETCODE_ALREADY_DELETED) {
      std::cerr << "RETCODE_ALREADY_DELETED" << std::endl;
    } else if (rcode == erc::ReturnCodeValue::RETCODE_TIMEOUT) {
      std::cerr << "RETCODE_TIMEOUT" << std::endl;
    } else if (rcode == erc::ReturnCodeValue::RETCODE_NO_DATA) {
      std::cerr << "RETCODE_NO_DATA" << std::endl;
    } else if (rcode == erc::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION) {
      std::cerr << "RETCODE_ILLEGAL_OPERATION" << std::endl;
    } else if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY) {
      std::cerr << "RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl;
    } else {
      std::cerr << "UNKNOWN" << std::endl;
    }
    return false;
  }

  bool CarlaOdometryPublisher::Init() {
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
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
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

  bool CarlaOdometryPublisher::Publish() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    return CheckWriteResult(_impl->_datawriter->write(&_impl->_odometry, instance_handle));
  }

  void CarlaOdometryPublisher::SetData(
      int32_t seconds,
      uint32_t nanoseconds,
      const float* location,
      const float* rotation,
      const float* linear_velocity,
      const float* angular_velocity) {

    const double tx = static_cast<double>(*location++);
    const double ty = static_cast<double>(*location++);
    const double tz = static_cast<double>(*location++);

    const float rx = ((*rotation++) * -1.0f) * (static_cast<float>(M_PI) / 180.0f);
    const float ry = ((*rotation++) * -1.0f) * (static_cast<float>(M_PI) / 180.0f);
    const float rz = *rotation++ * (static_cast<float>(M_PI) / 180.0f);

    const float cr = cosf(rz * 0.5f);
    const float sr = sinf(rz * 0.5f);
    const float cp = cosf(rx * 0.5f);
    const float sp = sinf(rx * 0.5f);
    const float cy = cosf(ry * 0.5f);
    const float sy = sinf(ry * 0.5f);

    geometry_msgs::msg::Point position;
    position.x(tx);
    position.y(-ty);
    position.z(tz);

    geometry_msgs::msg::Quaternion orientation;
    orientation.w(cr * cp * cy + sr * sp * sy);
    orientation.x(sr * cp * cy - cr * sp * sy);
    orientation.y(cr * sp * cy + sr * cp * sy);
    orientation.z(cr * cp * sy - sr * sp * cy);

    geometry_msgs::msg::Pose pose;
    pose.position(std::move(position));
    pose.orientation(std::move(orientation));

    geometry_msgs::msg::PoseWithCovariance pose_with_covariance;
    pose_with_covariance.pose(std::move(pose));

    geometry_msgs::msg::Vector3 linear;
    linear.x(*linear_velocity++);
    linear.y(-(*linear_velocity++));
    linear.z(*linear_velocity++);

    geometry_msgs::msg::Vector3 angular;
    angular.x(*angular_velocity++);
    angular.y(-(*angular_velocity++));
    angular.z(*angular_velocity++);

    geometry_msgs::msg::Twist twist;
    twist.linear(std::move(linear));
    twist.angular(std::move(angular));

    geometry_msgs::msg::TwistWithCovariance twist_with_covariance;
    twist_with_covariance.twist(std::move(twist));

    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id("carla_map");

    _impl->_odometry.header(std::move(header));
    _impl->_odometry.child_frame_id(_parent.empty() ? _frame_id : _parent);
    _impl->_odometry.pose(std::move(pose_with_covariance));
    _impl->_odometry.twist(std::move(twist_with_covariance));
  }

  CarlaOdometryPublisher::CarlaOdometryPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaOdometryPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaOdometryPublisher::~CarlaOdometryPublisher() {
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
  }

  CarlaOdometryPublisher::CarlaOdometryPublisher(const CarlaOdometryPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaOdometryPublisher& CarlaOdometryPublisher::operator=(const CarlaOdometryPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;

    return *this;
  }

  CarlaOdometryPublisher::CarlaOdometryPublisher(CarlaOdometryPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaOdometryPublisher& CarlaOdometryPublisher::operator=(CarlaOdometryPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);

    return *this;
  }
}
}
