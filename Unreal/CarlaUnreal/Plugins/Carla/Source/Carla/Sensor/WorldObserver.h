// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/DataStream.h"

class FCarlaActor;

class UCarlaEpisode;

/// Serializes and sends all the actors in the current UCarlaEpisode.
class FWorldObserver
{
public:

  /// Prevent this sensor to be spawned by users.
  using not_spawnable = void;

  /// Replace the Stream associated with this sensor.
  void SetStream(FDataMultiStream InStream)
  {
    Stream = std::move(InStream);
  }

  /// Return the token that allows subscribing to this sensor's stream.
  auto GetToken() const
  {
    return Stream.GetToken();
  }

  /// An actor that cannot move: active, of type Other or TrafficSign, not
  /// attached to another actor, with a Static root that simulates no physics
  /// (a static.prop.mesh spawned without mass, a traffic sign). Static actors
  /// are left out of the per-tick episode state; clients get them through the
  /// "get_static_actors" call, again only when the episode state says they
  /// changed. A traffic light is never static: its state changes.
  static bool IsStaticActor(const FCarlaActor &View);

  /// Order-independent hash and count of the static actors (ids and
  /// transforms) in the previous tick, and the version sent in the episode
  /// state, which changes when they do.
  struct FStaticActors
  {
    uint64 Hash = 0u;
    int32 Count = -1;
    uint32 Version = 0u;
  };

  /// Send a message to every connected client with the info about the given @a
  /// Episode.
  void BroadcastTick(
    const UCarlaEpisode &Episode,
    float DeltaSeconds,
    bool MapChange,
    bool PendingLightUpdate);

  /// Dummy. Required for compatibility with other sensors only.
  FTransform GetActorTransform() const
  {
    return {};
  }

private:

  FDataMultiStream Stream;

  FStaticActors StaticActors;
};
