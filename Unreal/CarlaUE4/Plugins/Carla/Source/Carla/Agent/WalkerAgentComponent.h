// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Agent/AgentComponent.h"

#include "WalkerAgentComponent.generated.h"

class ACharacter;

/// This component can be added to any ACharacter to be added as agent.
/// See UAgentComponent.
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CARLA_API UWalkerAgentComponent : public UAgentComponent
{
  GENERATED_BODY()

public:

  UWalkerAgentComponent(const FObjectInitializer &ObjectInitializer);

  /// Return forward speed in km/h.
  float GetForwardSpeed() const;

  FVector GetBoundingBoxExtent() const
  {
    /// @todo Perhaps the box it is not the same for every walker...
    return {45.0f, 35.0f, 100.0f};
  }

protected:

  virtual void BeginPlay() override;

  virtual void AcceptVisitor(IAgentComponentVisitor &Visitor) const final
  {
    Visitor.Visit(*this);
  }

private:

  UPROPERTY()
  ACharacter *Walker = nullptr;
};
