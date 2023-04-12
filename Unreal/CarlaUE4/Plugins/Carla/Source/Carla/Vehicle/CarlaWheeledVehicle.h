// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "WheeledVehicle.h"

#include "Vehicle/AckermannController.h"
#include "Vehicle/AckermannControllerSettings.h"
#include "Vehicle/CarlaWheeledVehicleState.h"
#include "Vehicle/VehicleAckermannControl.h"
#include "Vehicle/VehicleControl.h"
#include "Vehicle/VehicleLightState.h"
#include "Vehicle/VehicleInputPriority.h"
#include "Vehicle/VehiclePhysicsControl.h"
#include "VehicleVelocityControl.h"
#include "WheeledVehicleMovementComponent4W.h"
#include "WheeledVehicleMovementComponentNW.h"
#include "VehicleAnimInstance.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "MovementComponents/BaseCarlaMovementComponent.h"


#include "FoliageInstancedStaticMeshComponent.h"
#include "CoreMinimal.h"

//-----CARSIM--------------------------------
#ifdef WITH_CARSIM
#include "CarSimMovementComponent.h"
#endif
//-------------------------------------------

#include <utility>

#include "carla/rpc/VehicleFailureState.h"
#include "CarlaWheeledVehicle.generated.h"

class UBoxComponent;

UENUM()
enum class EVehicleWheelLocation : uint8 {

  FL_Wheel = 0,
  FR_Wheel = 1,
  BL_Wheel = 2,
  BR_Wheel = 3,
  ML_Wheel = 4,
  MR_Wheel = 5,
  //Use for bikes and bicycles
  Front_Wheel = 0,
  Back_Wheel = 1,
};

/// Type of door to open/close
// When adding new door types, make sure that All is the last one.
UENUM(BlueprintType)
enum class EVehicleDoor : uint8 {
  FL = 0,
  FR = 1,
  RL = 2,
  RR = 3,
  Hood = 4,
  Trunk = 5,
  All = 6
};

/// Base class for CARLA wheeled vehicles.
UCLASS()
class CARLA_API ACarlaWheeledVehicle : public AWheeledVehicle
{
  GENERATED_BODY()

  // ===========================================================================
  /// @name Constructor and destructor
  // ===========================================================================
  /// @{
public:

  ACarlaWheeledVehicle(const FObjectInitializer &ObjectInitializer);

  ~ACarlaWheeledVehicle();

  /// @}
  // ===========================================================================
  /// @name Get functions
  // ===========================================================================
  /// @{
public:

  /// Vehicle control currently applied to this vehicle.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  const FVehicleControl &GetVehicleControl() const
  {
    return LastAppliedControl;
  }

  /// Vehicle Ackermann control currently applied to this vehicle.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  const FVehicleAckermannControl &GetVehicleAckermannControl() const
  {
    return LastAppliedAckermannControl;
  }

  /// Transform of the vehicle. Location is shifted so it matches center of the
  /// vehicle bounds rather than the actor's location.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FTransform GetVehicleTransform() const
  {
    return GetActorTransform();
  }

  /// Forward speed in cm/s. Might be negative if goes backwards.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  float GetVehicleForwardSpeed() const;

  /// Orientation vector of the vehicle, pointing forward.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FVector GetVehicleOrientation() const;

  /// Active gear of the vehicle.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  int32 GetVehicleCurrentGear() const;

  /// Transform of the vehicle's bounding box relative to the vehicle.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FTransform GetVehicleBoundingBoxTransform() const;

  /// Extent of the vehicle's bounding box.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FVector GetVehicleBoundingBoxExtent() const;

  /// Get vehicle's bounding box component.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  UBoxComponent *GetVehicleBoundingBox() const
  {
    return VehicleBounds;
  }

  /// Get the maximum angle at which the front wheel can steer.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  float GetMaximumSteerAngle() const;

  /// @}
  // ===========================================================================
  /// @name AI debug state
  // ===========================================================================
  /// @{
public:

  /// @todo This function should be private to AWheeledVehicleAIController.
  void SetAIVehicleState(ECarlaWheeledVehicleState InState)
  {
    State = InState;
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  ECarlaWheeledVehicleState GetAIVehicleState() const
  {
    return State;
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FVehiclePhysicsControl GetVehiclePhysicsControl() const;

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FAckermannControllerSettings GetAckermannControllerSettings() const {
    return AckermannController.GetSettings();
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void RestoreVehiclePhysicsControl();

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FVehicleLightState GetVehicleLightState() const;

  void ApplyVehiclePhysicsControl(const FVehiclePhysicsControl &PhysicsControl);

  void ApplyAckermannControllerSettings(const FAckermannControllerSettings &AckermannControllerSettings) {
    return AckermannController.ApplySettings(AckermannControllerSettings);
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetSimulatePhysics(bool enabled);

  void SetWheelCollision(UWheeledVehicleMovementComponent4W *Vehicle4W, const FVehiclePhysicsControl &PhysicsControl);

  void SetWheelCollisionNW(UWheeledVehicleMovementComponentNW *VehicleNW, const FVehiclePhysicsControl &PhysicsControl);

  void SetVehicleLightState(const FVehicleLightState &LightState);

  void SetFailureState(const carla::rpc::VehicleFailureState &FailureState);

  UFUNCTION(BlueprintNativeEvent)
  bool IsTwoWheeledVehicle();
  virtual bool IsTwoWheeledVehicle_Implementation() {
    return false;
  }

  /// @}
  // ===========================================================================
  /// @name Vehicle input control
  // ===========================================================================
  /// @{
public:

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ApplyVehicleControl(const FVehicleControl &Control, EVehicleInputPriority Priority)
  {
    if (bAckermannControlActive) {
      AckermannController.Reset();
    }
    bAckermannControlActive = false;

    if (InputControl.Priority <= Priority)
    {
      InputControl.Control = Control;
      InputControl.Priority = Priority;
    }
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ApplyVehicleAckermannControl(const FVehicleAckermannControl &AckermannControl, EVehicleInputPriority Priority)
  {
    bAckermannControlActive = true;
    LastAppliedAckermannControl = AckermannControl;
    AckermannController.SetTargetPoint(AckermannControl);
  }

  bool IsAckermannControlActive() const
  {
    return bAckermannControlActive;
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ActivateVelocityControl(const FVector &Velocity);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void DeactivateVelocityControl();

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ShowDebugTelemetry(bool Enabled);

  /// @todo This function should be private to AWheeledVehicleAIController.
  void FlushVehicleControl();

  /// @}
  // ===========================================================================
  /// @name DEPRECATED Set functions
  // ===========================================================================
  /// @{
public:

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetThrottleInput(float Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetSteeringInput(float Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetBrakeInput(float Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetReverse(bool Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ToggleReverse()
  {
    SetReverse(!LastAppliedControl.bReverse);
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetHandbrakeInput(bool Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void HoldHandbrake()
  {
    SetHandbrakeInput(true);
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ReleaseHandbrake()
  {
    SetHandbrakeInput(false);
  }

  TArray<float> GetWheelsFrictionScale();

  void SetWheelsFrictionScale(TArray<float> &WheelsFrictionScale);

  void SetCarlaMovementComponent(UBaseCarlaMovementComponent* MoementComponent);

  template<typename T = UBaseCarlaMovementComponent>
  T* GetCarlaMovementComponent() const
  {
    return Cast<T>(BaseMovementComponent);
  }

  /// @}
  // ===========================================================================
  /// @name Overriden from AActor
  // ===========================================================================
  /// @{

protected:

  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

  UFUNCTION(BlueprintImplementableEvent)
  void RefreshLightState(const FVehicleLightState &VehicleLightState);

  UFUNCTION(BlueprintCallable, CallInEditor)
  void AdjustVehicleBounds();

  UPROPERTY(Category="Door Animation", EditAnywhere, BlueprintReadWrite)
  TArray<FName> ConstraintComponentNames;

  UPROPERTY(Category="Door Animation", EditAnywhere, BlueprintReadWrite)
  float DoorOpenStrength = 100.0f;

  UFUNCTION(BlueprintCallable, CallInEditor)
  void ResetConstraints();

private:

  /// Current state of the vehicle controller (for debugging purposes).
  UPROPERTY(Category = "AI Controller", VisibleAnywhere)
  ECarlaWheeledVehicleState State = ECarlaWheeledVehicleState::UNKNOWN;

  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditAnywhere)
  UVehicleVelocityControl* VelocityControl;

  struct
  {
    EVehicleInputPriority Priority = EVehicleInputPriority::INVALID;
    FVehicleControl Control;
    FVehicleLightState LightState;
  }
  InputControl;

  FVehicleControl LastAppliedControl;
  FVehicleAckermannControl LastAppliedAckermannControl;
  FVehiclePhysicsControl LastPhysicsControl;

  bool bAckermannControlActive = false;
  FAckermannController AckermannController;

  float RolloverBehaviorForce = 0.35;
  int RolloverBehaviorTracker = 0;
  float RolloverFlagTime = 5.0f;

  carla::rpc::VehicleFailureState FailureState = carla::rpc::VehicleFailureState::None;

public:
  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditDefaultsOnly)
  float DetectionSize { 750.0f };

  UPROPERTY(Category = "CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly)
  FBox FoliageBoundingBox;

  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditAnywhere)
  UBoxComponent *VehicleBounds;

  UFUNCTION()
  FBox GetDetectionBox() const;

  UFUNCTION()
  float GetDetectionSize() const;

  UFUNCTION()
  void UpdateDetectionBox();

  UFUNCTION()
  const TArray<int32> GetFoliageInstancesCloseToVehicle(const UInstancedStaticMeshComponent* Component) const;

  UFUNCTION(BlueprintCallable)
  void DrawFoliageBoundingBox() const;

  UFUNCTION()
  FBoxSphereBounds GetBoxSphereBounds() const;

  UFUNCTION()
  bool IsInVehicleRange(const FVector& Location) const;

  /// Set the rotation of the car wheels indicated by the user
  /// 0 = FL_VehicleWheel, 1 = FR_VehicleWheel, 2 = BL_VehicleWheel, 3 = BR_VehicleWheel
  /// NOTE : This is purely aesthetic. It will not modify the physics of the car in any way
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetWheelSteerDirection(EVehicleWheelLocation WheelLocation, float AngleInDeg);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  float GetWheelSteerAngle(EVehicleWheelLocation WheelLocation);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void OpenDoor(const EVehicleDoor DoorIdx);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void CloseDoor(const EVehicleDoor DoorIdx);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void OpenDoorPhys(const EVehicleDoor DoorIdx);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void CloseDoorPhys(const EVehicleDoor DoorIdx);

  virtual FVector GetVelocity() const override;

//-----CARSIM--------------------------------
  UPROPERTY(Category="CARLA Wheeled Vehicle", EditAnywhere)
  float CarSimOriginOffset = 150.f;
//-------------------------------------------

  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere)
  bool bIsNWVehicle = false;

  void SetRolloverFlag();

  carla::rpc::VehicleFailureState GetFailureState() const;

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  static FRotator GetPhysicsConstraintAngle(UPhysicsConstraintComponent* Component);
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  static void SetPhysicsConstraintAngle(
      UPhysicsConstraintComponent*Component, const FRotator &NewAngle);
 
private:

  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere)
  bool bPhysicsEnabled = true;

  // Small workarround to allow optional CarSim plugin usage
  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  UBaseCarlaMovementComponent * BaseMovementComponent = nullptr;

  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  TArray<UPhysicsConstraintComponent*> ConstraintsComponents;

  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  TMap<UPhysicsConstraintComponent*, UPrimitiveComponent*> ConstraintDoor;

  // container of the initial transform of the door, used to reset its position
  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  TMap<UPrimitiveComponent*, FTransform> DoorComponentsTransform;

  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  TMap<UPrimitiveComponent*, UPhysicsConstraintComponent*> CollisionDisableConstraints;

  /// Rollovers tend to have too much angular velocity, resulting in the vehicle doing a full 360º flip.
  /// This function progressively reduces the vehicle's angular velocity so that it ends up upside down instead.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ApplyRolloverBehavior();

  void CheckRollover(const float roll, const std::pair<float, float> threshold_roll);

  void AddReferenceToManager();
  void RemoveReferenceToManager();


  FTimerHandle TimerHandler;
public:
  float SpeedAnim { 0.0f };
  float RotationAnim { 0.0f };

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  float GetSpeedAnim() const { return SpeedAnim; }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetSpeedAnim(float Speed) { SpeedAnim = Speed; }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  float GetRotationAnim() const { return RotationAnim; }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetRotationAnim(float Rotation) { RotationAnim = Rotation; }
};
