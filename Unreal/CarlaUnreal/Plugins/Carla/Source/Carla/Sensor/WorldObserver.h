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

  /// Leave static actors (IsStaticActor) out of the per-tick episode state
  /// (-carla-omit-static-actors). Clients get them once through the
  /// "get_static_actors" call, again only when the frame says they changed.
  void SetOmitStaticActors(bool bOmit)
  {
    bOmitStaticActors = bOmit;
  }

  bool OmitsStaticActors() const
  {
    return bOmitStaticActors;
  }

  /// An actor that cannot move or change by itself: not dormant, of type
  /// Other, its root primitive Static and not simulating physics, and not
  /// attached to another actor (a static.prop.mesh spawned without mass).
  static bool IsStaticActor(const FCarlaActor &View);

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

  bool bOmitStaticActors = false;

  /// Order-independent hash and count of the static actors (ids and
  /// transforms) in the previous frame: a frame whose set differs is flagged
  /// StaticActorsChanged.
  uint64 StaticActorsHash = 0u;

  int32 StaticActorsCount = -1;
};
