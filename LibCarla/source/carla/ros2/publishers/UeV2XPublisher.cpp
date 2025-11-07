// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UeV2XPublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

UeV2XPublisher::UeV2XPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                                 std::shared_ptr<TransformPublisher> transform_publisher)
  : UePublisherBaseSensor(sensor_actor_definition, transform_publisher),
    _impl(std::make_shared<UeV2XPublisherImpl>()) {}

bool UeV2XPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  _initialized = _impl->InitHistoryPreallocatedWithReallocMemoryMode(domain_participant, get_topic_name(), get_topic_qos().reliable());
  return _initialized;
}

bool UeV2XPublisher::Publish() {
  return _impl->Publish();
}
bool UeV2XPublisher::SubscribersConnected() const {
  return _impl->SubscribersConnected();
}

void UeV2XPublisher::UpdateSensorData(
    std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
    carla::SharedBufferView buffer_view) {

  auto cam_v2x_data_vector = vector_view(buffer_view);

  if ( _impl->WasMessagePublished() ) {
    _impl->Message().data().clear();
  }

  for (carla::sensor::data::CAMData const &cam_v2x_data: cam_v2x_data_vector) {
    // General data and header
    carla_msgs::msg::CarlaV2XData carla_v2x_data;
    carla_v2x_data.power(cam_v2x_data.Power);
    carla_v2x_data.message().header().protocol_version() = cam_v2x_data.Message.header.protocolVersion;
    carla_v2x_data.message().header().message_id() = cam_v2x_data.Message.header.messageID;
    carla_v2x_data.message().header().station_id().value() = cam_v2x_data.Message.header.stationID;
    carla_v2x_data.message().cam().generation_delta_time().value() = cam_v2x_data.Message.cam.generationDeltaTime;

    // BasicContainer
    carla_v2x_data.message().cam().cam_parameters().basic_container().reference_position().altitude().altitude_value().value() = cam_v2x_data.Message.cam.camParameters.basicContainer.referencePosition.altitude.altitudeValue;
    carla_v2x_data.message().cam().cam_parameters().basic_container().reference_position().altitude().altitude_confidence().value() = cam_v2x_data.Message.cam.camParameters.basicContainer.referencePosition.altitude.altitudeConfidence;
    carla_v2x_data.message().cam().cam_parameters().basic_container().reference_position().latitude().value() = cam_v2x_data.Message.cam.camParameters.basicContainer.referencePosition.latitude;
    carla_v2x_data.message().cam().cam_parameters().basic_container().reference_position().longitude().value() = cam_v2x_data.Message.cam.camParameters.basicContainer.referencePosition.longitude;
    carla_v2x_data.message().cam().cam_parameters().basic_container().reference_position().position_confidence_ellipse().semi_major_confidence().value() = cam_v2x_data.Message.cam.camParameters.basicContainer.referencePosition.positionConfidenceEllipse.semiMajorConfidence;
    carla_v2x_data.message().cam().cam_parameters().basic_container().reference_position().position_confidence_ellipse().semi_major_orientation().value() = cam_v2x_data.Message.cam.camParameters.basicContainer.referencePosition.positionConfidenceEllipse.semiMajorOrientation;
    carla_v2x_data.message().cam().cam_parameters().basic_container().reference_position().position_confidence_ellipse().semi_minor_confidence().value() = cam_v2x_data.Message.cam.camParameters.basicContainer.referencePosition.positionConfidenceEllipse.semiMinorConfidence;
    carla_v2x_data.message().cam().cam_parameters().basic_container().station_type().value() = cam_v2x_data.Message.cam.camParameters.basicContainer.stationType;
    
    // HighFrequencyContainer
    switch (cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.present) {
      case CAMContainer::HighFrequencyContainer_PR::HighFrequencyContainer_PR_basicVehicleContainerHighFrequency:
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().choice() = etsi_its_cam_msgs::msg::HighFrequencyContainer_Constants::CHOICE_BASIC_VEHICLE_CONTAINER_HIGH_FREQUENCY;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().heading().heading_value().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.heading.headingValue;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().heading().heading_confidence().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.heading.headingConfidence;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().speed().speed_value().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.speed.speedValue;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().speed().speed_confidence().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.speed.speedConfidence;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().drive_direction().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.driveDirection;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().vehicle_length().vehicle_length_value().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.vehicleLength.vehicleLengthValue;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().vehicle_length().vehicle_length_confidence_indication().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.vehicleLength.vehicleLengthConfidenceIndication;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().vehicle_width().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.vehicleWidth;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().longitudinal_acceleration().longitudinal_acceleration_confidence().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.longitudinalAcceleration.longitudinalAccelerationConfidence;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().longitudinal_acceleration().longitudinal_acceleration_value().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.longitudinalAcceleration.longitudinalAccelerationValue;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().curvature().curvature_value().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.curvature.curvatureValue;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().curvature().curvature_confidence().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.curvature.curvatureConfidence;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().curvature_calculation_mode().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.curvatureCalculationMode;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().yaw_rate().yaw_rate_value().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.yawRate.yawRateValue;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().yaw_rate().yaw_rate_confidence().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.yawRate.yawRateConfidence;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().acceleration_control_is_present() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.accelerationControlAvailable;
        // TODO: carla implemenation differs from CAM definition, since ASN.1 defines a list
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().acceleration_control().value().push_back(cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.accelerationControl);
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().lane_position_is_present() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.lanePositionAvailable;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().lane_position().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.lanePosition;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().steering_wheel_angle_is_present() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.steeringWheelAngleAvailable;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().steering_wheel_angle().steering_wheel_angle_value().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.steeringWheelAngle.steeringWheelAngleValue;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().steering_wheel_angle().steering_wheel_angle_confidence().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.steeringWheelAngle.steeringWheelAngleConfidence;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().lateral_acceleration_is_present() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.lateralAccelerationAvailable;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().lateral_acceleration().lateral_acceleration_value().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.lateralAcceleration.lateralAccelerationValue;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().lateral_acceleration().lateral_acceleration_confidence().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.lateralAcceleration.lateralAccelerationConfidence;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().vertical_acceleration_is_present() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.verticalAccelerationAvailable;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().vertical_acceleration().vertical_acceleration_value().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.verticalAcceleration.verticalAccelerationValue;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().vertical_acceleration().vertical_acceleration_confidence().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.verticalAcceleration.verticalAccelerationConfidence;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().performance_class_is_present() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.performanceClassAvailable;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().performance_class().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.performanceClass;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().cen_dsrc_tolling_zone_is_present() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.cenDsrcTollingZoneAvailable;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().cen_dsrc_tolling_zone().cen_dsrc_tolling_zone_id_is_present() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.cenDsrcTollingZone.cenDsrcTollingZoneIDAvailable;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().cen_dsrc_tolling_zone().cen_dsrc_tolling_zone_id().value().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.cenDsrcTollingZone.cenDsrcTollingZoneID;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().cen_dsrc_tolling_zone().protected_zone_latitude().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.cenDsrcTollingZone.protectedZoneLatitude;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().basic_vehicle_container_high_frequency().cen_dsrc_tolling_zone().protected_zone_longitude().value() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency.cenDsrcTollingZone.protectedZoneLongitude;
        break;
      case CAMContainer::HighFrequencyContainer_PR::HighFrequencyContainer_PR_rsuContainerHighFrequency:
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().choice() = etsi_its_cam_msgs::msg::HighFrequencyContainer_Constants::CHOICE_BASIC_VEHICLE_CONTAINER_HIGH_FREQUENCY;
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().rsu_container_high_frequency().protected_communication_zones_rsu_is_present() = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.rsuContainerHighFrequency.protectedCommunicationZonesRSU.ProtectedCommunicationZoneCount > 0u;
        for (auto i=0u; i< cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.rsuContainerHighFrequency.protectedCommunicationZonesRSU.ProtectedCommunicationZoneCount; ++i) {
          auto const &protectedCommunicationZoneRSU = cam_v2x_data.Message.cam.camParameters.highFrequencyContainer.rsuContainerHighFrequency.protectedCommunicationZonesRSU.data[i];
          etsi_its_cam_msgs::msg::ProtectedCommunicationZone protectedCommunicationZone;
          protectedCommunicationZone.expiry_time_is_present() = protectedCommunicationZoneRSU.expiryTimeAvailable;
          protectedCommunicationZone.expiry_time().value() = protectedCommunicationZoneRSU.expiryTime;
          protectedCommunicationZone.protected_zone_id_is_present() = protectedCommunicationZoneRSU.protectedZoneIDAvailable;
          protectedCommunicationZone.protected_zone_id().value() = protectedCommunicationZoneRSU.protectedZoneID;
          protectedCommunicationZone.protected_zone_latitude().value() = protectedCommunicationZoneRSU.protectedZoneLatitude;
          protectedCommunicationZone.protected_zone_longitude().value() = protectedCommunicationZoneRSU.protectedZoneLongitude;
          protectedCommunicationZone.protected_zone_radius_is_present() = protectedCommunicationZoneRSU.protectedZoneRadiusAvailable;
          protectedCommunicationZone.protected_zone_radius().value() = protectedCommunicationZoneRSU.protectedZoneRadius;
          protectedCommunicationZone.protected_zone_type().value() = protectedCommunicationZoneRSU.protectedZoneType;
          carla_v2x_data.message().cam().cam_parameters().high_frequency_container().rsu_container_high_frequency().protected_communication_zones_rsu().array().push_back(protectedCommunicationZone);
        }
        break;
      case CAMContainer::HighFrequencyContainer_PR::HighFrequencyContainer_PR_NOTHING:
        // theorectically must not happen, since in the protocol Nothing is not defined and is translated into high_fequency_container_is_present, which is also not defined by the actual protocol!
        carla_v2x_data.message().cam().cam_parameters().high_frequency_container().choice() = etsi_its_cam_msgs::msg::HighFrequencyContainer_Constants::CHOICE_BASIC_VEHICLE_CONTAINER_HIGH_FREQUENCY;
        break;
    }

    // LowFrequencyContainer
    switch (cam_v2x_data.Message.cam.camParameters.lowFrequencyContainer.present) {
      case CAMContainer::LowFrequencyContainer_PR::LowFrequencyContainer_PR_NOTHING:
        carla_v2x_data.message().cam().cam_parameters().low_frequency_container_is_present() = false;
        break;
      case CAMContainer::LowFrequencyContainer_PR::LowFrequencyContainer_PR_basicVehicleContainerLowFrequency:
        carla_v2x_data.message().cam().cam_parameters().low_frequency_container_is_present() = true;
        carla_v2x_data.message().cam().cam_parameters().low_frequency_container().choice() = etsi_its_cam_msgs::msg::LowFrequencyContainer_Constants::CHOICE_BASIC_VEHICLE_CONTAINER_LOW_FREQUENCY;
        // TODO: carla implemenation differs from CAM definition, since ASN.1 defines a list
        carla_v2x_data.message().cam().cam_parameters().low_frequency_container().basic_vehicle_container_low_frequency().exterior_lights().value().push_back(cam_v2x_data.Message.cam.camParameters.lowFrequencyContainer.basicVehicleContainerLowFrequency.exteriorLights);
        for (auto i=0u; i< cam_v2x_data.Message.cam.camParameters.lowFrequencyContainer.basicVehicleContainerLowFrequency.pathHistory.NumberOfPathPoint; ++i) {
          auto const &pathPointCarla = cam_v2x_data.Message.cam.camParameters.lowFrequencyContainer.basicVehicleContainerLowFrequency.pathHistory.data[i];
          etsi_its_cam_msgs::msg::PathPoint pathPoint;
          pathPoint.path_delta_time_is_present() = false;
          if (pathPointCarla.pathDeltaTime != nullptr) {
            pathPoint.path_delta_time_is_present() = true;
            pathPoint.path_delta_time().value() = *pathPointCarla.pathDeltaTime; 
          } 
          pathPoint.path_position().delta_longitude().value() = pathPointCarla.pathPosition.deltaLongitude;
          pathPoint.path_position().delta_latitude().value() = pathPointCarla.pathPosition.deltaLatitude;
          pathPoint.path_position().delta_altitude().value() = pathPointCarla.pathPosition.deltaAltitude;
          carla_v2x_data.message().cam().cam_parameters().low_frequency_container().basic_vehicle_container_low_frequency().path_history().array().push_back(pathPoint);
        }
        carla_v2x_data.message().cam().cam_parameters().low_frequency_container().basic_vehicle_container_low_frequency().vehicle_role().value() = cam_v2x_data.Message.cam.camParameters.lowFrequencyContainer.basicVehicleContainerLowFrequency.vehicleRole;
    }

    // TODO: SpecialVehiclesContainer
    carla_v2x_data.message().cam().cam_parameters().special_vehicle_container_is_present() = false;

    // Finally send out
    _impl->Message().data().push_back(carla_v2x_data);
  }
  _impl->SetMessageUpdated();
}

}  // namespace ros2
}  // namespace carla
