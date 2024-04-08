// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "TrafficLightsPublisher.h"

#include <algorithm>
#include "carla/ros2/impl/fastdds/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

TrafficLightsPublisher::TrafficLightsPublisher()
  : PublisherBaseSensor(carla::ros2::types::ActorNameDefinition::CreateFromRoleName("traffic_lights")),
    _traffic_light_info(std::make_shared<TrafficLightsInfoPublisherImpl>()),
    _traffic_light_status(std::make_shared<TrafficLightsStatusPublisherImpl>()) {}

bool TrafficLightsPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _traffic_light_info->InitHistoryPreallocatedWithReallocMemoryMode(domain_participant, get_topic_name("info"),
                                                                           PublisherBase::get_topic_qos()) &&
         _traffic_light_status->InitHistoryPreallocatedWithReallocMemoryMode(
             domain_participant, get_topic_name("status"), PublisherBase::get_topic_qos());
}

bool TrafficLightsPublisher::Publish() {
  return _traffic_light_info->Publish() && _traffic_light_status->Publish();
}

bool TrafficLightsPublisher::SubscribersConnected() const {
  return _traffic_light_info->SubscribersConnected() || _traffic_light_status->SubscribersConnected();
}

void TrafficLightsPublisher::UpdateTrafficLightStatus(
    carla_msgs::msg::CarlaTrafficLightStatus const &traffic_light_status) {
  bool traffic_light_found = false;
  for (auto &traffic_light : _traffic_light_status->Message().traffic_lights()) {
    if (traffic_light.id() == traffic_light_status.id()) {
      traffic_light_found = true;
      traffic_light = traffic_light_status;
    }
  }
  if (!traffic_light_found) {
    _traffic_light_status->Message().traffic_lights().push_back(traffic_light_status);
  }
  _traffic_light_status->SetMessageUpdated();
}

void TrafficLightsPublisher::UpdateTrafficLightInfo(carla_msgs::msg::CarlaTrafficLightInfo const &traffic_light_info) {
  bool traffic_light_found = false;
  for (auto &traffic_light : _traffic_light_info->Message().traffic_lights()) {
    if (traffic_light.id() == traffic_light_info.id()) {
      traffic_light_found = true;
      traffic_light = traffic_light_info;
    }
  }
  if (!traffic_light_found) {
    _traffic_light_info->Message().traffic_lights().push_back(traffic_light_info);
  }
  _traffic_light_info->SetMessageUpdated();
}

void TrafficLightsPublisher::RemoveTrafficLight(carla::rpc::ActorId actor) {
  (void)std::remove_if(
      _traffic_light_status->Message().traffic_lights().begin(),
      _traffic_light_status->Message().traffic_lights().end(),
      [actor](carla_msgs::msg::CarlaTrafficLightStatus const traffic_light) { return traffic_light.id() == actor; });
  (void)std::remove_if(
      _traffic_light_info->Message().traffic_lights().begin(), _traffic_light_info->Message().traffic_lights().end(),
      [actor](carla_msgs::msg::CarlaTrafficLightInfo const traffic_light) { return traffic_light.id() == actor; });
}

}  // namespace ros2
}  // namespace carla
