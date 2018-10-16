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

void AWorldObserver::Tick(float DeltaSeconds)
{
  check(Episode != nullptr);
  Super::Tick(DeltaSeconds);

  GameTimeStamp += DeltaSeconds;

  Stream.Send_GameThread(
      *this,
      Stream.PopBufferFromPool(),
      GameTimeStamp,
      FPlatformTime::Seconds(),
      Episode->GetActorRegistry());
}
