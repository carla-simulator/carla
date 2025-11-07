// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <deque>
#include <memory>

#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "carla/Logging.h"
#include "carla/ros2/impl/DdsDomainParticipantImpl.h"
#include "carla/ros2/impl/DdsQoS.h"
#include "carla/ros2/impl/DdsReturnCode.h"
#include "carla/ros2/services/ServiceInterface.h"

namespace carla {
namespace ros2 {

template <typename REQUEST_TYPE, typename REQUEST_PUB_TYPE, typename RESPONSE_TYPE, typename RESPONSE_PUB_TYPE>
class DdsServiceImpl : public ServiceInterface, public eprosima::fastdds::dds::DataReaderListener {
public:
  DdsServiceImpl() = default;

  virtual ~DdsServiceImpl() {
    carla::log_info("DdsServiceImpl[", _request_topic != nullptr ? _request_topic->get_name() : "nulltopic",
                    "]::Destructor()");

    if (_datawriter) {
      _publisher->delete_datawriter(_datawriter);
      _datawriter = nullptr;
    }

    if (_publisher) {
      _participant->delete_publisher(_publisher);
      _publisher = nullptr;
    }

    if (_response_topic) {
      _participant->delete_topic(_response_topic);
      _response_topic = nullptr;
    }

    if (_datareader) {
      _subscriber->delete_datareader(_datareader);
      _datareader = nullptr;
    }

    if (_subscriber) {
      _participant->delete_subscriber(_subscriber);
      _subscriber = nullptr;
    }

    if (_request_topic) {
      _participant->delete_topic(_request_topic);
      _request_topic = nullptr;
    }
  }

  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant, std::string topic_name) {
    carla::log_info("DdsServiceImpl[", topic_name, "]::Init()");

    _participant = domain_participant->GetDomainParticipant();
    if (_participant == nullptr) {
      carla::log_error("DdsServiceImpl[", topic_name, "]::Init(): Invalid Participant");
      return false;
    }

    auto request_name = topic_name + "Request";
    request_name.replace(0u, 2u, "rq");
    if (_request_type == nullptr) {
      carla::log_error("DdsServiceImpl[", topic_name, "]::Init(): Invalid Request TypeSupport");
      return false;
    }
    _request_type.register_type(_participant);
    auto topic_qos = eprosima::fastdds::dds::TOPIC_QOS_DEFAULT;
    topic_qos.history().kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
    topic_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    _request_topic =
        _participant->create_topic(request_name, _request_type->getName(), topic_qos);
    if (_request_topic == nullptr) {
      carla::log_error("DdsServiceImpl[", topic_name, "]::Init(): Failed to create Request Topic");
      return false;
    }
    auto subscriber_qos= eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT;
    _subscriber = _participant->create_subscriber(subscriber_qos);
    if (_subscriber == nullptr) {
      carla::log_error("DdsServiceImpl[", topic_name, "]::Init(): Failed to create Subscriber");
      return false;
    }
    eprosima::fastdds::dds::DataReaderListener* reader_listener =
        static_cast<eprosima::fastdds::dds::DataReaderListener*>(this);
    auto datareader_qos = eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT;
    datareader_qos.history().kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
    datareader_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    _datareader =
        _subscriber->create_datareader(_request_topic, datareader_qos, reader_listener);
    if (_datareader == nullptr) {
      carla::log_error("DdsServiceImpl[", topic_name, "]::Init(): Failed to create DataReader");
      return false;
    }

    auto response_name = topic_name + "Reply";
    response_name.replace(0u, 2u, "rr");
    if (_resonse_type == nullptr) {
      carla::log_error("DdsServiceImpl[", topic_name, "]::Init(): Invalid Response TypeSupport");
      return false;
    }
    _resonse_type.register_type(_participant);
    _response_topic =
        _participant->create_topic(response_name, _resonse_type->getName(), topic_qos);
    if (_response_topic == nullptr) {
      carla::log_error("DdsServiceImpl[", topic_name, "]::Init(): Failed to create Response Topic");
      return false;
    }
    auto publisher_qos = eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT;
    _publisher = _participant->create_publisher(publisher_qos);
    if (_publisher == nullptr) {
      carla::log_error("DdsServiceImpl[", _response_topic->get_name(), "]::Init() Failed to create Publisher");
      return false;
    }

    auto writer_qos = eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT;
    writer_qos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    writer_qos.history().kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
    writer_qos.durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    writer_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    _datawriter = _publisher->create_datawriter(_response_topic, writer_qos);
    if (_datawriter == nullptr) {
      carla::log_error("DdsServiceImpl[", _response_topic->get_name(), "]::Init() Failed to create DataWriter");
      return false;
    }

    return true;
  }

  using ServiceCallbackType = std::function<RESPONSE_TYPE(const REQUEST_TYPE&)>;
  void SetServiceCallback(ServiceCallbackType callback) {
    _callback = callback;
  }

  void on_data_available(eprosima::fastdds::dds::DataReader* reader) override {
    eprosima::fastdds::dds::SampleInfo info;
    REQUEST_TYPE request;
    auto rcode = reader->take_next_sample(&request, &info);
    if (rcode == eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_OK) {
      if (eprosima::fastdds::dds::InstanceStateKind::ALIVE_INSTANCE_STATE == info.instance_state) {
        carla::log_debug("DdsServiceImpl[", _request_topic->get_name(), "]::on_data_available(): Incoming request ");
        _incoming_requests.push_back({request, info.sample_identity});
      } else {
        carla::log_error("DdsServiceImpl[", _request_topic->get_name(),
                         "]::on_data_available(): Error not a request instance");
      }
    } else {
      carla::log_error("DdsServiceImpl[", _request_topic->get_name(), "]::on_data_available(): Error ",
                       std::to_string(rcode));
    }
  }

  void CheckRequest() override {
    if (!_callback) {
      carla::log_warning("DdsServiceImpl[", _request_topic->get_name(), "]::CheckRequest(): No callback defined yet");
      return;
    }
    while (!_incoming_requests.empty()) {
      carla::log_debug("DdsServiceImpl[", _request_topic->get_name(), "]::CheckRequest(): New Request");
      auto const incoming_request = _incoming_requests.front();
      RESPONSE_TYPE response = _callback(incoming_request._request);
      carla::log_debug("DdsServiceImpl[", _response_topic->get_name(), "]::CheckRequest(): Callback returned");

      eprosima::fastrtps::rtps::WriteParams write_params;
      write_params.related_sample_identity() = incoming_request._request_identity;
      auto rcode = _datawriter->write(reinterpret_cast<void*>(&response), write_params);
      if (rcode != bool(eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_OK)) {
        // strange: getting error while the result is actually sent out
        carla::log_debug("DdsServiceImpl[", _response_topic->get_name(),
                         "]::CheckRequest() Failed to write data; Error ", std::to_string(rcode));
      }
      carla::log_debug("DdsServiceImpl[", _response_topic->get_name(), "]::CheckRequest() Response sent");

      _incoming_requests.pop_front();
    }
  }

private:
  eprosima::fastdds::dds::DomainParticipant* _participant{nullptr};

  eprosima::fastdds::dds::TypeSupport _request_type{new REQUEST_PUB_TYPE()};
  eprosima::fastdds::dds::Topic* _request_topic{nullptr};
  eprosima::fastdds::dds::Subscriber* _subscriber{nullptr};
  eprosima::fastdds::dds::DataReader* _datareader{nullptr};

  eprosima::fastdds::dds::TypeSupport _resonse_type{new RESPONSE_PUB_TYPE()};
  eprosima::fastdds::dds::Topic* _response_topic{nullptr};
  eprosima::fastdds::dds::Publisher* _publisher{nullptr};
  eprosima::fastdds::dds::DataWriter* _datawriter{nullptr};

  ServiceCallbackType _callback{nullptr};

  struct IncomingRequest {
    REQUEST_TYPE _request{};
    eprosima::fastrtps::rtps::SampleIdentity _request_identity;
  };
  std::deque<IncomingRequest> _incoming_requests;
};
}  // namespace ros2
}  // namespace carla
