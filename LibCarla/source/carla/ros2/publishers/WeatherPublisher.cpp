// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "WeatherPublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"
#include "carla/ros2/types/WeatherParameters.h"

namespace carla {
namespace ros2 {

WeatherPublisher::WeatherPublisher(carla::rpc::RpcServerInterface &carla_server)
  : PublisherBase(carla::ros2::types::ActorNameDefinition::CreateFromRoleName("weather")),
    _impl(std::make_shared<CarlaWeatherParametersPublisherImpl>()),
    _carla_server(carla_server)
{}

bool WeatherPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->Init(domain_participant, get_topic_name(), get_topic_qos().keep_last(1));
}

bool WeatherPublisher::Publish() {
  return _impl->Publish();
}

bool WeatherPublisher::SubscribersConnected() const {
  return _impl->SubscribersConnected();
}

bool WeatherPublisher::ProcessMessages() {
  // the weather data is not transferred by the sensor data stream,
  // it has to be requested separately from the server,
  // This should happen within the message processing step, when also other calls are expected
  // to ensure the simulation internal data is actually locked and its safe to acceess it. 
  if (_impl->SubscribersConnected()) {
    auto response = _carla_server.call_get_weather_parameters();
    if (response.HasError()) {
      carla::log_warning("WeatherPublisher: Failed to get weather parameters "
                         "from CARLA server: ", response.GetError().What());
    }
    else {
      carla::ros2::types::WeatherParameters weather_parameters(response.Get());
      auto const new_weather_parameters = weather_parameters.weather_parameters_msg();
      if ( new_weather_parameters != _impl->Message())
      {
        // send only out if parameters change
        _impl->Message() = new_weather_parameters;
        _impl->SetMessageUpdated();
      }
    }
  }
  return true;
}

}  // namespace ros2
}  // namespace carla
