// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <limits>

#include "carla/geom/BoundingBox.h"
#include "carla/ros2/ROS2NameRegistry.h"
#include "carla/ros2/types/AcceleratedMovement.h"
#include "carla/ros2/types/Polygon.h"
#include "carla/ros2/types/Timestamp.h"
#include "carla/ros2/types/TrafficLightActorDefinition.h"
#include "carla/ros2/types/TrafficSignActorDefinition.h"
#include "carla/ros2/types/Transform.h"
#include "carla/ros2/types/VehicleActorDefinition.h"
#include "carla/ros2/types/WalkerActorDefinition.h"
#include "carla/rpc/VehiclePhysicsControl.h"
#include "carla/sensor/data/ActorDynamicState.h"
#include "derived_object_msgs/msg/Object.h"
#include "carla_msgs/msg/CarlaActorInfo.h"

namespace carla {
namespace ros2 {
namespace types {

/**
  Convert a carla (linear) acceleration to a ROS accel (linear part)

  Considers the conversion from left-handed system (unreal) to right-handed
  system (ROS)
*/
class Object {
public:
  /**
   * The representation of an object in the sense of derived_object_msgs::msg::Object.
   *
   * classification is one of the derived_object_msgs::msg::Object_Constants::CLASSIFICATION_* constants
   */
  explicit Object(std::shared_ptr<carla::ros2::types::VehicleActorDefinition> vehicle_actor_definition)
    : _actor_name_definition(
          std::static_pointer_cast<carla::ros2::types::ActorNameDefinition>(vehicle_actor_definition)) {

    if (_actor_name_definition->base_type == "Bus" || _actor_name_definition->base_type == "Truck" ) {
      _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_TRUCK;
    }
    else if (_actor_name_definition->base_type == "car" || _actor_name_definition->base_type == "van") {
      _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_CAR;
    }
    else if (_actor_name_definition->base_type == "motorcycle") {
      _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_MOTORCYCLE;
    } else if (_actor_name_definition->base_type == "bicycle") {
      _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_BIKE;
    }
    else {
      // as long as we don't have the concrete information within a blueprint ...
      // we estimate the class based on the vehicle mass (motorbikes are also 4wheeled vehicles!)
      if (vehicle_actor_definition->vehicle_physics_control.mass > 2000.f) {
        _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_TRUCK;
      }
      /* microlino has 513kg */
      else if (vehicle_actor_definition->vehicle_physics_control.mass > 500.f) {
        _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_CAR;
      }
      /* gazelle bike has 150 (ok, when 130kg person is sitting on it ;-), but yamaha 140kg how should that work out??
        TODO: update Blueprint masses to more realistic values */
      else if (vehicle_actor_definition->vehicle_physics_control.mass > 100.f) {
        _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_MOTORCYCLE;
      } 
      else {
        _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_BIKE;
      }
      carla::log_warning("Unknown Vehicle Object[", _actor_name_definition->type_id, "] id: ", _actor_name_definition->id,
                      " object_type: ", _actor_name_definition->object_type,
                      " base_type: ", _actor_name_definition->base_type,
                      " mass: ", vehicle_actor_definition->vehicle_physics_control.mass, " ROS-class: ", _classification);
    }
  }
  /**
   * The representation of an object in the sense of derived_object_msgs::msg::Object.
   *
   * classification is one of the derived_object_msgs::msg::Object_Constants::CLASSIFICATION_* constants
   */
  explicit Object(std::shared_ptr<carla::ros2::types::WalkerActorDefinition> walker_actor_definition)
    : _actor_name_definition(
          std::static_pointer_cast<carla::ros2::types::ActorNameDefinition>(walker_actor_definition)) {
    _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_PEDESTRIAN;
    carla::log_info("Creating Walker Object[", _actor_name_definition->type_id, "] id: ", _actor_name_definition->id,
                    " object_type: ", _actor_name_definition->object_type,
                    " base_type: ", _actor_name_definition->base_type, " ROS-class: ", _classification);
  }
  /**
   * The representation of an object in the sense of derived_object_msgs::msg::Object.
   *
   * classification is one of the derived_object_msgs::msg::Object_Constants::CLASSIFICATION_* constants
   */
  explicit Object(std::shared_ptr<carla::ros2::types::TrafficLightActorDefinition> traffic_light_actor_definition)
    : _actor_name_definition(
          std::static_pointer_cast<carla::ros2::types::ActorNameDefinition>(traffic_light_actor_definition)) {
    _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_SIGN;
    carla::log_info("Creating Traffic Light Object[", _actor_name_definition->type_id,
                    "] id: ", _actor_name_definition->id, " object_type: ", _actor_name_definition->object_type,
                    " base_type: ", _actor_name_definition->base_type, " ROS-class: ", _classification);
  }
  /**
   * The representation of an object in the sense of derived_object_msgs::msg::Object.
   *
   * classification is one of the derived_object_msgs::msg::Object_Constants::CLASSIFICATION_* constants
   */
  explicit Object(std::shared_ptr<carla::ros2::types::TrafficSignActorDefinition> traffic_sign_actor_definition)
    : _actor_name_definition(
          std::static_pointer_cast<carla::ros2::types::ActorNameDefinition>(traffic_sign_actor_definition)) {
    _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_SIGN;
    carla::log_info("Creating Traffic Sign Object[", _actor_name_definition->type_id,
                    "] id: ", _actor_name_definition->id, " object_type: ", _actor_name_definition->object_type,
                    " base_type: ", _actor_name_definition->base_type, " ROS-class: ", _classification);
  }
  ~Object() = default;
  Object(const Object&) = delete;
  Object& operator=(const Object&) = delete;
  Object(Object&&) = delete;
  Object& operator=(Object&&) = delete;

  void UpdateObject(carla::ros2::types::Timestamp const& timestamp,
                    carla::sensor::data::ActorDynamicState const& actor_dynamic_state) {
    auto actor_definition = std::dynamic_pointer_cast<carla::ros2::types::ActorDefinition>(_actor_name_definition);
    if (nullptr != actor_definition) {
      _bounding_box.extent = actor_definition->bounding_box.extent;
      _bounding_box.location = actor_dynamic_state.transform.location;
      _bounding_box.rotation = actor_dynamic_state.transform.rotation;
    }
    _transform = carla::ros2::types::Transform(actor_dynamic_state.transform, actor_dynamic_state.quaternion);
    _accelerated_movement.UpdateSpeed(
        carla::ros2::types::Speed(carla::geom::Velocity(actor_dynamic_state.velocity), actor_dynamic_state.quaternion),
        carla::ros2::types::AngularVelocity(carla::geom::AngularVelocity(actor_dynamic_state.angular_velocity)),
        timestamp);
    if (_classification_age < std::numeric_limits<uint32_t>::max()) {
      ++_classification_age;
    }
  }

  derived_object_msgs::msg::Object object() const {
    derived_object_msgs::msg::Object object;
    object.header().stamp(_accelerated_movement.Timestamp().time());
    object.header().frame_id("map");
    object.id(_actor_name_definition->id);
    object.detection_level(derived_object_msgs::msg::Object_Constants::OBJECT_TRACKED);
    object.object_classified(true);
    object.pose(_transform.pose());
    object.twist(_accelerated_movement.twist());
    object.accel(_accelerated_movement.accel());

    auto actor_definition = std::dynamic_pointer_cast<carla::ros2::types::ActorDefinition>(_actor_name_definition);
    if (nullptr != actor_definition) {
      object.shape().type(shape_msgs::msg::SolidPrimitive_Constants::BOX);
      auto const ros_extent = _bounding_box.extent * 2.f;
      object.shape().dimensions({ros_extent.x, ros_extent.y, ros_extent.z});
      object.shape().polygon().points(*Polygon(_bounding_box.GetLocalVertices()).polygon());
    } else {
      object.shape().type(shape_msgs::msg::SolidPrimitive_Constants::BOX_X);
    }
    object.classification(_classification);
    object.classification_certainty(255u);
    object.classification_age(_classification_age);
    return object;
  }

  carla_msgs::msg::CarlaActorInfo carla_actor_info(std::shared_ptr<ROS2NameRegistry> name_registry) const {
    carla_msgs::msg::CarlaActorInfo actor_info;
    actor_info.id(_actor_name_definition->id);
    actor_info.parent_id(name_registry->ParentActorId(_actor_name_definition->id));
    actor_info.type(_actor_name_definition->type_id);
    actor_info.rosname(_actor_name_definition->ros_name);
    actor_info.rolename(_actor_name_definition->role_name);
    actor_info.object_type(_actor_name_definition->object_type);
    actor_info.base_type(_actor_name_definition->base_type);
    auto topic_prefix = name_registry->TopicPrefix(_actor_name_definition->id);
    // remove "rt" prefix
    actor_info.topic_prefix(topic_prefix.substr(3));
    return actor_info;
  }

  carla::ros2::types::Timestamp const& Timestamp() const {
    return _accelerated_movement.Timestamp();
  }
  carla::ros2::types::Transform const& Transform() const {
    return _transform;
  }
  carla::ros2::types::Speed const& Speed() const {
    return _accelerated_movement.Speed();
  }
  carla::ros2::types::AngularVelocity const& AngularVelocity() const {
    return _accelerated_movement.AngularVelocity();
  }
  carla::ros2::types::AcceleratedMovement const& AcceleratedMovement() const {
    return _accelerated_movement;
  }

  uint8_t classification() {
    return _classification;
  }

private:
  std::shared_ptr<carla::ros2::types::ActorNameDefinition> _actor_name_definition;
  uint8_t _classification;
  carla::geom::BoundingBox _bounding_box;
  carla::ros2::types::Transform _transform;
  carla::ros2::types::AcceleratedMovement _accelerated_movement;
  uint32_t _classification_age{0u};
};
}  // namespace types
}  // namespace ros2
}  // namespace carla