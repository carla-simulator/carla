// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
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
  YieldSign                 UMETA(DisplayName = "Yield Sign")
};

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

  UFUNCTION(BlueprintImplementableEvent)
  UBoxComponent *GetTriggerVolume() const;

  TArray<UBoxComponent*> GetTriggerVolumes() const;

private:

  UPROPERTY(Category = "Traffic Sign", EditAnywhere)
  ETrafficSignState TrafficSignState = ETrafficSignState::UNKNOWN;
};
