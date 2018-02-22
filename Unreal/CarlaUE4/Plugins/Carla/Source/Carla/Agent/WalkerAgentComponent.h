// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Agent/AgentComponent.h"

#include "WalkerAgentComponent.generated.h"

/// This component can be added to any ACharacter to be added as agent.
/// See UAgentComponent.
UCLASS()
class CARLA_API UWalkerAgentComponent : public UAgentComponent
{
  GENERATED_BODY()

public:

  UWalkerAgentComponent(const FObjectInitializer &ObjectInitializer);

protected:

  virtual void OnComponentCreated() override;

  virtual void AcceptVisitor(IAgentComponentVisitor &Visitor) const final
  {
    Visitor.Visit(*this);
  }

private:


};
