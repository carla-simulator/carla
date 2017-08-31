// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "PlayerCameraController.h"
#include "CarlaVehicleController.generated.h"

class ACarlaHUD;
class ACarlaPlayerState;
class ACarlaWheeledVehicle;
class ASceneCaptureCamera;
class URoadMap;
struct FCameraDescription;

/**
 *
 */
UCLASS()
class CARLA_API ACarlaVehicleController : public APlayerCameraController
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

  virtual void SetupInputComponent() override;

  virtual void Possess(APawn *aPawn) override;

  virtual void BeginPlay() override;

  virtual void Tick(float DeltaTime) override;

  /// @}
  // ===========================================================================
  /// @name Player state
  // ===========================================================================
  /// @{
public:

  bool IsPossessingAVehicle() const
  {
    return Vehicle != nullptr;
  }

  ACarlaWheeledVehicle *GetPossessedVehicle() const
  {
    return Vehicle;
  }

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
      const FCameraPostProcessParameters *OverridePostProcessParameters = nullptr);

  /// @}
  // ===========================================================================
  /// @name Manual mode
  ///
  /// Manual mode refers here to whether the car can be controlled from the
  /// server side. This is disabled by default when the client connects since it
  /// interferes with the client input.
  // ===========================================================================
  /// @{
public:

  bool IsInManualMode() const
  {
    return bManualMode;
  }

  void SetManualMode(bool On);

  void ToggleManualMode();

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
  /// @name Input bindings
  // ===========================================================================
  /// @{
private:

  void SetupControllerInput();

  /// @}
  // ===========================================================================
  /// @name Other
  // ===========================================================================
  /// @{
private:

  void IntersectPlayerWithRoadMap();

  /// @}
  // ===========================================================================
  // -- Member variables -------------------------------------------------------
  // ===========================================================================
private:

  /// Manual input mode. Not to be mistaken with the vehicle manual/automatic
  /// transmission.
  UPROPERTY()
  bool bManualMode = false;

  UPROPERTY()
  ACarlaWheeledVehicle *Vehicle;

  UPROPERTY()
  TArray<ASceneCaptureCamera *> SceneCaptureCameras;

  UPROPERTY()
  URoadMap *RoadMap;

  // Cast for quick access to the custom player state.
  UPROPERTY()
  ACarlaPlayerState *CarlaPlayerState;

  // Cast for quick access to the custom HUD.
  UPROPERTY()
  ACarlaHUD *CarlaHUD;
};
