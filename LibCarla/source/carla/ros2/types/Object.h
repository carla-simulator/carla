// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <limits>

#include "carla/geom/BoundingBox.h"
#include "carla/ros2/types/AcceleratedMovement.h"
#include "carla/ros2/types/Polygon.h"
#include "carla/ros2/types/Timestamp.h"
#include "carla/ros2/types/TrafficLightActorDefinition.h"
#include "carla/ros2/types/TrafficSignActorDefinition.h"
#include "carla/ros2/types/Transform.h"
#include "carla/ros2/types/VehicleActorDefinition.h"
#include "carla/ros2/types/WalkerActorDefinition.h"
#include "carla/rpc/VehiclePhysicsControl.h"
#include "carla/rpc/EnvironmentObject.h"
#include "carla/sensor/data/ActorDynamicState.h"
#include "derived_object_msgs/msg/Object.h"
#include "derived_object_msgs/msg/ObjectWithCovariance.h"


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
    : _actor_definition(
          std::static_pointer_cast<carla::ros2::types::ActorDefinition>(vehicle_actor_definition)) {
    
    _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_OTHER_VEHICLE;
    if (_actor_definition->base_type == "Bus" || _actor_definition->base_type == "Truck"
        || _actor_definition->base_type == "bus" || _actor_definition->base_type == "truck") {
      _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_TRUCK;
    } else if (_actor_definition->base_type == "car" || _actor_definition->base_type == "van") {
      _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_CAR;
    } else if (_actor_definition->base_type == "motorcycle") {
      _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_MOTORCYCLE;
    } else if (_actor_definition->base_type == "bicycle") {
      _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_BIKE;
    } else {
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
      } else {
        _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_BIKE;
      }
      carla::log_warning(
          "Unknown Vehicle Object[", _actor_definition->type_id, "] id: ", _actor_definition->id,
          " object_type: ", _actor_definition->object_type, " base_type: ", _actor_definition->base_type,
          " mass: ", vehicle_actor_definition->vehicle_physics_control.mass, " estimated ROS-class based on mass: ", classification_string());
    }
  }
  /**
   * The representation of an object in the sense of derived_object_msgs::msg::Object.
   *
   * classification is one of the derived_object_msgs::msg::Object_Constants::CLASSIFICATION_* constants
   */
  explicit Object(std::shared_ptr<carla::ros2::types::WalkerActorDefinition> walker_actor_definition)
    : _actor_definition(
          std::static_pointer_cast<carla::ros2::types::ActorDefinition>(walker_actor_definition)) {
    _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_PEDESTRIAN;
    carla::log_verbose("Creating Walker Object[", _actor_definition->type_id, "] id: ", _actor_definition->id,
                    " object_type: ", _actor_definition->object_type,
                    " base_type: ", _actor_definition->base_type, " ROS-class: ", classification_string());
  }
  /**
   * The representation of an object in the sense of derived_object_msgs::msg::Object.
   *
   * classification is one of the derived_object_msgs::msg::Object_Constants::CLASSIFICATION_* constants
   */
  explicit Object(std::shared_ptr<carla::ros2::types::TrafficLightActorDefinition> traffic_light_actor_definition)
    : _actor_definition(
          std::static_pointer_cast<carla::ros2::types::ActorDefinition>(traffic_light_actor_definition)) {
    _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_SIGN;
    _classification_age = std::numeric_limits<uint32_t>::max();
    carla::log_verbose("Creating Traffic Light Object[", _actor_definition->type_id,
                    "] id: ", _actor_definition->id, " object_type: ", _actor_definition->object_type,
                    " base_type: ", _actor_definition->base_type, " ROS-class: ", classification_string());
  }
  /**
   * The representation of an object in the sense of derived_object_msgs::msg::Object.
   *
   * classification is one of the derived_object_msgs::msg::Object_Constants::CLASSIFICATION_* constants
   */
  explicit Object(std::shared_ptr<carla::ros2::types::TrafficSignActorDefinition> traffic_sign_actor_definition)
    : _actor_definition(
          std::static_pointer_cast<carla::ros2::types::ActorDefinition>(traffic_sign_actor_definition)) {
    _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_SIGN;
    _classification_age = std::numeric_limits<uint32_t>::max();
    carla::log_verbose("Creating Traffic Sign Object[", _actor_definition->type_id,
                    "] id: ", _actor_definition->id, " object_type: ", _actor_definition->object_type,
                    " base_type: ", _actor_definition->base_type, " ROS-class: ", classification_string());
  }

  explicit Object(carla::rpc::EnvironmentObject environment_object, bool enable_for_ros)
    : _actor_definition(std::make_shared<carla::ros2::types::ActorDefinition>(environment_object, enable_for_ros)) {
    _classification_age = std::numeric_limits<uint32_t>::max();

    // derived object msgs are somewhat limited in terms of classification support
    // therefore also an actor list for environment objects will be published
    // containing the exact tag
    switch(environment_object.type) {
      case carla::rpc::CityObjectLabel::Pedestrians:
          _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_PEDESTRIAN;
          break;
      case carla::rpc::CityObjectLabel::Rider:
      case carla::rpc::CityObjectLabel::Bicycle:
          _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_BIKE;
          break;
      case carla::rpc::CityObjectLabel::Car:
          _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_CAR;
          break;
      case carla::rpc::CityObjectLabel::Motorcycle:
          _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_MOTORCYCLE;
          break;
      case carla::rpc::CityObjectLabel::Bus:
      case carla::rpc::CityObjectLabel::Truck:
          _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_TRUCK;
          break;
      case carla::rpc::CityObjectLabel::TrafficLight:
      case carla::rpc::CityObjectLabel::TrafficSigns:
        _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_SIGN;
        break;
      case carla::rpc::CityObjectLabel::Train:
          _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_OTHER_VEHICLE;
          break;
      case carla::rpc::CityObjectLabel::Poles:
      case carla::rpc::CityObjectLabel::Fences:
      case carla::rpc::CityObjectLabel::Walls:
          _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_BARRIER;
          break;
      case carla::rpc::CityObjectLabel::Buildings:
      case carla::rpc::CityObjectLabel::Static:
      default:
          _classification = derived_object_msgs::msg::Object_Constants::CLASSIFICATION_UNKNOWN;
    }

    // and put in our object state update
    carla::sensor::data::ActorDynamicState actor_dynamic_state;
    actor_dynamic_state.id = actor_id();
    actor_dynamic_state.transform = environment_object.transform;
    actor_dynamic_state.quaternion = carla::geom::Quaternion(environment_object.transform.rotation);
    UpdateObject(carla::ros2::types::Timestamp(), actor_dynamic_state);
    carla::log_verbose("Creating Environment Object[", _actor_definition->type_id,
                    "] id: ", actor_id(), " object_type: ", _actor_definition->object_type,
                    " base_type: ", _actor_definition->base_type, " ROS-class: ", classification_string());
  }

  ~Object() = default;
  Object(const Object&) = delete;
  Object& operator=(const Object&) = delete;
  Object(Object&&) = delete;
  Object& operator=(Object&&) = delete;

  void UpdateObject(carla::ros2::types::Timestamp const& timestamp,
                    carla::sensor::data::ActorDynamicState const& actor_dynamic_state) {
    _bounding_box.extent = _actor_definition->bounding_box.extent;
    _bounding_box.location = actor_dynamic_state.transform.location;
    _bounding_box.rotation = actor_dynamic_state.transform.rotation;
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
    object.id(_actor_definition->id);
    object.detection_level(derived_object_msgs::msg::Object_Constants::OBJECT_TRACKED);
    object.object_classified(true);
    object.pose(_transform.pose());
    object.twist(_accelerated_movement.twist());
    object.accel(_accelerated_movement.accel());
    object.shape().type(shape_msgs::msg::SolidPrimitive_Constants::BOX);
    auto const ros_extent = _bounding_box.extent * 2.f;
    object.shape().dimensions({ros_extent.x, ros_extent.y, ros_extent.z});
    object.shape().polygon().points(*Polygon(_bounding_box.GetLocalVertices()).polygon());
    object.classification(_classification);
    object.classification_certainty(255u);
    object.classification_age(_classification_age);
    return object;
  }

  derived_object_msgs::msg::ObjectWithCovariance object_with_covariance() const {
    derived_object_msgs::msg::ObjectWithCovariance object;
    object.header().stamp(_accelerated_movement.Timestamp().time());
    object.header().frame_id("map");
    object.id(_actor_definition->id);
    object.detection_level(derived_object_msgs::msg::Object_Constants::OBJECT_TRACKED);
    object.object_classified(true);
    object.pose(_transform.pose_with_covariance());
    object.twist(_accelerated_movement.twist_with_covariance());
    object.accel(_accelerated_movement.accel_with_covariance());
    object.shape().type(shape_msgs::msg::SolidPrimitive_Constants::BOX);
    auto const ros_extent = _bounding_box.extent * 2.f;
    object.shape().dimensions({ros_extent.x, ros_extent.y, ros_extent.z});
    object.classification(_classification);
    object.classification_certainty(255u);
    object.classification_age(_classification_age);
    return object;
  }

  /**
   * @brief check if dynamic content has changed (ignoring timestamp)
   */
  bool has_dynamic_data_changed(derived_object_msgs::msg::Object const &other) const {
     return (other.id()!=_actor_definition->id)
           || (other.pose() != _transform.pose())
           || (other.twist() != _accelerated_movement.twist())
           || (other.accel() != _accelerated_movement.accel());
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

  uint8_t classification() const {
    return _classification;
  }

  std::string classification_string() const {
    switch (_classification) {
      case derived_object_msgs::msg::Object_Constants::CLASSIFICATION_UNKNOWN:
        return "UNKNOWN";
      case derived_object_msgs::msg::Object_Constants::CLASSIFICATION_UNKNOWN_SMALL:
        return "UNKNOWN_SMALL";
      case derived_object_msgs::msg::Object_Constants::CLASSIFICATION_UNKNOWN_MEDIUM:
        return "UNKNOWN_MEDIUM";
      case derived_object_msgs::msg::Object_Constants::CLASSIFICATION_UNKNOWN_BIG:
        return "UNKNOWN_BIG";
      case derived_object_msgs::msg::Object_Constants::CLASSIFICATION_PEDESTRIAN:
        return "PEDESTRIAN";
      case derived_object_msgs::msg::Object_Constants::CLASSIFICATION_BIKE:
        return "BIKE";
      case derived_object_msgs::msg::Object_Constants::CLASSIFICATION_CAR:
        return "CAR";
      case derived_object_msgs::msg::Object_Constants::CLASSIFICATION_TRUCK:
        return "TRUCK";
      case derived_object_msgs::msg::Object_Constants::CLASSIFICATION_MOTORCYCLE:
        return "MOTORCYCLE";
      case derived_object_msgs::msg::Object_Constants::CLASSIFICATION_OTHER_VEHICLE:
        return "OTHER_VEHICLE";
      case derived_object_msgs::msg::Object_Constants::CLASSIFICATION_BARRIER:
        return "BARRIER";
      case derived_object_msgs::msg::Object_Constants::CLASSIFICATION_SIGN:
        return "SIGN";
      default:
        return "N/A";
    }
  }

  carla_msgs::msg::CarlaActorInfo carla_actor_info(std::shared_ptr<ROS2NameRegistry> name_registry = nullptr) const {
    return _actor_definition->carla_actor_info(name_registry);
  }

  carla::streaming::detail::actor_id_type actor_id() const { return _actor_definition->id; }

  const carla::ros2::types::ActorDefinition& actor_definition()const { return *_actor_definition; }

  const carla::ros2::types::ActorNameDefinition& actor_name_definition()const { return *_actor_definition; }

private:
  std::shared_ptr<carla::ros2::types::ActorDefinition> _actor_definition;
  uint8_t _classification{derived_object_msgs::msg::Object_Constants::CLASSIFICATION_UNKNOWN};
  carla::geom::BoundingBox _bounding_box;
  carla::ros2::types::Transform _transform;
  carla::ros2::types::AcceleratedMovement _accelerated_movement;
  uint32_t _classification_age{0u};
};
}  // namespace types
}  // namespace ros2
}  // namespace carla