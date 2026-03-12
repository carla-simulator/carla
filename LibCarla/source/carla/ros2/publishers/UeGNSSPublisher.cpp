// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UeGNSSPublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

UeGNSSPublisher::UeGNSSPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                                 std::shared_ptr<TransformPublisher> transform_publisher)
  : UePublisherBase(sensor_actor_definition, transform_publisher),
    _impl(std::make_shared<UeGNSSPublisherImpl>())
    {

      if ( sensor_actor_definition->attributes.find("noise_lat_stddev") != sensor_actor_definition->attributes.end() ) {
        _noise_lat_covar = std::stod(sensor_actor_definition->attributes["noise_lat_stddev"]);
        _noise_lat_covar = _noise_lat_covar * _noise_lat_covar; // covariance = stddev^2
      }
      if ( sensor_actor_definition->attributes.find("noise_lon_stddev") != sensor_actor_definition->attributes.end() ) {
        _noise_lon_covar = std::stod(sensor_actor_definition->attributes["noise_lon_stddev"]);
        _noise_lon_covar = _noise_lon_covar * _noise_lon_covar; // covariance = stddev^2
      }
      if ( sensor_actor_definition->attributes.find("noise_alt_stddev") != sensor_actor_definition->attributes.end() ) {
        _noise_alt_covar = std::stod(sensor_actor_definition->attributes["noise_alt_stddev"]);
        _noise_alt_covar = _noise_alt_covar * _noise_alt_covar; // covariance = stddev^2
      }
    }

bool UeGNSSPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->InitHistoryPreallocatedWithReallocMemoryMode(domain_participant, get_topic_name(), get_topic_qos());
}

bool UeGNSSPublisher::Publish() {
  return _impl->Publish();
}
bool UeGNSSPublisher::SubscribersConnected() const {
  return _impl->SubscribersConnected();
}

void UeGNSSPublisher::UpdateSensorData(
    std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
    carla::SharedBufferView buffer_view) {
  auto gnss_data = data(buffer_view);

  _impl->SetMessageHeader(GetTime(sensor_header), frame_id());
  _impl->Message().latitude(gnss_data.latitude);
  _impl->Message().longitude(gnss_data.longitude);
  _impl->Message().altitude(gnss_data.altitude);
  _impl->Message().position_covariance_type(sensor_msgs::msg::NavSatFix_Constants::COVARIANCE_TYPE_DIAGONAL_KNOWN);
  _impl->Message().position_covariance({ _noise_lat_covar, 0.0, 0.0, 0.0, _noise_lon_covar, 0.0, 0.0, 0.0, _noise_alt_covar });
}
}  // namespace ros2
}  // namespace carla
