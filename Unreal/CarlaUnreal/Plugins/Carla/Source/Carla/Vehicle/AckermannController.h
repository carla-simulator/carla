// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Vehicle/AckermannControllerSettings.h"
#include "Vehicle/VehicleAckermannControl.h"
#include "Vehicle/VehicleControl.h"

// Forward declaration
class ACarlaWheeledVehicle;

class PID
{
  public:
    PID() = default;
    PID(float Kp, float Ki, float Kd): Kp(Kp), Ki(Ki), Kd(Kd) {}
    ~PID() = default;

    void SetTargetPoint(float Point)
    {
      SetPoint = Point;
    }

    float Run(float Input, float DeltaTime)
    {
      float Error = SetPoint - Input;

      Proportional = Kp * Error;
      Integral += Ki * Error * DeltaTime;
      // Avoid integral windup
      Integral = FMath::Clamp(Integral, MinOutput, MaxOutput);
      Derivative = (-Kd * (Input - LastInput)) / DeltaTime;

      float Out = Proportional + Integral + Derivative;
      Out = FMath::Clamp(Out, MinOutput, MaxOutput);

      // Keep track of the state
      LastError = Out;
      LastInput = Input;

      return Out;
    }

  void Reset()
  {
    Proportional = 0.0f;
    Integral = 0.0f;
    Integral = FMath::Clamp(Integral, MinOutput, MaxOutput);
    Derivative = 0.0f;

    LastError = 0.0f;
    LastInput = 0.0f;
  }

  public:
    float Kp = 0.0f;
    float Ki = 0.0f;
    float Kd = 0.0f;

  private:
    float SetPoint;

    // Out Limits
    float MinOutput = -1.0f;
    float MaxOutput = 1.0f;

    // Internal state.
    float Proportional = 0.0f;
    float Integral = 0.0f;
    float Derivative = 0.0f;

    float LastError = 0.0f;
    float LastInput = 0.0f;
};


class FAckermannController
{
public:

  FAckermannController() = default;
  ~FAckermannController();

public:

  FAckermannControllerSettings GetSettings() const;
  void ApplySettings(const FAckermannControllerSettings& Settings);

  void UpdateVehicleState(const ACarlaWheeledVehicle* Vehicle);
  void UpdateVehiclePhysics(const ACarlaWheeledVehicle* Vehicle);

  void SetTargetPoint(const FVehicleAckermannControl& AckermannControl);
  void Reset();

  void RunLoop(FVehicleControl& Control);

private:

  // Lateral Control
  void RunControlSteering();

  // Longitudinal Control
  bool RunControlFullStop();
  void RunControlReverse();
  void RunControlSpeed();
  void RunControlAcceleration();
  void UpdateVehicleControlCommand();

private:

  PID SpeedController = PID(0.15f, 0.0f, 0.25f);
  PID AccelerationController = PID(0.01f, 0.0f, 0.01f);

  FVehicleAckermannControl UserTargetPoint;

  // Target point after aplying restrictions
  float TargetSteer = 0.0;
  float TargetSteerSpeed = 0.0;
  float TargetSpeed = 0.0;
  float TargetAcceleration = 0.0;
  float TargetJerk = 0.0;

  // Restricitions parameters
  float MaxAccel = 3.0f;  // [m/s2]
  float MaxDecel = 8.0f;  // [m/s2]

  // Control parameters
  float Steer = 0.0f;
  float Throttle = 0.0f;
  float Brake = 0.0f;
  bool bReverse = false;

  // Internal control parameters
  float SpeedControlAccelDelta = 0.0f;
  float SpeedControlAccelTarget = 0.0f;

  float AccelControlPedalDelta = 0.0f;
  float AccelControlPedalTarget = 0.0f;

  // Simulation state
  float DeltaTime = 0.0f;                // [s]

  // Vehicle state
  float VehicleMaxSteering = 0.0f;       // [rad]

  float VehicleSteer = 0.0f;             // [rad]
  float VehicleSpeed = 0.0f;             // [m/s]
  float VehicleAcceleration = 0.0f;      // [m/s2]

  float LastVehicleSpeed = 0.0f;         // [m/s]
  float LastVehicleAcceleration = 0.0f;  // [m/s2]
};
