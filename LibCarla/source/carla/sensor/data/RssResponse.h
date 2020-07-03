// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rss/RssCheck.h"
#include "carla/sensor/SensorData.h"

namespace carla {
namespace sensor {
namespace data {

/// A RSS Response
class RssResponse : public SensorData {
public:
  explicit RssResponse(size_t frame_number, double timestamp, const rpc::Transform &sensor_transform,
                       const bool &response_valid, const ::ad::rss::state::ProperResponse &response,
                       const ::ad::rss::state::RssStateSnapshot &rss_state_snapshot,
                       const ::ad::rss::situation::SituationSnapshot &situation_snapshot,
                       const ::ad::rss::world::WorldModel &world_model,
                       const carla::rss::EgoDynamicsOnRoute &ego_dynamics_on_route)
    : SensorData(frame_number, timestamp, sensor_transform),
      _response_valid(response_valid),
      _response(response),
      _rss_state_snapshot(rss_state_snapshot),
      _situation_snapshot(situation_snapshot),
      _world_model(world_model),
      _ego_dynamics_on_route(ego_dynamics_on_route) {}

  bool GetResponseValid() const {
    return _response_valid;
  }

  const ::ad::rss::state::ProperResponse &GetProperResponse() const {
    return _response;
  }

  const ::ad::rss::state::RssStateSnapshot &GetRssStateSnapshot() const {
    return _rss_state_snapshot;
  }

  const ::ad::rss::situation::SituationSnapshot &GetSituationSnapshot() const {
    return _situation_snapshot;
  }

  const ::ad::rss::world::WorldModel &GetWorldModel() const {
    return _world_model;
  }

  const carla::rss::EgoDynamicsOnRoute &GetEgoDynamicsOnRoute() const {
    return _ego_dynamics_on_route;
  }

private:
  /*!
   * The validity of RSS calculation.
   */
  bool _response_valid;

  ::ad::rss::state::ProperResponse _response;

  ::ad::rss::state::RssStateSnapshot _rss_state_snapshot;

  ::ad::rss::situation::SituationSnapshot _situation_snapshot;

  ::ad::rss::world::WorldModel _world_model;

  carla::rss::EgoDynamicsOnRoute _ego_dynamics_on_route;
};

}  // namespace data
}  // namespace sensor
}  // namespace carla
