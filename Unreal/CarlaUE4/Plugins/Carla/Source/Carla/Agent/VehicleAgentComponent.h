// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Agent/AgentComponent.h"

#include "VehicleAgentComponent.generated.h"

class ACarlaWheeledVehicle;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CARLA_API UVehicleAgentComponent : public UAgentComponent
{
  GENERATED_BODY()

public:

  UVehicleAgentComponent(const FObjectInitializer &ObjectInitializer);

  ACarlaWheeledVehicle &GetVehicle() const
  {
    check(WheeledVehicle != nullptr);
    return *WheeledVehicle;
  }

protected:

  virtual void BeginPlay() override;

  virtual void AcceptVisitor(IAgentComponentVisitor &Visitor) const final
  {
    Visitor.Visit(*this);
  }

private:

  ACarlaWheeledVehicle *WheeledVehicle = nullptr;
};
