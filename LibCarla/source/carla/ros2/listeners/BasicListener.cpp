#include "BasicListener.h"
#include <iostream>

#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include "carla/ros2/types/String.h"
#include "carla/ros2/subscribers/BasicSubscriber.h"
#include "carla/ros2/ROS2CallbackData.h"

namespace carla {
namespace ros2 {

  namespace efd = eprosima::fastdds::dds;
  using erc = eprosima::fastrtps::types::ReturnCode_t;

    class BasicListenerImpl : public efd::DataReaderListener {
      public:
      void on_subscription_matched(
              efd::DataReader* reader,
              const efd::SubscriptionMatchedStatus& info) override;
      void on_data_available(efd::DataReader* reader) override;

      int _matched {0};
      BasicSubscriber* _owner {nullptr};
      std_msgs::msg::String _message {};
    };

    void BasicListenerImpl::on_subscription_matched(efd::DataReader* reader, const efd::SubscriptionMatchedStatus& info)
    {
      if (info.current_count_change == 1) {
          _matched = info.total_count;
      } else if (info.current_count_change == -1) {
          _matched = info.total_count;
          if (_matched == 0) {
            _owner->DestroySubscriber();
          }
      } else {
          std::cerr << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
      }
    }

    void BasicListenerImpl::on_data_available(efd::DataReader* reader)
    {
      efd::SampleInfo info;
      eprosima::fastrtps::types::ReturnCode_t rcode = reader->take_next_sample(&_message, &info);
      if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        _owner->ForwardMessage(_message.data());
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
          std::cerr << "RETCODE_ERROR" << std::endl;
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
          std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_BAD_PARAMETER) {
          std::cerr << "RETCODE_BAD_PARAMETER" << std::endl;
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET) {
          std::cerr << "RETCODE_PRECONDITION_NOT_MET" << std::endl;
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES) {
          std::cerr << "RETCODE_OUT_OF_RESOURCES" << std::endl;
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ENABLED) {
          std::cerr << "RETCODE_NOT_ENABLED" << std::endl;
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY) {
          std::cerr << "RETCODE_IMMUTABLE_POLICY" << std::endl;
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY) {
          std::cerr << "RETCODE_INCONSISTENT_POLICY" << std::endl;
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_ALREADY_DELETED) {
          std::cerr << "RETCODE_ALREADY_DELETED" << std::endl;
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_TIMEOUT) {
          std::cerr << "RETCODE_TIMEOUT" << std::endl;
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_NO_DATA) {
          std::cerr << "RETCODE_NO_DATA" << std::endl;
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION) {
          std::cerr << "RETCODE_ILLEGAL_OPERATION" << std::endl;
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY) {
          std::cerr << "RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl;
      }
    }

    void BasicListener::SetOwner(BasicSubscriber* owner) {
        _impl->_owner = owner;
    }

    BasicListener::BasicListener(BasicSubscriber* owner) :
    _impl(std::make_unique<BasicListenerImpl>()) {
        _impl->_owner = owner;
    }

    BasicListener::~BasicListener() {}

}}
