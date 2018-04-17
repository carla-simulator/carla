// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Agent/AgentComponent.h"

#include "Vehicle/CarlaWheeledVehicle.h"

#include "VehicleAgentComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CARLA_API UVehicleAgentComponent : public UAgentComponent
{
  GENERATED_BODY()

public:

  UVehicleAgentComponent(const FObjectInitializer &ObjectInitializer);

  /// Return forward speed in cm/s.
  float GetForwardSpeed() const
  {
    return Vehicle->GetVehicleForwardSpeed();
  }

  FTransform GetTransform() const
  {
    return Vehicle->GetVehicleTransform();
  }

  FTransform GetBoundingBoxTransform() const
  {
    return Vehicle->GetVehicleBoundingBoxTransform();
  }

  FVector GetBoundingBoxExtent() const
  {
    return Vehicle->GetVehicleBoundingBoxExtent();
  }

protected:

  virtual void BeginPlay() override;

  virtual void AcceptVisitor(IAgentComponentVisitor &Visitor) const final
  {
    Visitor.Visit(*this);
  }

private:

  UPROPERTY()
  ACarlaWheeledVehicle *Vehicle = nullptr;
};
