// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Sensor/DataStream.h"
#include "Carla/Util/RandomEngine.h"

#include "GameFramework/Actor.h"

#include "Sensor.generated.h"

struct FActorDescription;

/// Base class for sensors.
UCLASS(Abstract, hidecategories = (Collision, Attachment, Actor))
class CARLA_API ASensor : public AActor
{
  GENERATED_BODY()

public:

  ASensor(const FObjectInitializer &ObjectInitializer);

  /// TODO: delete once the new tick pipeline is done
  virtual const FTransform &GetSyncActorTransform() const;

  void SetEpisode(const UCarlaEpisode &InEpisode)
  {
    Episode = &InEpisode;
  }

  virtual void Set(const FActorDescription &Description);

  /// Replace the FDataStream associated with this sensor.
  ///
  /// @warning Do not change the stream after BeginPlay. It is not thread-safe.
  void SetDataStream(FDataStream InStream)
  {
    Stream = std::move(InStream);
  }

  /// Return the token that allows subscribing to this sensor's stream.
  auto GetToken() const
  {
    return Stream.GetToken();
  }

  UFUNCTION(BlueprintCallable)
  URandomEngine *GetRandomEngine()
  {
    return RandomEngine;
  }

  UFUNCTION(BlueprintCallable)
  int32 GetSeed() const
  {
    return Seed;
  }

  UFUNCTION(BlueprintCallable)
  void SetSeed(int32 InSeed);

protected:

  void PostActorCreated() override;

  void EndPlay(EEndPlayReason::Type EndPlayReason) override;

  const UCarlaEpisode &GetEpisode() const
  {
    check(Episode != nullptr);
    return *Episode;
  }

  /// Return the FDataStream associated with this sensor.
  ///
  /// You need to provide a reference to self, this is necessary for template
  /// deduction.
  template <typename SensorT>
  FAsyncDataStream GetDataStream(const SensorT &Self)
  {
    return Stream.MakeAsyncDataStream(Self, GetEpisode().GetElapsedGameTime());
  }

  /// Seed of the pseudo-random engine.
  UPROPERTY(Category = "Random Engine", EditAnywhere)
  int32 Seed = 123456789;

  /// Random Engine used to provide noise for sensor output.
  UPROPERTY()
  URandomEngine *RandomEngine = nullptr;

private:

  FDataStream Stream;

  const UCarlaEpisode *Episode = nullptr;
};
