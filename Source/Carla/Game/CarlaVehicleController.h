// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "PlayerCameraController.h"
#include "CarlaVehicleController.generated.h"

class ACarlaHUD;
class ACarlaPlayerState;
class ASceneCaptureCamera;
class UBoxComponent;
class URoadMap;
class UWheeledVehicleMovementComponent;
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
  /// @name Vehicle pawn info
  // ===========================================================================
  /// @{
public:

  bool IsPossessingAVehicle() const
  {
    return MovementComponent != nullptr;
  }

  /// World transform of the vehicle.
  FTransform GetVehicleTransform() const;

  /// World location of the vehicle.
  FVector GetVehicleLocation() const;

  /// Speed forward in km/h. Might be negative if goes backwards.
  float GetVehicleForwardSpeed() const;

  /// Orientation vector of the vehicle, pointing forward.
  FVector GetVehicleOrientation() const;

  int32 GetVehicleCurrentGear() const;

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
  /// @name Vehicle movement
  // ===========================================================================
  /// @{
public:

  void SetThrottleInput(float Value);

  void SetSteeringInput(float Value);

  void SetBrakeInput(float Value);

  void SetReverse(bool Value);

  void ToggleReverse()
  {
    SetReverse(!bIsInReverse);
  }

  void SetHandbrakeInput(bool Value);

  void HoldHandbrake()
  {
    SetHandbrakeInput(true);
  }

  void ReleaseHandbrake()
  {
    SetHandbrakeInput(false);
  }

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
public:

  void SetRoadMap(URoadMap *inRoadMap)
  {
    RoadMap = inRoadMap;
  }

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
  bool bIsInReverse = false;

  UPROPERTY()
  UWheeledVehicleMovementComponent *MovementComponent;

  UPROPERTY()
  TArray<ASceneCaptureCamera *> SceneCaptureCameras;

  UPROPERTY()
  URoadMap *RoadMap;

  UPROPERTY()
  UBoxComponent *VehicleBounds;

  // Cast for quick access to the custom player state.
  UPROPERTY()
  ACarlaPlayerState *CarlaPlayerState;

  // Cast for quick access to the custom HUD.
  UPROPERTY()
  ACarlaHUD *CarlaHUD;
};
