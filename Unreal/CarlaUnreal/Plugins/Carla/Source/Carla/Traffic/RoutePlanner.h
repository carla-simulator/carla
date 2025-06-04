// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"
#include <util/ue-header-guard-end.h>

#include "RoutePlanner.generated.h"

class AWheeledVehicleAIController;

/// Assign a random route to every ACarlaWheeledVehicle entering the trigger
/// volume. Routes must be added in editor after placing this actor into the
/// world. Spline tangents are ignored, only locations are taken into account
/// for making the route.
UCLASS()
class CARLA_API ARoutePlanner : public AActor
{
  GENERATED_BODY()

public:

  ARoutePlanner(const FObjectInitializer &ObjectInitializer);

  virtual void BeginDestroy() override;

  void Init();

  void SetBoxExtent(const FVector &Extent)
  {
    TriggerVolume->SetBoxExtent(Extent);
  }

  void DrawRoutes();

  void AddRoute(float probability, const TArray<FVector> &routePoints);

  void CleanRoute();

  void AssignRandomRoute(AWheeledVehicleAIController &Controller) const;

protected:

#if WITH_EDITOR
  virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;

#endif // WITH_EDITOR

  virtual void BeginPlay() override;

  virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

  UFUNCTION()
  void OnTriggerBeginOverlap(
      UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult);

public:

  UPROPERTY(EditAnywhere)
  UBoxComponent *TriggerVolume;

  UPROPERTY(BlueprintReadWrite, Category = "Traffic Routes", EditAnywhere)
  TArray<USplineComponent *> Routes;

  UPROPERTY(BlueprintReadWrite, Category = "Traffic Routes", EditAnywhere, EditFixedSize)
  TArray<float> Probabilities;

  UPROPERTY(BlueprintReadWrite, Category = "Traffic Routes", EditAnywhere, EditFixedSize)
  bool bIsIntersection = false;
};
