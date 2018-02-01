// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "WheeledVehicleController.h"
#include "CarlaVehicleController.generated.h"

class ACarlaHUD;
class ACarlaPlayerState;
class ASceneCaptureCamera;
class ALidar;
struct FCameraDescription;
struct FLidarDescription;

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

  virtual void BeginPlay() override;

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
  /// @name Scene Capture
  // ===========================================================================
  /// @{
public:

  void AddSceneCaptureCamera(
      const FCameraDescription &CameraDescription,
      const FCameraPostProcessParameters *OverridePostProcessParameters);

  void AddSceneCaptureLidar(
      const FLidarDescription &LidarDescription);

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

  UPROPERTY()
  TArray<ASceneCaptureCamera *> SceneCaptureCameras;

  UPROPERTY()
  TArray<ALidar *> SceneCaptureLidars;

  // Cast for quick access to the custom player state.
  UPROPERTY()
  ACarlaPlayerState *CarlaPlayerState;

  // Cast for quick access to the custom HUD.
  UPROPERTY()
  ACarlaHUD *CarlaHUD;
};
