// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "SensorManager.h"
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
  UE_LOG(LogCarla, Log, TEXT("Num sensors %d"), SensorList.Num());
  for(ASensor* Sensor : SensorList)
  {
    UE_LOG(LogCarla, Log, TEXT("Running sensor %s"), *Sensor->GetName());
    Sensor->PostPhysTickInternal(World, TickType, DeltaSeconds);
  }
  UE_LOG(LogCarla, Log, TEXT("Done with sensors"));
}
