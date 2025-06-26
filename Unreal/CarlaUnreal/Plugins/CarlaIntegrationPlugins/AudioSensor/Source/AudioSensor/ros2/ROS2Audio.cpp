// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ROS2Audio.h"
#include "carla/Logging.h"
#include "carla/geom/GeoLocation.h"
#include "carla/geom/Vector3D.h"
#include "carla/sensor/data/DVSEvent.h"
#include "carla/sensor/data/LidarData.h"
#include "carla/sensor/data/SemanticLidarData.h"
#include "carla/sensor/data/RadarData.h"
#include "carla/sensor/data/Image.h"
#include "carla/sensor/s11n/ImageSerializer.h"
#include "carla/sensor/s11n/SensorHeaderSerializer.h"

#include "carla/ros2/plugin-utils/enable-fastdds-include.h" // start fastdds includes
#include "carla/ros2/publishers/CarlaPublisher.h"
#include "carla/ros2/publishers/CarlaClockPublisher.h"
#include "carla/ros2/publishers/CarlaTransformPublisher.h"
#include "carla/ros2/publishers/BasicPublisher.h"
#include "AudioSensor/ros2/publishers/CarlaAudioPublisher.h"
#include "carla/ros2/plugin-utils/disable-fastdds-include.h" // end fastdds includes

#include <vector>

namespace carla {
namespace ros2 {

// static fields
std::shared_ptr<ROS2Audio> ROS2Audio::_instance;

void ROS2Audio::Enable(bool enable) {
  _enabled = enable;
  log_info("ROS2Audio enabled: ", _enabled);
}

void ROS2Audio::SetFrame(uint64_t frame) {
  _frame = frame;
}

void ROS2Audio::SetTimestamp(double timestamp) {
  if (IsEnabled()){
    double integral;
    const double fractional = modf(timestamp, &integral);
    const double multiplier = 1000000000.0;
    _seconds = static_cast<int32_t>(integral);
    _nanoseconds = static_cast<uint32_t>(fractional * multiplier);
  }
}

void ROS2Audio::Shutdown() {
  for (auto& element : _publishers) {
    element.second.reset();
  }
  for (auto& element : _transforms) {
    element.second.reset();
  }
  _enabled = false;
}

void ROS2Audio::RemoveActorRosPublishers(void *actor)
{
  auto p_it = _publishers.find(actor);
  if (p_it != _publishers.end()) {
    _publishers.erase(actor);
  }

  auto t_it = _transforms.find(actor);
  if (t_it != _transforms.end()) {
    _transforms.erase(actor);
  }
}

std::pair<std::shared_ptr<CarlaAudioPublisher>, std::shared_ptr<CarlaTransformPublisher>> ROS2Audio::GetOrCreateSensor(carla::streaming::detail::stream_id_type id, void* actor) {
  auto it_publishers = _publishers.find(actor);
  auto it_transforms = _transforms.find(actor);
  std::shared_ptr<CarlaAudioPublisher> publisher {};
  std::shared_ptr<CarlaTransformPublisher> transform {};
  auto ROS2Interfaces = UActorDispatcher::GetInterfaces();
  if (it_publishers != _publishers.end()) {
    publisher = it_publishers->second;
    if (it_transforms != _transforms.end()) {
      transform = it_transforms->second;
    }
  } else {
    //Sensor not found, creating one of the given type
    const std::string string_id = std::to_string(id);
    std::string ros_name = ROS2Interfaces->GetActorRosName(actor);
    std::string parent_ros_name = ROS2Interfaces->GetActorParentRosName(actor);
    if (ros_name == "audio__") {
      ros_name.pop_back();
      ros_name.pop_back();
      ros_name += string_id;
      ROS2Interfaces->UpdateActorRosName(actor, ros_name);
    }
    std::shared_ptr<CarlaAudioPublisher> new_publisher = std::make_shared<CarlaAudioPublisher>(ros_name.c_str(), parent_ros_name.c_str());
    if (new_publisher->Init()) {
      _publishers.insert({actor, new_publisher});
      publisher = new_publisher;
    }
    std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
    if (new_transform->Init()) {
      _transforms.insert({actor, new_transform});
      transform = new_transform;
    }
  }
  return { publisher, transform };
}

void ROS2Audio::ProcessDataFromAudio(
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    uint32_t num_channels, uint32_t sample_rate,
    uint32_t chunk_size, const int16_t *data,
    void *actor) {

    log_info("Sensor AudioSensor to ROS data: frame.", _frame, "stream.", stream_id);
    {
      auto sensors = GetOrCreateSensor(stream_id, actor);
      if (sensors.first) {
        sensors.first->SetData(_seconds, _nanoseconds, num_channels, sample_rate, chunk_size, data);
        sensors.first->Publish();
      }
      if (sensors.second) {
        sensors.second->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
        sensors.second->Publish();
      }
    }
}

} // namespace ros2
} // namespace carla
