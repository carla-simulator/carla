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
 * The values were obtained in experiments with no compensation mechanism in
 * place. The Lincoln MKZ vehicle was used in the experiments.
 * Desired steering angle is what was set as an input.
 * Actual steering angle is what was observed based on the input (average of
 * two steering wheels).
 * Only positive values stored; absolute value used for lookup.
 */
constexpr std::array DATA{
  MakeDataPoint(0.01,  6.7395889065761E-05 ),
  MakeDataPoint(0.025, 0.000423556502873984),
  MakeDataPoint(0.05,  0.00169376349924416 ),
  MakeDataPoint(0.1,   0.0067762146409292  ),
  MakeDataPoint(0.15,  0.0152460793347012  ),
  MakeDataPoint(0.2,   0.0271043087850024  ),
  MakeDataPoint(0.25,  0.0423533289994932  ),
  MakeDataPoint(0.3,   0.0609965171902762  ),
  MakeDataPoint(0.35,  0.0830410990204546  ),
  MakeDataPoint(0.4,   0.108495495703669   ),
  MakeDataPoint(0.45,  0.137377527051583   ),
  MakeDataPoint(0.5,   0.16971091208873    ),
  MakeDataPoint(0.55,  0.205528463005049   ),
  MakeDataPoint(0.6,   0.244878167628323   ),
  MakeDataPoint(0.65,  0.287820327084208   ),
  MakeDataPoint(0.7,   0.334437486719677   ),
  MakeDataPoint(0.75,  0.384837333349578   ),
  MakeDataPoint(0.8,   0.439157791618051   ),
  MakeDataPoint(0.85,  0.497580960452602   ),
  MakeDataPoint(0.9,   0.560342476755542   ),
  MakeDataPoint(0.95,  0.627753061493605   ),
  MakeDataPoint(1.0,   0.700223224833506   ),
  MakeDataPoint(1.05,  0.778318235286733   ),
  MakeDataPoint(1.1,   0.862832016950082   ),
  MakeDataPoint(1.15,  0.954930519577053   ),
  MakeDataPoint(1.2,   1.05646350419544    )
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
