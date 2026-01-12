// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "VehiclePublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"
#include "carla/ros2/types/Speed.h"
#include "carla/ros2/types/VehicleAckermannControl.h"
#include "carla/ros2/types/VehicleControl.h"

namespace carla {
namespace ros2 {

VehiclePublisher::VehiclePublisher(std::shared_ptr<carla::ros2::types::VehicleActorDefinition> vehicle_actor_definition,
                                   std::shared_ptr<TransformPublisher> transform_publisher,
                                   std::shared_ptr<ObjectsPublisher> objects_publisher,
                                   std::shared_ptr<ObjectsWithCovariancePublisher> objects_with_covariance_publisher,
                                   carla::rpc::RpcServerInterface &carla_server)
  : PublisherBaseTransform(std::static_pointer_cast<carla::ros2::types::ActorNameDefinition>(vehicle_actor_definition),
                           transform_publisher),
    _carla_server(carla_server),
    _vehicle_info_publisher(std::make_shared<VehicleInfoPublisherImpl>()),
    _vehicle_status_publisher(std::make_shared<EgoVehicleStatusPublisherImpl>()),
    _vehicle_odometry_publisher(std::make_shared<VehicleOdometryPublisherImpl>()),
    _vehicle_speed_publisher(std::make_shared<VehicleSpeedPublisherImpl>()),
    _vehicle_telemetry_publisher(std::make_shared<VehicleTelemetryDataPublisherImpl>()),
    _vehicle_object_publisher(std::make_shared<ObjectPublisher>(*this, objects_publisher)),
    _vehicle_object_with_covariance_publisher(std::make_shared<ObjectWithCovariancePublisher>(*this, objects_with_covariance_publisher)) {
  // prefill some vehicle info data
  _vehicle_info_publisher->Message().id(vehicle_actor_definition->id);
  _vehicle_info_publisher->Message().type(vehicle_actor_definition->type_id);
  _vehicle_info_publisher->Message().rolename(vehicle_actor_definition->role_name);
  for (auto wheel : vehicle_actor_definition->vehicle_physics_control.GetWheels()) {
    auto wheel_info = carla_msgs::msg::CarlaEgoVehicleInfoWheel();
    wheel_info.tire_friction(wheel.tire_friction);
    wheel_info.damping_rate(wheel.damping_rate);
    wheel_info.max_steer_angle(carla::geom::Math::ToRadians(wheel.max_steer_angle));
    wheel_info.radius(wheel.radius);
    wheel_info.max_brake_torque(wheel.max_brake_torque);
    wheel_info.max_handbrake_torque(wheel.max_handbrake_torque);

    auto wheel_position = wheel.position;
    // TODO: do we have to divide here by 100? (such was in ros brigde, but to my undertanding and search in the source
    // code, it might be already correct. If not, then better to switch type of wheel_position from Vector3D to Location
    // to have automatic cm -> m conversion object->Transform().GetTransform().InverseTransformPoint(wheel_position);
    wheel_info.position(CoordinateSystemTransform::TransformLocationToVector3Msg(wheel_position));
    _vehicle_info_publisher->Message().wheels().push_back(wheel_info);
  }
  _vehicle_info_publisher->Message().max_rpm(vehicle_actor_definition->vehicle_physics_control.max_rpm);
  _vehicle_info_publisher->Message().moi(vehicle_actor_definition->vehicle_physics_control.moi);
  _vehicle_info_publisher->Message().damping_rate_full_throttle(
      vehicle_actor_definition->vehicle_physics_control.damping_rate_full_throttle);
  _vehicle_info_publisher->Message().damping_rate_zero_throttle_clutch_engaged(
      vehicle_actor_definition->vehicle_physics_control.damping_rate_zero_throttle_clutch_engaged);
  _vehicle_info_publisher->Message().damping_rate_zero_throttle_clutch_disengaged(
      vehicle_actor_definition->vehicle_physics_control.damping_rate_zero_throttle_clutch_disengaged);
  _vehicle_info_publisher->Message().use_gear_autobox(vehicle_actor_definition->vehicle_physics_control.use_gear_autobox);
  _vehicle_info_publisher->Message().gear_switch_time(vehicle_actor_definition->vehicle_physics_control.gear_switch_time);
  _vehicle_info_publisher->Message().clutch_strength(vehicle_actor_definition->vehicle_physics_control.clutch_strength);
  _vehicle_info_publisher->Message().mass(vehicle_actor_definition->vehicle_physics_control.mass);
  _vehicle_info_publisher->Message().drag_coefficient(vehicle_actor_definition->vehicle_physics_control.drag_coefficient);
  _vehicle_info_publisher->Message().center_of_mass(CoordinateSystemTransform::TransformLocationToVector3Msg(
      vehicle_actor_definition->vehicle_physics_control.center_of_mass));
  _vehicle_info_publisher->SetMessageUpdated();
}

bool VehiclePublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _vehicle_info_publisher->Init(domain_participant, get_topic_name("vehicle_info"), PublisherBase::get_topic_qos()) &&
         _vehicle_status_publisher->Init(domain_participant, get_topic_name("vehicle_status"), get_topic_qos()) &&
         _vehicle_odometry_publisher->Init(domain_participant, get_topic_name("odometry"), get_topic_qos()) &&
         _vehicle_speed_publisher->Init(domain_participant, get_topic_name("speed"), get_topic_qos()) &&
         _vehicle_telemetry_publisher->Init(domain_participant, get_topic_name("vehicle_telemetry"), get_topic_qos()) &&
         _vehicle_object_publisher->Init(domain_participant) &&
         _vehicle_object_with_covariance_publisher->Init(domain_participant);
}

bool VehiclePublisher::Publish() {
  bool success = _vehicle_info_publisher->Publish();
  success &= _vehicle_status_publisher->Publish();
  success &= _vehicle_odometry_publisher->Publish();
  success &= _vehicle_speed_publisher->Publish();
  success &= _vehicle_telemetry_publisher->Publish();
  success &= _vehicle_object_publisher->Publish();
  success &= _vehicle_object_with_covariance_publisher->Publish();
  return success;
}

bool VehiclePublisher::SubscribersConnected() const {
  return _vehicle_info_publisher->SubscribersConnected() || _vehicle_status_publisher->SubscribersConnected() ||
         _vehicle_odometry_publisher->SubscribersConnected() || _vehicle_speed_publisher->SubscribersConnected() ||
         _vehicle_telemetry_publisher->SubscribersConnected() ||
         _vehicle_object_publisher->SubscribersConnected() || 
         _vehicle_object_with_covariance_publisher->SubscribersConnected();
}

bool VehiclePublisher::ProcessMessages() {
  // the telemetry data is not transferred by the sensor data stream,
  // it has to be requested separately from the server,
  // This should happen within the message processing step, when also other calls are expected
  // to ensure the simulation internal data is actually locked and its safe to acceess it. 
  if (_vehicle_telemetry_publisher->SubscribersConnected()) {
    auto telemetry_data_response = _carla_server.call_get_telemetry_data(_actor_name_definition->id);
    if (telemetry_data_response.HasError()) {
      carla::log_warning("VehiclePublisher: Failed to get telemetry data for actor id ",
                        std::to_string(_actor_name_definition->id), ":", telemetry_data_response.GetError().What());
    }
    else {
      auto const telemetry_data = telemetry_data_response.Get();
      _vehicle_telemetry_publisher->Message().throttle(telemetry_data.throttle);
      _vehicle_telemetry_publisher->Message().steer(telemetry_data.steer);
      _vehicle_telemetry_publisher->Message().brake(telemetry_data.brake);
      _vehicle_telemetry_publisher->Message().engine_rpm(telemetry_data.engine_rpm);
      _vehicle_telemetry_publisher->Message().gear(telemetry_data.gear);
      _vehicle_telemetry_publisher->Message().drag(telemetry_data.drag);
      
      _vehicle_telemetry_publisher->Message().wheels().clear();
      for (auto const &wheel: telemetry_data.wheels) {
        carla_msgs::msg::CarlaEgoVehicleTelemetryDataWheel wheel_msg;
        wheel_msg.tire_friction(wheel.tire_friction);
        wheel_msg.lat_slip(wheel.lat_slip);
        wheel_msg.long_slip(wheel.long_slip);
        wheel_msg.omega(wheel.omega);
        wheel_msg.tire_load(wheel.tire_load);
        wheel_msg.normalized_tire_load(wheel.normalized_tire_load);
        wheel_msg.torque(wheel.torque);
        wheel_msg.long_force(wheel.long_force);
        wheel_msg.lat_force(wheel.lat_force);
        wheel_msg.normalized_long_force(wheel.normalized_long_force);
        wheel_msg.normalized_lat_force(wheel.normalized_lat_force);
        _vehicle_telemetry_publisher->Message().wheels().push_back(wheel_msg);
      }

      _vehicle_telemetry_publisher->SetMessageUpdated();
    }
    auto light_state_response = _carla_server.call_get_vehicle_light_state(_actor_name_definition->id);
    if (light_state_response.HasError()) {
      carla::log_warning("VehiclePublisher: Failed to get vehicle light state for actor id ",
                        std::to_string(_actor_name_definition->id), ":", light_state_response.GetError().What());
    } else {
      auto const light_state = light_state_response.Get();
      _vehicle_telemetry_publisher->Message().light_state_flags(light_state.light_state);
      _vehicle_telemetry_publisher->SetMessageUpdated();
    }
  }
  return true;
}

void VehiclePublisher::UpdateVehicle(std::shared_ptr<const carla::ros2::types::Object> &object,
                                     carla::sensor::data::ActorDynamicState const &actor_dynamic_state) {
  _vehicle_odometry_publisher->SetMessageHeader(object->Timestamp().time(), "map");
  _vehicle_odometry_publisher->Message().child_frame_id(frame_id());
  _vehicle_odometry_publisher->Message().pose(object->Transform().pose_with_covariance());
  _vehicle_odometry_publisher->Message().twist(object->AcceleratedMovement().twist_with_covariance());
  
  _vehicle_speed_publisher->Message().data(object->Speed().speed().data());
  _vehicle_speed_publisher->SetMessageUpdated();

  // add the timestamp and frame_id to telemetry data
  _vehicle_telemetry_publisher->SetMessageHeader(object->Timestamp().time(), "map");

  _vehicle_status_publisher->SetMessageHeader(object->Timestamp().time(), frame_id());
  _vehicle_status_publisher->Message().velocity(object->Speed().speed().data());
  _vehicle_status_publisher->Message().acceleration(object->AcceleratedMovement().accel());
  _vehicle_status_publisher->Message().orientation(object->Transform().pose().orientation());
  _vehicle_status_publisher->Message().active_control_type(carla::ros2::types::GetVehicleControlType(actor_dynamic_state));
  _vehicle_status_publisher->Message().last_applied_vehicle_control(
      carla::ros2::types::VehicleControl(actor_dynamic_state.state.vehicle_data.GetVehicleControl())
          .carla_vehicle_control());
  _vehicle_status_publisher->Message().last_applied_ackermann_control(
      carla::ros2::types::VehicleAckermannControl(actor_dynamic_state.state.vehicle_data.GetAckermannControl())
          .carla_vehicle_ackermann_control());

  _vehicle_object_publisher->UpdateObject(object);
  _vehicle_object_with_covariance_publisher->UpdateObject(object);
}

}  // namespace ros2
}  // namespace carla
