// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/subscribers/WeatherControlSubscriber.h"
#include "carla/ros2/types/WeatherParameters.h"
#include "carla/ros2/impl/DdsSubscriberImpl.h"

namespace carla {
namespace ros2 {

WeatherControlSubscriber::WeatherControlSubscriber(ROS2NameRecord& parent,
                                                  carla::rpc::RpcServerInterface &carla_server)
  : SubscriberBase(parent),
    _impl(std::make_shared<WeatherControlSubscriberImpl>(*this)),
    _carla_server(carla_server) {}

bool WeatherControlSubscriber::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->Init(domain_participant, get_topic_name("weather_control"), get_topic_qos());
}

void WeatherControlSubscriber::ProcessMessages() {
  while (_impl->HasPublishersConnected() && _impl->HasNewMessage()) {
    carla::ros2::types::WeatherParameters weather_parameter(_impl->GetMessage());
    _carla_server.call_set_weather_parameters(weather_parameter.weather_parameters_rpc());
  }
}

}  // namespace ros2
}  // namespace carla
