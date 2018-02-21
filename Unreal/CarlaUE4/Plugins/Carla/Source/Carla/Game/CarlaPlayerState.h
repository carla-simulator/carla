// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/PlayerState.h"
#include "AI/TrafficLightState.h"
#include "AI/RouteCommand.h"
#include "CapturedImage.h"
#include "CarlaPlayerState.generated.h"

/// Current state of the player, updated every frame by ACarlaVehicleController.
///
/// This class matches the reward that it is sent to the client over the
/// network.
UCLASS()
class CARLA_API ACarlaPlayerState : public APlayerState
{
  GENERATED_BODY()

  // ===========================================================================
  // -- APlayerState -----------------------------------------------------------
  // ===========================================================================
public:

  virtual void Reset() override;

  virtual void CopyProperties(APlayerState *PlayerState) override;

  // ===========================================================================
  // -- Getters ----------------------------------------------------------------
  // ===========================================================================
public:

  // ===========================================================================
  /// @name Timing
  // ===========================================================================
  /// @{

  UFUNCTION(BlueprintCallable)
  float GetFramesPerSecond() const
  {
    return FramesPerSecond;
  }

  UFUNCTION(BlueprintCallable)
  int32 GetPlatformTimeStamp() const
  {
    return PlatformTimeStamp;
  }

  UFUNCTION(BlueprintCallable)
  int32 GetGameTimeStamp() const
  {
    return GameTimeStamp;
  }

  /// @}
  // ===========================================================================
  /// @name Transform and dynamics
  // ===========================================================================
  /// @{

  UFUNCTION(BlueprintCallable)
  const FTransform &GetTransform() const
  {
    return Transform;
  }

  UFUNCTION(BlueprintCallable)
  FVector GetLocation() const
  {
    return Transform.GetLocation();
  }

  UFUNCTION(BlueprintCallable)
  FVector GetOrientation() const
  {
    return Transform.GetRotation().GetForwardVector();
  }

  UFUNCTION(BlueprintCallable)
  float GetForwardSpeed() const
  {
    return ForwardSpeed;
  }

  UFUNCTION(BlueprintCallable)
  const FVector &GetAcceleration() const
  {
    return Acceleration;
  }

  /// @}
  // ===========================================================================
  /// @name Vehicle control
  // ===========================================================================
  /// @{

  UFUNCTION(BlueprintCallable)
  float GetThrottle() const
  {
    return Throttle;
  }

  UFUNCTION(BlueprintCallable)
  float GetSteer() const
  {
    return Steer;
  }

  UFUNCTION(BlueprintCallable)
  float GetBrake() const
  {
    return Brake;
  }

  UFUNCTION(BlueprintCallable)
  bool GetHandBrake() const
  {
    return bHandBrake;
  }

  UFUNCTION(BlueprintCallable)
  int32 GetCurrentGear() const
  {
    return CurrentGear;
  }

  UFUNCTION(BlueprintCallable)
  float GetSpeedLimit() const
  {
    return SpeedLimit;
  }

  UFUNCTION(BlueprintCallable)
  ETrafficLightState GetTrafficLightState() const
  {
    return TrafficLightState;
  }

  UFUNCTION(BlueprintCallable)
  ERouteCommand GetRouteCommand() const
  {
    return RouteCommand;
  }

  /// @}
  // ===========================================================================
  /// @name Collision
  // ===========================================================================
  /// @{

  UFUNCTION(BlueprintCallable)
  float GetCollisionIntensityCars() const
  {
    return CollisionIntensityCars;
  }

  UFUNCTION(BlueprintCallable)
  float GetCollisionIntensityPedestrians() const
  {
    return CollisionIntensityPedestrians;
  }

  UFUNCTION(BlueprintCallable)
  float GetCollisionIntensityOther() const
  {
    return CollisionIntensityOther;
  }

  /// @}
  // ===========================================================================
  /// @name Road intersection
  // ===========================================================================
  /// @{

  UFUNCTION(BlueprintCallable)
  float GetOtherLaneIntersectionFactor() const
  {
    return OtherLaneIntersectionFactor;
  }

  UFUNCTION(BlueprintCallable)
  float GetOffRoadIntersectionFactor() const
  {
    return OffRoadIntersectionFactor;
  }

  /// @}
  // ===========================================================================
  /// @name Images
  // ===========================================================================
  /// @{

  UFUNCTION(BlueprintCallable)
  int32 GetNumberOfImages() const
  {
    return Images.Num();
  }

  UFUNCTION(BlueprintCallable)
  bool HasImages() const
  {
    return GetNumberOfImages() > 0;
  }

  const TArray<FCapturedImage> &GetImages() const
  {
    return Images;
  }

  /// @}
  // ===========================================================================
  // -- Modifiers --------------------------------------------------------------
  // ===========================================================================
private:

  void RegisterCollision(
      AActor *Actor,
      AActor *OtherActor,
      const FVector &NormalImpulse,
      const FHitResult &Hit);

  void UpdateTimeStamp(float DeltaSeconds);

  // ===========================================================================
  // -- Private members --------------------------------------------------------
  // ===========================================================================
private:

  friend class ACarlaVehicleController;

  // If you add another variable here, don't forget to copy it inside
  // CopyProperties if necessary.

  UPROPERTY(VisibleAnywhere)
  float FramesPerSecond;

  UPROPERTY(VisibleAnywhere)
  int32 PlatformTimeStamp;

  UPROPERTY(VisibleAnywhere)
  int32 GameTimeStamp = 0.0f;

  UPROPERTY(VisibleAnywhere)
  FTransform Transform;

  UPROPERTY(VisibleAnywhere)
  float ForwardSpeed = 0.0f;

  UPROPERTY(VisibleAnywhere)
  FVector Acceleration;

  UPROPERTY(VisibleAnywhere)
  float Throttle = 0.0f;

  UPROPERTY(VisibleAnywhere)
  float Steer = 0.0f;

  UPROPERTY(VisibleAnywhere)
  float Brake = 0.0f;

  UPROPERTY(VisibleAnywhere)
  bool bHandBrake = false;

  UPROPERTY(VisibleAnywhere)
  int32 CurrentGear;

  UPROPERTY(VisibleAnywhere)
  float SpeedLimit = -1.0f;

  UPROPERTY(VisibleAnywhere)
  ETrafficLightState TrafficLightState = ETrafficLightState::Green;

  UPROPERTY(VisibleAnywhere)
  ERouteCommand RouteCommand = ERouteCommand::LaneFollow;

  UPROPERTY(VisibleAnywhere)
  float CollisionIntensityCars = 0.0f;

  UPROPERTY(VisibleAnywhere)
  float CollisionIntensityPedestrians = 0.0f;

  UPROPERTY(VisibleAnywhere)
  float CollisionIntensityOther = 0.0f;

  UPROPERTY(VisibleAnywhere)
  float OtherLaneIntersectionFactor = 0.0f;

  UPROPERTY(VisibleAnywhere)
  float OffRoadIntersectionFactor = 0.0f;

  UPROPERTY(VisibleAnywhere)
  TArray<FCapturedImage> Images;
};
