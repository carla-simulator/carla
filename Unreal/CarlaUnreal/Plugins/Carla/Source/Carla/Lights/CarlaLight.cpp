// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaLight.h"
#include "CarlaLightSubsystem.h"
#include "Carla/Game/CarlaStatics.h"
#include "BlueprintLibary/LightDefaultsJsonUtils.h"

#include <util/ue-header-guard-begin.h>
#include "Components/LocalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInstanceDynamic.h"
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

void UCarlaLight::OnRegister()
{
  Super::OnRegister();
  // Same rationale as BeginPlay below: a copy-pasted/duplicated actor's
  // component inherits the source's already-Registered flag verbatim, which
  // silently skipped subsystem registration for every editor-placed
  // duplicate (confirmed with bus stops/billboards -- 11 and 4 placed
  // instances respectively, only the original ever showed up in a scan,
  // since RegisterLight() below no-ops when this flag is already set).
  flags &= ~ECarlaLightFlags::Registered;
  RegisterLight();
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
  ApplyLightAssetDefault();

  UWorld *World = GetWorld();
  if (World != nullptr)
  {
    // OnRegister now runs for every world, including inactive worlds
    // (e.g. double-clicking a map asset in the Content Browser opens an
    // EWorldType::Inactive preview world). UCarlaLightSubsystem doesn't
    // support that world type, so GetSubsystem returns nullptr there.
    if (UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>())
    {
      CarlaLightSubsystem->RegisterLight(this);
    }
  }
  flags |= ECarlaLightFlags::Registered;
}

void UCarlaLight::OnUnregister()
{
  UWorld* World = GetWorld();
  if (World != nullptr)
  {
    if (UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>())
    {
      CarlaLightSubsystem->UnregisterLight(this);
    }
  }
  flags &= ~ECarlaLightFlags::Registered;
  Super::OnUnregister();
}

void UCarlaLight::OnComponentDestroyed(bool bDestroyingHierarchy)
{
  // OnRegister above now registers in the editor too, not just BeginPlay --
  // so a component actually destroyed (level edit, not just a construction-
  // script re-register, which doesn't hit this at all) needs to unregister
  // here too, or deleting a light in the editor leaves a dangling pointer in
  // the subsystem until the next EndPlay/Deinitialize. Safe to call
  // alongside EndPlay's own unregister: both go through UnregisterLight,
  // which no-ops if already absent.
  UWorld* World = GetWorld();
  if (World != nullptr)
  {
    if (UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>())
      CarlaLightSubsystem->UnregisterLight(this);
  }
  flags &= ~ECarlaLightFlags::Registered;

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
  // ULocalLightComponent, not UPointLightComponent: URectLightComponent is a
  // sibling of UPointLightComponent (both derive from ULocalLightComponent),
  // not a subclass -- every GetComponents<UPointLightComponent> call in this
  // file used to silently find nothing on any RectLight-based prop (bus
  // stops, billboards), making their Intensity/Color controls complete
  // no-ops on the actual light despite the panel looking like it worked.
  // Widen undersized authored attenuation radii so the light actually
  // reaches the road (see cvar comment above).
  const float MinRadius = CVarCarlaLightMinAttenuationRadius.GetValueOnGameThread();
  if (MinRadius <= 0.0f || Owner == nullptr)
  {
    return;
  }
  TArray<ULocalLightComponent*> LightComponents;
  Owner->GetComponents<ULocalLightComponent>(LightComponents);
  for (ULocalLightComponent* LightComponent : LightComponents)
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
  TArray<ULocalLightComponent*> LightComponents;
  Owner->GetComponents<ULocalLightComponent>(LightComponents);
  for (ULocalLightComponent* LightComponent : LightComponents)
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

void UCarlaLight::ApplyIntensityToComponents(float Intensity)
{
  // Drives the light components directly instead of going through the
  // BlueprintImplementableEvent UpdateLights -> ChangeLight dispatcher ->
  // SetLight chain: confirmed by instrumented testing that that chain never
  // actually reaches the native component (PRE/POST-UpdateLights logged
  // identical component intensity regardless of the requested value, both at
  // registration and on a live Reapply). Several ported lamp blueprints have
  // dead UpdateLights graphs (see ApplyLightOnToComponents); intensity had no
  // native workaround the way on/off does, until now.
  AActor* Owner = GetOwner();
  if (Owner == nullptr)
  {
    return;
  }
  const float Scale = GetLegacyIntensityScale(LightType);
  TArray<ULocalLightComponent*> LightComponents;
  Owner->GetComponents<ULocalLightComponent>(LightComponents);
  for (ULocalLightComponent* LightComponent : LightComponents)
  {
    LightComponent->SetIntensity(Intensity * Scale);
  }
}

void UCarlaLight::ApplyColorToComponents(FLinearColor Color)
{
  AActor* Owner = GetOwner();
  if (Owner == nullptr)
  {
    return;
  }
  TArray<ULocalLightComponent*> LightComponents;
  Owner->GetComponents<ULocalLightComponent>(LightComponents);
  for (ULocalLightComponent* LightComponent : LightComponents)
  {
    LightComponent->SetLightColor(Color);
  }
}

void UCarlaLight::SetLightIntensity(float Intensity)
{
  LightIntensity = Intensity;
  ApplyIntensityToComponents(Intensity);
}

float UCarlaLight::GetLightIntensity() const
{
  return LightIntensity;
}

void UCarlaLight::SetLightColor(FLinearColor Color)
{
  LightColor = Color;
  ApplyColorToComponents(Color);
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
  ApplyLightOnToComponents(bOn);
  ApplyLegacyComponentConversion();
  RecordLightChange();
}

void UCarlaLight::HandleDayTimeChanged(bool bIsDay)
{
  DayTimeChanged(bIsDay);
  // Street already auto-toggled; Building and Vehicle (parked-car decoration,
  // not the ego vehicle's driver-controlled lights) should too -- "Other" is
  // left alone, its members are too varied to assume night-on is always right
  // for the real light component. The emissive bulb/glow material follows
  // the exact same gate: a lit bulb visible in broad daylight looks wrong for
  // street lamps specifically (confirmed -- night was fine, day never turned
  // it back off once this was scoped down to a night-only restore). Forcing
  // it to 0 for EVERY LightType including "Other" was tried first and
  // reverted: that broke the tool's live-editing/testing use during daytime
  // (ClearNoon, the default and most common state) for lights that don't
  // want automatic day/night emissive behavior at all.
  if (LightType == ELightType::Street
      || LightType == ELightType::Building
      || LightType == ELightType::Vehicle)
  {
    SetLightOn(!bIsDay);
    // ApplyEmissiveVisualState rather than SetEmissiveIntensity so the
    // authored/saved EmissiveIntensity member isn't clobbered by the
    // day-time 0 -- the next edit (or the next night) still has the real
    // value to restore. Re-pushed (not just toggled) on the night side
    // because the ported lamp blueprints' own DayTimeChanged graphs don't
    // reliably restore it themselves (see ApplyEmissiveToComponents),
    // leaving it stuck off after a day->night round trip otherwise.
    ApplyEmissiveVisualState(bIsDay ? 0.0f : EmissiveIntensity);
  }
}

void UCarlaLight::ApplyLightAssetDefault()
{
  AActor* Owner = GetOwner();
  if (Owner == nullptr)
  {
    return;
  }

  // GetLightClassKey resolves the owning actor's class (BP_StreetLight01_C,
  // ...), or a shared building-material family for buildings like
  // BP_Apt20_A_C..F_C -- not this component's own class: every light-bearing
  // actor places the same BP_Lights component, so GetClass() here would
  // always read "BP_Lights_C" and collapse every light in the level into one
  // bucket.
  bool bIsBuildingClass = false;
  FLightAssetDefault Default;
  if (!ULightDefaultsJsonUtils::GetLightDefault(ULightDefaultsJsonUtils::GetLightClassKey(Owner, bIsBuildingClass), LightType, Default))
  {
    return;
  }
  SetLightColor(Default.Color);
  SetLightIntensity(Default.Intensity);
  SetEmissiveIntensity(Default.EmissiveIntensity);
}

void UCarlaLight::SetEmissiveIntensity(float Value)
{
  EmissiveIntensity = Value;
  ApplyEmissiveVisualState(Value);
}

void UCarlaLight::ApplyEmissiveVisualState(float Value)
{
  // Apply immediately -- correct for every already-settled actor (which is
  // most callers: the Light Defaults panel's live edits, Reapply To Level,
  // day/night toggling). Also apply once more a tick later: called from
  // RegisterLight (OnRegister), a MID created that early is orphaned --
  // confirmed by logging the MID's own name, which differs between a
  // registration-time apply (named after the static parent material, e.g.
  // "M_StreetLight06_0") and a later apply once the level has finished
  // settling (named "MaterialInstanceDynamic_N" as expected) -- two
  // different objects, not the same one reused. Whatever resets the
  // material slot back to its authored asset happens after OnRegister but
  // before the component's real render proxy is built, so the same-frame
  // MID never shows -- confirmed recurring on some content (food trucks)
  // whose meshes finish setting up later still than a single deferred tick
  // covers, needing a manual re-edit to actually show. Both applies are
  // idempotent (same scalar values), so doing both is harmless.
  ApplyEmissiveToComponents(Value);
  if (UWorld* World = GetWorld())
  {
    // Cancel any still-pending retry from an earlier call first -- see
    // DeferredEmissiveTimerHandle's comment. Without this, a burst of calls
    // could leave several independent one-shot timers in flight, each
    // capturing its own now-stale Value, any of which firing after this
    // call's immediate apply above would silently undo it.
    World->GetTimerManager().ClearTimer(DeferredEmissiveTimerHandle);
    TWeakObjectPtr<UCarlaLight> WeakThis(this);
    DeferredEmissiveTimerHandle = World->GetTimerManager().SetTimerForNextTick([WeakThis, Value]()
    {
      if (WeakThis.IsValid())
      {
        WeakThis->ApplyEmissiveToComponents(Value);
      }
    });
  }
}

float UCarlaLight::GetEmissiveIntensity() const
{
  return EmissiveIntensity;
}

void UCarlaLight::ApplyEmissiveToComponents(float Value)
{
  AActor* Owner = GetOwner();
  if (Owner == nullptr)
  {
    return;
  }
  // The lamp materials gate their whole emissive Lerp on "On/Off" (authored
  // defaulting to 0 -- confirmed via M_StreetLight06/M_Artificial_Lamp), so
  // EmissiveIntensity alone never did anything; flip the gate here too.
  const float OnOff = (Value > 0.0f) ? 1.0f : 0.0f;
  TArray<UStaticMeshComponent*> MeshComponents;
  Owner->GetComponents<UStaticMeshComponent>(MeshComponents);
  for (UStaticMeshComponent* MeshComponent : MeshComponents)
  {
    const int32 NumMaterials = MeshComponent->GetNumMaterials();
    for (int32 MaterialIndex = 0; MaterialIndex < NumMaterials; ++MaterialIndex)
    {
      if (UMaterialInstanceDynamic* MID = MeshComponent->CreateAndSetMaterialInstanceDynamic(MaterialIndex))
      {
        MID->SetScalarParameterValue(TEXT("EmissiveIntensity"), Value);
        MID->SetScalarParameterValue(TEXT("On/Off"), OnOff);
      }
    }
  }
}

void UCarlaLight::ApplyLightOnToComponents(bool bOn)
{
  AActor* Owner = GetOwner();
  if (Owner == nullptr)
  {
    return;
  }
  TArray<ULocalLightComponent*> LightComponents;
  Owner->GetComponents<ULocalLightComponent>(LightComponents);
  for (ULocalLightComponent* LightComponent : LightComponents)
  {
    LightComponent->SetVisibility(bOn);
  }
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
  ApplyLightOnToComponents(LightState._active);
  ApplyLegacyComponentConversion();
  RecordLightChange();
}

FVector UCarlaLight::GetLocation() const
{
  // Reached from the RPC thread (get_lights); during a map transition or a
  // World Partition stream-out the owner and the game mode can be gone
  // while this component is still registered.
  const AActor* Owner = GetOwner();
  auto Location = Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
  ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(GetWorld());
  ALargeMapManager* LargeMap = GameMode ? GameMode->GetLMManager() : nullptr;
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
