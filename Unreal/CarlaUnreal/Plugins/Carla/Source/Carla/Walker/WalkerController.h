// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Walker/WalkerBoneControlIn.h"
#include "Carla/Walker/WalkerBoneControlOut.h"
#include "Carla/Walker/WalkerControl.h"

#include <util/disable-ue4-macros.h>
#include <carla/rpc/WalkerBoneControlIn.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "AIController.h"
#include <util/ue-header-guard-end.h>

#include "WalkerController.generated.h"

/// Possesses every walker pawn. Historically a plain AController that only
/// relayed the client-computed FWalkerControl into the character movement;
/// it is now an AAIController with a UCrowdFollowingComponent so the server
/// can also path-follow on the engine navmesh (server-side pedestrian
/// navigation on World Partition maps). Manual FWalkerControl input and
/// navmesh following are mutually exclusive: applying a manual control
/// aborts any active navigation request.
UCLASS()
class CARLA_API AWalkerController : public AAIController
{
  GENERATED_BODY()

public:

  AWalkerController(const FObjectInitializer &ObjectInitializer);

  void OnPossess(APawn *InPawn) override;

  void Tick(float DeltaSeconds) override;

  /// Maximum walk speed allowed in centimetres per second.
  UFUNCTION(BlueprintCallable)
  float GetMaximumWalkSpeed() const
  {
    return 4096.0f; // ~147 km/h
  }

  UFUNCTION(BlueprintCallable)
  void ApplyWalkerControl(const FWalkerControl &InControl);

  UFUNCTION(BlueprintCallable)
  const FWalkerControl GetWalkerControl() const
  {
    return Control;
  }

  UFUNCTION(BlueprintCallable)
  void GetBonesTransform(FWalkerBoneControlOut &WalkerBones);

  UFUNCTION(BlueprintCallable)
  void SetBonesTransform(const FWalkerBoneControlIn &WalkerBones);

  UFUNCTION(BlueprintCallable)
  void BlendPose(float Blend);

  UFUNCTION(BlueprintCallable)
  void GetPoseFromAnimation();

  /// -- Server-side navmesh navigation ---------------------------------------

  /// Enter navigation mode (idempotent). Returns false if no walker pawn is
  /// possessed. The manual control input is suppressed while navigating.
  /// @a bIsCrosser selects the query filter: crossers may traverse
  /// road/crosswalk nav areas (drawn once per walker by the episode with
  /// probability pedestrians_cross_factor); the flag is ignored when
  /// navigation is already active, so repeated Start calls cannot re-draw.
  bool StartNavigation(bool bIsCrosser = false);

  /// Path-follow to @a WorldLocation (world-local centimeters) on the navmesh
  /// through the crowd following component. Implies StartNavigation().
  bool GoToNavLocation(const FVector &WorldLocation);

  /// Walking speed used while navigating, in cm/s. The manual control path
  /// scales its input against GetMaximumWalkSpeed(), so the character's
  /// MaxWalkSpeed is only lowered to this value while navigation is active
  /// and restored when it stops.
  bool SetNavMaxSpeed(float SpeedCmPerSec);

  /// Abort any navigation request and return to manual control.
  bool StopNavigation();

  bool IsNavigationActive() const
  {
    return bNavigationActive;
  }

private:

  void SetNavigationActive(bool bActive);

  FWalkerControl Control;

  /// Walking speed applied while navigating, cm/s (defaults to the usual
  /// pedestrian speed the Python clients use, 1.39 m/s).
  float NavMaxSpeed = 139.0f;

  bool bNavigationActive = false;

  /// Whether this walker may traverse road/crosswalk nav areas. Drawn once
  /// per walker when its navigation starts.
  bool bNavCrosser = false;
};
