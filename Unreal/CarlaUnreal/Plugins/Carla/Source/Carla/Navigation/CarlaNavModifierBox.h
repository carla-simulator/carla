// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <util/ue-header-guard-end.h>

#include "CarlaNavModifierBox.generated.h"

class UBoxComponent;
class UNavModifierComponent;
class UNavArea;

/// Oriented box that stamps a nav area class onto the navmesh polys it
/// overlaps (crosswalks generated from the OpenDRIVE by the CarlaCrosswalkNav
/// world-partition builder). The box body only provides the modifier bounds:
/// it collides with nothing and is itself navigation-irrelevant (no Pawn or
/// Vehicle block response), so it never rasterizes geometry into the navmesh.
UCLASS()
class CARLA_API ACarlaNavModifierBox : public AActor
{
  GENERATED_BODY()

public:

  ACarlaNavModifierBox(const FObjectInitializer &ObjectInitializer);

  /// Set the stamped area and the box half-extent (called by the builder
  /// before the actor package is saved).
  void Configure(TSubclassOf<UNavArea> AreaClass, const FVector &BoxExtent);

  UPROPERTY(VisibleAnywhere, Category = "Carla Nav Modifier")
  TObjectPtr<UBoxComponent> Box;

  UPROPERTY(VisibleAnywhere, Category = "Carla Nav Modifier")
  TObjectPtr<UNavModifierComponent> NavModifier;
};
