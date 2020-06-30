// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaLight.h"
#include "CarlaLightSubsystem.h"
#include "Carla/Game/CarlaStatics.h"

UCarlaLight::UCarlaLight()
{
  PrimaryComponentTick.bCanEverTick = false;
}

void UCarlaLight::BeginPlay()
{
  Super::BeginPlay();

  UWorld *World = GetWorld();
  if(World)
  {
    UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>();
    CarlaLightSubsystem->RegisterLight(this);
  }
}

void UCarlaLight::OnComponentDestroyed(bool bDestroyingHierarchy)
{
  Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UCarlaLight::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  UWorld *World = GetWorld();
  if(World)
  {
    UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>();
    CarlaLightSubsystem->UnregisterLight(this);
  }
}

void UCarlaLight::SetLightIntensity(float Intensity)
{
  LightIntensity = Intensity;
  UpdateLights();
}

float UCarlaLight::GetLightIntensity() const
{
  return LightIntensity;
}

void UCarlaLight::SetLightColor(FLinearColor Color)
{
  LightColor = Color;
  UpdateLights();
  RecordLightChange();
}

FLinearColor UCarlaLight::GetLightColor() const
{
  return LightColor;
}

void UCarlaLight::SetLightOn(bool bOn)
{
  bLightOn = bOn;
  UpdateLights();
  RecordLightChange();
}

bool UCarlaLight::GetLightOn() const
{
  return bLightOn;
}

void UCarlaLight::SetLightType(ELightType Type)
{
  LightType = Type;
}

ELightType UCarlaLight::GetLightType() const
{
  return LightType;
}

carla::rpc::LightState UCarlaLight::GetLightState()
{
  carla::rpc::LightState state(
    GetLocation(),
    LightIntensity,
    static_cast<carla::rpc::LightState::LightGroup>(LightType),
    LightColor,
    bLightOn
  );

  state._id = GetId();

  return state;
}

void UCarlaLight::SetLightState(carla::rpc::LightState LightState)
{
  LightIntensity = LightState._intensity;
  LightColor = LightState._color;
  LightType = static_cast<ELightType>(LightState._group);
  bLightOn = LightState._active;
  UpdateLights();
  RecordLightChange();
}

FVector UCarlaLight::GetLocation() const
{
  return GetOwner()->GetActorLocation();
}

int UCarlaLight::GetId() const
{
  return Id;
}

void UCarlaLight::SetId(int InId)
{
  Id = InId;
}

void UCarlaLight::RecordLightChange() const
{
  auto* Episode = UCarlaStatics::GetCurrentEpisode(GetWorld());
  if (Episode)
  {
    auto* Recorder = Episode->GetRecorder();
    if (Recorder && Recorder->IsEnabled())
    {
      Recorder->AddEventLightSceneChanged(this);
    }
  }
}
