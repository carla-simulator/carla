// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"

#include "Carla/Sensor/DataStream.h"

#include "WorldObserver.generated.h"

class UCarlaEpisode;

/// Serializes and sends all the actors in a UCarlaEpisode.
UCLASS()
class CARLA_API AWorldObserver : public AActor
{
  GENERATED_BODY()

public:

  /// Prevent this sensor to be spawned by users.
  using not_spawnable = void;

  AWorldObserver(const FObjectInitializer& ObjectInitializer);

  /// Set the episode that will be observed.
  void SetEpisode(UCarlaEpisode &InEpisode)
  {
    checkf(Episode == nullptr, TEXT("Cannot set episode twice!"));
    Episode = &InEpisode;
  }

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

protected:

  void Tick(float DeltaSeconds) final;

private:

  UCarlaEpisode *Episode = nullptr;

  FDataMultiStream Stream;

  double GameTimeStamp = 0.0;
};
