// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "SensorManager.h"
#include "Sensor.h"
#include "Carla/Sensor/ImageUtil.h"

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
  for(ASensor* Sensor : SensorList)
  {
    Sensor->PostPhysTickInternal(World, TickType, DeltaSeconds);
  }
  // Every camera above only recorded its GPU copy; synchronize the GPU once
  // and deliver the whole batch (one pipeline drain per tick instead of one
  // per camera).
  ImageUtil::FlushBatchedReadbacks();
}
