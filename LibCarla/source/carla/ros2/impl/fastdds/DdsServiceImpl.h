// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "carla/Logging.h"
#include "carla/ros2/impl/fastdds/DdsPublisherImpl.h"
#include "carla/ros2/impl/fastdds/DdsSubscriberImpl.h"
#include "carla/ros2/services/ServiceInterface.h"

namespace carla {
namespace ros2 {

template <typename REQUEST_TYPE, typename REQUEST_PUB_TYPE, typename RESPONSE_TYPE, typename RESPONSE_PUB_TYPE>
class DdsServiceImpl : public ServiceInterface {
public:
  DdsServiceImpl() = default;

  virtual ~DdsServiceImpl() {
    carla::log_warning("DdsServiceImpl[]::Destructor()");
  }

  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant, std::string topic_name) {
    carla::log_warning("DdsServiceImpl[", topic_name, "]::Init()");

    auto service_request_name = topic_name;
    service_request_name.replace(0u, 2u, "rq");

    auto service_response_name = topic_name;
    service_response_name.replace(0u, 2u, "rr");

    return _request_subscriber.Init(domain_participant, service_request_name, DEFAULT_ROS2_QOS) &&
           _response_publisher.Init(domain_participant, service_response_name, DEFAULT_ROS2_QOS);
  }

  using ServiceCallbackType = std::function<RESPONSE_TYPE(const REQUEST_TYPE &)>;
  void SetServiceCallback(ServiceCallbackType callback) {
    _callback = callback;
  }

  void CheckRequest() override {
    if (!_callback) {
      carla::log_warning("DdsServiceImpl[", _request_subscriber._topic->get_name(),
                         "]::CheckRequest(): No callback defined yet");
      return;
    }
    if (_request_subscriber.HasNewMessage()) {
      REQUEST_TYPE const &request = _request_subscriber.GetMessage();
      RESPONSE_TYPE const response = _callback(request);
      _response_publisher.Message() = response;
      _response_publisher.SetMessageUpdated();
      (void)_response_publisher.Publish();
    }
  }

private:
  DdsSubscriberImpl<REQUEST_TYPE, REQUEST_PUB_TYPE> _request_subscriber;
  DdsPublisherImpl<RESPONSE_TYPE, RESPONSE_PUB_TYPE> _response_publisher;
  ServiceCallbackType _callback{nullptr};
};
}  // namespace ros2
}  // namespace carla
