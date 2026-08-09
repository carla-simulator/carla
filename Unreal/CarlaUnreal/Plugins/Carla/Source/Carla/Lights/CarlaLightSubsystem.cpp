// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaLightSubsystem.h"
#include "Carla/Weather/Weather.h"

#include <util/ue-header-guard-begin.h>
#include "Kismet/GameplayStatics.h"
#include <util/ue-header-guard-end.h>

//using cr = carla::rpc;

void UCarlaLightSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
  // TODO: Subscribe to map change
}

void UCarlaLightSubsystem::Deinitialize()
{

}

void UCarlaLightSubsystem::RegisterLight(UCarlaLight* CarlaLight)
{
  if(CarlaLight)
  {
    auto LightId = CarlaLight->GetId();
    if (Lights.Contains(LightId))
    {
      UE_LOG(LogCarla, Warning, TEXT("Light Id overlapping"));
      return;
    }
    Lights.Add(LightId, CarlaLight);
    DayTimeChangeEvent.AddUniqueDynamic(CarlaLight, &UCarlaLight::HandleDayTimeChanged);
  }
  SetClientStatesdirty("");
}

void UCarlaLightSubsystem::UnregisterLight(UCarlaLight* CarlaLight)
{
  if(CarlaLight)
  {
    Lights.Remove(CarlaLight->GetId());
    DayTimeChangeEvent.RemoveDynamic(CarlaLight, &UCarlaLight::HandleDayTimeChanged);
  }
  SetClientStatesdirty("");
}

void UCarlaLightSubsystem::NotifyDayTimeChange(bool bIsDay)
{
  DayTimeChangeEvent.Broadcast(bIsDay);
  // Blueprints bind this event too (BlueprintAssignable) and their handlers
  // push raw authored UE4-era intensities into the light components; a
  // per-light conversion inside the broadcast gets overwritten by whichever
  // handler runs later. Convert once the whole broadcast is done.
  for (auto& LightPair : Lights)
  {
    if (UCarlaLight* CarlaLight = LightPair.Value)
    {
      CarlaLight->ApplyLegacyComponentConversion();
    }
  }
  // A day/night change flips light states server-side; flag every connected
  // client so their LightManager re-queries instead of serving stale is_on
  // values from its local cache.
  SetClientStatesdirty("");
}

bool UCarlaLightSubsystem::IsUpdatePending() const
{
  for (auto ClientPair : ClientStates)
  {
    if(ClientPair.Value)
    {
      return true;
    }
  }
  return false;
}

std::vector<carla::rpc::LightState> UCarlaLightSubsystem::GetLights(FString Client)
{
  std::vector<carla::rpc::LightState> result;

  ClientStates.FindOrAdd(Client) = false;

  for(auto& Light : Lights)
  {
    UCarlaLight* CarlaLight = Light.Value;

    result.push_back(CarlaLight->GetLightState());
  }
  return result;
}

void UCarlaLightSubsystem::SetLights(
  FString Client,
  std::vector<carla::rpc::LightState> LightsToSet,
  bool DiscardClient)
{
  bool* ClientState = ClientStates.Find(Client);

  if(ClientState) {
    for(auto& LightState : LightsToSet) {
      UCarlaLight* CarlaLight = Lights.FindRef(LightState._id);
      if(CarlaLight) {
        CarlaLight->SetLightState(LightState);
      }
    }
    *ClientState = true;

    if(DiscardClient)
    {
      ClientStates.Remove(Client);
    }
  }

}

UCarlaLight* UCarlaLightSubsystem::GetLight(int Id)
{
  if (Lights.Contains(Id))
  {
    return Lights[Id];
  }
  return nullptr;
}

void UCarlaLightSubsystem::SetDayNightCycle(const bool active) {
  TArray<AActor*> WeatherActors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWeather::StaticClass(), WeatherActors);
  if (WeatherActors.Num())
  {
    if (AWeather* WeatherActor = Cast<AWeather>(WeatherActors[0]))
    {
      WeatherActor->SetDayNightCycle(active);
    }
  }
}

void UCarlaLightSubsystem::SetClientStatesdirty(FString ClientThatUpdate)
{
  for(auto& Client : ClientStates)
  {
    if(Client.Key != ClientThatUpdate)
    {
      Client.Value = true;
    }

  }
}
