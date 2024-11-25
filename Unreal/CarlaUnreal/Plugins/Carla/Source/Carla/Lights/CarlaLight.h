// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
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
