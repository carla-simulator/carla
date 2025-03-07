#include <iostream>

#include "carla/ros2/plugin-utils/enable-fastdds-include.h" // start fastdds includes
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include "carla/ros2/plugin-utils/disable-fastdds-include.h" // end fastdds includes

#include "TopicSubscription.h"
#include "carla/ros2/ROS2CallbackData.h"

namespace carla {
namespace ros2 {

  namespace efd = eprosima::fastdds::dds;
  using erc = eprosima::fastrtps::types::ReturnCode_t;

  template<class PubSubTypes, class MessageType>
    class SubscriberListenerImpl : public efd::DataReaderListener {
        
      public:
      void on_subscription_matched(
              efd::DataReader* reader,
              const efd::SubscriptionMatchedStatus& info) override;
      void on_data_available(efd::DataReader* reader) override;

      int _matched {0};
      bool _first_connected {false};
      TopicSubscription<PubSubTypes, MessageType>* _owner {nullptr};
      MessageType _message {};
    };

    template<class PubSubTypes, class MessageType>
    void SubscriberListenerImpl<PubSubTypes, MessageType>::on_subscription_matched(efd::DataReader* reader, const efd::SubscriptionMatchedStatus& info)
    {
      if (info.current_count_change == 1) {
          _matched = info.total_count;
          _first_connected = true;
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

    template<class PubSubTypes, class MessageType>
    void SubscriberListenerImpl<PubSubTypes, MessageType>::on_data_available(efd::DataReader* reader)
    {
      efd::SampleInfo info;
      eprosima::fastrtps::types::ReturnCode_t rcode = reader->take_next_sample(&_message, &info);
      if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        _owner->ForwardMessage(_message);
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

    template<class PubSubTypes, class MessageType>
    void SubscriberListener<PubSubTypes, MessageType>::SetOwner(TopicSubscription<PubSubTypes, MessageType>* owner) {
        _impl->_owner = owner;
    }

    template<class PubSubTypes, class MessageType>
    SubscriberListener<PubSubTypes, MessageType>::SubscriberListener(TopicSubscription<PubSubTypes, MessageType>* owner) :
    _impl(std::make_unique<SubscriberListenerImpl<PubSubTypes, MessageType>>()) {
        _impl->_owner = owner;
    }

    template<class PubSubTypes, class MessageType>
    SubscriberListener<PubSubTypes, MessageType>::~SubscriberListener() {}

}}

