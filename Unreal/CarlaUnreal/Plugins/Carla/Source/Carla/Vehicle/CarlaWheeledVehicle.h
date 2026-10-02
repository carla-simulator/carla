// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Vehicle/AckermannController.h"
#include "Carla/Vehicle/AckermannControllerSettings.h"
#include "Carla/Vehicle/CarlaWheeledVehicleState.h"
#include "Carla/Vehicle/VehicleAckermannControl.h"
#include "Carla/Vehicle/VehicleControl.h"
#include "Carla/Vehicle/VehicleLightState.h"
#include "Carla/Vehicle/VehicleInputPriority.h"
#include "Carla/Vehicle/VehiclePhysicsControl.h"
#include "Carla/Vehicle/VehicleTelemetryData.h"
#include "Carla/Vehicle/VehicleVelocityControl.h"
#include "Carla/Vehicle/VehicleAccelerationControl.h"
#include "Carla/Vehicle/WheeledVehicleMovementComponentNW.h"
#include "Carla/Vehicle/MovementComponents/BaseCarlaMovementComponent.h"

//-----CARSIM--------------------------------
#ifdef WITH_CARSIM
#include "CarSimMovementComponent.h"
#endif
//-------------------------------------------

#include <util/disable-ue4-macros.h>
#include <carla/rpc/VehicleFailureState.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "Components/SkeletalMeshComponent.h"
#include "WheeledVehiclePawn.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "FoliageInstancedStaticMeshComponent.h"
#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include <util/ue-header-guard-end.h>

#include <utility>

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
class CARLA_API ACarlaWheeledVehicle : public AWheeledVehiclePawn
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

  /// Get chaos wheeled vehicle movement component casted
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  UChaosWheeledVehicleMovementComponent* GetChaosWheeledVehicleMovementComponent() const
  {
    return Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());
  }
  
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

  /// Returns a snapshot of the vehicle's runtime telemetry: forward speed,
  /// last applied control inputs, engine RPM, current gear, and per-wheel
  /// lateral slip, longitudinal slip, and angular velocity sourced from
  /// the Chaos vehicle physics state.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FVehicleTelemetryData GetVehicleTelemetryData() const;

  FVector GetCenterOfMass(UChaosWheeledVehicleMovementComponent &VehicleMovComponent) const;

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FAckermannControllerSettings GetAckermannControllerSettings() const {
    return AckermannController.GetSettings();
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void RestoreVehiclePhysicsControl();

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FVehicleLightState GetVehicleLightState() const;

  void ApplyVehiclePhysicsControl(const FVehiclePhysicsControl &PhysicsControl);

  void SetCenterOfMass(UChaosWheeledVehicleMovementComponent &VehicleMovComponent, const FVehiclePhysicsControl &PhysicsControl);

  void ApplyAckermannControllerSettings(const FAckermannControllerSettings &AckermannControllerSettings) {
    return AckermannController.ApplySettings(AckermannControllerSettings);
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetSimulatePhysics(bool enabled);

  void SetVehicleLightState(const FVehicleLightState &LightState);

  void SetFailureState(const carla::rpc::VehicleFailureState &FailureState);

  UFUNCTION(BlueprintNativeEvent)
  bool IsTwoWheeledVehicle();
  virtual bool IsTwoWheeledVehicle_Implementation() {
    return false;
  }

  /// Two-wheeled vehicles (bikes/motorcycles) embed their rider as a
  /// SkeletalMeshComponent (named RiderMeshComponentName) meant to sit on
  /// VehicleMeshSeatSocketName. That positioning is still driven by
  /// Blueprint graph logic left over from the UE4-to-UE5 port, and it
  /// drifts the rider backward off the seat while the vehicle is moving.
  /// Rather than rewrite that graph, OnRiderBoneTransformsFinalized (see
  /// below) snaps the rider back onto the seat socket every frame, after
  /// this component's animation has finished evaluating for that frame --
  /// nothing runs afterwards that could undo it before render, regardless
  /// of what the Blueprint graph or its animation does. Only takes effect
  /// when IsTwoWheeledVehicle() is true and both component names resolve
  /// to real components on this actor.
  ///
  /// This does not fully fix the drift by itself -- the rider's own Anim
  /// Blueprint separately queries this same seat socket in its own Event
  /// BlueprintUpdateAnimation, and that read intermittently races ahead of
  /// the vehicle mesh's physics update for the frame (confirmed by logging
  /// both reads: on the frames it happens, the Blueprint's read exactly
  /// matches the PREVIOUS frame's real seat transform -- a one-frame-stale
  /// read on some frames but not others, which looks like a pop, not a
  /// continuous drift). See GetCachedVehicleSocketWorldTransform() below,
  /// which the Blueprint should read instead of querying the socket
  /// itself -- for this one and every other vehicle-mesh socket its Data
  /// Gather reads.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CARLA Wheeled Vehicle|Rider")
  FName RiderMeshComponentName = TEXT("SkeletalMesh");

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CARLA Wheeled Vehicle|Rider")
  FName VehicleMeshSeatSocketName = TEXT("Seat");

  /// Chaos suspension has small, real, high-frequency vertical jitter on
  /// the vehicle mesh every frame (visible as the rider's torso/hip
  /// bouncing, the seat popping the rider up) -- physically real, not a
  /// bug, but the seat-lock's hard snap transmits it 1:1 to the rider with
  /// zero smoothing. This is the speed (see FMath::FInterpTo) used to
  /// low-pass just the VERTICAL (Z) component of the cached base transform
  /// that GetCachedVehicleSocketWorldTransform() extrapolates from --
  /// purely cosmetic, applied only to what the rider reads, never to the
  /// vehicle's actual physics transform. X/Y and rotation are never
  /// smoothed (see OnRiderBoneTransformsFinalized's comment -- smoothing
  /// those trails a moving target by more the faster the vehicle goes,
  /// which looked exactly like the original slide bug). Higher = snappier
  /// (less smoothing); lower = smoother but more vertical lag.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CARLA Wheeled Vehicle|Rider")
  float RiderSeatSmoothingSpeed = 10.0f;

  /// Cached, extrapolated world transform of ANY socket on the vehicle
  /// mesh (Seat, HandlerLeftSocket, HandlerRightSocket, LeftPedalSocket,
  /// RightPedalSocket, ...), captured from OnRiderBoneTransformsFinalized
  /// (which fires late enough in the frame to reliably see this frame's
  /// real, physics-updated position -- confirmed by logging it against
  /// the vehicle's actual movement, always smooth, never stale).
  ///
  /// The rider's Anim Blueprint should read this instead of querying the
  /// vehicle mesh's sockets itself (directly, or via a vehicle-side
  /// variable computed that way) for EVERY seat/handler/pedal target used
  /// in its Data Gather: that independent read, from the Blueprint's own
  /// Event BlueprintUpdateAnimation, intermittently races ahead of the
  /// physics update and returns last frame's transform instead --
  /// confirmed first on the seat (fixed the torso pop) and then on the
  /// same pattern showing up on the handler/pedal sockets driving the
  /// arm/leg IK (hands/feet still popping to a stale pose after the torso
  /// fix). This trades that flickering race for a lag that's constant
  /// instead of intermittent, then extrapolates it away using the linear
  /// velocity it was cached with -- all vehicle-mesh sockets share that
  /// velocity, moving rigidly together, so one shared velocity/timestamp
  /// covers every socket looked up here.
  UFUNCTION(BlueprintPure, Category = "CARLA Wheeled Vehicle|Rider")
  FTransform GetCachedVehicleSocketWorldTransform(FName SocketName) const
  {
    // Extrapolating each socket's own cached translation in a straight
    // line (the first version of this function) ignores the vehicle's
    // ANGULAR velocity -- it yaws/pitches/rolls a little essentially all
    // the time (steering, suspension travel), even on flat road. Seat and
    // the handler/elbow sockets sit close to the mesh's own origin, so
    // that rotation barely displaces them and the straight-line
    // approximation looked like a full fix. Sockets far from the origin
    // -- the pedals, down at the sides -- get swept a lot further by the
    // same small rotation (lever arm), which is exactly the "still
    // sliding, constant, like before" the pedals kept showing after
    // torso/hands were confirmed fixed.
    // Fix: extrapolate the vehicle MESH's own rigid-body transform
    // (translation + rotation, both from its cached linear/angular
    // velocity) instead of each socket individually, then re-place every
    // socket at its fixed (skeleton-rigid) offset from that predicted
    // mesh transform -- this correctly sweeps far-out sockets through the
    // rotation instead of leaving them on their own straight line.
    //
    // Extrapolating from SmoothedVehicleMeshTransform (see
    // RiderSeatSmoothingSpeed's comment) here, not the raw
    // CachedVehicleMeshTransform -- that raw value is still used below,
    // unsmoothed, to compute each socket's RIGID offset (a static
    // skeleton relationship, smoothing it would just introduce a phantom
    // wobble between sockets instead of removing one).
    FTransform PredictedMeshTransform = SmoothedVehicleMeshTransform;
    if (const UWorld* World = GetWorld())
    {
      const float TimeSinceCache = World->GetTimeSeconds() - CachedVehicleTransformsTimeSeconds;
      // X/Y (world space) zeroed for the same reason as the linear
      // velocity's Z above -- Yaw is by definition rotation around the
      // WORLD Z axis (true regardless of the vehicle's current heading),
      // so keeping only the world-Z component of this angular velocity
      // isolates exactly the yaw-rate contribution this extrapolation was
      // meant to correct for (steering), while dropping the noisy
      // Pitch/Roll rates that would otherwise reintroduce the same
      // full-body rider bounce through every far-off socket (pedals,
      // handlers) via their own lever arm.
      FVector AngularVelocityRadians = FMath::DegreesToRadians(CachedVehicleWorldAngularVelocityDegrees);
      AngularVelocityRadians.X = 0.0f;
      AngularVelocityRadians.Y = 0.0f;
      const FQuat DeltaRotation = FQuat::MakeFromRotationVector(AngularVelocityRadians * TimeSinceCache);
      PredictedMeshTransform.SetRotation((DeltaRotation * PredictedMeshTransform.GetRotation()).GetNormalized());
      // Z zeroed out here -- CachedVehicleWorldVelocity comes straight from
      // physics (GetComponentVelocity(), unsmoothed) and its own vertical
      // component is exactly as noisy as the position jitter this function
      // is smoothing away above, so adding it back in would reintroduce
      // the same uniform full-body bounce through every socket at once.
      // Only X/Y matter for this extrapolation anyway (that's what the
      // one-frame lag at real driving speed needed); vertical lag of a
      // frame is imperceptible, unlike horizontal lag at speed.
      FVector VelocityForExtrapolation = CachedVehicleWorldVelocity;
      VelocityForExtrapolation.Z = 0.0f;
      PredictedMeshTransform.AddToTranslation(VelocityForExtrapolation * TimeSinceCache);
    }

    // LeftPedalSocket/RightPedalSocket sit on LeftPedalGeo/RightPedalGeo,
    // whose rotation isn't animated in the usual sense -- the AnimGraph's
    // Transform (Modify) Bone on that bone is Component Space + Replace
    // Existing, wired only on Rotation Y (Pitch) from Pedal Y (Roll/Yaw
    // left at literal 0) -- meaning the bone's ENTIRE component-space
    // rotation, every frame, IS exactly FRotator(+-PedalRotationAngle, 0, 0),
    // nothing else composed in. A first attempt here tried to track "how
    // much has the angle changed since the cache" and add that as a
    // delta -- that's extra arithmetic (and an extra chance to get a sign
    // or composition order wrong) for a value that doesn't need tracking
    // at all: PedalRotationAngle itself is never stale (it's our own
    // TickActor state, not read from a racing physics/Blueprint source),
    // so the pivot's rotation can be reconstructed outright from it
    // instead of extrapolated. Only the pivot's POSITION still needs the
    // cached-and-extrapolated vehicle motion above -- its own spin
    // doesn't move its origin, only the vehicle carrying it does.
    static const FName LeftPedalSocket(TEXT("LeftPedalSocket"));
    static const FName RightPedalSocket(TEXT("RightPedalSocket"));
    static const FName LeftPedalPivotBone(TEXT("LeftPedalGeo"));
    static const FName RightPedalPivotBone(TEXT("RightPedalGeo"));
    if (SocketName == LeftPedalSocket || SocketName == RightPedalSocket)
    {
      const FName PivotBoneName = (SocketName == LeftPedalSocket) ? LeftPedalPivotBone : RightPedalPivotBone;
      const FTransform* CachedSocket = CachedVehicleSocketTransforms.Find(SocketName);
      const FTransform* CachedPivot = CachedVehicleSocketTransforms.Find(PivotBoneName);
      if (CachedSocket && CachedPivot)
      {
        const FTransform SocketRelativeToPivot = CachedSocket->GetRelativeTransform(*CachedPivot);
        FTransform PredictedPivot = CachedPivot->GetRelativeTransform(CachedVehicleMeshTransform) * PredictedMeshTransform;
        // The AnimGraph feeds LeftPedalGeo the negative of Pedal Y and
        // RightPedalGeo Pedal Y directly (see the Pedals block).
        const float SignedPedalAngle = (SocketName == LeftPedalSocket) ? -PedalRotationAngle : PedalRotationAngle;
        const FQuat ComponentSpaceRotation(FRotator(SignedPedalAngle, 0.0f, 0.0f));
        PredictedPivot.SetRotation((ComponentSpaceRotation * PredictedMeshTransform.GetRotation()).GetNormalized());
        return SocketRelativeToPivot * PredictedPivot;
      }
    }

    if (const FTransform* CachedSocket = CachedVehicleSocketTransforms.Find(SocketName))
    {
      return CachedSocket->GetRelativeTransform(CachedVehicleMeshTransform) * PredictedMeshTransform;
    }
    return FTransform::Identity;
  }

  /// The rider mesh's rest pose isn't authored to sit flush with the
  /// socket's own local axes -- its component template ships with this
  /// exact relative rotation baked in (no translation), so the seat-lock
  /// composes it on top of the socket's world transform instead of using
  /// the bare socket transform, which twisted the pose.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CARLA Wheeled Vehicle|Rider")
  FRotator RiderSeatRelativeRotationOffset = FRotator(0.0f, -90.0f, 15.0f);

  /// Two-wheeled vehicles spin a decorative pedal/crank bone (in the
  /// vehicle mesh's own Anim Blueprint, unrelated to the rider) via a
  /// looping Timeline whose curve doesn't start and end at the same
  /// value -- every loop the Timeline restarts, the bone's rotation pops
  /// instantly back to the curve's start value instead of continuing
  /// smoothly, and the rider's foot IK (which targets a socket on that
  /// bone) pops along with it. Play Rate scales with vehicle speed, so
  /// the pops get more frequent the faster the vehicle goes, and their
  /// size is capped at the curve's fixed peak value -- both are the
  /// exact symptoms reported ("teleports", "there's a cap").
  /// GetPedalRotation() replaces that Timeline: an angle continuously
  /// accumulated from real vehicle speed and wrapped with Fmod, which
  /// never pops (359.9 degrees to 0.1 degrees is the same rotation, not a
  /// discontinuity) the way a Timeline restarting to an unrelated value
  /// does. Bind it directly to whatever currently reads the Timeline's
  /// output (Engine Speed -> Pedal Y) in the vehicle Blueprint's Anim
  /// Blueprint instead.
  UFUNCTION(BlueprintPure, Category = "CARLA Wheeled Vehicle|Rider")
  float GetPedalRotation() const
  {
    return PedalRotationAngle;
  }

  /// Degrees the pedal bone rotates per centimeter of real vehicle
  /// travel (see GetPedalRotation). Tune in-editor to match the desired
  /// pedaling cadence -- no recompile needed.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CARLA Wheeled Vehicle|Rider")
  float PedalRotationDegreesPerCm = 0.5f;

  void PrintROS2Message(const char* Message);

  /// @}
  // ===========================================================================
  /// @name Vehicle input control
  // ===========================================================================
  /// @{
public:

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ApplyVehicleControl(const FVehicleControl &Control, EVehicleInputPriority Priority);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ApplyVehicleAckermannControl(const FVehicleAckermannControl &AckermannControl, EVehicleInputPriority Priority);

  bool IsAckermannControlActive() const
  {
    return bAckermannControlActive;
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ActivateVelocityControl(const FVector &Velocity);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void DeactivateVelocityControl();

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ActivateAccelerationControl(const FVector &Acceleration);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void DeactivateAccelerationControl();

  /// Apply control from Autoware /control/command/control_cmd (acceleration [m/s^2] + steering)
  void ApplyVehicleAccelerationControl(float LongitudinalAccelerationMps2, float Steer, float SteerSpeed);

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
  virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
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

public:
  struct
  {
    EVehicleInputPriority Priority = EVehicleInputPriority::INVALID;
    FVehicleControl Control;
    FVehicleLightState LightState;
  }
  InputControl;

private:

  /// Current state of the vehicle controller (for debugging purposes).
  UPROPERTY(Category = "AI Controller", VisibleAnywhere)
  ECarlaWheeledVehicleState State = ECarlaWheeledVehicleState::UNKNOWN;

  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditAnywhere)
  TObjectPtr<UVehicleVelocityControl> VelocityControl;

  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditAnywhere)
  TObjectPtr<UVehicleAccelerationControl> AccelerationControl;


  FVehicleControl LastAppliedControl;
  FVehicleAckermannControl LastAppliedAckermannControl;
  FVehiclePhysicsControl LastPhysicsControl;

  bool bAckermannControlActive = false;
  FAckermannController AckermannController;

  float RolloverBehaviorForce = 0.35;
  int RolloverBehaviorTracker = 0;
  float RolloverFlagTime = 5.0f;

  carla::rpc::VehicleFailureState FailureState = carla::rpc::VehicleFailureState::None;

  // Light Defaults tool integration: loaded once in
  // ActivateVehicleLightComponents (BeginPlay), then reapplied by
  // SetVehicleLightState on every light-state change -- see
  // ULightDefaultsJsonUtils::ApplyVehicleLightsRuntimeState.
  bool bHasSavedVehicleLightDefault = false;
  float SavedVehicleLightIntensity = 1.0f;
  TMap<FString, float> SavedVehicleLightGroupIntensity;
  void ApplyVehicleLightDefaultsForCurrentState();

  // Resolved once (BeginPlay) rather than looked up by name every tick.
  // bRiderComponentsResolved distinguishes "looked up, none found" (a
  // non-two-wheeled vehicle, or bad names) from "not looked up yet", so
  // TickActor doesn't retry the lookup every frame for vehicles that
  // simply don't have a rider.
  UPROPERTY(Transient)
  TObjectPtr<USkeletalMeshComponent> RiderMeshComponent = nullptr;

  UPROPERTY(Transient)
  TObjectPtr<USkeletalMeshComponent> VehicleMeshForRiderSeat = nullptr;

  bool bRiderComponentsResolved = false;

  void ResolveRiderComponentsIfNeeded();

  // Bound (once, from ResolveRiderComponentsIfNeeded) to
  // RiderMeshComponent's OnBoneTransformsFinalized -- see the comment on
  // RiderMeshComponentName above for why this runs there and not from
  // TickActor.
  void OnRiderBoneTransformsFinalized();

  // Backing values for GetCachedVehicleSocketWorldTransform() above --
  // every vehicle-mesh socket's transform as of the last cache update, the
  // vehicle mesh's own transform and linear/angular velocity at that same
  // moment (to extrapolate the whole rigid body forward, not each socket
  // independently), and the world time it was cached at (to know how far
  // to extrapolate).
  TMap<FName, FTransform> CachedVehicleSocketTransforms;
  FTransform CachedVehicleMeshTransform;
  FVector CachedVehicleWorldVelocity = FVector::ZeroVector;
  FVector CachedVehicleWorldAngularVelocityDegrees = FVector::ZeroVector;
  float CachedVehicleTransformsTimeSeconds = 0.0f;

  // Backing value for RiderSeatSmoothingSpeed's low-pass filter -- see its
  // comment. Updated in OnRiderBoneTransformsFinalized, read (as the
  // extrapolation base) in GetCachedVehicleSocketWorldTransform().
  FTransform SmoothedVehicleMeshTransform;
  bool bSmoothedVehicleMeshTransformInitialized = false;

  // Backing value for GetPedalRotation() -- accumulated from real vehicle
  // speed each TickActor, wrapped to [0, 360) with Fmod. See the comment
  // on GetPedalRotation() in the public Rider section above.
  float PedalRotationAngle = 0.0f;

public:
  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditDefaultsOnly)
  float DetectionSize { 750.0f };

  UPROPERTY(Category = "CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly)
  FBox FoliageBoundingBox;

  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditAnywhere)
  TObjectPtr<UBoxComponent> VehicleBounds;

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

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void RecordDoorChange(const EVehicleDoor DoorIdx, const bool bIsOpen); 

  virtual FVector GetVelocity() const override;

  UFUNCTION()
  FPoseSnapshot GetWorldTransformedPose();

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

  /// Some vehicle Blueprints (Tesla, Mustang, DodgeCharger, ...) author real
  /// Spot/Point light components for their headlights instead of relying on
  /// emissive materials alone, but those components are not wrapped in a
  /// UCarlaLight -- so they never got UE5's bAutoActivate=false activation
  /// fix or the UE4-to-UE5 photometric intensity conversion (see CarlaLight.h).
  /// Applies both fixes directly to this actor's light components.
  void ActivateVehicleLightComponents();


  FTimerHandle TimerHandler;
public:
  float SpeedAnim { 0.0f };
  float RotationAnim { 0.0f };
  FPoseSnapshot WorldTransformedPose;

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  float GetSpeedAnim() const { return SpeedAnim; }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetSpeedAnim(float Speed) { SpeedAnim = Speed; }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  float GetRotationAnim() const { return RotationAnim; }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetRotationAnim(float Rotation) { RotationAnim = Rotation; }
};
