// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Agent/AgentComponentVisitor.h"

#include "Components/SceneComponent.h"
#include "Templates/SharedPointer.h"

#include "AgentComponent.generated.h"

/// Actors with an UAgentComponent are registered as agents in the scene and
/// their status is sent to the client each frame (if requested by the client).
UCLASS(Abstract)
class CARLA_API UAgentComponent : public USceneComponent
{
  GENERATED_BODY()

public:

  UAgentComponent(const FObjectInitializer& ObjectInitializer);

  uint32 GetId() const
  {
    return GetTypeHash(this);
  }

  virtual void AcceptVisitor(IAgentComponentVisitor &Visitor) const;

protected:

  virtual void BeginPlay() override;

  virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

protected:

  /** If true, this component will be registered at BeginPlay. */
  UPROPERTY(Category = "Agent Component", EditAnywhere)
  bool bRegisterAgentComponent = true;

private:

  /** Whether this component has been registered. */
  UPROPERTY(Category = "Agent Component", VisibleAnywhere, AdvancedDisplay)
  bool bAgentComponentIsRegistered = false;
};
