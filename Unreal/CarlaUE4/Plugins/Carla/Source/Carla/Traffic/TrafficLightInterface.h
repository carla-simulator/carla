// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "TrafficLightState.h"
#include "TrafficLightInterface.generated.h"

UINTERFACE(BlueprintType)
class CARLA_API UTrafficLightInterface : public UInterface
{
  GENERATED_BODY()
};

class ITrafficLightInterface
{
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Traffic Light Interface")
  void LightChanged(ETrafficLightState NewLightState);
};
