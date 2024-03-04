// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "AckermannController.h"
#include "CarlaWheeledVehicle.h"

// =============================================================================
// -- Constructor and destructor -----------------------------------------------
// =============================================================================

FAckermannController::~FAckermannController() {}

// =============================================================================
// -- FAckermannController -----------------------------------------------------
// =============================================================================

FAckermannControllerSettings FAckermannController::GetSettings() const {
  FAckermannControllerSettings Settings;

  Settings.SpeedKp = SpeedController.Kp;
  Settings.SpeedKi = SpeedController.Ki;
  Settings.SpeedKd = SpeedController.Kd;

  Settings.AccelKp = AccelerationController.Kp;
  Settings.AccelKi = AccelerationController.Ki;
  Settings.AccelKd = AccelerationController.Kd;

  return Settings;
}

void FAckermannController::ApplySettings(const FAckermannControllerSettings& Settings) {
  SpeedController.Kp = Settings.SpeedKp;
  SpeedController.Ki = Settings.SpeedKi;
  SpeedController.Kd = Settings.SpeedKd;

  AccelerationController.Kp = Settings.AccelKp;
  AccelerationController.Ki = Settings.AccelKi;
  AccelerationController.Kd = Settings.AccelKd;
}

void FAckermannController::SetTargetPoint(const FVehicleAckermannControl& AckermannControl) {
  UserTargetPoint = AckermannControl;

  TargetSteer = FMath::Clamp(UserTargetPoint.Steer, -VehicleMaxSteering, VehicleMaxSteering);
  TargetSteerSpeed = FMath::Abs(UserTargetPoint.SteerSpeed);
  TargetSpeed = UserTargetPoint.Speed;
  TargetAcceleration = FMath::Abs(UserTargetPoint.Acceleration);
  TargetJerk = FMath::Abs(UserTargetPoint.Jerk);

}

void FAckermannController::Reset() {
  // Reset controllers
  SpeedController.Reset();
  AccelerationController.Reset();

  // Reset control parameters
  Steer = 0.0f;
  Throttle = 0.0f;
  Brake = 0.0f;
  bReverse = false;

  SpeedControlAccelDelta = 0.0f;
  SpeedControlAccelTarget = 0.0f;

  AccelControlPedalDelta = 0.0f;
  AccelControlPedalTarget = 0.0f;

  // Reset vehicle state
  VehicleSpeed = 0.0f;
  VehicleAcceleration = 0.0f;

  LastVehicleSpeed = 0.0f;
  LastVehicleAcceleration = 0.0f;
}

void FAckermannController::RunLoop(FVehicleControl& Control) {
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

  // Lateral Control
  RunControlSteering();

  // Longitudinal Control
  bool bStopped = RunControlFullStop();
  if (!bStopped) {
    RunControlReverse();
    RunControlSpeed();
    RunControlAcceleration();
    UpdateVehicleControlCommand();
  }

  // Update control command
  Control.Steer = Steer / VehicleMaxSteering;
  Control.Throttle = FMath::Clamp(Throttle, 0.0f, 1.0f);
  Control.Brake = FMath::Clamp(Brake, 0.0f, 1.0f);
  Control.bReverse = bReverse;
}

void FAckermannController::RunControlSteering() {
  if (FMath::Abs(TargetSteerSpeed) < 0.001) {
    Steer = TargetSteer;
  } else {
    float SteerDelta = TargetSteerSpeed * DeltaTime;
    if (FMath::Abs(TargetSteer - VehicleSteer) < SteerDelta) {
      Steer = TargetSteer;
    } else {
      float SteerDirection = (TargetSteer > VehicleSteer) ? 1.0f : -1.0f;
      Steer = VehicleSteer + SteerDirection * (TargetSteerSpeed * DeltaTime);
    }

  }
}

bool FAckermannController::RunControlFullStop() {
  // From this velocity on full brake is turned on
  float FullStopEpsilon = 0.1; //[m/s]

  if (FMath::Abs(VehicleSpeed) < FullStopEpsilon && FMath::Abs(UserTargetPoint.Speed) < FullStopEpsilon) {
    Brake = 1.0;
    Throttle = 0.0;
    return true;
  }
  return false;
}

void FAckermannController::RunControlReverse() {
  // From this position on it is allowed to switch to reverse gear
  float StandingStillEpsilon = 0.1;  // [m/s]

  if (FMath::Abs(VehicleSpeed) < StandingStillEpsilon) {
    // Standing still, change of driving direction allowed
    if (UserTargetPoint.Speed < 0) {
      // Change of driving direction to reverse.
      bReverse = true;
    } else if (UserTargetPoint.Speed >= 0) {
      // Change of driving direction to forward.
      bReverse = false;
    }
  } else {
    if (FMath::Sign(VehicleSpeed) * FMath::Sign(UserTargetPoint.Speed) == -1) {
      // Requested for change of driving direction.
      // First we have to come to full stop before changing driving direction
      TargetSpeed = 0.0;
    }
  }

}

void FAckermannController::RunControlSpeed() {
  SpeedController.SetTargetPoint(TargetSpeed);
  SpeedControlAccelDelta = SpeedController.Run(VehicleSpeed, DeltaTime);

  // Clipping borders
  float ClippingLowerBorder = -FMath::Abs(TargetAcceleration);
  float ClippingUpperBorder = FMath::Abs(TargetAcceleration);
  if (FMath::Abs(TargetAcceleration) < 0.0001f) {
    // Per definition of AckermannDrive: if zero, then use max value
    ClippingLowerBorder = -MaxDecel;
    ClippingUpperBorder = MaxAccel;
  }
  SpeedControlAccelTarget += SpeedControlAccelDelta;
  SpeedControlAccelTarget = FMath::Clamp(SpeedControlAccelTarget,
      ClippingLowerBorder, ClippingUpperBorder);
}

void FAckermannController::RunControlAcceleration() {
  AccelerationController.SetTargetPoint(SpeedControlAccelTarget);
  AccelControlPedalDelta = AccelerationController.Run(VehicleAcceleration, DeltaTime);

  // Clipping borders
  AccelControlPedalTarget += AccelControlPedalDelta;
  AccelControlPedalTarget = FMath::Clamp(AccelControlPedalTarget, -1.0f, 1.0f);

}

void FAckermannController::UpdateVehicleControlCommand() {

  if (AccelControlPedalTarget < 0.0f) {
    if (bReverse) {
      Throttle = FMath::Abs(AccelControlPedalTarget);
      Brake = 0.0f;
    } else {
      Throttle = 0.0f;
      Brake = FMath::Abs(AccelControlPedalTarget);
    }
  } else {
    if (bReverse) {
      Throttle = 0.0f;
      Brake = FMath::Abs(AccelControlPedalTarget);
    } else {
      Throttle = FMath::Abs(AccelControlPedalTarget);
      Brake = 0.0f;
    }
  }
}

void FAckermannController::UpdateVehicleState(const ACarlaWheeledVehicle* Vehicle) {
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

  LastVehicleSpeed = VehicleSpeed;
  LastVehicleAcceleration = VehicleAcceleration;

  // Update simulation state
  DeltaTime = Vehicle->GetWorld()->GetDeltaSeconds();

  // Update Vehicle state
  VehicleSteer = Vehicle->GetVehicleControl().Steer * VehicleMaxSteering;
  VehicleSpeed = Vehicle->GetVehicleForwardSpeed() / 100.0f;  // From cm/s to m/s
  float CurrentAcceleration = (VehicleSpeed - LastVehicleSpeed) / DeltaTime;
  // Apply an average filter for the acceleration.
  VehicleAcceleration = (4.0f*LastVehicleAcceleration + CurrentAcceleration) / 5.0f;
}

void FAckermannController::UpdateVehiclePhysics(const ACarlaWheeledVehicle* Vehicle) {
  VehicleMaxSteering = FMath::DegreesToRadians(Vehicle->GetMaximumSteerAngle());
}
