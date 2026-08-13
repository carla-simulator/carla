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
  /// widens the attenuation radius of every Point/Spot light component found
  /// on Owner (see the fix comments in RegisterLight). Exposed statically so
  /// actors that own real light components without a UCarlaLight wrapper --
  /// vehicle headlights, notably, which are authored directly on the vehicle
  /// Blueprint -- can share the same UE5 activation fix instead of silently
  /// rendering nothing under bAutoActivate=false.
  static void ActivateAndConfigureLightComponents(AActor* Owner);

  /// Multiplies the Intensity of every Point/Spot light component on Owner
  /// that still looks like an authored UE4 value (see
  /// CarlaLightMaxAuthoredIntensity in the .cpp) by the scale registered for
  /// LightType. Shared with vehicle headlights for the same reason as above.
  static void ScaleLightComponentIntensities(AActor* Owner, ELightType LightType);

  /// The UE4->UE5 intensity conversion factor for a light type (the value of
  /// carla.Light.StreetIntensityScale or carla.Light.LegacyIntensityScale),
  /// for callers that scale blueprint-side values instead of components.
  static float GetLegacyIntensityScale(ELightType LightType);

private:

  /// Show or hide every Point/Spot light component on the owner. The
  /// blueprint UpdateLights event is supposed to react to state changes, but
  /// several ported lamp blueprints have dead UpdateLights graphs, which left
  /// the client light API (turn_on/turn_off/set_light_state) without any
  /// visual effect. Enforce the on/off state from C++ so the API works
  /// regardless of the content blueprint.
  void ApplyLightOnToComponents(bool bOn);

protected:
	
  UPROPERTY(EditAnywhere, Category = "Carla Light")
  FLinearColor LightColor;

  UPROPERTY(EditAnywhere, Category = "Carla Light")
  float LightIntensity;

  UPROPERTY(EditAnywhere, Category = "Carla Light")
  ELightType LightType = ELightType::Street;

  UPROPERTY(EditAnywhere, Category = "Carla Light")
  int Id = -1;

  UPROPERTY(EditAnywhere, Category = "Carla Light")
  ECarlaLightFlags flags;

private:

  void RecordLightChange() const;

};
