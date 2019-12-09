// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/client/detail/ActorVariant.h"
#include "carla/geom/Vector3D.h"
#include "carla/sensor/SensorData.h"
#include "carla/sensor/s11n/CollisionEventSerializer.h"

namespace carla {
namespace sensor {
namespace data {

  /// A registered collision.
  class CollisionEvent : public SensorData {
    using Super = SensorData;
  protected:

    using Serializer = s11n::CollisionEventSerializer;

    friend Serializer;

    explicit CollisionEvent(const RawData &data)
      : Super(data),
        _self_actor(Serializer::DeserializeRawData(data).self_actor),
        _other_actor(Serializer::DeserializeRawData(data).other_actor),
        _normal_impulse(Serializer::DeserializeRawData(data).normal_impulse) {}

  public:

    /// Get "self" actor. Actor that measured the collision.
    SharedPtr<client::Actor> GetActor() const {
      return _self_actor.Get(GetEpisode());
    }

    /// Get the actor to which we collided.
    SharedPtr<client::Actor> GetOtherActor() const {
      return _other_actor.Get(GetEpisode());
    }

    /// Normal impulse result of the collision.
    const geom::Vector3D &GetNormalImpulse() const {
      return _normal_impulse;
    }

  private:

    client::detail::ActorVariant _self_actor;

    client::detail::ActorVariant _other_actor;

    geom::Vector3D _normal_impulse;
  };

} // namespace data
} // namespace sensor
} // namespace carla
