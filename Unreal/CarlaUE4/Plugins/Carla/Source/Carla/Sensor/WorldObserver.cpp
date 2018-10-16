// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/WorldObserver.h"

#include "CoreGlobals.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/sensor/SensorRegistry.h>
#include <compiler/enable-ue4-macros.h>

void AWorldObserver::BeginPlay()
{
  Super::BeginPlay();
  check(Stream.has_value());
  check(Episode != nullptr);
}

void AWorldObserver::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  GameTimeStamp += DeltaSeconds;

  (*Stream).Write(carla::sensor::SensorRegistry::Serialize(
      *this,
      (*Stream).MakeBuffer(),
      GFrameCounter,
      GameTimeStamp,
      FPlatformTime::Seconds(),
      Episode->GetActorRegistry()));
}
