// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ros2/publishers/OpticalFlowEncoding.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

TEST(OpticalFlowEncoding, zero_flow_is_black_with_zero_alpha) {
  // No motion -> magnitude 0 -> intensity clamped to 0 -> all RGB channels 0.
  // The publisher convention sets the alpha byte to 0.
  const auto bgra = carla::ros2::EncodeFlowPixelToBgra(0.0f, 0.0f);
  EXPECT_EQ(bgra[0], 0u);
  EXPECT_EQ(bgra[1], 0u);
  EXPECT_EQ(bgra[2], 0u);
  EXPECT_EQ(bgra[3], 0u);
}

TEST(OpticalFlowEncoding, alpha_is_always_zero) {
  // The encoding deliberately sets the alpha byte to 0 regardless of input;
  // ROS 2 subscribers rely on that to detect missing-data pixels.
  for (float vx : {-1.0f, -0.1f, 0.0f, 0.1f, 1.0f, 10.0f}) {
    for (float vy : {-1.0f, -0.1f, 0.0f, 0.1f, 1.0f, 10.0f}) {
      const auto bgra = carla::ros2::EncodeFlowPixelToBgra(vx, vy);
      EXPECT_EQ(bgra[3], 0u) << "vx=" << vx << " vy=" << vy;
    }
  }
}

TEST(OpticalFlowEncoding, large_magnitude_saturates_brightness) {
  // A very large flow magnitude pushes log(norm + shift) above 1; the clamp
  // pins the intensity at 1 so at least one channel saturates near 255.
  const auto bgra = carla::ros2::EncodeFlowPixelToBgra(1000.0f, 1000.0f);
  const int max_channel = std::max({static_cast<int>(bgra[0]),
                                    static_cast<int>(bgra[1]),
                                    static_cast<int>(bgra[2])});
  EXPECT_GE(max_channel, 250);
}

TEST(OpticalFlowEncoding, positive_vx_lands_in_cyan_hemisphere) {
  // vx > 0, vy = 0 -> atan2(0, vx) = 0 -> hue = 180 deg (cyan-ish). HSV's
  // h_60 = 3 (case 3) puts the colour in the {0, x, c} channel triple, so
  // the red channel must be the smallest of the three RGB bytes.
  const auto bgra = carla::ros2::EncodeFlowPixelToBgra(2.0f, 0.0f);
  // bgra order: B, G, R, A. Red is index 2.
  EXPECT_LE(bgra[2], bgra[0]);
  EXPECT_LE(bgra[2], bgra[1]);
}

TEST(OpticalFlowEncoding, negative_vx_lands_in_red_hemisphere) {
  // vx < 0, vy = 0 -> atan2(0, -vx) = pi -> hue wraps to 0 (red-ish). Red is
  // the dominant channel.
  const auto bgra = carla::ros2::EncodeFlowPixelToBgra(-2.0f, 0.0f);
  EXPECT_GE(bgra[2], bgra[0]);
  EXPECT_GE(bgra[2], bgra[1]);
}

TEST(OpticalFlowEncoding, buffer_length_matches_height_times_width_times_four) {
  // height * width * 4 BGRA bytes out, exactly.
  constexpr uint32_t h = 4;
  constexpr uint32_t w = 5;
  std::vector<float> input(static_cast<size_t>(h) * w * 2, 0.5f);
  const auto out = carla::ros2::EncodeFlowImageToBgra(h, w, input.data());
  EXPECT_EQ(out.size(), static_cast<size_t>(h) * w * 4u);
}

TEST(OpticalFlowEncoding, buffer_per_pixel_matches_pixel_helper) {
  // The buffer-level helper is just a loop over the per-pixel helper. Verify
  // it: for a 2x2 image with distinct (vx, vy) per pixel, every output BGRA
  // quad equals the per-pixel encoding.
  std::array<float, 8> input = {
      0.0f, 0.0f,
      1.0f, 0.0f,
      0.0f, 1.0f,
      -1.0f, -1.0f,
  };
  const auto out = carla::ros2::EncodeFlowImageToBgra(2u, 2u, input.data());
  for (size_t i = 0; i < 4; ++i) {
    const auto expected = carla::ros2::EncodeFlowPixelToBgra(
        input[i * 2], input[i * 2 + 1]);
    EXPECT_EQ(out[i * 4 + 0], expected[0]) << "pixel " << i;
    EXPECT_EQ(out[i * 4 + 1], expected[1]) << "pixel " << i;
    EXPECT_EQ(out[i * 4 + 2], expected[2]) << "pixel " << i;
    EXPECT_EQ(out[i * 4 + 3], expected[3]) << "pixel " << i;
  }
}
