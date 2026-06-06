// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ros2/publishers/CarlaCameraPublisher.h>
#include <carla/ros2/publishers/CarlaRGBCameraPublisher.h>
#include <carla/ros2/publishers/CarlaDepthCameraPublisher.h>
#include <carla/ros2/publishers/CarlaNormalsCameraPublisher.h>
#include <carla/ros2/publishers/CarlaSSCameraPublisher.h>
#include <carla/ros2/publishers/CarlaISCameraPublisher.h>
#include <carla/ros2/publishers/CarlaOpticalFlowCameraPublisher.h>

#include <type_traits>

// These checks lock the type partitioning that ROS2::GetOrCreateCameraSensor's
// cache-hit guard relies on. The guard casts a cached publisher with
// dynamic_pointer_cast<CameraT> to enforce the one-actor-one-camera-type
// invariant. Its safety rests on two facts verified here:
//   1. RGB/Depth/SS/IS/Normals all alias the SAME CarlaRGBCameraPublisher
//      (shared BGRA passthrough), so a cache hit across those types is intended
//      and the cast succeeds.
//   2. CarlaOpticalFlowCameraPublisher is a DISTINCT sibling type; both derive
//      from CarlaCameraPublisher. An RGB <-> OpticalFlow cache hit therefore
//      fails the cast, which is exactly the dangerous mismatch the guard catches.
// If anyone later promotes one of the BGRA aliases to a real subclass, these
// static_asserts break loudly so the guard's assumption is revisited.

namespace carla {
namespace ros2 {

static_assert(
    std::is_same_v<CarlaDepthCameraPublisher, CarlaRGBCameraPublisher>,
    "Depth camera must alias the RGB BGRA-passthrough publisher.");
static_assert(
    std::is_same_v<CarlaNormalsCameraPublisher, CarlaRGBCameraPublisher>,
    "Normals camera must alias the RGB BGRA-passthrough publisher.");
static_assert(
    std::is_same_v<CarlaSSCameraPublisher, CarlaRGBCameraPublisher>,
    "Semantic-segmentation camera must alias the RGB BGRA-passthrough publisher.");
static_assert(
    std::is_same_v<CarlaISCameraPublisher, CarlaRGBCameraPublisher>,
    "Instance-segmentation camera must alias the RGB BGRA-passthrough publisher.");

static_assert(
    !std::is_same_v<CarlaOpticalFlowCameraPublisher, CarlaRGBCameraPublisher>,
    "Optical-flow camera must stay a distinct type from the RGB publisher.");

static_assert(
    std::is_base_of_v<CarlaCameraPublisher, CarlaRGBCameraPublisher>,
    "RGB publisher must derive from CarlaCameraPublisher.");
static_assert(
    std::is_base_of_v<CarlaCameraPublisher, CarlaOpticalFlowCameraPublisher>,
    "Optical-flow publisher must derive from CarlaCameraPublisher.");

static_assert(
    std::is_polymorphic_v<CarlaCameraPublisher>,
    "CarlaCameraPublisher must be polymorphic for the dynamic_pointer_cast guard.");

}  // namespace ros2
}  // namespace carla

namespace {

TEST(CameraPublisherTypes, bgra_cameras_share_one_publisher_type) {
  // The five BGRA-passthrough cameras collapse to a single publisher type, so a
  // cache hit across them in GetOrCreateCameraSensor is intended and safe.
  EXPECT_TRUE((std::is_same_v<carla::ros2::CarlaDepthCameraPublisher,
                              carla::ros2::CarlaRGBCameraPublisher>));
  EXPECT_TRUE((std::is_same_v<carla::ros2::CarlaNormalsCameraPublisher,
                              carla::ros2::CarlaRGBCameraPublisher>));
  EXPECT_TRUE((std::is_same_v<carla::ros2::CarlaSSCameraPublisher,
                              carla::ros2::CarlaRGBCameraPublisher>));
  EXPECT_TRUE((std::is_same_v<carla::ros2::CarlaISCameraPublisher,
                              carla::ros2::CarlaRGBCameraPublisher>));
}

TEST(CameraPublisherTypes, optical_flow_is_a_distinct_sibling) {
  // Optical flow is the one camera type the cache-hit guard must reject when it
  // collides with an RGB-family publisher (and vice versa).
  EXPECT_FALSE((std::is_same_v<carla::ros2::CarlaOpticalFlowCameraPublisher,
                               carla::ros2::CarlaRGBCameraPublisher>));
  EXPECT_TRUE((std::is_base_of_v<carla::ros2::CarlaCameraPublisher,
                                 carla::ros2::CarlaOpticalFlowCameraPublisher>));
  EXPECT_TRUE((std::is_base_of_v<carla::ros2::CarlaCameraPublisher,
                                 carla::ros2::CarlaRGBCameraPublisher>));
}

}  // namespace
