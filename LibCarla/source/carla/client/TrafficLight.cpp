// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/TrafficLight.h"
#include "carla/client/detail/Simulator.h"
#include "carla/client/ActorList.h"

namespace carla {
namespace client {

  void TrafficLight::SetState(rpc::TrafficLightState state) {
    GetEpisode().Lock()->SetTrafficLightState(*this, state);
  }

  rpc::TrafficLightState TrafficLight::GetState() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.state;
  }

  void TrafficLight::SetGreenTime(float green_time) {
    GetEpisode().Lock()->SetTrafficLightGreenTime(*this, green_time);
  }

  float TrafficLight::GetGreenTime() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.green_time;
  }

  void TrafficLight::SetYellowTime(float yellow_time) {
    GetEpisode().Lock()->SetTrafficLightYellowTime(*this, yellow_time);
  }

  float TrafficLight::GetYellowTime() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.yellow_time;
  }

  void TrafficLight::SetRedTime(float red_time) {
    GetEpisode().Lock()->SetTrafficLightRedTime(*this, red_time);
  }

  float TrafficLight::GetRedTime() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.red_time;
  }

  float TrafficLight::GetElapsedTime() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.elapsed_time;
  }

  void TrafficLight::Freeze(bool freeze) {
    //GetEpisode().Lock()->FreezeTrafficLight(*this, freeze);
    GetEpisode().Lock()->FreezeAllTrafficLights(freeze);
  }

  bool TrafficLight::IsFrozen() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.time_is_frozen;
  }

  uint32_t TrafficLight::GetPoleIndex()
  {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.pole_index;
  }

  std::vector<SharedPtr<TrafficLight>> TrafficLight::GetGroupTrafficLights() {
    std::vector<SharedPtr<TrafficLight>> result;
    auto ids = GetEpisode().Lock()->GetGroupTrafficLights(*this);
    for (auto id : ids) {
      SharedPtr<Actor> actor = GetWorld().GetActors()->Find(id);
      result.push_back(boost::static_pointer_cast<TrafficLight>(actor));
    }
    return result;
  }

  void TrafficLight::ResetGroup() {
    GetEpisode().Lock()->ResetTrafficLightGroup(*this);
  }

  std::vector<SharedPtr<Waypoint>> TrafficLight::GetEffectPositions() const {
    std::vector<SharedPtr<Waypoint>> result;
    SharedPtr<Map> carla_map = GetEpisode().Lock()->GetCurrentMap();
    std::vector<SharedPtr<Landmark>> landmarks = carla_map->GetLandmarksFromId(GetOpenDRIVEID());
    for (auto& landmark : landmarks) {
      for (const road::LaneValidity& validity : landmark->GetValidities()) {
        if (validity._from_lane < validity._to_lane) {
          for (int lane_id = validity._from_lane; lane_id <= validity._to_lane; ++lane_id) {
            if(lane_id == 0) continue;
            result.emplace_back(
                carla_map->GetWaypointXODR(
                landmark->GetRoadId(), lane_id, landmark->GetS()));
          }
        } else {
          for (int lane_id = validity._from_lane; lane_id >= validity._to_lane; --lane_id) {
            if(lane_id == 0) continue;
            result.emplace_back(
                carla_map->GetWaypointXODR(
                landmark->GetRoadId(), lane_id, landmark->GetS()));
          }
        }
      }
    }
    return result;
  }

  std::vector<geom::BoundingBox> TrafficLight::GetLightBoxes() const {
    return GetEpisode().Lock()->GetLightBoxes(*this);
  }

  road::SignId TrafficLight::GetOpenDRIVEID() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.sign_id;
  }

} // namespace client
} // namespace carla
