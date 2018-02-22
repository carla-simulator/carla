// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Agent/AgentComponent.h"

#include "TrafficSignAgentComponent.generated.h"

/// This component can be added to any AWheeledVehicle to be added as agent.
/// See UAgentComponent.
UCLASS()
class CARLA_API UTrafficSignAgentComponent : public UAgentComponent
{
  GENERATED_BODY()

public:

  UTrafficSignAgentComponent(const FObjectInitializer &ObjectInitializer);

protected:

  virtual void OnComponentCreated() override;

  virtual void AcceptVisitor(IAgentComponentVisitor &Visitor) const final
  {
    Visitor.Visit(*this);
  }

private:


};
