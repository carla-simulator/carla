// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaLight.h"
#include "CarlaLightSubsystem.h"


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

float UCarlaLight::GetLightIntensity()
{
  return LightIntensity;
}

void UCarlaLight::SetLightColor(FLinearColor Color)
{
  LightColor = Color;
  UpdateLights();
}

FLinearColor UCarlaLight::GetLightColor()
{
  return LightColor;
}

void UCarlaLight::SetLightOn(bool bOn)
{
  bLightOn = bOn;
  UpdateLights();
}

bool UCarlaLight::GetLightOn()
{
  return bLightOn;
}

void UCarlaLight::SetLightType(ELightType Type)
{
  LightType = Type;
}

ELightType UCarlaLight::GetLightType()
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
}

FVector UCarlaLight::GetLocation() const
{
  return GetOwner()->GetActorLocation();
}

uint32 UCarlaLight::GetId() const
{
  return GetUniqueID();
}
