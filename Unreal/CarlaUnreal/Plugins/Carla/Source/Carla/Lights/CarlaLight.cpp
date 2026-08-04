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
// UE4: a street lamp's CarlaLight ships intensity ~20, and the lamp blueprints
// feed that same number both to the light component's SetIntensity AND to the
// mesh material's EmissiveIntensity parameter (the glowing bulb). Under
// UE 5.8's inverse-square photometric units, 20 lumens emits next to nothing
// and night scenes render black. Scaling CarlaLight::LightIntensity itself (a
// previous fix, x10000) cannot work: the blueprint pushes that one value to
// both sinks, so any factor that makes the light visible also drives the
// material emissive to hundreds of thousands and turns every pole into a
// white-hot rod that drags the auto-exposure down and blinds the scene.
// Instead, keep LightIntensity at its authored value (the emissive path and
// the client-facing light API stay UE4-compatible) and scale the LIGHT
// COMPONENT's intensity right after every code path that lets the blueprint
// push authored values into it (see ApplyLegacyComponentConversion).
// Same story for attenuation radius: UE4-era lamp content ships spot/point
// lights with ~10-16 m attenuation, so even a bright lamp lights almost
// nothing under UE5 -- the pool dies a few meters past each pole. Enforce a
// minimum radius on every light component of a registered CarlaLight;
// 7000 cm (70 m) lets 30 m-spaced street lamp pools overlap with inverse-
// square falloff doing the actual attenuation well before the cutoff.
static TAutoConsoleVariable<float> CVarCarlaLightMinAttenuationRadius(
    TEXT("carla.Light.MinAttenuationRadius"),
    7000.0f,
    TEXT("Minimum attenuation radius (cm) enforced on the light components of every ")
    TEXT("registered CarlaLight. Set 0 to leave authored radii untouched."),
    ECVF_Default);

// Two scales, split by light type. The content itself defines the street
// factor: the re-authored legacy street lamps (00_LegacyAssets) carry
// CarlaLight intensities of 6-10 MILLION where the modern blueprints carry
// 12-20 -- a ratio of ~500k -- and those legacy values are what reads as a
// proper lamp pool under this project's fixed day-calibrated exposure
// (auto exposure is disabled project-wide). Building/other/vehicle lights
// are far denser per map (hundreds on Town10), so the same factor floods the
// whole town white; they keep the older, milder calibration.
static TAutoConsoleVariable<float> CVarCarlaLightStreetIntensityScale(
    TEXT("carla.Light.StreetIntensityScale"),
    500000.0f,
    TEXT("Multiplier converting small UE4-era CarlaLight intensities of STREET lights to UE5 ")
    TEXT("photometric units, applied to the owner's light components after every blueprint ")
    TEXT("intensity push. Set 1 to leave component intensities untouched."),
    ECVF_Default);

static TAutoConsoleVariable<float> CVarCarlaLightLegacyIntensityScale(
    TEXT("carla.Light.LegacyIntensityScale"),
    10000.0f,
    TEXT("Multiplier converting small UE4-era CarlaLight intensities of non-street lights ")
    TEXT("(building, vehicle, other) to UE5 photometric units, applied to the owner's light ")
    TEXT("components after every blueprint intensity push. Set 1 to leave component ")
    TEXT("intensities untouched."),
    ECVF_Default);

// Blueprint pushes write ABSOLUTE authored values (tens to hundreds) into the
// component; the conversion then multiplies the component intensity by the
// scale above (giving hundreds of thousands to millions). Values already at
// converted magnitude must not be scaled again when the conversion re-runs
// after a state change whose blueprint handler did not push a fresh
// intensity. Authored UE4 content tops out around a few hundred, so anything
// at or above this threshold is treated as already converted.
static constexpr float CarlaLightMaxAuthoredIntensity = 1000.0f;

UCarlaLight::UCarlaLight()
{
  PrimaryComponentTick.bCanEverTick = false;
}

void UCarlaLight::BeginPlay()
{
  Super::BeginPlay();

  // A fresh BeginPlay means this instance is not registered with this
  // world's subsystem, whatever a copied flag claims: template-based
  // spawning (PCG Spawn Actor and friends) copies the template component's
  // flags wholesale, which used to make every cloned street light skip
  // registration (1 of 133 PCG-spawned lamps registered), and EndPlay never
  // cleared the flag either.
  flags &= ~ECarlaLightFlags::Registered;
  RegisterLight();
}

void UCarlaLight::RegisterLight()
{
  if ((flags & ECarlaLightFlags::Registered) != ECarlaLightFlags())
  {
    return;
  }

  if (GetOwner() != nullptr)
  {
    ActivateAndConfigureLightComponents(GetOwner());
  }

  ApplyLegacyComponentConversion();

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
  // Allow re-registration if this component gets a new BeginPlay (level
  // streaming, actor reuse); the flag used to stay set forever.
  flags &= ~ECarlaLightFlags::Registered;
  Super::EndPlay(EndPlayReason);
}

void UCarlaLight::ApplyLegacyComponentConversion()
{
  if (GetOwner() == nullptr)
  {
    return;
  }
  ScaleLightComponentIntensities(GetOwner(), LightType);
}

void UCarlaLight::ActivateAndConfigureLightComponents(AActor* Owner)
{
  // Widen undersized authored attenuation radii so the light actually
  // reaches the road (see cvar comment above).
  const float MinRadius = CVarCarlaLightMinAttenuationRadius.GetValueOnGameThread();
  if (MinRadius <= 0.0f || Owner == nullptr)
  {
    return;
  }
  TArray<UPointLightComponent*> LightComponents;
  Owner->GetComponents<UPointLightComponent>(LightComponents);
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
    // Some lamp content ships decorative IES gobos (e.g. XArrowDiffuse on
    // BP_StreetLight01) that mask virtually all of the light's output; a
    // street light needs its raw cone. Drop the profile.
    LightComponent->SetIESTexture(nullptr);
    // Registered lights are driven at runtime (day/night, client API);
    // Stationary components with no built lighting contribute nothing in
    // -game. Make them movable so they always render dynamically.
    LightComponent->SetMobility(EComponentMobility::Movable);
    // UE4-era lamp content ships light components with bAutoActivate off
    // (UE4 rendered lights regardless of active state, so nobody noticed);
    // UE5 culls inactive light components outright, which blacked out
    // every lamp no matter the intensity. Activate them -- on/off is
    // driven through intensity/visibility by the blueprints, not through
    // component activation.
    if (!LightComponent->IsActive())
    {
      LightComponent->SetActive(true);
    }
  }
}

float UCarlaLight::GetLegacyIntensityScale(ELightType LightType)
{
  return (LightType == ELightType::Street)
      ? CVarCarlaLightStreetIntensityScale.GetValueOnGameThread()
      : CVarCarlaLightLegacyIntensityScale.GetValueOnGameThread();
}

void UCarlaLight::ScaleLightComponentIntensities(AActor* Owner, ELightType LightType)
{
  const float Scale = GetLegacyIntensityScale(LightType);
  if (Scale == 1.0f || Owner == nullptr)
  {
    return;
  }
  TArray<UPointLightComponent*> LightComponents;
  Owner->GetComponents<UPointLightComponent>(LightComponents);
  for (UPointLightComponent* LightComponent : LightComponents)
  {
    const float Current = LightComponent->Intensity;
    UE_LOG(LogCarla, VeryVerbose, TEXT("CarlaLight conversion: owner %s component %s intensity %f visible %d"),
        *Owner->GetName(), *LightComponent->GetName(), Current, LightComponent->IsVisible() ? 1 : 0);
    if (Current > 0.0f && Current < CarlaLightMaxAuthoredIntensity)
    {
      LightComponent->SetIntensity(Current * Scale);
    }
  }
}

void UCarlaLight::SetLightIntensity(float Intensity)
{
  LightIntensity = Intensity;
  UpdateLights();
  ApplyLegacyComponentConversion();
}

float UCarlaLight::GetLightIntensity() const
{
  return LightIntensity;
}

void UCarlaLight::SetLightColor(FLinearColor Color)
{
  LightColor = Color;
  UpdateLights();
  ApplyLegacyComponentConversion();
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
  ApplyLegacyComponentConversion();
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
  ApplyLegacyComponentConversion();
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
