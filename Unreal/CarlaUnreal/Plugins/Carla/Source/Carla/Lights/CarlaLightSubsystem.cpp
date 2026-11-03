// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaLightSubsystem.h"
#include "Carla/Weather/Weather.h"

#include <util/ue-header-guard-begin.h>
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
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
    // OnRegister and BeginPlay both register: the same light coming back
    // keeps its entry. Taken for a clone below, it got a second id on every
    // call and the stale entries were never switched (World Partition maps
    // ended up with ~3 entries per lamp).
    if (const int* ExistingId = Lights.FindKey(CarlaLight))
    {
      CarlaLight->SetId(*ExistingId);
    }
    auto LightId = CarlaLight->GetId();
    if (Lights.Contains(LightId) && Lights[LightId] != CarlaLight)
    {
      // Runtime-spawned lights cloned from a template (PCG Spawn Actor,
      // duplicated actors...) all arrive carrying the template's id;
      // dropping them here silently disconnected every clone from the
      // subsystem (day/night events, client light API). Assign the next
      // free id instead.
      while (Lights.Contains(LightId))
      {
        ++LightId;
      }
      CarlaLight->SetId(LightId);
    }
    Lights.Add(LightId, CarlaLight);
    DayTimeChangeEvent.AddUniqueDynamic(CarlaLight, &UCarlaLight::HandleDayTimeChanged);
    // The day/night state is only broadcast when the weather changes, so a
    // light that registers later (a World Partition cell streamed in at
    // night) stayed in its default, day, state until the next change. Applied
    // on the next tick: from inside registration the lamp blueprint has not
    // run its BeginPlay yet, and its SetLight looped forever (PIE stopped).
    if (bHasDayTimeState && bDayNightCycle)
    {
      ScheduleDayTimeState(CarlaLight);
    }
  }
  SetClientStatesdirty("");
}

void UCarlaLightSubsystem::ScheduleDayTimeState(UCarlaLight* CarlaLight)
{
  UWorld* World = GetWorld();
  if (World == nullptr || !World->IsGameWorld())
  {
    return;
  }
  const bool bFirst = PendingDayTimeLights.Num() == 0;
  PendingDayTimeLights.Add(CarlaLight);
  if (!bFirst)
  {
    return;
  }
  World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
  {
    TSet<TWeakObjectPtr<UCarlaLight>> Pending = MoveTemp(PendingDayTimeLights);
    PendingDayTimeLights.Reset();
    for (const TWeakObjectPtr<UCarlaLight>& Weak : Pending)
    {
      UCarlaLight* Light = Weak.Get();
      // Only lights still registered here: one streamed out meanwhile is skipped.
      if (IsValid(Light) && Lights.FindKey(Light) != nullptr)
      {
        Light->HandleDayTimeChanged(bLastIsDay);
        Light->ApplyLegacyComponentConversion();
      }
    }
    SetClientStatesdirty("");
  }));
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
  // Every weather push notifies (AWeather and the sky rig, twice per
  // set_weather), but only an actual day/night change needs the broadcast:
  // it walks every registered light (blueprint handlers, emissive material
  // instances, intensity conversion) and cost ~270 ms per set_weather on
  // Town12 with ~4400 lights loaded. Lights registering later get the
  // current state through ScheduleDayTimeState.
  // The sky rig notifies too, so the client's set_day_night_cycle(False) is
  // enforced here rather than only on AWeather.
  if (!bDayNightCycle || (bHasDayTimeState && bLastIsDay == bIsDay))
  {
    return;
  }
  bHasDayTimeState = true;
  bLastIsDay = bIsDay;
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
    // World Partition can stream out a lamp's owner between the light's GC
    // death and its EndPlay unregistration; this RPC runs on the server
    // thread and must not touch such carcasses.
    if (!IsValid(CarlaLight) || !IsValid(CarlaLight->GetOwner()))
    {
      continue;
    }
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
  bDayNightCycle = active;
  // Re-enabled: the next weather push broadcasts again, even with the same
  // state, so lights the client changed meanwhile follow the cycle again.
  if (active)
  {
    bHasDayTimeState = false;
  }
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
