// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaLightSubsystem.h"

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
  }
}

void UCarlaLightSubsystem::UnregisterLight(UCarlaLight* CarlaLight)
{
  if(CarlaLight)
  {
    Lights.Remove(CarlaLight->GetId());
  }
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
