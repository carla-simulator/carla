// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/client/detail/ActorVariant.h"
#include "carla/sensor/SensorData.h"
#include "carla/sensor/s11n/ObstacleDetectionEventSerializer.h"


namespace carla {
namespace sensor {
namespace data {

  /// A registered detection.
  class ObstacleDetectionEvent : public SensorData  {
    using Super = SensorData;
  protected:

    using Serializer = s11n::ObstacleDetectionEventSerializer;

    friend Serializer;

    explicit ObstacleDetectionEvent(const RawData &data)
      : Super(data),
        _self_actor(nullptr),
        _other_actor(nullptr) {
      auto ddata = Serializer::DeserializeRawData(data);
      _self_actor = std::move(ddata.self_actor);
      _other_actor = std::move(ddata.other_actor);
      _distance = ddata.distance;
    }

  public:

    /// Get "self" actor. Actor that measured the collision.
    SharedPtr<client::Actor> GetActor() const {
      return _self_actor.Get(GetEpisode());
    }

    /// Get the actor to which we collided.
    SharedPtr<client::Actor> GetOtherActor() const {
      return _other_actor.Get(GetEpisode());
    }

    /// Get obstacle distance.
    float GetDistance() const {
      return _distance;
    }

  private:

    client::detail::ActorVariant _self_actor;

    client::detail::ActorVariant _other_actor;

    float _distance;
  };

} // namespace data
} // namespace sensor
} // namespace carla
