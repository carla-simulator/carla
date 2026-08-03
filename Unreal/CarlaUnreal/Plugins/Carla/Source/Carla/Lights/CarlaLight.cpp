// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaLight.h"
#include "CarlaLightSubsystem.h"
#include "Carla/Game/CarlaStatics.h"

#include <util/ue-header-guard-begin.h>
#include "Components/PointLightComponent.h"
#include "HAL/IConsoleManager.h"
#include <util/ue-header-guard-end.h>

// The CARLA light content (street lamps, building lights...) was authored for
// UE4, whose light components used unitless brightness; under UE 5.8's
// photometric units the same numbers (a street lamp ships with intensity ~150)
// emit next to no light and night scenes render black. Scale legacy content
// values once, when the component registers; set to 1 to opt out. 10000 was
// calibrated empirically on Town10 at night: x100 still reads as pitch black
// under the scene's auto-exposure, x10000 produces realistic street-lamp
// pools (a 150-intensity lamp becomes 1.5M units).
// Same story for attenuation radius: UE4-era lamp content ships spot/point
// lights with ~10 m attenuation, so even a bright lamp lights almost nothing
// under UE5 -- night scenes read as black a few meters past each pole.
// Enforce a minimum radius on every light component of a registered
// CarlaLight; 3500 cm (35 m) makes 30 m-spaced street lamps overlap.
static TAutoConsoleVariable<float> CVarCarlaLightMinAttenuationRadius(
    TEXT("carla.Light.MinAttenuationRadius"),
    3500.0f,
    TEXT("Minimum attenuation radius (cm) enforced on the light components of every ")
    TEXT("registered CarlaLight. Set 0 to leave authored radii untouched."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaLightLegacyIntensityScale(
    TEXT("carla.Light.LegacyIntensityScale"),
    10000.0f,
    TEXT("One-time multiplier applied to every CarlaLight's authored intensity at registration, ")
    TEXT("converting UE4-era unitless brightness values to UE5 photometric units."),
    ECVF_Default);

UCarlaLight::UCarlaLight()
{
  PrimaryComponentTick.bCanEverTick = false;
}

void UCarlaLight::BeginPlay()
{
  Super::BeginPlay();

  RegisterLight();
}

void UCarlaLight::RegisterLight()
{
  if ((flags & ECarlaLightFlags::Registered) != ECarlaLightFlags())
  {
    return;
  }

  // Convert the authored UE4-era intensity exactly once (the Registered flag
  // above guards re-entry, so streaming in/out cannot compound the scale).
  LightIntensity *= CVarCarlaLightLegacyIntensityScale.GetValueOnGameThread();

  // Widen undersized authored attenuation radii so the light actually
  // reaches the road (see cvar comment above).
  const float MinRadius = CVarCarlaLightMinAttenuationRadius.GetValueOnGameThread();
  if (MinRadius > 0.0f && GetOwner() != nullptr)
  {
    TArray<UPointLightComponent*> LightComponents;
    GetOwner()->GetComponents<UPointLightComponent>(LightComponents);
    for (UPointLightComponent* LightComponent : LightComponents)
    {
      if (LightComponent->AttenuationRadius < MinRadius)
      {
        LightComponent->SetAttenuationRadius(MinRadius);
      }
      // Lamp blueprints commonly embed the light inside the opaque lamp-head
      // mesh; with shadow casting on, the head fully occludes its own light
      // and the lamp illuminates nothing. Street furniture doesn't need
      // per-lamp shadows.
      LightComponent->SetCastShadows(false);
    }
  }

  UWorld *World = GetWorld();
  if (World != nullptr)
  {
    UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>();
    CarlaLightSubsystem->RegisterLight(this);
  }
  flags |= ECarlaLightFlags::Registered;
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
  Super::EndPlay(EndPlayReason);
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
  flags = bOn ? (flags | ECarlaLightFlags::TurnedOn) : (flags & ~ECarlaLightFlags::TurnedOn);
  UpdateLights();
  RecordLightChange();
}

bool UCarlaLight::GetLightOn() const
{
  return (flags & ECarlaLightFlags::TurnedOn) != ECarlaLightFlags();
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
    GetLightOn()
  );

  state._id = GetId();

  return state;
}

void UCarlaLight::SetLightState(carla::rpc::LightState LightState)
{
  LightIntensity = LightState._intensity;
  LightColor = LightState._color;
  LightType = static_cast<ELightType>(LightState._group);
  flags = LightState._active ? (flags | ECarlaLightFlags::TurnedOn) : (flags & ~ECarlaLightFlags::TurnedOn);
  UpdateLights();
  RecordLightChange();
}

FVector UCarlaLight::GetLocation() const
{
  auto Location = GetOwner()->GetActorLocation();
  ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(GetWorld());
  ALargeMapManager* LargeMap = GameMode->GetLMManager();
  if (LargeMap)
  {
    Location = LargeMap->LocalToGlobalLocation(Location);
  }
  return Location;
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
