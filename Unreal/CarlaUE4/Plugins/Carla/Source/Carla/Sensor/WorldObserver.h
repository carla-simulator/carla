// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"

#include "Carla/Game/CarlaEpisode.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Optional.h>
#include <carla/streaming/Stream.h>
#include <compiler/enable-ue4-macros.h>

#include "WorldObserver.generated.h"

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
  void SetStream(carla::streaming::MultiStream InStream)
  {
    Stream = std::move(InStream);
  }

  void BeginPlay() final;

  void Tick(float DeltaSeconds) final;

private:

  UCarlaEpisode *Episode = nullptr;

  carla::Optional<carla::streaming::MultiStream> Stream;

  double GameTimeStamp = 0.0;
};
