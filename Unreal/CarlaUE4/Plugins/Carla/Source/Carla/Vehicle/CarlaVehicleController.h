// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Vehicle/WheeledVehicleController.h"

#include "CarlaVehicleController.generated.h"

struct FCameraPostProcessParameters;
class ACarlaHUD;
class ACarlaPlayerState;
class ALidar;
class ASceneCaptureCamera;
class UCameraDescription;
class ULidarDescription;
struct FCameraPostProcessParameters;

/// The CARLA player controller.
UCLASS()
class CARLA_API ACarlaVehicleController : public AWheeledVehicleController
{
  GENERATED_BODY()

  // ===========================================================================
  /// @name Constructor and destructor
  // ===========================================================================
  /// @{
public:

  ACarlaVehicleController(const FObjectInitializer& ObjectInitializer);

  ~ACarlaVehicleController();

  /// @}
  // ===========================================================================
  /// @name APlayerController overrides
  // ===========================================================================
  /// @{
public:

  virtual void Possess(APawn *aPawn) override;

  /// @}
  // ===========================================================================
  /// @name AActor overrides
  // ===========================================================================
  /// @{
public:

  virtual void Tick(float DeltaTime) override;

  /// @}
  // ===========================================================================
  /// @name AWheeledVehicleAIController overrides
  // ===========================================================================
  /// @{
public:

  virtual bool IsPossessingThePlayer() const final
  {
    return true;
  }

  /// @}
  // ===========================================================================
  /// @name Player state
  // ===========================================================================
  /// @{
public:

  const ACarlaPlayerState &GetPlayerState() const
  {
    return *CarlaPlayerState;
  }

  /// @}
  // ===========================================================================
  /// @name Events
  // ===========================================================================
  /// @{
private:

  UFUNCTION()
  void OnCollisionEvent(
      AActor* Actor,
      AActor* OtherActor,
      FVector NormalImpulse,
      const FHitResult& Hit);

  /// @}
  // ===========================================================================
  /// @name Private methods
  // ===========================================================================
  /// @{
private:

  void IntersectPlayerWithRoadMap();

  /// @}
  // ===========================================================================
  // -- Member variables -------------------------------------------------------
  // ===========================================================================
private:

  // Cast for quick access to the custom player state.
  UPROPERTY()
  ACarlaPlayerState *CarlaPlayerState;

  // Cast for quick access to the custom HUD.
  UPROPERTY()
  ACarlaHUD *CarlaHUD;
};
