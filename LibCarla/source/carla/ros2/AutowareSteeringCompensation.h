// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <tuple>

namespace carla {
namespace ros2 {
/**
 * Ported verbatim from tier4/autoware-support (commit 63e831e85 lineage).
 *
 * @note This solution is not ideal and it should be temporary. Ideally a root
 * cause should be found and some other solution should be implemented based on
 * the findings. This solution is purely based on experiments. For better
 * accuracy more data points can be added to the lookup table.
 */
namespace autoware_steering_compensation {
namespace detail {

constexpr std::tuple<float, float> MakeDataPoint(const float desired, const float actual) {
  return std::make_tuple(desired, actual);
}

/**
 * @note Steering compensation lookup table.
 * Tuples of (desired_steering_angle, actual_steering_angle).
 * Desired steering angle is the normalized steer input applied to the vehicle;
 * actual steering angle is the steady-state tire angle in radians derived from
 * the measured yaw rate via the bicycle model (delta = atan(L * w / v)).
 * Measured on the UE5.8 Chaos Lincoln MKZ (2026-08-20 sweep at ~3 m/s); the
 * response is near-linear, unlike the strongly degressive UE5.5 vehicle the
 * original tier4 table was calibrated against.
 * Only positive values stored; absolute value used for lookup.
 */
constexpr std::array DATA{
  MakeDataPoint(0.025, 0.0246614),
  MakeDataPoint(0.05,  0.0495395),
  MakeDataPoint(0.1,   0.0949895),
  MakeDataPoint(0.15,  0.142492 ),
  MakeDataPoint(0.2,   0.188953 ),
  MakeDataPoint(0.25,  0.239746 ),
  MakeDataPoint(0.3,   0.287899 ),
  MakeDataPoint(0.35,  0.333537 ),
  MakeDataPoint(0.4,   0.381975 ),
  MakeDataPoint(0.45,  0.416428 ),
  MakeDataPoint(0.5,   0.466122 ),
  MakeDataPoint(0.55,  0.516003 ),
  MakeDataPoint(0.6,   0.559125 ),
  MakeDataPoint(0.65,  0.592443 ),
  MakeDataPoint(0.7,   0.628645 )
};

// Linear interpolation function
inline float Lerp(const float a, const float b, const float t) {
  return a + t * (b - a);
}

// Get steering compensation ratio from lookup table with LERP
template<std::size_t key_idx, std::size_t value_idx>
float InterpolateSteeringData(const float angle) {
  const auto AdjustSign = [angle](const float value) -> float {
    return angle < 0.0f ? -value : value;
  };

  // Use absolute value for symmetric steering
  const float key_angle = std::abs(angle);

  // Handle edge cases
  if (const auto front_key = std::get<key_idx>(DATA.front()); key_angle <= front_key) {
    // Interpolate between 0 and first data point
    const float t = key_angle / front_key;
    return AdjustSign(Lerp(0.0f, std::get<value_idx>(DATA.front()), t));
  }
  if (key_angle >= std::get<key_idx>(DATA.back())) {
    return AdjustSign(std::get<value_idx>(DATA.back()));
  }

  // Find the two points to interpolate between
  for (size_t i = 0; i < DATA.size() - 1; ++i) {
    const auto current_key_angle   = std::get<key_idx  >(DATA[i]);
    const auto current_value_angle = std::get<value_idx>(DATA[i]);
    const auto next_key_angle      = std::get<key_idx  >(DATA[i + 1]);
    const auto next_value_angle    = std::get<value_idx>(DATA[i + 1]);

    if (current_key_angle <= key_angle && key_angle <= next_key_angle) {
      // Calculate interpolation factor
      const float t = (key_angle - current_key_angle) / (next_key_angle - current_key_angle);
      // Interpolate between the two ratio values
      return AdjustSign(Lerp(current_value_angle, next_value_angle, t));
    }
  }

  // Default fallback (should not reach here)
  return angle;
}
} // namespace detail

/// Desired vehicle-input angle -> observed (actual) tire angle. Used when
/// publishing the SteeringReport so Autoware sees the angle the wheels really
/// reached.
inline float GetSteeringOutput(const float input_angle) {
  return detail::InterpolateSteeringData<0, 1>(input_angle);
}

/// Target (actual) tire angle -> vehicle input that produces it. Used when
/// applying Autoware's steering command to the vehicle.
inline float GetSteeringInput(const float output_angle) {
  return detail::InterpolateSteeringData<1, 0>(output_angle);
}
} // namespace autoware_steering_compensation
} // namespace ros2
} // namespace carla
