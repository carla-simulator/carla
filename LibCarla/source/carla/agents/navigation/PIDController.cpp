// Copyright (c) 2018-2020 CVC.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/agents/navigation/PIDController.h"

#include "carla/agents/navigation/Misc.h"
#include "carla/client/Vehicle.h"
#include "carla/client/Waypoint.h"
#include "carla/geom/Location.h"
#include "carla/geom/Transform.h"
#include "carla/geom/Vector3D.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace carla {
namespace agents {
namespace navigation {

namespace {

constexpr std::size_t kErrorBufferMaxLen = 10;

// Pop oldest entries until the deque is at most max_len long. Mirrors the
// behaviour of a Python `deque(maxlen=N)`.
template <typename T>
void TrimDeque(std::deque<T> &q, std::size_t max_len) {
  while (q.size() > max_len) {
    q.pop_front();
  }
}

} // namespace

// =========================================================================
// PIDLongitudinalController
// =========================================================================

PIDLongitudinalController::PIDLongitudinalController(
    SharedPtr<client::Vehicle> vehicle,
    float K_P, float K_I, float K_D, float dt)
  : _vehicle(std::move(vehicle)),
    _k_p(K_P),
    _k_i(K_I),
    _k_d(K_D),
    _dt(dt) {}

float PIDLongitudinalController::RunStep(float target_speed_kmh, bool debug) {
  const float current_speed_kmh = GetSpeed(_vehicle);
  if (debug) {
    std::cout << "Current speed = " << current_speed_kmh << '\n';
  }
  return PidControl(target_speed_kmh, current_speed_kmh);
}

float PIDLongitudinalController::PidControl(
    float target_speed_kmh, float current_speed_kmh) {
  const float error = target_speed_kmh - current_speed_kmh;
  _error_buffer.push_back(error);
  TrimDeque(_error_buffer, kErrorBufferMaxLen);

  float de = 0.0f;
  float ie = 0.0f;
  if (_error_buffer.size() >= 2) {
    de = (_error_buffer.back() - _error_buffer[_error_buffer.size() - 2]) / _dt;
    float sum = 0.0f;
    for (float e : _error_buffer) sum += e;
    ie = sum * _dt;
  }

  const float u = (_k_p * error) + (_k_d * de) + (_k_i * ie);
  return std::clamp(u, -1.0f, 1.0f);
}

void PIDLongitudinalController::ChangeParameters(
    float K_P, float K_I, float K_D, float dt) {
  _k_p = K_P;
  _k_i = K_I;
  _k_d = K_D;
  _dt  = dt;
}

// =========================================================================
// PIDLateralController
// =========================================================================

PIDLateralController::PIDLateralController(
    SharedPtr<client::Vehicle> vehicle,
    float offset,
    float K_P, float K_I, float K_D, float dt)
  : _vehicle(std::move(vehicle)),
    _k_p(K_P),
    _k_i(K_I),
    _k_d(K_D),
    _dt(dt),
    _offset(offset) {}

float PIDLateralController::RunStep(SharedPtr<client::Waypoint> waypoint) {
  const auto vehicle_transform = _vehicle->GetTransform();

  // Vehicle forward vector projected onto the XY plane.
  const auto fwd = vehicle_transform.GetForwardVector();
  const float vx = fwd.x;
  const float vy = fwd.y;

  // Optionally laterally offset the waypoint along its right vector.
  geom::Location w_loc;
  if (_offset != 0.0f) {
    const auto w_tran = waypoint->GetTransform();
    const auto r_vec  = w_tran.GetRightVector();
    w_loc = geom::Location(
        w_tran.location.x + _offset * r_vec.x,
        w_tran.location.y + _offset * r_vec.y,
        w_tran.location.z);
  } else {
    w_loc = waypoint->GetTransform().location;
  }

  const float wx = w_loc.x - vehicle_transform.location.x;
  const float wy = w_loc.y - vehicle_transform.location.y;

  // Python computes `np.linalg.norm(w) * np.linalg.norm(v)`. v_z is 0 by
  // construction so the 2-D length is enough for both.
  const float w_len = std::sqrt(wx * wx + wy * wy);
  const float v_len = std::sqrt(vx * vx + vy * vy);
  const float wv_linalg = w_len * v_len;

  float dot;
  if (wv_linalg == 0.0f) {
    // Matches the Python `_dot = 1` early-out (degenerate inputs).
    dot = 1.0f;
  } else {
    const float cos_theta = std::clamp((wx * vx + wy * vy) / wv_linalg,
                                       -1.0f, 1.0f);
    dot = std::acos(cos_theta);
  }

  // np.cross(v, w)[2] for 2-D vectors lifted into 3-D is just vx*wy - vy*wx.
  const float cross_z = vx * wy - vy * wx;
  if (cross_z < 0.0f) {
    dot *= -1.0f;
  }

  _e_buffer.push_back(dot);
  TrimDeque(_e_buffer, kErrorBufferMaxLen);

  float de = 0.0f;
  float ie = 0.0f;
  if (_e_buffer.size() >= 2) {
    de = (_e_buffer.back() - _e_buffer[_e_buffer.size() - 2]) / _dt;
    float sum = 0.0f;
    for (float e : _e_buffer) sum += e;
    ie = sum * _dt;
  }

  const float u = (_k_p * dot) + (_k_d * de) + (_k_i * ie);
  return std::clamp(u, -1.0f, 1.0f);
}

void PIDLateralController::SetOffset(float offset) {
  _offset = offset;
}

void PIDLateralController::ChangeParameters(
    float K_P, float K_I, float K_D, float dt) {
  _k_p = K_P;
  _k_i = K_I;
  _k_d = K_D;
  _dt  = dt;
}

// =========================================================================
// VehiclePIDController
// =========================================================================

VehiclePIDController::VehiclePIDController(
    SharedPtr<client::Vehicle> vehicle,
    const LateralArgs &lateral,
    const LongitudinalArgs &longitudinal,
    float offset,
    float max_throttle,
    float max_brake,
    float max_steering)
  : _vehicle(vehicle),
    _max_throttle(max_throttle),
    _max_brake(max_brake),
    _max_steering(max_steering),
    _past_steering(vehicle ? vehicle->GetControl().steer : 0.0f),
    _lon_controller(vehicle, longitudinal.K_P, longitudinal.K_I,
                    longitudinal.K_D, longitudinal.dt),
    _lat_controller(vehicle, offset, lateral.K_P, lateral.K_I,
                    lateral.K_D, lateral.dt) {}

rpc::VehicleControl VehiclePIDController::RunStep(
    float target_speed_kmh,
    SharedPtr<client::Waypoint> wp) {
  const float acceleration = _lon_controller.RunStep(target_speed_kmh);
  float current_steering   = _lat_controller.RunStep(wp);

  rpc::VehicleControl control;
  if (acceleration >= 0.0f) {
    control.throttle = std::min(acceleration, _max_throttle);
    control.brake = 0.0f;
  } else {
    control.throttle = 0.0f;
    control.brake = std::min(std::abs(acceleration), _max_brake);
  }

  // Rate-limit steering changes (mirrors controller.py).
  if (current_steering > _past_steering + 0.1f) {
    current_steering = _past_steering + 0.1f;
  } else if (current_steering < _past_steering - 0.1f) {
    current_steering = _past_steering - 0.1f;
  }

  float steering;
  if (current_steering >= 0.0f) {
    steering = std::min(_max_steering, current_steering);
  } else {
    steering = std::max(-_max_steering, current_steering);
  }

  control.steer = steering;
  control.hand_brake = false;
  control.manual_gear_shift = false;
  _past_steering = steering;

  return control;
}

void VehiclePIDController::ChangeLongitudinalPID(const LongitudinalArgs &args) {
  _lon_controller.ChangeParameters(args.K_P, args.K_I, args.K_D, args.dt);
}

void VehiclePIDController::ChangeLateralPID(const LateralArgs &args) {
  _lat_controller.ChangeParameters(args.K_P, args.K_I, args.K_D, args.dt);
}

void VehiclePIDController::SetOffset(float offset) {
  _lat_controller.SetOffset(offset);
}

} // namespace navigation
} // namespace agents
} // namespace carla
