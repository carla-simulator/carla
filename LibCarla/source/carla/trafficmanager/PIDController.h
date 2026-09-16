// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <algorithm>
#include <cmath>

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/DataStructures.h"

namespace carla {
namespace traffic_manager {

namespace chr = std::chrono;

using namespace constants::PID;

using TimeInstance = chr::time_point<chr::system_clock, chr::nanoseconds>;

namespace PID {

/// Speed the longitudinal loop is asked to reach on this step, which closes on
/// @a target_velocity at COMFORT_ACCELERATION instead of stepping to it. See
/// COMFORT_ACCELERATION: the target itself steps -- the landmark that holds a
/// vehicle at 10-15 km/h through a junction leaves the path buffer in a single
/// frame -- and a step of more than 7 per cent puts the throttle on its bound
/// in one frame.
///
/// @a emergency_stop says the vehicle was held rather than driven on the
/// previous step, so @a previous_reference says nothing about the speed it is
/// at now. A stale reference from any other cause needs no such guard: the
/// bounds below pull it back to within one ramp step of the vehicle in a
/// single cycle.
[[nodiscard]] inline float ShapeReferenceVelocity(
    const float previous_reference,
    const float vehicle_speed,
    const float target_velocity,
    const float control_dt,
    const bool emergency_stop) {

  float reference{emergency_stop ? vehicle_speed : previous_reference};

  // The ramp uses the same period bounds as the controller, so a frame long
  // enough to be clamped there cannot advance the reference further than the
  // loop is compensated for.
  reference = std::min(
      reference + COMFORT_ACCELERATION * std::clamp(control_dt, MIN_CONTROL_DT, MAX_CONTROL_DT),
      target_velocity);
  // Never under the current speed, so the ramp cannot brake a vehicle that is
  // already faster than it; never over the target, so it cannot cancel a
  // deceleration the target is asking for.
  reference = std::max(reference, std::min(vehicle_speed, target_velocity));
  // How far the reference may lead the vehicle is bounded, so one that cannot
  // follow the ramp still gets full throttle and no more.
  return std::min(reference, vehicle_speed + REFERENCE_LEAD_FRACTION * target_velocity);
}

/// Velocity error the longitudinal loop acts on, normalised by the target.
///
/// The target reaches exactly zero behind a stopped vehicle, so a stopped
/// vehicle behind one divides zero by zero: the resulting NaN survives the
/// throttle branch, is stored as the step's deviation, and the next step's
/// derivative term turns it into a NaN steering command.
[[nodiscard]] inline float RelativeVelocityDeviation(
    const float reference_velocity,
    const float vehicle_speed,
    const float target_velocity) {

  return (reference_velocity - vehicle_speed) /
         std::max(target_velocity, constants::MotionPlan::EPSILON_RELATIVE_SPEED);
}

/// Conditions the controller output before it is commanded to the vehicle:
/// ramps the throttle in while pulling away from a standstill, then holds the
/// previous command while the new one is within a deadband of it, so the
/// per-frame dither around the trim point does not reach the actuators. A zero
/// pedal demand is always honoured, so the pedals rest exactly at zero and
/// nothing here delays a deceleration.
inline void SmoothActuation(
    const StateEntry &previous_state,
    const float control_dt,
    const float vehicle_speed,
    ActuationSignal &actuation_signal) {

  // What a steering deadband can hide is a lateral acceleration, which grows
  // with the square of speed, so the band is scaled down with speed like the
  // steering envelope.
  const float speed_ratio{
      STEER_DEADBAND_REF_SPEED / std::max(vehicle_speed, STEER_DEADBAND_REF_SPEED)};
  const float steer_deadband{STEER_DEADBAND * speed_ratio * speed_ratio};

  if (vehicle_speed < LAUNCH_RAMP_SPEED) {
    const float dt{std::clamp(control_dt, MIN_CONTROL_DT, MAX_CONTROL_DT)};
    actuation_signal.throttle = std::min(
        actuation_signal.throttle,
        previous_state.throttle + MAX_LAUNCH_THROTTLE_RISE_RATE * dt);
  }

  if (actuation_signal.throttle > 0.0f &&
      std::abs(actuation_signal.throttle - previous_state.throttle) < THROTTLE_DEADBAND) {
    actuation_signal.throttle = previous_state.throttle;
  }
  if (actuation_signal.brake > 0.0f &&
      std::abs(actuation_signal.brake - previous_state.brake) < BRAKE_DEADBAND) {
    actuation_signal.brake = previous_state.brake;
  }
  if (std::abs(actuation_signal.steer - previous_state.steer) < steer_deadband) {
    actuation_signal.steer = previous_state.steer;
  }
}

/// This function calculates the actuation signals based on the resent state
/// change of the vehicle to minimize PID error.
///
/// control_dt is the measured simulation-time period between the two states.
/// The gains are tuned at the nominal DT (sync-mode 0.05 s); in asynchronous
/// mode the real period is one server frame, which under render load (e.g. a
/// path-traced rt_lens sensor) stretches to 2-4x nominal. Left uncompensated
/// that halves the loop's phase margin and the lateral PID goes from damped
/// to divergent weaving (measured on Town12: steer_std 0.11 at 19 fps ->
/// 0.71 with full-lock swings at 11 fps), which then trips the stuck/K-turn
/// recovery into alternating full-lock "jerking".
/// lateral_pursuit_steer replaces the linearized proportional term of the
/// lateral loop: the caller computes the geometric pure-pursuit command
/// atan(wheelbase * margin * 2 sin(alpha) / d) / wheel_lock from the
/// vehicle's own steering geometry, which is exact for every wheelbase and
/// lock where a fixed linear gain is only correct for the car-class fleet it
/// was tuned on. The integral term keeps its original form and units.
/// lateral_damping_scale rescales the derivative term with the loop
/// bandwidth (pursuit-distance schedule x steering-authority correction).
/// steer_authority_correction (>= 1) scales the STEER_LIMIT_GAIN envelope:
/// the envelope is a physical curvature/lateral-acceleration guard, so it
/// must cap curvature, not raw normalized command.
inline ActuationSignal RunStep(StateEntry present_state,
                        StateEntry previous_state,
                        const std::vector<float> &longitudinal_parameters,
                        const std::vector<float> &lateral_parameters,
                        const float vehicle_speed,
                        const float control_dt = DT,
                        const float lateral_pursuit_steer = 0.0f,
                        const float lateral_damping_scale = 1.0f,
                        const float steer_authority_correction = 1.0f) {

  const float dt = std::max(MIN_CONTROL_DT, std::min(control_dt, MAX_CONTROL_DT));
  const float inv_dt = 1.0f / dt;
  // Sample-rate compensation for the lateral loop. The dominant effect of a
  // longer control period is added loop delay: the crossover frequency the
  // gains were tuned for no longer fits inside the slower sampling, so the
  // loop must be slowed down proportionally. Never scale gains UP for fast
  // ticks -- the tuning point is the ceiling.
  const float gain_scale = std::min(1.0f, DT * inv_dt);

  // Longitudinal PID calculation. The speed loop is first-order and far less
  // delay-sensitive than the lateral loop, so only the integral/derivative
  // discretization uses the measured period; the proportional gain stays.
  const float expr_v =
      longitudinal_parameters[0] * present_state.velocity_deviation +
      longitudinal_parameters[1] * (present_state.velocity_deviation + previous_state.velocity_deviation) * dt +
      longitudinal_parameters[2] * (present_state.velocity_deviation - previous_state.velocity_deviation) * inv_dt;

  float throttle;
  float brake;

  if (expr_v > 0.0f) {
    throttle = std::min(expr_v, MAX_THROTTLE);
    brake = 0.0f;
  } else {
    throttle = 0.0f;
    brake = std::min(std::abs(expr_v), MAX_BRAKE);
  }

  // Lateral PID calculation. The derivative input is clamped to the maximum
  // physically plausible heading-error rate: a vehicle yaws well under
  // 180 deg/s (MAX_DEVIATION_RATE in normalised units), so rates above that
  // can only come from the target point jumping discontinuously (lane change
  // start, buffer reseed). An unclamped derivative turns such jumps into a
  // one-tick full-lock steer spike that swings the vehicle across the lane.
  const float max_deviation_delta = MAX_DEVIATION_RATE * dt;
  float deviation_delta = present_state.angular_deviation - previous_state.angular_deviation;
  deviation_delta = std::max(-max_deviation_delta, std::min(deviation_delta, max_deviation_delta));
  float steer = gain_scale * (
      lateral_pursuit_steer +
      lateral_parameters[1] * (present_state.angular_deviation + previous_state.angular_deviation) * dt +
      lateral_damping_scale * lateral_parameters[2] * deviation_delta * inv_dt);

  // Steering slew limit, applied as a rate so the physical steering speed is
  // independent of the tick rate (a fixed per-tick step would triple the
  // allowed rate at 60 fps and strangle it at 7 fps).
  const float max_steering_diff = MAX_STEERING_RATE * dt;
  steer = std::max(previous_state.steer - max_steering_diff, std::min(steer, previous_state.steer + max_steering_diff));
  steer = std::max(-MAX_STEERING, std::min(steer, MAX_STEERING));

  // Speed-scaled steering envelope (lateral-acceleration cap). See
  // STEER_LIMIT_GAIN in Constants.h. The cap is a physical curvature bound,
  // so it scales with the vehicle's steering-authority correction: without
  // it, long vehicles saturate below the curvature a junction turn needs
  // (measured: 45% of truck junction ticks pinned at the car-anchored limit)
  // and sweep into the adjacent lane.
  const float speed_sq = std::max(vehicle_speed * vehicle_speed, 1.0f);
  const float steer_limit = std::min(
      MAX_STEERING, steer_authority_correction * STEER_LIMIT_GAIN / speed_sq);
  steer = std::max(-steer_limit, std::min(steer, steer_limit));

  return ActuationSignal{throttle, brake, steer};
}

} // namespace PID
} // namespace traffic_manager
} // namespace carla
