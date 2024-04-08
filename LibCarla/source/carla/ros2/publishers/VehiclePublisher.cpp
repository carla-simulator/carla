// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "VehiclePublisher.h"

#include "carla/ros2/impl/fastdds/DdsPublisherImpl.h"
#include "carla/ros2/types/Speed.h"
#include "carla/ros2/types/VehicleAckermannControl.h"
#include "carla/ros2/types/VehicleControl.h"

namespace carla {
namespace ros2 {

VehiclePublisher::VehiclePublisher(std::shared_ptr<carla::ros2::types::VehicleActorDefinition> vehicle_actor_definition,
                                   std::shared_ptr<TransformPublisher> transform_publisher,
                                   std::shared_ptr<ObjectsPublisher> objects_publisher)
  : PublisherBaseTransform(std::static_pointer_cast<carla::ros2::types::ActorNameDefinition>(vehicle_actor_definition),
                           transform_publisher),
    _vehicle_info(std::make_shared<VehicleInfoPublisherImpl>()),
    _vehicle_status(std::make_shared<VehicleStatusPublisherImpl>()),
    _vehicle_object_publisher(std::make_shared<ObjectPublisher>(*this, objects_publisher)) {
  // prefill some vehicle info data
  _vehicle_info->Message().id(vehicle_actor_definition->id);
  _vehicle_info->Message().type(vehicle_actor_definition->type_id);
  _vehicle_info->Message().rolename(vehicle_actor_definition->role_name);
  _vehicle_info->Message().shape().type(shape_msgs::msg::SolidPrimitive_Constants::BOX);
  auto const ros_extent = vehicle_actor_definition->bounding_box.extent * 2.f;
  _vehicle_info->Message().shape().dimensions({ros_extent.x, ros_extent.y, ros_extent.z});
  _vehicle_info->Message().shape().polygon().points(*vehicle_actor_definition->vertex_polygon.polygon());
  for (auto wheel : vehicle_actor_definition->vehicle_physics_control.GetWheels()) {
    auto wheel_info = carla_msgs::msg::CarlaVehicleInfoWheel();
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
    _vehicle_info->Message().wheels().push_back(wheel_info);
  }
  _vehicle_info->Message().max_rpm(vehicle_actor_definition->vehicle_physics_control.max_rpm);
  _vehicle_info->Message().moi(vehicle_actor_definition->vehicle_physics_control.moi);
  _vehicle_info->Message().damping_rate_full_throttle(
      vehicle_actor_definition->vehicle_physics_control.damping_rate_full_throttle);
  _vehicle_info->Message().damping_rate_zero_throttle_clutch_engaged(
      vehicle_actor_definition->vehicle_physics_control.damping_rate_zero_throttle_clutch_engaged);
  _vehicle_info->Message().damping_rate_zero_throttle_clutch_disengaged(
      vehicle_actor_definition->vehicle_physics_control.damping_rate_zero_throttle_clutch_disengaged);
  _vehicle_info->Message().use_gear_autobox(vehicle_actor_definition->vehicle_physics_control.use_gear_autobox);
  _vehicle_info->Message().gear_switch_time(vehicle_actor_definition->vehicle_physics_control.gear_switch_time);
  _vehicle_info->Message().clutch_strength(vehicle_actor_definition->vehicle_physics_control.clutch_strength);
  _vehicle_info->Message().mass(vehicle_actor_definition->vehicle_physics_control.mass);
  _vehicle_info->Message().drag_coefficient(vehicle_actor_definition->vehicle_physics_control.drag_coefficient);
  _vehicle_info->Message().center_of_mass(CoordinateSystemTransform::TransformLocationToVector3Msg(
      vehicle_actor_definition->vehicle_physics_control.center_of_mass));
  _vehicle_info->SetMessageUpdated();
}

bool VehiclePublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _vehicle_info->Init(domain_participant, get_topic_name("vehicle_info"), PublisherBase::get_topic_qos()) &&
         _vehicle_status->Init(domain_participant, get_topic_name("vehicle_status"), get_topic_qos()) &&
         _vehicle_object_publisher->Init(domain_participant);
}

bool VehiclePublisher::Publish() {
  if (!_vehicle_info_published) {
    _vehicle_info_published = _vehicle_info->Publish();
  }
  bool success = _vehicle_info_published;
  success &= _vehicle_status->Publish();
  success &= _vehicle_object_publisher->Publish();
  return success;
}

bool VehiclePublisher::SubscribersConnected() const {
  return _vehicle_info->SubscribersConnected() || _vehicle_status->SubscribersConnected() ||
         _vehicle_object_publisher->SubscribersConnected();
}

void VehiclePublisher::UpdateVehicle(std::shared_ptr<carla::ros2::types::Object> &object,
                                     carla::sensor::data::ActorDynamicState const &actor_dynamic_state) {
  _vehicle_status->SetMessageHeader(object->Timestamp().time(), "map");
  _vehicle_status->Message().pose(object->Transform().pose());
  _vehicle_status->Message().twist(object->AcceleratedMovement().twist());
  _vehicle_status->Message().velocity(object->Speed().speed().data());
  _vehicle_status->Message().acceleration(object->AcceleratedMovement().accel());
  _vehicle_status->Message().active_control_type(carla::ros2::types::GetVehicleControlType(actor_dynamic_state));
  _vehicle_status->Message().last_applied_vehicle_control(
      carla::ros2::types::VehicleControl(actor_dynamic_state.state.vehicle_data.GetVehicleControl())
          .carla_vehicle_control());
  _vehicle_status->Message().last_applied_ackermann_control(
      carla::ros2::types::VehicleAckermannControl(actor_dynamic_state.state.vehicle_data.GetAckermannControl())
          .carla_vehicle_ackermann_control());

  _vehicle_object_publisher->UpdateObject(object);
}

}  // namespace ros2
}  // namespace carla
