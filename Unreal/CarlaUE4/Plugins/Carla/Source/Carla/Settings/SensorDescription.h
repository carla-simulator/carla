// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "SensorDescriptionVisitor.h"
#include "SensorDescription.generated.h"

UCLASS(Abstract)
class USensorDescription : public UObject
{
  GENERATED_BODY()

public:

  virtual void AcceptVisitor(ISensorDescriptionVisitor &Visitor) const {}

  /** Display name of the sensor. */
  UPROPERTY(Category = "Sensor Description", EditDefaultsOnly)
  FName Name;

  /** Position relative to the player. */
  UPROPERTY(Category = "Sensor Description", EditDefaultsOnly)
  FVector Position = {170.0f, 0.0f, 150.0f};

  /** Rotation relative to the player. */
  UPROPERTY(Category = "Sensor Description", EditDefaultsOnly)
  FRotator Rotation = {0.0f, 0.0f, 0.0f};
};
