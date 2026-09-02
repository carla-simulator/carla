// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"

#include "TrafficSignBase.generated.h"

class UBoxComponent;

UENUM(BlueprintType)
enum class ETrafficSignState : uint8 {
  Null = 0, // Workarround for UE4.24 issue with enums
  UNKNOWN            = 0u   UMETA(DisplayName = "UNKNOWN"),
  TrafficLightRed    = 1u   UMETA(DisplayName = "Traffic Light - Red"),
  TrafficLightYellow = 2u   UMETA(DisplayName = "Traffic Light - Yellow"),
  TrafficLightGreen  = 3u   UMETA(DisplayName = "Traffic Light - Green"),
  SpeedLimit_30             UMETA(DisplayName = "Speed Limit - 30"),
  SpeedLimit_40             UMETA(DisplayName = "Speed Limit - 40"),
  SpeedLimit_50             UMETA(DisplayName = "Speed Limit - 50"),
  SpeedLimit_60             UMETA(DisplayName = "Speed Limit - 60"),
  SpeedLimit_90             UMETA(DisplayName = "Speed Limit - 90"),
  SpeedLimit_100            UMETA(DisplayName = "Speed Limit - 100"),
  SpeedLimit_120            UMETA(DisplayName = "Speed Limit - 120"),
  SpeedLimit_130            UMETA(DisplayName = "Speed Limit - 130"),
  StopSign                  UMETA(DisplayName = "Stop Sign"),
  YieldSign                 UMETA(DisplayName = "Yield Sign"),
  // Appended after the original set so serialized values keep their meaning.
  // Speed limits the regional sign catalog carries (km/h): the OpenDRIVE subtype of a
  // type-274 signal is always km/h, mph plates are a rendering of the km/h value.
  SpeedLimit_5              UMETA(DisplayName = "Speed Limit - 5"),
  SpeedLimit_10             UMETA(DisplayName = "Speed Limit - 10"),
  SpeedLimit_15             UMETA(DisplayName = "Speed Limit - 15"),
  SpeedLimit_20             UMETA(DisplayName = "Speed Limit - 20"),
  SpeedLimit_25             UMETA(DisplayName = "Speed Limit - 25"),
  SpeedLimit_70             UMETA(DisplayName = "Speed Limit - 70"),
  SpeedLimit_80             UMETA(DisplayName = "Speed Limit - 80"),
  SpeedLimit_110            UMETA(DisplayName = "Speed Limit - 110"),
  /// Any other km/h value; the actual limit is ATrafficSignBase::SpeedLimitKmh (twin builds
  /// with a US profile convert mph limits, e.g. 35 mph -> 56 km/h).
  SpeedLimit                UMETA(DisplayName = "Speed Limit - custom (km/h)")
};

/// km/h of a speed-limit state, -1 for any other state (or for the custom state, whose
/// value lives on the actor).
inline int32 KmhForSpeedLimitState(ETrafficSignState State)
{
  switch (State)
  {
    case ETrafficSignState::SpeedLimit_5:   return 5;
    case ETrafficSignState::SpeedLimit_10:  return 10;
    case ETrafficSignState::SpeedLimit_15:  return 15;
    case ETrafficSignState::SpeedLimit_20:  return 20;
    case ETrafficSignState::SpeedLimit_25:  return 25;
    case ETrafficSignState::SpeedLimit_30:  return 30;
    case ETrafficSignState::SpeedLimit_40:  return 40;
    case ETrafficSignState::SpeedLimit_50:  return 50;
    case ETrafficSignState::SpeedLimit_60:  return 60;
    case ETrafficSignState::SpeedLimit_70:  return 70;
    case ETrafficSignState::SpeedLimit_80:  return 80;
    case ETrafficSignState::SpeedLimit_90:  return 90;
    case ETrafficSignState::SpeedLimit_100: return 100;
    case ETrafficSignState::SpeedLimit_110: return 110;
    case ETrafficSignState::SpeedLimit_120: return 120;
    case ETrafficSignState::SpeedLimit_130: return 130;
    default:                                return -1;
  }
}

/// The dedicated state for a km/h value, ETrafficSignState::SpeedLimit when there is none
/// (then store the value in ATrafficSignBase::SpeedLimitKmh), UNKNOWN for a non-positive value.
inline ETrafficSignState SpeedLimitStateForKmh(int32 Kmh)
{
  if (Kmh <= 0)
  {
    return ETrafficSignState::UNKNOWN;
  }
  static const ETrafficSignState Dedicated[] = {
    ETrafficSignState::SpeedLimit_5,  ETrafficSignState::SpeedLimit_10,  ETrafficSignState::SpeedLimit_15,
    ETrafficSignState::SpeedLimit_20, ETrafficSignState::SpeedLimit_25,  ETrafficSignState::SpeedLimit_30,
    ETrafficSignState::SpeedLimit_40, ETrafficSignState::SpeedLimit_50,  ETrafficSignState::SpeedLimit_60,
    ETrafficSignState::SpeedLimit_70, ETrafficSignState::SpeedLimit_80,  ETrafficSignState::SpeedLimit_90,
    ETrafficSignState::SpeedLimit_100, ETrafficSignState::SpeedLimit_110, ETrafficSignState::SpeedLimit_120,
    ETrafficSignState::SpeedLimit_130 };
  for (ETrafficSignState State : Dedicated)
  {
    if (KmhForSpeedLimitState(State) == Kmh)
    {
      return State;
    }
  }
  return ETrafficSignState::SpeedLimit;
}

inline bool IsSpeedLimitState(ETrafficSignState State)
{
  return State == ETrafficSignState::SpeedLimit || KmhForSpeedLimitState(State) > 0;
}

UCLASS()
class CARLA_API ATrafficSignBase : public AActor {

  GENERATED_BODY()

public:

  ATrafficSignBase(const FObjectInitializer &ObjectInitializer);

  UFUNCTION(BlueprintCallable)
  ETrafficSignState GetTrafficSignState() const
  {
    return TrafficSignState;
  }

  UFUNCTION(BlueprintCallable)
  void SetTrafficSignState(ETrafficSignState State)
  {
    TrafficSignState = State;
  }

  /// km/h this sign limits to, -1 when it is not a speed-limit sign. Dedicated states
  /// answer from the enum; the custom SpeedLimit state answers from SpeedLimitKmh.
  UFUNCTION(BlueprintCallable)
  int32 GetSpeedLimitKmh() const
  {
    if (TrafficSignState == ETrafficSignState::SpeedLimit)
    {
      return SpeedLimitKmh > 0 ? SpeedLimitKmh : -1;
    }
    return KmhForSpeedLimitState(TrafficSignState);
  }

  /// Make this a speed-limit sign for Kmh: picks the dedicated state when one exists,
  /// the custom state (value kept in SpeedLimitKmh) otherwise.
  UFUNCTION(BlueprintCallable)
  void SetSpeedLimitKmh(int32 Kmh)
  {
    TrafficSignState = SpeedLimitStateForKmh(Kmh);
    SpeedLimitKmh = Kmh > 0 ? Kmh : 0;
  }

  UFUNCTION(BlueprintImplementableEvent)
  UBoxComponent *GetTriggerVolume() const;

  TArray<UBoxComponent*> GetTriggerVolumes() const;

private:

  UPROPERTY(Category = "Traffic Sign", EditAnywhere)
  ETrafficSignState TrafficSignState = ETrafficSignState::UNKNOWN;

  /// Limit in km/h for the custom SpeedLimit state (ignored for the dedicated states).
  UPROPERTY(Category = "Traffic Sign", EditAnywhere, meta = (ClampMin = "0"))
  int32 SpeedLimitKmh = 0;
};
