// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/DataStream.h"

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

  /// Send a message to every connected client with the info about the given @a
  /// Episode.
  void BroadcastTick(
    const UCarlaEpisode &Episode,
    float DeltaSeconds,
    bool MapChange,
    bool PendingLightUpdate);

  /// Dummy. Required for compatibility with other sensors only.
  /// @TODO: raname to 'GetActorTransform' once the new tick pipeline is done
  FTransform GetSyncActorTransform() const
  {
    return {};
  }

private:

  FDataMultiStream Stream;
};
