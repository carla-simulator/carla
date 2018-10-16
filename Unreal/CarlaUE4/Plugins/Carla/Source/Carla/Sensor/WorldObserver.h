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

  using not_spawnable = void;

  /// Set the episode that will observe.
  void SetEpisode(UCarlaEpisode &InEpisode)
  {
    checkf(Episode == nullptr, TEXT("Cannot set episode twice!"));
    Episode = &InEpisode;
  }

  /// Replace the Stream associated with this sensor.
  ///
  /// @warning Do not change the stream after BeginPlay. It is not thread-safe.
  void SetStream(FDataMultiStream InStream)
  {
    Stream = std::move(InStream);
  }

  auto GetStreamToken() const
  {
    return Stream.GetToken();
  }

  void Tick(float DeltaSeconds) final;

private:

  UCarlaEpisode *Episode = nullptr;

  FDataMultiStream Stream;

  double GameTimeStamp = 0.0;
};
