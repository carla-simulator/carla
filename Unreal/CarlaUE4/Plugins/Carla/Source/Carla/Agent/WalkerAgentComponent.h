// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Agent/AgentComponent.h"

#include "GameFramework/Character.h"

#include "WalkerAgentComponent.generated.h"

/// This component can be added to any ACharacter to be added as agent.
/// See UAgentComponent.
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CARLA_API UWalkerAgentComponent : public UAgentComponent
{
  GENERATED_BODY()

public:

  UWalkerAgentComponent(const FObjectInitializer &ObjectInitializer);

  /// Return forward speed in cm/s.
  float GetForwardSpeed() const
  {
    return FVector::DotProduct(Walker->GetVelocity(), Walker->GetActorRotation().Vector());
  }

  FTransform GetTransform() const
  {
    return Walker->GetActorTransform();
  }

  FTransform GetBoundingBoxTransform() const
  {
    return FTransform();
  }

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
