#pragma once

#include <carla/Memory.h>
#include <carla/PythonUtil.h>
#include <carla/Time.h>

#include <carla/geom/BoundingBox.h>
#include <carla/geom/GeoLocation.h>
#include <carla/geom/Location.h>
#include <carla/geom/Rotation.h>
#include <carla/geom/Transform.h>
#include <carla/geom/Vector2D.h>
#include <carla/geom/Vector3D.h>

#include <rpc/config.h>
#include <rpc/rpc_error.h>
#include <carla/rpc/VehicleAckermannControl.h>
#include <carla/rpc/VehicleControl.h>
#include <carla/rpc/VehiclePhysicsControl.h>
#include <carla/rpc/WheelPhysicsControl.h>
#include <carla/rpc/WalkerControl.h>
#include <carla/rpc/WalkerBoneControlIn.h>
#include <carla/rpc/WalkerBoneControlOut.h>
#include <carla/rpc/TrafficLightState.h>
#include <carla/rpc/ActorId.h>
#include <carla/rpc/Command.h>
#include <carla/rpc/CommandResponse.h>
#include <carla/rpc/WeatherParameters.h>
#include <carla/rpc/EnvironmentObject.h>
#include <carla/rpc/ObjectLabel.h>

#include <carla/client/Actor.h>
#include <carla/client/TrafficLight.h>
#include <carla/client/Vehicle.h>
#include <carla/client/Walker.h>
#include <carla/client/WalkerAIController.h>
#include <carla/client/LightManager.h>
#include <carla/client/Junction.h>
#include <carla/client/Map.h>
#include <carla/client/Waypoint.h>
#include <carla/client/Landmark.h>
#include <carla/client/ClientSideSensor.h>
#include <carla/client/LaneInvasionSensor.h>
#include <carla/client/Sensor.h>
#include <carla/client/ServerSideSensor.h>
#include <carla/client/BlueprintLibrary.h>
#include <carla/client/ActorBlueprint.h>
#include <carla/client/Client.h>
#include <carla/client/ActorList.h>
#include <carla/client/World.h>

#include <carla/trafficmanager/TrafficManager.h>
#include <carla/trafficmanager/SimpleWaypoint.h>

#include <carla/Logging.h>
#include <carla/FileSystem.h>

#include <carla/road/element/LaneMarking.h>
#include <carla/road/SignalType.h>

#include <carla/pointcloud/PointCloudIO.h>

#include <carla/sensor/SensorData.h>
#include <carla/sensor/data/CollisionEvent.h>
#include <carla/sensor/data/IMUMeasurement.h>
#include <carla/sensor/data/ObstacleDetectionEvent.h>
#include <carla/sensor/data/Image.h>
#include <carla/sensor/data/LaneInvasionEvent.h>
#include <carla/sensor/data/LidarMeasurement.h>
#include <carla/sensor/data/SemanticLidarMeasurement.h>
#include <carla/sensor/data/GnssMeasurement.h>
#include <carla/sensor/data/RadarMeasurement.h>
#include <carla/sensor/data/DVSEventArray.h>
#include <carla/sensor/data/RadarData.h>

#include <carla/image/ImageConverter.h>
#include <carla/image/ImageIO.h>
#include <carla/image/ImageView.h>

#include <cstdio>
#include <ostream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <thread>
#include <fstream>
#include <chrono>
#include <memory>
#include <sstream>
#include <type_traits>
#include <vector>


#if __has_include(<OSM2ODR.h>)
  #define CARLA_PYTHON_API_HAS_OSM2ODR
#endif

template <typename OptionalT>
static auto OptionalToPyObject(OptionalT &optional) {
  return optional.has_value() ?
    boost::python::object(*optional) :
    boost::python::object();
}

// Convenient for requests without arguments.
#define CALL_WITHOUT_GIL(cls, fn) +[](cls &self) { \
      carla::PythonUtil::ReleaseGIL unlock; \
      return self.fn(); \
    }

// Convenient for requests with 1 argument.
#define CALL_WITHOUT_GIL_1(cls, fn, T1_) +[](cls &self, T1_ t1) { \
      carla::PythonUtil::ReleaseGIL unlock; \
      return self.fn(std::forward<T1_>(t1)); \
    }

// Convenient for requests with 2 arguments.
#define CALL_WITHOUT_GIL_2(cls, fn, T1_, T2_) +[](cls &self, T1_ t1, T2_ t2) { \
      carla::PythonUtil::ReleaseGIL unlock; \
      return self.fn(std::forward<T1_>(t1), std::forward<T2_>(t2)); \
    }

// Convenient for requests with 3 arguments.
#define CALL_WITHOUT_GIL_3(cls, fn, T1_, T2_, T3_) +[](cls &self, T1_ t1, T2_ t2, T3_ t3) { \
      carla::PythonUtil::ReleaseGIL unlock; \
      return self.fn(std::forward<T1_>(t1), std::forward<T2_>(t2), std::forward<T3_>(t3)); \
    }

// Convenient for requests with 4 arguments.
#define CALL_WITHOUT_GIL_4(cls, fn, T1_, T2_, T3_, T4_) +[](cls &self, T1_ t1, T2_ t2, T3_ t3, T4_ t4) { \
      carla::PythonUtil::ReleaseGIL unlock; \
      return self.fn(std::forward<T1_>(t1), std::forward<T2_>(t2), std::forward<T3_>(t3), std::forward<T4_>(t4)); \
    }

// Convenient for requests with 5 arguments.
#define CALL_WITHOUT_GIL_5(cls, fn, T1_, T2_, T3_, T4_, T5_) +[](cls &self, T1_ t1, T2_ t2, T3_ t3, T4_ t4, T5_ t5) { \
      carla::PythonUtil::ReleaseGIL unlock; \
      return self.fn(std::forward<T1_>(t1), std::forward<T2_>(t2), std::forward<T3_>(t3), std::forward<T4_>(t4), std::forward<T5_>(t5)); \
    }

// Convenient for const requests without arguments.
#define CONST_CALL_WITHOUT_GIL(cls, fn) CALL_WITHOUT_GIL(const cls, fn)
#define CONST_CALL_WITHOUT_GIL_1(cls, fn, T1_) CALL_WITHOUT_GIL_1(const cls, fn, T1_)
#define CONST_CALL_WITHOUT_GIL_2(cls, fn, T1_, T2_) CALL_WITHOUT_GIL_2(const cls, fn, T1_, T2_)
#define CONST_CALL_WITHOUT_GIL_3(cls, fn, T1_, T2_, T3_) CALL_WITHOUT_GIL_3(const cls, fn, T1_, T2_, T3_)
#define CONST_CALL_WITHOUT_GIL_4(cls, fn, T1_, T2_, T3_, T4_) CALL_WITHOUT_GIL_4(const cls, fn, T1_, T2_, T3_, T4_)

// Convenient for const requests that need to make a copy of the returned value.
#define CALL_RETURNING_COPY(cls, fn) +[](const cls &self) \
        -> std::decay_t<std::invoke_result_t<decltype(&cls::fn), cls*>> { \
      return self.fn(); \
    }

// Convenient for const requests that need to make a copy of the returned value.
#define CALL_RETURNING_COPY_1(cls, fn, T1_) +[](const cls &self, T1_ t1) \
        -> std::decay_t<std::invoke_result_t<decltype(&cls::fn), cls*, T1_>> { \
      return self.fn(std::forward<T1_>(t1)); \
    }

template<typename T>
std::vector<T> PyListToVector(boost::python::list &input) {
  std::vector<T> result;
  auto list_size = boost::python::len(input);
  for (decltype(list_size) i = 0; i < list_size; ++i)
    result.emplace_back(boost::python::extract<T>(input[i]));
  return result;
}

// Convenient for const requests that needs to convert the return value to a
// Python list.
#define CALL_RETURNING_LIST(cls, fn) +[](const cls &self) { \
      boost::python::list result; \
      for (auto &&item : self.fn()) { \
        result.append(item); \
      } \
      return result; \
    }

// Convenient for const requests that needs to convert the return value to a
// Python list.
#define CALL_RETURNING_LIST_1(cls, fn, T1_) +[](const cls &self, T1_ t1) { \
      boost::python::list result; \
      for (auto &&item : self.fn(std::forward<T1_>(t1))) { \
        result.append(item); \
      } \
      return result; \
    }

#define CALL_RETURNING_LIST_2(cls, fn, T1_, T2_) +[](const cls &self, T1_ t1, T2_ t2) { \
      boost::python::list result; \
      for (auto &&item : self.fn(std::forward<T1_>(t1), std::forward<T2_>(t2))) { \
        result.append(item); \
      } \
      return result; \
    }

#define CALL_RETURNING_LIST_3(cls, fn, T1_, T2_, T3_) +[](const cls &self, T1_ t1, T2_ t2, T3_ t3) { \
      boost::python::list result; \
      for (auto &&item : self.fn(std::forward<T1_>(t1), std::forward<T2_>(t2), std::forward<T3_>(t3))) { \
        result.append(item); \
      } \
      return result; \
    }

#define CALL_RETURNING_OPTIONAL(cls, fn) +[](const cls &self) { \
      auto optional = self.fn(); \
      return OptionalToPyObject(optional); \
    }

#define CALL_RETURNING_OPTIONAL_1(cls, fn, T1_) +[](const cls &self, T1_ t1) { \
      auto optional = self.fn(std::forward<T1_>(t1)); \
      return OptionalToPyObject(optional); \
    }

#define CALL_RETURNING_OPTIONAL_2(cls, fn, T1_, T2_) +[](const cls &self, T1_ t1, T2_ t2) { \
      auto optional = self.fn(std::forward<T1_>(t1), std::forward<T2_>(t2)); \
      return OptionalToPyObject(optional); \
    }

#define CALL_RETURNING_OPTIONAL_3(cls, fn, T1_, T2_, T3_) +[](const cls &self, T1_ t1, T2_ t2, T3_ t3) { \
      auto optional = self.fn(std::forward<T1_>(t1), std::forward<T2_>(t2), std::forward<T3_>(t3)); \
      return OptionalToPyObject(optional); \
    }

#define CALL_RETURNING_OPTIONAL_WITHOUT_GIL(cls, fn) +[](const cls &self) { \
      auto call = CONST_CALL_WITHOUT_GIL(cls, fn); \
      auto optional = call(self); \
      return optional.has_value() ? boost::python::object(*optional) : boost::python::object(); \
    }

template <typename T>
void PrintListItem(std::ostream &out, const T &item) {
  out << item;
}

template <typename T>
void PrintListItem(std::ostream &out, const carla::SharedPtr<T> &item) {
  if (item == nullptr) {
    out << "nullptr";
  } else {
    out << *item;
  }
}

template <typename Iterable>
std::ostream &PrintList(std::ostream &out, const Iterable &list) {
  out << '[';
  if (!list.empty()) {
    auto it = list.begin();
    PrintListItem(out, *it);
    for (++it; it != list.end(); ++it) {
      out << ", ";
      PrintListItem(out, *it);
    }
  }
  out << ']';
  return out;
}

namespace std {
  template <typename T>
  inline std::ostream& operator<<(std::ostream& out, const std::vector<T>& vector_of_stuff) {
    return PrintList(out, vector_of_stuff);
  }

  template <typename T, typename H>
  inline std::ostream& operator<<(std::ostream& out, const std::pair<T, H>& data) {
    out << "(" << data.first << "," << data.second << ")";
    return out;
  }
} // namespace std

namespace carla {
namespace geom {

  template <typename T>
  void WriteVector2D(std::ostream &out, const char *name, const T &vector2D) {
    out << name
        << "(x=" << std::to_string(vector2D.x)
        << ", y=" << std::to_string(vector2D.y) << ')';
  }

  template <typename T>
  void WriteVector3D(std::ostream &out, const char *name, const T &vector3D) {
    out << name
        << "(x=" << std::to_string(vector3D.x)
        << ", y=" << std::to_string(vector3D.y)
        << ", z=" << std::to_string(vector3D.z) << ')';
  }

  inline std::ostream &operator<<(std::ostream &out, const Vector2D &vector2D) {
    WriteVector2D(out, "Vector2D", vector2D);
    return out;
  }

  inline std::ostream &operator<<(std::ostream &out, const Vector3D &vector3D) {
    WriteVector3D(out, "Vector3D", vector3D);
    return out;
  }

  inline std::ostream &operator<<(std::ostream &out, const Location &location) {
    WriteVector3D(out, "Location", location);
    return out;
  }

  inline std::ostream &operator<<(std::ostream &out, const Rotation &rotation) {
    out << "Rotation(pitch=" << std::to_string(rotation.pitch)
        << ", yaw=" << std::to_string(rotation.yaw)
        << ", roll=" << std::to_string(rotation.roll) << ')';
    return out;
  }

  inline std::ostream &operator<<(std::ostream &out, const Transform &transform) {
    out << "Transform(" << transform.location << ", " << transform.rotation << ')';
    return out;
  }

  inline std::ostream &operator<<(std::ostream &out, const BoundingBox &box) {
    out << "BoundingBox(" << box.location << ", ";
    WriteVector3D(out, "Extent", box.extent);
    out << ", " << box.rotation;
    out << ')';
    return out;
  }

  inline std::ostream &operator<<(std::ostream &out, const GeoLocation &geo_location) {
    out << "GeoLocation(latitude=" << std::to_string(geo_location.latitude)
        << ", longitude=" << std::to_string(geo_location.longitude)
        << ", altitude=" << std::to_string(geo_location.altitude) << ')';
    return out;
  }

} // namespace geom

namespace sensor {
namespace data {

  inline std::ostream &operator<<(std::ostream &out, const Color &color) {
    out << "Color(" << std::to_string(color.r)
        << ',' << std::to_string(color.g)
        << ',' << std::to_string(color.b)
        << ',' << std::to_string(color.a) << ')';
    return out;
  }

  inline std::ostream &operator<<(std::ostream &out, const OpticalFlowPixel &color) {
    out << "Color(" << std::to_string(color.x)
        << ',' << std::to_string(color.y) << ')';
    return out;
  }

} // namespace data
} // namespace sensor

namespace client {

  inline std::ostream &operator<<(std::ostream &out, const ActorAttribute &attr) {
    using Type = carla::rpc::ActorAttributeType;
    static_assert(static_cast<uint8_t>(Type::SIZE) == 6u, "Please update this function.");
    out << "ActorAttribute(id=" << attr.GetId();
    switch (attr.GetType()) {
      case Type::Bool:
        out << ",type=bool,value=" << (attr.As<bool>() ? "True" : "False");
        break;
      case Type::Int:
        out << ",type=int,value=" << attr.As<int>();
        break;
      case Type::Float:
        out << ",type=float,value=" << attr.As<float>();
        break;
      case Type::String:
        out << ",type=str,value=" << attr.As<std::string>();
        break;
      case Type::RGBColor:
        out << ",type=Color,value=" << attr.As<sensor::data::Color>();
        break;
      case Type::Vector:
        out << ",type=vector,value= (not implemented yet)";
        break;
      default:
        out << ",INVALID";
    }
    if (!attr.IsModifiable()) {
      out << "(const)";
    }
    out << ')';
    return out;
  }

  inline std::ostream &operator<<(std::ostream &out, const ActorBlueprint &bp) {
    out << "ActorBlueprint(id=" << bp.GetId() << ",tags=" << bp.GetTags() << ')';
    return out;
  }

  inline std::ostream &operator<<(std::ostream &out, const BlueprintLibrary &blueprints) {
    return PrintList(out, blueprints);
  }

} // namespace client

namespace client {

  inline std::ostream &operator<<(std::ostream &out, const Actor &actor) {
    out << "Actor(id=" << actor.GetId() << ", type=" << actor.GetTypeId() << ')';
    return out;
  }

} // namespace client

namespace rpc {

  inline auto boolalpha(bool b) {
    return b ? "True" : "False";
  }

  inline std::ostream &operator<<(std::ostream &out, const VehicleControl &control) {
    out << "VehicleControl(throttle=" << std::to_string(control.throttle)
        << ", steer=" << std::to_string(control.steer)
        << ", brake=" << std::to_string(control.brake)
        << ", hand_brake=" << boolalpha(control.hand_brake)
        << ", reverse=" << boolalpha(control.reverse)
        << ", manual_gear_shift=" << boolalpha(control.manual_gear_shift)
        << ", gear=" << std::to_string(control.gear) << ')';
    return out;
  }

  inline std::ostream &operator<<(std::ostream &out, const VehicleAckermannControl &control) {
    out << "VehicleAckermannControl(steer=" << std::to_string(control.steer)
        << ", steer_speed=" << std::to_string(control.steer_speed)
        << ", speed=" << std::to_string(control.speed)
        << ", acceleration=" << std::to_string(control.acceleration)
        << ", jerk=" << std::to_string(control.jerk) << ')';
    return out;
  }

  inline std::ostream &operator<<(std::ostream &out, const WalkerControl &control) {
    out << "WalkerControl(direction=" << control.direction
        << ", speed=" << std::to_string(control.speed)
        << ", jump=" << boolalpha(control.jump) << ')';
    return out;
  }

  inline std::ostream &operator<<(std::ostream &out, const WalkerBoneControlIn &control) {
    out << "WalkerBoneControlIn(bone_transforms(";
    for (auto bone_transform : control.bone_transforms) {
      out << "(name="  << bone_transform.first
          << ", transform=" << bone_transform.second << ')';
    }
    out << "))";
    return out;
  }

  inline std::ostream &operator<<(std::ostream &out, const BoneTransformDataOut &data) {
    out << "BoneTransformDataOut(name="  << data.bone_name << ", world=" << data.world << ", component=" << data.component << ", relative=" << data.relative << ')';
    return out;
  }

  inline std::ostream &operator<<(std::ostream &out, const WalkerBoneControlOut &control) {
    out << "WalkerBoneControlOut(bone_transforms(";
    for (auto bone_transform : control.bone_transforms) {
      out << "(name="  << bone_transform.bone_name
          << ", world=" << bone_transform.world << ", component=" << bone_transform.component << ", relative=" << bone_transform.relative << ')';
    }
    out << "))";
    return out;
  }

  inline std::ostream &operator<<(std::ostream &out, const WheelPhysicsControl &control) {
    out << "WheelPhysicsControl(axle_type=" << std::to_string(control.axle_type)
      << ", offset" << control.offset
      << ", wheel_radius=" << std::to_string(control.wheel_radius)
      << ", wheel_width=" << std::to_string(control.wheel_width)
      << ", wheel_mass=" << std::to_string(control.wheel_mass)
      << ", cornering_stiffness=" << std::to_string(control.cornering_stiffness)
      << ", friction_force_multiplier=" << std::to_string(control.friction_force_multiplier)
      << ", side_slip_modifier=" << std::to_string(control.side_slip_modifier)
      << ", slip_threshold=" << std::to_string(control.slip_threshold)
      << ", skid_threshold=" << std::to_string(control.skid_threshold)
      << ", max_steer_angle=" << std::to_string(control.max_steer_angle)
      << ", affected_by_steering=" << std::to_string(control.affected_by_steering)
      << ", affected_by_brake=" << std::to_string(control.affected_by_brake)
      << ", affected_by_handbrake=" << std::to_string(control.affected_by_handbrake)
      << ", affected_by_engine=" << std::to_string(control.affected_by_engine)
      << ", abs_enabled=" << std::to_string(control.abs_enabled)
      << ", traction_control_enabled=" << std::to_string(control.traction_control_enabled)
      << ", max_wheelspin_rotation=" << std::to_string(control.max_wheelspin_rotation)
      << ", external_torque_combine_method=" << std::to_string(control.external_torque_combine_method)
      << ", lateral_slip_graph=" << control.lateral_slip_graph
      << ", suspension_axis=" << control.suspension_axis
      << ", suspension_force_offset=" << control.suspension_force_offset
      << ", suspension_max_raise=" << std::to_string(control.suspension_max_raise)
      << ", suspension_max_drop=" << std::to_string(control.suspension_max_drop)
      << ", suspension_damping_ratio=" << std::to_string(control.suspension_damping_ratio)
      << ", wheel_load_ratio=" << std::to_string(control.wheel_load_ratio)
      << ", spring_rate=" << std::to_string(control.spring_rate)
      << ", spring_preload=" << std::to_string(control.spring_preload)
      << ", suspension_smoothing=" << std::to_string(control.suspension_smoothing)
      << ", rollbar_scaling=" << std::to_string(control.rollbar_scaling)
      << ", sweep_shape=" << std::to_string(control.sweep_shape)
      << ", sweep_type=" << std::to_string(control.sweep_type)
      << ", max_brake_torque=" << std::to_string(control.max_brake_torque)
      << ", max_hand_brake_torque=" << std::to_string(control.max_hand_brake_torque)
      << ", wheel_index=" << std::to_string(control.wheel_index)
      << ", location=" << control.location
      << ", old_location=" << control.old_location
      << ", velocity=" << control.velocity
      << ')';
    return out;
  }

  inline std::ostream &operator<<(std::ostream &out, const VehiclePhysicsControl &control) {
    out << "VehiclePhysicsControl(torque_curve=" << control.torque_curve
      << ", max_torque=" << control.max_torque
      << ", max_rpm=" << control.max_rpm
      << ", idle_rpm=" << control.idle_rpm
      << ", brake_effect=" << control.brake_effect
      << ", rev_up_moi=" << control.rev_up_moi
      << ", rev_down_rate=" << control.rev_down_rate
      << ", differential_type=" << control.differential_type
      << ", front_rear_split=" << control.front_rear_split
      << ", use_automatic_gears=" << control.use_automatic_gears
      << ", gear_change_time=" << control.gear_change_time
      << ", final_ratio=" << control.final_ratio
      << ", forward_gear_ratios=" << control.forward_gear_ratios
      << ", reverse_gear_ratios=" << control.reverse_gear_ratios
      << ", change_up_rpm=" << control.change_up_rpm
      << ", change_down_rpm=" << control.change_down_rpm
      << ", transmission_efficiency=" << control.transmission_efficiency
      << ", mass=" << control.mass
      << ", drag_coefficient=" << control.drag_coefficient
      << ", center_of_mass=" << control.center_of_mass
      << ", chassis_width=" << control.chassis_width
      << ", chassis_height=" << control.chassis_height
      << ", downforce_coefficient=" << control.downforce_coefficient
      << ", drag_area=" << control.drag_area
      << ", inertia_tensor_scale=" << control.inertia_tensor_scale
      << ", sleep_threshold=" << control.sleep_threshold
      << ", sleep_slope_limit=" << control.sleep_slope_limit
      << ", steering_curve=" << control.steering_curve
      << ", wheels=" << control.wheels
      << ", use_sweep_wheel_collision=" << control.use_sweep_wheel_collision
      << ")";
    return out;
  }

  inline std::ostream &operator<<(std::ostream &out, const AckermannControllerSettings &settings) {
    out << "AckermannControllerSettings(speed_kp=" << std::to_string(settings.speed_kp)
        << ", speed_ki=" << std::to_string(settings.speed_ki)
        << ", speed_kd=" << std::to_string(settings.speed_kd)
        << ", accel_kp=" << std::to_string(settings.accel_kp)
        << ", accel_ki=" << std::to_string(settings.accel_ki)
        << ", accel_kd=" << std::to_string(settings.accel_kd)  << ')';
    return out;
  }

} // namespace rpc

namespace client {

  inline std::ostream &operator<<(std::ostream &out, const Map &map) {
    out << "Map(name=" << map.GetName() << ')';
    return out;
  }

  inline std::ostream &operator<<(std::ostream &out, const Waypoint &waypoint) {
    out << "Waypoint(" << waypoint.GetTransform() << ')';
    return out;
  }

} // namespace client
} // namespace carla

inline carla::time_duration TimeDurationFromSeconds(double seconds) {
  size_t ms = static_cast<size_t>(1e3 * seconds);
  return carla::time_duration::milliseconds(ms);
}

inline auto MakeCallback(boost::python::object callback) {
  namespace py = boost::python;
  // Make sure the callback is actually callable.
  if (!PyCallable_Check(callback.ptr())) {
    PyErr_SetString(PyExc_TypeError, "callback argument must be callable!");
    py::throw_error_already_set();
  }

  // We need to delete the callback while holding the GIL.
  using Deleter = carla::PythonUtil::AcquireGILDeleter;
  auto callback_ptr = carla::SharedPtr<py::object>{new py::object(callback), Deleter()};

  // Make a lambda callback.
  return [callback=std::move(callback_ptr)](auto message) {
    carla::PythonUtil::AcquireGIL lock;
    try {
      py::call<void>(callback->ptr(), py::object(message));
    } catch (const py::error_already_set &) {
      PyErr_Print();
    }
  };
}
