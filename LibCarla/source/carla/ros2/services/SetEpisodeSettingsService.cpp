// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/services/SetEpisodeSettingsService.h"

#include "carla/ros2/impl/DdsServiceImpl.h"
#include "carla/ros2/types/EpisodeSettings.h"

namespace carla {
namespace ros2 {

SetEpisodeSettingsService::SetEpisodeSettingsService(
    carla::rpc::RpcServerInterface &carla_server,
    std::shared_ptr<carla::ros2::types::ActorNameDefinition> actor_name_definition)
  : ServiceBase(carla_server, actor_name_definition), _impl(std::make_shared<SetEpisodeSettingsServiceImpl>()) {}

bool SetEpisodeSettingsService::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  _impl->SetServiceCallback(std::bind(&SetEpisodeSettingsService::SetEpisodeSettings, this, std::placeholders::_1));
  return _impl->Init(domain_participant, get_topic_name());
}

void SetEpisodeSettingsService::CheckRequest() {
  _impl->CheckRequest();
}

carla_msgs::srv::SetEpisodeSettings_Response SetEpisodeSettingsService::SetEpisodeSettings(
    carla_msgs::srv::SetEpisodeSettings_Request const &request) {

  carla_msgs::srv::SetEpisodeSettings_Response response;
  carla::ros2::types::EpisodeSettings episode_settings(request.episode_settings());
  auto result = _carla_server.call_set_episode_settings(episode_settings.GetEpisodeSettings());
  if ( result > 0 ) {
    response.success(true);
  }
  else {
    response.success(false);
  }

  return response;
}

}  // namespace ros2
}  // namespace carla
