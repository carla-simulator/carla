// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "ChaosVehicleWheel.h"
#include <util/ue-header-guard-end.h>

#include "WheelPhysicsControl.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FWheelPhysicsControl
{
  GENERATED_BODY()

  FWheelPhysicsControl() :
    AxleType(EAxleType::Undefined),
    Offset(FVector(0.0f, 0.0f, 0.0f)),
    WheelRadius(1.0f),
    WheelWidth(1.0f),
    WheelMass(1),
    CorneringStiffness(1.0f),
    FrictionForceMultiplier(1.0f),
    SideSlipModifier(0.5f),
    SlipThreshold(1.0f),
    SkidThreshold(1.0f),
    MaxSteerAngle(60.0f),
    bAffectedBySteering(true),
    bAffectedByBrake(true),
    bAffectedByHandbrake(true),
    bAffectedByEngine(true),
    bABSEnabled(true),
    bTractionControlEnabled(true),
    MaxWheelspinRotation(1.0f),
    ExternalTorqueCombineMethod(ETorqueCombineMethod::None),
    LateralSlipGraph(FRichCurve()),
    SuspensionAxis(FVector(0.0f, 0.0f, 1.0f)),
    SuspensionForceOffset(1.0f),
    SuspensionMaxRaise(1.0f),
    SuspensionMaxDrop(1.0f),
    SuspensionDampingRatio(0.5f),
    WheelLoadRatio(0.5f),
    SpringRate(1.0f),
    SpringPreload(1.0f),
    SuspensionSmoothing(5.0f),
    RollbarScaling(0.0f),
    SweepShape(ESweepShape::Spherecast),
    SweepType(ESweepType::ComplexSweep),
    MaxBrakeTorque(1.0f),
    MaxHandBrakeTorque(1.0f),
    WheelIndex(0),
    Location(FVector(0.0f, 0.0f, 0.0f)),
    OldLocation(FVector(0.0f, 0.0f, 0.0f)),
    Velocity(FVector(0.0f, 0.0f, 0.0f))
  {

  }
  /** If left undefined then the bAffectedByEngine value is used, if defined then bAffectedByEngine is ignored and the differential setup on the vehicle defines which wheels get power from the engine */
  UPROPERTY(EditAnywhere, Category = Wheel)
  EAxleType AxleType;

  /**
   * If BoneName is specified, offset the wheel from the bone's location.
   * Otherwise this offsets the wheel from the vehicle's origin.
   */
  UPROPERTY(EditAnywhere, Category = Wheel)
  FVector Offset;

  /** Radius of the wheel */
  UPROPERTY(EditAnywhere, Category = Wheel, meta = (ClampMin = "0.01", UIMin = "0.01"))
  float WheelRadius;

  /** Width of the wheel */
  UPROPERTY(EditAnywhere, Category = Wheel, meta = (ClampMin = "0.01", UIMin = "0.01"))
  float WheelWidth;

  /** Mass of the wheel Kg */
  UPROPERTY(EditAnywhere, Category = Wheel, meta = (ClampMin = "0.01", UIMin = "0.01"))
  float WheelMass;

  /** Tyre Cornering Ability */
  UPROPERTY(EditAnywhere, Category = Wheel)
  float CorneringStiffness;

  /** Friction Force Multiplier */
  UPROPERTY(EditAnywhere, Category = Wheel)
  float FrictionForceMultiplier;

  /** Wheel Lateral Skid Grip Loss, lower number less grip on skid */
  UPROPERTY(EditAnywhere, Category = Wheel, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
  float SideSlipModifier;

  /** Wheel Longitudinal Slip Threshold */
  UPROPERTY(EditAnywhere, Category = Wheel, meta = (ClampMin = "0.0", UIMin = "0.0"))
  float SlipThreshold;

  /** Wheel Lateral Skid Threshold */
  UPROPERTY(EditAnywhere, Category = Wheel, meta = (ClampMin = "0.0", UIMin = "0.0"))
  float SkidThreshold;

  // steer angle in degrees for this wheel
  UPROPERTY(EditAnywhere, Category = WheelsSetup)
  float MaxSteerAngle;

  /** Whether steering should affect this wheel */
  UPROPERTY(EditAnywhere, Category = WheelsSetup)
  bool bAffectedBySteering;

  /** Whether brake should affect this wheel */
  UPROPERTY(EditAnywhere, Category = Wheel)
  bool bAffectedByBrake;

  /** Whether handbrake should affect this wheel */
  UPROPERTY(EditAnywhere, Category = Wheel)
  bool bAffectedByHandbrake;

  /** Whether engine should power this wheel */
  UPROPERTY(EditAnywhere, Category = Wheel)
  bool bAffectedByEngine;

  /** Advanced Braking System Enabled */
  UPROPERTY(EditAnywhere, Category = Wheel)
  bool bABSEnabled;

  /** Straight Line Traction Control Enabled */
  UPROPERTY(EditAnywhere, Category = Wheel)
  bool bTractionControlEnabled;

  /** Max Wheelspin rotation rad/sec */
  UPROPERTY(EditAnywhere, Category = Wheel)
  float MaxWheelspinRotation;

  /** Determines how the SetDriveTorque/SetBrakeTorque inputs are combined with the internal torques */
  UPROPERTY(EditAnywhere, Category = Wheel)
  ETorqueCombineMethod ExternalTorqueCombineMethod;

  UPROPERTY(EditAnywhere, Category = Setup)
  FRichCurve LateralSlipGraph;

  /** Local body direction in which where suspension forces are applied (typically along -Z-axis) */
  UPROPERTY(EditAnywhere, Category = Suspension)
  FVector SuspensionAxis;

  /** Vertical offset from where suspension forces are applied (along Z-axis) */
  UPROPERTY(EditAnywhere, Category = Suspension)
  FVector SuspensionForceOffset;

  /** How far the wheel can go above the resting position */
  UPROPERTY(EditAnywhere, Category = Suspension)
  float SuspensionMaxRaise;

  /** How far the wheel can drop below the resting position */
  UPROPERTY(EditAnywhere, Category = Suspension)
  float SuspensionMaxDrop;

  /** Suspension damping, larger value causes the suspension to come to rest faster [range 0 to 1] */
  UPROPERTY(EditAnywhere, Category = Suspension)
  float SuspensionDampingRatio;

  /**
   *	Amount wheel load effects wheel friction.
    At 0 wheel friction is completely independent of the loading on the wheel (This is artificial as it always assumes even balance between all wheels)
    At 1 wheel friction is based on the force pressing wheel into the ground. This is more realistic.
    Lower value cures lift off over-steer, generally makes vehicle easier to handle under extreme motions.
   */
  UPROPERTY(EditAnywhere, Category = Suspension, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
  float WheelLoadRatio;

  /** Spring Force (N/m) */
  UPROPERTY(EditAnywhere, Category = Suspension)
  float SpringRate;

  /** Spring Preload (N/m) */
  UPROPERTY(EditAnywhere, Category = Suspension)
  float SpringPreload;

  /** Smooth suspension [0-off, 10-max] - Warning might cause momentary visual inter-penetration of the wheel against objects/terrain */
  UPROPERTY(EditAnywhere, Category = Suspension, meta = (ClampMin = "0.0", UIMin = "0", ClampMax = "10.0", UIMax = "10"))
  int SuspensionSmoothing;

  /** Anti-roll effect */
  UPROPERTY(EditAnywhere, Category = Suspension, meta = (ClampMin = "0.0", UIMin = "0", ClampMax = "1.0", UIMax = "1"))
  float RollbarScaling;

  /** Wheel suspension trace type, defaults to ray trace */
  UPROPERTY(EditAnywhere, Category = Suspension)
  ESweepShape SweepShape;

  /** Whether wheel suspension considers simple, complex */
  UPROPERTY(EditAnywhere, Category = Suspension)
  ESweepType SweepType;

  /** max brake torque for this wheel (Nm) */
  UPROPERTY(EditAnywhere, Category = Brakes)
  float MaxBrakeTorque;

  /**
   *	Max handbrake brake torque for this wheel (Nm). A handbrake should have a stronger brake torque
   *	than the brake. This will be ignored for wheels that are not affected by the handbrake.
   */
  UPROPERTY(EditAnywhere, Category = Brakes)
  float MaxHandBrakeTorque;

  // Our index in the vehicle's (and setup's) wheels array
  UPROPERTY(transient)
  int32 WheelIndex;

  // Worldspace location of this wheel
  UPROPERTY(transient)
  FVector Location;

  // Worldspace location of this wheel last frame
  UPROPERTY(transient)
  FVector OldLocation;

  // Current velocity of the wheel center (change in location over time)
  UPROPERTY(Category = "Wheel Position", EditAnywhere, BlueprintReadWrite)
  FVector Velocity;

};
