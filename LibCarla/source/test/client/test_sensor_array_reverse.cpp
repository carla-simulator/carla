// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/sensor/SensorRegistry.h>
#include <carla/sensor/data/Image.h>
#include <carla/sensor/data/RadarMeasurement.h>
#include <carla/sensor/s11n/SensorHeaderSerializer.h>

#include <cstring>
#include <iterator>
#include <utility>
#include <vector>

namespace {

  using carla::sensor::SensorRegistry;
  using carla::sensor::data::Image;
  using carla::sensor::data::RadarMeasurement;
  using carla::sensor::s11n::SensorHeaderSerializer;

  template <typename Measurement>
  carla::SharedPtr<Measurement> Decode(size_t sensor_type, const carla::Buffer &payload) {
    const auto header = SensorHeaderSerializer::Serialize(sensor_type, 42u, 0.5, {});
    carla::Buffer message;
    message.copy_from(SensorHeaderSerializer::header_offset, payload);
    std::memcpy(message.data(), header.data(), header.size());
    return std::dynamic_pointer_cast<Measurement>(SensorRegistry::Deserialize(std::move(message)));
  }

  carla::SharedPtr<RadarMeasurement> Radar(size_t count) {
    carla::sensor::data::RadarData detections;
    for (size_t i = 0u; i < count; ++i) {
      detections.WriteDetection({0.0f, 0.0f, 0.0f, static_cast<float>(i + 1u)});
    }
    // RadarSerializer does not read its sensor argument; the inert integer
    // represents that producer boundary. Payload/header/registry are native.
    const auto payload = carla::sensor::s11n::RadarSerializer::Serialize(0, detections, carla::Buffer{});
    return Decode<RadarMeasurement>(SensorRegistry::get<ARadar *>::index, payload);
  }

  struct CameraDescription {
    uint32_t GetImageWidth() const { return 3u; }
    uint32_t GetImageHeight() const { return 1u; }
    float GetFOVAngle() const { return 90.0f; }
  };

  carla::SharedPtr<Image> Pixels() {
    using Serializer = carla::sensor::s11n::ImageSerializer;
    const std::vector<carla::sensor::data::Color> pixels = {{1u, 0u, 0u}, {2u, 0u, 0u}, {3u, 0u, 0u}};
    carla::Buffer bitmap;
    bitmap.copy_from(Serializer::header_offset, pixels);
    const auto payload = Serializer::Serialize(CameraDescription{}, std::move(bitmap));
    return Decode<Image>(SensorRegistry::get<ASceneCaptureCamera *>::index, payload);
  }

  TEST(sensor_array_reverse, mutable_endpoints_enclose_the_array_in_reverse) {
    const auto radar = Radar(3u);
    ASSERT_NE(radar, nullptr);
    EXPECT_EQ(radar->rbegin().base(), radar->end());
    EXPECT_EQ(radar->rend().base(), radar->begin());
    EXPECT_EQ(std::distance(radar->rbegin(), radar->rend()), 3);
  }

  TEST(sensor_array_reverse, const_endpoints_enclose_the_array_in_reverse) {
    const auto radar = Radar(3u);
    ASSERT_NE(radar, nullptr);
    const auto &view = std::as_const(*radar);
    EXPECT_EQ(view.crbegin().base(), view.cend());
    EXPECT_EQ(view.crend().base(), view.cbegin());
    EXPECT_EQ(view.rbegin(), view.crbegin());
    EXPECT_EQ(view.rend(), view.crend());
    EXPECT_EQ(std::distance(view.crbegin(), view.crend()), 3);
  }

  TEST(sensor_array_reverse, mutable_reverse_iteration_returns_last_detection_first) {
    const auto radar = Radar(3u);
    ASSERT_NE(radar, nullptr);
    ASSERT_EQ(std::distance(radar->rbegin(), radar->rend()), 3);
    std::vector<float> depths;
    for (auto it = radar->rbegin(); it != radar->rend(); ++it) {
      depths.push_back(it->depth);
    }
    EXPECT_EQ(depths, (std::vector<float>{3.0f, 2.0f, 1.0f}));
  }

  TEST(sensor_array_reverse, const_reverse_iteration_returns_last_detection_first) {
    const auto radar = Radar(3u);
    ASSERT_NE(radar, nullptr);
    const auto &view = std::as_const(*radar);
    ASSERT_EQ(std::distance(view.rbegin(), view.rend()), 3);
    std::vector<float> depths;
    for (auto it = view.rbegin(); it != view.rend(); ++it) {
      depths.push_back(it->depth);
    }
    EXPECT_EQ(depths, (std::vector<float>{3.0f, 2.0f, 1.0f}));
  }

  TEST(sensor_array_reverse, reverse_mutation_updates_the_last_detection) {
    const auto radar = Radar(3u);
    ASSERT_NE(radar, nullptr);
    ASSERT_EQ(radar->rbegin().base(), radar->end());
    radar->rbegin()->depth = 9.0f;
    EXPECT_FLOAT_EQ((*radar)[0u].depth, 1.0f);
    EXPECT_FLOAT_EQ((*radar)[1u].depth, 2.0f);
    EXPECT_FLOAT_EQ((*radar)[2u].depth, 9.0f);
  }

  TEST(sensor_array_reverse, singleton_has_one_reverse_element) {
    const auto radar = Radar(1u);
    ASSERT_NE(radar, nullptr);
    ASSERT_EQ(std::distance(radar->rbegin(), radar->rend()), 1);
    EXPECT_FLOAT_EQ(radar->rbegin()->depth, 1.0f);
  }

  TEST(sensor_array_reverse, image_header_offset_does_not_become_a_pixel) {
    const auto image = Pixels();
    ASSERT_NE(image, nullptr);
    ASSERT_EQ(image->GetWidth(), 3u);
    ASSERT_EQ(image->size(), 3u);
    ASSERT_EQ(std::distance(image->crbegin(), image->crend()), 3);
    std::vector<uint8_t> reds;
    for (auto it = image->crbegin(); it != image->crend(); ++it) {
      reds.push_back(it->r);
    }
    EXPECT_EQ(reds, (std::vector<uint8_t>{3u, 2u, 1u}));
  }

  TEST(sensor_array_reverse, empty_array_reverse_range_is_empty_control) {
    const auto radar = Radar(0u);
    ASSERT_NE(radar, nullptr);
    EXPECT_EQ(radar->rbegin(), radar->rend());
    EXPECT_EQ(radar->crbegin(), radar->crend());
    EXPECT_EQ(std::distance(radar->rbegin(), radar->rend()), 0);
  }

  TEST(sensor_array_reverse, forward_data_and_header_are_preserved_control) {
    const auto radar = Radar(3u);
    ASSERT_NE(radar, nullptr);
    EXPECT_EQ(radar->GetFrame(), 42u);
    EXPECT_DOUBLE_EQ(radar->GetTimestamp(), 0.5);
    std::vector<float> depths;
    for (const auto &detection : *radar) {
      depths.push_back(detection.depth);
    }
    EXPECT_EQ(depths, (std::vector<float>{1.0f, 2.0f, 3.0f}));
  }

} // namespace
