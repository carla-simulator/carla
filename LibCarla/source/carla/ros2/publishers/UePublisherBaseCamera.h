// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Exception.h"
#include "carla/ros2/publishers/UePublisherBaseSensor.h"
#include "carla/sensor/s11n/ImageSerializer.h"
#include "sensor_msgs/msg/CameraInfoPubSubTypes.h"
#include "sensor_msgs/msg/ImagePubSubTypes.h"

namespace carla {
namespace ros2 {

template <class ALLOCATOR>
using UeImagePublisherImpl =
    DdsPublisherImpl<sensor_msgs::msg::ImageT<ALLOCATOR>, sensor_msgs::msg::ImagePubSubTypeT<ALLOCATOR>>;
using UeCameraInfoPublisherImpl =
    DdsPublisherImpl<sensor_msgs::msg::CameraInfo, sensor_msgs::msg::CameraInfoPubSubType>;

/**
A Publisher base class for camera sensors.
Extends UePublisherBaseSensor by an image and camera_info publisher providing default implemenations for sending the
camera data from the rendering buffer copyless via DDS
*/
template <class ALLOCATOR>
class UePublisherBaseCamera : public UePublisherBaseSensor {
public:
  using allocator_type = ALLOCATOR;

  UePublisherBaseCamera(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                        std::shared_ptr<TransformPublisher> transform_publisher);
  virtual ~UePublisherBaseCamera() = default;

  /**
   * Some Encodings from
   * https://github.com/ros/common_msgs/blob/846bfcb/sensor_msgs/include/sensor_msgs/image_encodings.h Extend this list
   * (and the corresponding implementations of to_string(), num_channels(), bit_depth()
   */
  enum class Encoding { RGB8, RGBA8, RGB16, RGBA16, BGR8, BGRA8, BGR16, BGRA16, MONO8, MONO16 };

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

  /**
   * Implement PublisherInterface::Publish interface
   */
  bool Publish() override;
  /**
   * Implement PublisherInterface::SubsribersConnected interface
   */
  bool SubsribersConnected() const override;

  /**
   * Implements UePublisherBaseSensor::UpdateSensorData() interface
   */
  void UpdateSensorData(std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
                        const carla::SharedBufferView buffer_view) override;

protected:
  void UpdateCameraInfo(const builtin_interfaces::msg::Time &stamp, sensor_msgs::msg::CameraInfo const &camera_info);
  void UpdateImageHeader(const builtin_interfaces::msg::Time &stamp, sensor_msgs::msg::CameraInfo const &camera_info);

  virtual void SetImageDataFromBuffer(const carla::SharedBufferView buffer_view);

  /**
   * encoding of the stream, defaults to Encoding::BGRA8
   */
  virtual Encoding encoding() const {
    return Encoding::BGRA8;
  }

  using ImageHeaderConst = carla::sensor::s11n::ImageSerializer::ImageHeader const;

  /**
   * @brief provides access to the image header stored at the start of the buffer
   */
  std::shared_ptr<ImageHeaderConst> header_view(const carla::SharedBufferView buffer_view) {
    return std::shared_ptr<ImageHeaderConst>(buffer_view,
                                             reinterpret_cast<ImageHeaderConst *>(buffer_view.get()->data()));
  }

  /**
   * @brief calculates the number of elements of the buffer view by reducing the buffer size by the
   * carla::sensor::s11n::ImageSerializer::header_offset and division by sizeof(T)
   */
  template <typename T>
  std::size_t number_of_elements(const carla::SharedBufferView buffer_view) const {
    return carla::sensor::data::number_of_elements<T>(buffer_view, carla::sensor::s11n::ImageSerializer::header_offset);
  }

  /**
   * @brief the vector creation method matching the ALLOCATOR type of this class
   *
   * This function is called for sensor_msgs::msg::Image types
   */
  template <typename T, typename A = ALLOCATOR,
            typename std::enable_if<std::is_same<sensor_msgs::msg::Image::allocator_type, A>::value, int>::type = 0>
  std::vector<T> buffer_data_2_vector(const carla::SharedBufferView buffer_view) const {
    return carla::sensor::data::buffer_data_copy_to_std_vector<T>(buffer_view,
                                                                  carla::sensor::s11n::ImageSerializer::header_offset);
  }

  /**
   * @brief the vector creation method matching the ALLOCATOR type of this class
   *
   * This function is called for sensor_msgs::msg::ImageFromBuffer types
   */
  template <typename T, typename A = ALLOCATOR,
            typename std::enable_if<std::is_same<sensor_msgs::msg::ImageFromBuffer::allocator_type, A>::value,
                                    int>::type val = 0>
  std::vector<T, carla::sensor::data::SerializerVectorAllocator<T>> buffer_data_2_vector(
      const carla::SharedBufferView buffer_view) const {
    return carla::sensor::data::buffer_data_accessed_by_vector<T>(buffer_view,
                                                                  carla::sensor::s11n::ImageSerializer::header_offset);
  }

  std::string encoding_as_string() const {
    switch (encoding()) {
      case UePublisherBaseCamera::Encoding::RGB8:
        return "rgb8";
      case UePublisherBaseCamera::Encoding::RGBA8:
        return "rgba8";
      case UePublisherBaseCamera::Encoding::RGB16:
        return "rgb16";
      case UePublisherBaseCamera::Encoding::RGBA16:
        return "rgba16";
      case UePublisherBaseCamera::Encoding::BGR8:
        return "bgr8";
      case UePublisherBaseCamera::Encoding::BGRA8:
        return "bgra8";
      case UePublisherBaseCamera::Encoding::BGR16:
        return "bgr16";
      case UePublisherBaseCamera::Encoding::BGRA16:
        return "bgra16";
      case UePublisherBaseCamera::Encoding::MONO8:
        return "mono8";
      case UePublisherBaseCamera::Encoding::MONO16:
        return "mono16";
      default:
        carla::throw_exception(std::invalid_argument("UePublisherBaseCamera::to_string encoding " +
                                                     std::to_string(int(encoding())) + " not found"));
    }
  }

  uint32_t num_channels() const {
    switch (encoding()) {
      case UePublisherBaseCamera::Encoding::MONO8:
      case UePublisherBaseCamera::Encoding::MONO16:
        return 1u;
      case UePublisherBaseCamera::Encoding::RGB8:
      case UePublisherBaseCamera::Encoding::BGR8:
      case UePublisherBaseCamera::Encoding::RGB16:
      case UePublisherBaseCamera::Encoding::BGR16:
        return 3u;
      case UePublisherBaseCamera::Encoding::RGBA8:
      case UePublisherBaseCamera::Encoding::BGRA8:
      case UePublisherBaseCamera::Encoding::RGBA16:
      case UePublisherBaseCamera::Encoding::BGRA16:
        return 4u;
      default:
        carla::throw_exception(std::invalid_argument("UePublisherBaseCamera::pixel_size_in_byte encoding " +
                                                     std::to_string(int(encoding())) + " not found"));
    }
  }

  uint32_t bit_depth() const {
    switch (encoding()) {
      case UePublisherBaseCamera::Encoding::RGB8:
      case UePublisherBaseCamera::Encoding::RGBA8:
      case UePublisherBaseCamera::Encoding::BGR8:
      case UePublisherBaseCamera::Encoding::BGRA8:
      case UePublisherBaseCamera::Encoding::MONO8:
        return 1u;
      case UePublisherBaseCamera::Encoding::RGB16:
      case UePublisherBaseCamera::Encoding::RGBA16:
      case UePublisherBaseCamera::Encoding::BGR16:
      case UePublisherBaseCamera::Encoding::BGRA16:
      case UePublisherBaseCamera::Encoding::MONO16:
        return 2u;
      default:
        carla::throw_exception(std::invalid_argument("UePublisherBaseCamera::pixel_size_in_byte encoding " +
                                                     std::to_string(int(encoding())) + " not found"));
    }
  }

  uint32_t pixel_stride() const {
    return bit_depth() * num_channels();
  }

  uint32_t line_stride() const {
    return bit_depth() * num_channels() * width();
  }

  uint32_t image_size() const {
    return line_stride() * height();
  }
  uint32_t width() const;
  uint32_t height() const;

  std::shared_ptr<UeImagePublisherImpl<ALLOCATOR>> _image;
  std::shared_ptr<UeCameraInfoPublisherImpl> _camera_info;
  bool _camera_info_initialized{false};
};
}  // namespace ros2
}  // namespace carla

#include "UePublisherBaseCamera.cc"
