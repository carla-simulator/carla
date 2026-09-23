// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/disable-ue4-macros.h>
#include <carla/rpc/LightState.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <util/ue-header-guard-end.h>

#include <type_traits>
#include <utility>

#include "CarlaLight.generated.h"



#define CARLA_ENUM_FROM_RPC(e) static_cast<uint8>(carla::rpc::LightState::LightGroup:: e)

UENUM(BlueprintType)
enum class ELightType : uint8
{
  Null      = 0, // Workarround for UE4.24 issue with enums
  Vehicle   = CARLA_ENUM_FROM_RPC(Vehicle)    UMETA(DisplayName = "Vehicle"),
  Street    = CARLA_ENUM_FROM_RPC(Street)     UMETA(DisplayName = "Street"),
  Building  = CARLA_ENUM_FROM_RPC(Building)   UMETA(DisplayName = "Building"),
  Other     = CARLA_ENUM_FROM_RPC(Other)      UMETA(DisplayName = "Other"),
};

#undef CARLA_ENUM_FROM_RPC



UENUM(BlueprintType, Meta = (Bitflags))
enum class ECarlaLightFlags : uint8
{
	Default		= 0 UMETA(Hidden),
	Registered	= 1 UMETA(Hidden),
	TurnedOn	= 2
};

constexpr auto operator&(ECarlaLightFlags lhs, ECarlaLightFlags rhs)
{
	using U = std::underlying_type_t<ECarlaLightFlags>;
	return static_cast<ECarlaLightFlags>(static_cast<U>(lhs) & static_cast<U>(rhs));
}

constexpr auto operator|(ECarlaLightFlags lhs, ECarlaLightFlags rhs)
{
	using U = std::underlying_type_t<ECarlaLightFlags>;
	return static_cast<ECarlaLightFlags>(static_cast<U>(lhs) | static_cast<U>(rhs));
}

constexpr auto operator^(ECarlaLightFlags lhs, ECarlaLightFlags rhs)
{
	using U = std::underlying_type_t<ECarlaLightFlags>;
	return static_cast<ECarlaLightFlags>(static_cast<U>(lhs) ^ static_cast<U>(rhs));
}

constexpr auto operator~(ECarlaLightFlags lhs)
{
	using U = std::underlying_type_t<ECarlaLightFlags>;
	return static_cast<ECarlaLightFlags>(~static_cast<U>(lhs));
}

constexpr auto operator&=(ECarlaLightFlags& lhs, ECarlaLightFlags rhs)
{
	lhs = lhs & rhs;
	return lhs;
}

constexpr auto operator|=(ECarlaLightFlags& lhs, ECarlaLightFlags rhs)
{
	lhs = lhs | rhs;
	return lhs;
}

constexpr auto operator^=(ECarlaLightFlags& lhs, ECarlaLightFlags rhs)
{
	lhs = lhs ^ rhs;
	return lhs;
}




struct FCarlaLightOptions
{
	FLinearColor LightColor;

	float LightIntensity;

	ELightType LightType = ELightType::Street;

};




// Class representing a light in the scene
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CARLA_API UCarlaLight :
	public UActorComponent
{
  GENERATED_BODY()
public:

  UCarlaLight();

  // Registers with UCarlaLightSubsystem so it can preview day/night toggling
  // in the editor too, not just once actually playing -- BeginPlay (the only
  // other place that calls RegisterLight) never fires outside Play/PIE, so
  // in a pure editor world nothing was ever registered and the day/night
  // broadcast had zero listeners. RegisterLight() is idempotent (checks its
  // own Registered flag), so this is harmless alongside BeginPlay's call.
  void OnRegister() override;

  // Paired with OnRegister: the editor re-registers components routinely
  // (property edits, undo/redo, hot reload...) without ever going through
  // OnComponentDestroyed/EndPlay, calling OnUnregister/OnRegister again
  // instead. Without this override, that cycle left a stale entry under the
  // old Id in the subsystem's map (RegisterLight's collision-avoidance loop
  // just hands out a fresh one) every time it happened -- confirmed via a
  // scanned bus stop count of 44 for 11 actually-placed actors, exactly 4x.
  void OnUnregister() override;

  void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  void OnComponentDestroyed(bool bDestroyingHierarchy) override;

  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void RegisterLight();

  UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Carla Light")
  void UpdateLights();

  UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Carla Light")
  void DayTimeChanged(bool bIsDay);

  /// C++ handler bound to UCarlaLightSubsystem::DayTimeChangeEvent. Forwards
  /// to the blueprint DayTimeChanged event, then syncs the CarlaLight on/off
  /// state for street lights: ported UE4 lamp blueprints render the change
  /// but never update the flags, so the client API reported every lamp as
  /// off while it was visibly lit.
  UFUNCTION()
  void HandleDayTimeChanged(bool bIsDay);

  /// Scale the owner's light components from UE4-era authored intensities to
  /// UE5 photometric units (see carla.Light.LegacyIntensityScale). Must run
  /// after every path that lets blueprints push authored values into the
  /// components -- including the subsystem's day/night broadcast, whose
  /// blueprint-bound handlers can fire after any per-light delegate; guarded
  /// so already-converted intensities are left alone.
  void ApplyLegacyComponentConversion();

  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetLightIntensity(float Intensity);

  UFUNCTION(BlueprintPure, Category = "Carla Light")
  float GetLightIntensity() const;

  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetLightColor(FLinearColor Color);

  UFUNCTION(BlueprintPure, Category = "Carla Light")
  FLinearColor GetLightColor() const;

  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetLightOn(bool bOn);

  UFUNCTION(BlueprintPure, Category = "Carla Light")
  bool GetLightOn() const;

  /// Independent of LightIntensity/LightColor -- some assets want an
  /// emissive glow with no CarlaLight-driven light at all, or vice versa.
  /// Applied natively via ApplyEmissiveToComponents (see there for why the
  /// old BlueprintImplementableEvent-based SetEmissive path never worked).
  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetEmissiveIntensity(float Value);

  UFUNCTION(BlueprintPure, Category = "Carla Light")
  float GetEmissiveIntensity() const;

  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetLightType(ELightType Type);

  UFUNCTION(BlueprintPure, Category = "Carla Light")
  ELightType GetLightType() const;

  carla::rpc::LightState GetLightState();

  void SetLightState(carla::rpc::LightState LightState);

  FVector GetLocation() const;

  UFUNCTION(BlueprintPure, Category = "Carla Light")
  int GetId() const;

  UFUNCTION(BlueprintCallable, Category = "Carla Light")
  void SetId(int InId);

  /// Activates, disables shadow-casting on, strips the IES profile from, and
  /// widens the attenuation radius of every Point/Spot/Rect light component found
  /// on Owner (see the fix comments in RegisterLight). Exposed statically so
  /// actors that own real light components without a UCarlaLight wrapper --
  /// vehicle headlights, notably, which are authored directly on the vehicle
  /// Blueprint -- can share the same UE5 activation fix instead of silently
  /// rendering nothing under bAutoActivate=false.
  static void ActivateAndConfigureLightComponents(AActor* Owner);

  /// Multiplies the Intensity of every Point/Spot/Rect light component on Owner
  /// that still looks like an authored UE4 value (see
  /// CarlaLightMaxAuthoredIntensity in the .cpp) by the scale registered for
  /// LightType. Shared with vehicle headlights for the same reason as above.
  static void ScaleLightComponentIntensities(AActor* Owner, ELightType LightType);

  /// The UE4->UE5 intensity conversion factor for a light type (the value of
  /// carla.Light.StreetIntensityScale or carla.Light.LegacyIntensityScale),
  /// for callers that scale blueprint-side values instead of components.
  static float GetLegacyIntensityScale(ELightType LightType);

private:

  /// Show or hide every Point/Spot/Rect light component on the owner. The
  /// blueprint UpdateLights event is supposed to react to state changes, but
  /// several ported lamp blueprints have dead UpdateLights graphs, which left
  /// the client light API (turn_on/turn_off/set_light_state) without any
  /// visual effect. Enforce the on/off state from C++ so the API works
  /// regardless of the content blueprint.
  void ApplyLightOnToComponents(bool bOn);

  /// Sets the Intensity of every Point/Spot/Rect light component on the owner
  /// directly, scaled by GetLegacyIntensityScale(LightType). Same rationale as
  /// ApplyLightOnToComponents: the blueprint UpdateLights event never reaches
  /// the native component (confirmed by instrumented testing -- logged
  /// component intensity was identical before and after UpdateLights() in
  /// every case, at registration and on a live Reapply), so intensity needs
  /// the same native enforcement on/off already has.
  void ApplyIntensityToComponents(float Intensity);

  /// Sets the LightColor of every Point/Spot/Rect light component on the owner
  /// directly. Same rationale as ApplyIntensityToComponents.
  void ApplyColorToComponents(FLinearColor Color);

  /// Pushes the "EmissiveIntensity" scalar (and the "On/Off" gate scalar) of
  /// every StaticMeshComponent's materials on the owner, via a per-instance
  /// MID. The lamp materials (M_Artificial_Lamp / M_MaterialMaster and their
  /// instances, e.g. M_StreetLight06) author "On/Off" defaulting to 0, which
  /// forces their emissive Lerp to black regardless of EmissiveIntensity --
  /// confirmed the reason editing emissiveIntensity from the Light Defaults
  /// tool never visibly did anything: nothing ever flipped that gate. Setting
  /// On/Off alongside EmissiveIntensity here (on whenever Value > 0) fixes
  /// that without touching the ported material graph. No-ops safely on
  /// materials without these parameters (e.g. the glass lens materials).
  void ApplyEmissiveToComponents(float Value);

  /// Applies Value to the material immediately and once more a tick later
  /// (see the comment in SetEmissiveIntensity's .cpp body) WITHOUT touching
  /// the EmissiveIntensity member -- the day/night handler uses this to
  /// visually turn the glow off by day and back on by night while keeping
  /// the authored/saved intensity intact for the next real edit.
  void ApplyEmissiveVisualState(float Value);

  /// The pending "next tick" retry's handle -- a NEW call reuses/cancels it
  /// instead of always scheduling an independent one. Without this, a burst
  /// of calls landing in the same or adjacent tick (e.g. dragging
  /// SunAltitudeAngle: each edit re-broadcasts day/night to every light)
  /// left multiple stale timers in flight, each capturing its OWN Value from
  /// when it was scheduled -- an OLDER one could fire after a NEWER call's
  /// immediate apply and silently overwrite it, which read as the on/off
  /// state lagging one edit behind.
  FTimerHandle DeferredEmissiveTimerHandle;

  /// Looks this instance's owning class up in the light-defaults catalog
  /// (ULightDefaultsJsonUtils, class entry falling back to LightType's group
  /// entry) and pushes color/intensity/emissive if a match is found. A no-op
  /// if the catalog has neither -- content keeps whatever it was authored
  /// with. Called from RegisterLight, after the legacy intensity conversion.
  void ApplyLightAssetDefault();

protected:
	
  UPROPERTY(EditAnywhere, Category = "Carla Light")
  FLinearColor LightColor;

  UPROPERTY(EditAnywhere, Category = "Carla Light")
  float LightIntensity;

  UPROPERTY(EditAnywhere, Category = "Carla Light")
  float EmissiveIntensity = 0.0f;

  UPROPERTY(EditAnywhere, Category = "Carla Light")
  ELightType LightType = ELightType::Street;

  UPROPERTY(EditAnywhere, Category = "Carla Light")
  int Id = -1;

  UPROPERTY(EditAnywhere, Category = "Carla Light")
  ECarlaLightFlags flags;

private:

  void RecordLightChange() const;

};
