// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
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
    carla::log_debug("DdsServiceImpl[", _request_topic != nullptr ? _request_topic->get_name() : "nulltopic",
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
    carla::log_debug("DdsServiceImpl[", topic_name, "]::Init()");

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
    datareader_qos.history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
    datareader_qos.history().depth = 50;
    datareader_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    datareader_qos.durability().kind = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
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
    writer_qos.history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
    writer_qos.history().depth = 10;
    writer_qos.durability().kind = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
    writer_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    writer_qos.publish_mode().kind = eprosima::fastdds::dds::SYNCHRONOUS_PUBLISH_MODE;
    _datawriter = _publisher->create_datawriter(_response_topic, writer_qos);
    if (_datawriter == nullptr) {
      carla::log_error("DdsServiceImpl[", _response_topic->get_name(), "]::Init() Failed to create DataWriter");
      return false;
    }

    return true;
  }

  using SyncServiceCallbackType = std::function<RESPONSE_TYPE(const REQUEST_TYPE&)>;
  void SetSyncServiceCallback(SyncServiceCallbackType callback) {
    _sync_callback = callback;
  }

  using RequestPtrType = std::shared_ptr<const REQUEST_TYPE>;
  using AsyncServiceCallbackType = std::function<void(RequestPtrType)>;
  void SetAsyncServiceCallback(AsyncServiceCallbackType callback) {
    _async_callback = callback;
  }

  void on_data_available(eprosima::fastdds::dds::DataReader* reader) override {
    auto incoming_request = std::make_shared<IncomingRequest>();
    eprosima::fastrtps::types::ReturnCode_t rcode = reader->take_next_sample(&incoming_request->request, &incoming_request->info);
    if (rcode == eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_OK) {
      if (eprosima::fastdds::dds::InstanceStateKind::ALIVE_INSTANCE_STATE == incoming_request->info.instance_state) {
        carla::log_debug("DdsServiceImpl[", _request_topic->get_name(), "]::on_data_available(): Incoming request ");
        _incoming_requests.push_back(incoming_request);
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
    while (!_incoming_requests.empty()) {
      carla::log_debug("DdsServiceImpl[", _request_topic->get_name(), "]::CheckRequest(): New Request");
      auto const incoming_request = _incoming_requests.front();
      if ( _sync_callback) {
        carla::log_debug("DdsServiceImpl[", _request_topic->get_name(), "]::CheckRequest(): Calling sync callback");
        auto response = _sync_callback(incoming_request->request);
        carla::log_debug("DdsServiceImpl[", _request_topic->get_name(), "]::CheckRequest(): Sync callback returned");
        SendResponseInternal(response, incoming_request->info.sample_identity);
      } else if (_async_callback) {
        carla::log_debug("DdsServiceImpl[", _request_topic->get_name(), "]::CheckRequest(): Calling async callback");
        _pending_async_requests.push_back(incoming_request);
        auto request_ptr = std::shared_ptr<REQUEST_TYPE>(incoming_request, &incoming_request->request);
        _async_callback(request_ptr);
      } else {
        carla::log_warning("DdsServiceImpl[", _request_topic->get_name(),
                           "]::CheckRequest(): No sync or async callback defined yet");
      }
      _incoming_requests.pop_front();
    }
  }

  void SendResponse(RequestPtrType request_ptr, RESPONSE_TYPE response) {
    carla::log_debug("DdsServiceImpl[", _request_topic->get_name(), "]::SendResponse(): Sending async response");
    auto it = std::find_if(_pending_async_requests.begin(), _pending_async_requests.end(),
                           [request_ptr](std::shared_ptr<IncomingRequest> const& pending_request) {
                             return &pending_request->request == request_ptr.get();
                           });
    if (it != _pending_async_requests.end()) {
       SendResponseInternal(response, it->get()->info.sample_identity);
      _pending_async_requests.erase(it);
    } else {
      carla::log_error("DdsServiceImpl[", _request_topic->get_name(),
                       "]::SendResponse(): Could not find matching pending request for async response");
    }
  }
private:
  void SendResponseInternal(RESPONSE_TYPE& response, const eprosima::fastrtps::rtps::SampleIdentity& related_request_identity) {
    eprosima::fastrtps::rtps::WriteParams write_params;

    write_params.related_sample_identity() = related_request_identity;
    eprosima::fastrtps::types::ReturnCode_t rcode = _datawriter->write(&response, write_params);
    if (rcode != eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_OK) {
      carla::log_debug("DdsServiceImpl[", _response_topic->get_name(),
                       "]::SendResponse() Failed to write data; Error ", std::to_string(rcode), " , ", related_request_identity.sequence_number());
    }
    carla::log_debug("DdsServiceImpl[", _response_topic->get_name(), "]::SendResponse() Response sent");
  }

  eprosima::fastdds::dds::DomainParticipant* _participant{nullptr};

  eprosima::fastdds::dds::TypeSupport _request_type{new REQUEST_PUB_TYPE()};
  eprosima::fastdds::dds::Topic* _request_topic{nullptr};
  eprosima::fastdds::dds::Subscriber* _subscriber{nullptr};
  eprosima::fastdds::dds::DataReader* _datareader{nullptr};

  eprosima::fastdds::dds::TypeSupport _resonse_type{new RESPONSE_PUB_TYPE()};
  eprosima::fastdds::dds::Topic* _response_topic{nullptr};
  eprosima::fastdds::dds::Publisher* _publisher{nullptr};
  eprosima::fastdds::dds::DataWriter* _datawriter{nullptr};

  SyncServiceCallbackType _sync_callback{nullptr};
  AsyncServiceCallbackType _async_callback{nullptr};

  struct IncomingRequest {
    REQUEST_TYPE request{};
    eprosima::fastdds::dds::SampleInfo info;
  };
  std::deque<std::shared_ptr<IncomingRequest>> _incoming_requests;

  std::list<std::shared_ptr<IncomingRequest>> _pending_async_requests;
};
}  // namespace ros2
}  // namespace carla
