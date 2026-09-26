// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "SensorManager.h"
#include "Carla/Sensor/CarlaLidarSubsystem.h"
#include "Engine/World.h"
#include "Sensor.h"

void FSensorManager::RegisterSensor(ASensor* Sensor)
{
  SensorList.Emplace(Sensor);
}

void FSensorManager::DeRegisterSensor(ASensor* Sensor)
{
  SensorList.Remove(Sensor);
}

void FSensorManager::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FSensorManager::PostPhysTick);

  UCarlaLidarSubsystem* LidarSubsystem = World ? World->GetSubsystem<UCarlaLidarSubsystem>() : nullptr;
  if (LidarSubsystem != nullptr) 
  {
    LidarSubsystem->StartLidarSimulations(World, DeltaSeconds);
  }

  for (ASensor * Sensor : SensorList) 
  {
    Sensor->PostPhysTickInternal(World, TickType, DeltaSeconds);
  }

  if (LidarSubsystem != nullptr) 
  {
    LidarSubsystem->FinishLidarSimulations(World, DeltaSeconds);
  }
}
