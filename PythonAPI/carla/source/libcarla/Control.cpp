// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/rpc/VehicleAckermannControl.h>
#include <carla/rpc/VehicleControl.h>
#include <carla/rpc/VehiclePhysicsControl.h>
#include <carla/rpc/VehicleTelemetryData.h>
#include <carla/rpc/WheelPhysicsControl.h>
#include <carla/rpc/WheelTelemetryData.h>
#include <carla/rpc/WalkerControl.h>
#include <carla/rpc/WalkerBoneControlIn.h>
#include <carla/rpc/WalkerBoneControlOut.h>

#include <ostream>

namespace carla {
namespace rpc {

  static auto boolalpha(bool b) {
    return b ? "True" : "False";
  }

  std::ostream &operator<<(std::ostream &out, const VehicleControl &control) {
    out << "VehicleControl(throttle=" << std::to_string(control.throttle)
        << ", steer=" << std::to_string(control.steer)
        << ", brake=" << std::to_string(control.brake)
        << ", hand_brake=" << boolalpha(control.hand_brake)
        << ", reverse=" << boolalpha(control.reverse)
        << ", manual_gear_shift=" << boolalpha(control.manual_gear_shift)
        << ", gear=" << std::to_string(control.gear) << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const VehicleAckermannControl &control) {
    out << "VehicleAckermannControl(steer=" << std::to_string(control.steer)
        << ", steer_speed=" << std::to_string(control.steer_speed)
        << ", speed=" << std::to_string(control.speed)
        << ", acceleration=" << std::to_string(control.acceleration)
        << ", jerk=" << std::to_string(control.jerk) << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const WalkerControl &control) {
    out << "WalkerControl(direction=" << control.direction
        << ", speed=" << std::to_string(control.speed)
        << ", jump=" << boolalpha(control.jump) << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const WalkerBoneControlIn &control) {
    out << "WalkerBoneControlIn(bone_transforms(";
    for (auto bone_transform : control.bone_transforms) {
      out << "(name="  << bone_transform.first
          << ", transform=" << bone_transform.second << ')';
    }
    out << "))";
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const BoneTransformDataOut &data) {
    out << "BoneTransformDataOut(name="  << data.bone_name << ", world=" << data.world << ", component=" << data.component << ", relative=" << data.relative << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const WalkerBoneControlOut &control) {
    out << "WalkerBoneControlOut(bone_transforms(";
    for (auto bone_transform : control.bone_transforms) {
      out << "(name="  << bone_transform.bone_name
          << ", world=" << bone_transform.world << ", component=" << bone_transform.component << ", relative=" << bone_transform.relative << ')';
    }
    out << "))";
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const GearPhysicsControl &control) {
    out << "GearPhysicsControl(ratio=" << std::to_string(control.ratio)
        << ", down_ratio=" << std::to_string(control.down_ratio)
        << ", up_ratio=" << std::to_string(control.up_ratio) << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const WheelPhysicsControl &control) {
    out << "WheelPhysicsControl(tire_friction=" << std::to_string(control.tire_friction)
        << ", damping_rate=" << std::to_string(control.damping_rate)
        << ", max_steer_angle=" << std::to_string(control.max_steer_angle)
        << ", radius=" << std::to_string(control.radius)
        << ", max_brake_torque=" << std::to_string(control.max_brake_torque)
        << ", max_handbrake_torque=" << std::to_string(control.max_handbrake_torque)
        << ", lat_stiff_max_load=" << std::to_string(control.lat_stiff_max_load)
        << ", lat_stiff_value=" << std::to_string(control.lat_stiff_value)
        << ", long_stiff_value=" << std::to_string(control.long_stiff_value)
        << ", position=" << control.position << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const VehiclePhysicsControl &control) {
    out << "VehiclePhysicsControl(torque_curve=" << control.torque_curve
    << ", max_rpm=" << std::to_string(control.max_rpm)
    << ", moi=" << std::to_string(control.moi)
    << ", damping_rate_full_throttle=" << std::to_string(control.damping_rate_full_throttle)
    << ", damping_rate_zero_throttle_clutch_engaged=" << std::to_string(control.damping_rate_zero_throttle_clutch_engaged)
    << ", damping_rate_zero_throttle_clutch_disengaged=" << std::to_string(control.damping_rate_zero_throttle_clutch_disengaged)
    << ", use_gear_autobox=" << boolalpha(control.use_gear_autobox)
    << ", gear_switch_time=" << std::to_string(control.gear_switch_time)
    << ", clutch_strength=" << std::to_string(control.clutch_strength)
    << ", final_ratio=" << std::to_string(control.final_ratio)
    << ", forward_gears=" << control.forward_gears
    << ", mass=" << std::to_string(control.mass)
    << ", drag_coefficient=" << std::to_string(control.drag_coefficient)
    << ", center_of_mass=" << control.center_of_mass
    << ", steering_curve=" << control.steering_curve
    << ", wheels=" << control.wheels
    << ", use_sweep_wheel_collision=" << control.use_sweep_wheel_collision << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const WheelTelemetryData &telemetry) {
    out << "WheelTelemetryData(tire_friction=" << std::to_string(telemetry.tire_friction)
        << ", lat_slip=" << std::to_string(telemetry.lat_slip)
        << ", long_slip=" << std::to_string(telemetry.long_slip)
        << ", omega=" << std::to_string(telemetry.omega)
        << ", tire_load=" << std::to_string(telemetry.tire_load)
        << ", normalized_tire_load=" << std::to_string(telemetry.normalized_tire_load)
        << ", torque=" << std::to_string(telemetry.torque)
        << ", long_force=" << std::to_string(telemetry.long_force)
        << ", lat_force=" << std::to_string(telemetry.lat_force)
        << ", normalized_long_force=" << std::to_string(telemetry.normalized_long_force)
        << ", normalized_lat_force=" << std::to_string(telemetry.normalized_lat_force) << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const VehicleTelemetryData &telemetry) {
    out << "VehicleTelemetryData(speed=" << std::to_string(telemetry.speed)
    << ", steer=" << std::to_string(telemetry.steer)
    << ", throttle=" << std::to_string(telemetry.throttle)
    << ", brake=" << std::to_string(telemetry.brake)
    << ", engine_rpm=" << std::to_string(telemetry.engine_rpm)
    << ", gear=" << std::to_string(telemetry.gear)
    << ", drag=" << std::to_string(telemetry.drag)
    << ", wheels=" << telemetry.wheels << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const AckermannControllerSettings &settings) {
    out << "AckermannControllerSettings(speed_kp=" << std::to_string(settings.speed_kp)
        << ", speed_ki=" << std::to_string(settings.speed_ki)
        << ", speed_kd=" << std::to_string(settings.speed_kd)
        << ", accel_kp=" << std::to_string(settings.accel_kp)
        << ", accel_ki=" << std::to_string(settings.accel_ki)
        << ", accel_kd=" << std::to_string(settings.accel_kd)  << ')';
    return out;
  }

} // namespace rpc
} // namespace carla

static auto GetVectorOfVector2DFromList(const boost::python::list &list) {
  std::vector<carla::geom::Vector2D> v;

  auto length = boost::python::len(list);
  v.reserve(static_cast<size_t>(length));
  for (auto i = 0u; i < length; ++i) {
    boost::python::extract<carla::geom::Vector2D> ext(list[i]);
    if (ext.check()) {
      v.push_back(ext);
    } else {
      v.push_back(carla::geom::Vector2D{
        boost::python::extract<float>(list[i][0u]),
        boost::python::extract<float>(list[i][1u])});
    }
  }
  return v;
}

static auto GetVectorOfBoneTransformFromList(const boost::python::list &list) {
  std::vector<carla::rpc::BoneTransformDataIn> v;

  auto length = boost::python::len(list);
  v.reserve(static_cast<size_t>(length));
  for (auto i = 0u; i < length; ++i) {
    boost::python::extract<carla::rpc::BoneTransformDataIn> ext(list[i]);
    if (ext.check()) {
      v.push_back(ext);
    } else {
      v.push_back(carla::rpc::BoneTransformDataIn{
        boost::python::extract<std::string>(list[i][0u]),
        boost::python::extract<carla::geom::Transform>(list[i][1u])});
    }
  }
  return v;
}

static auto GetWheels(const carla::rpc::VehiclePhysicsControl &self) {
  const auto &wheels = self.GetWheels();
  boost::python::object get_iter = boost::python::iterator<std::vector<carla::rpc::WheelPhysicsControl>>();
  boost::python::object iter = get_iter(wheels);
  return boost::python::list(iter);
}

static void SetWheels(carla::rpc::VehiclePhysicsControl &self, const boost::python::list &list) {
  std::vector<carla::rpc::WheelPhysicsControl> wheels;
  auto length = boost::python::len(list);
  for (auto i = 0u; i < length; ++i) {
    wheels.push_back(boost::python::extract<carla::rpc::WheelPhysicsControl &>(list[i]));
  }
  self.wheels = wheels;
}

static auto GetForwardGears(const carla::rpc::VehiclePhysicsControl &self) {
  const auto &gears = self.GetForwardGears();
  boost::python::object get_iter = boost::python::iterator<std::vector<carla::rpc::GearPhysicsControl>>();
  boost::python::object iter = get_iter(gears);
  return boost::python::list(iter);
}

static void SetForwardGears(carla::rpc::VehiclePhysicsControl &self, const boost::python::list &list) {
  std::vector<carla::rpc::GearPhysicsControl> gears;
  auto length = boost::python::len(list);
  for (auto i = 0u; i < length; ++i) {
    gears.push_back(boost::python::extract<carla::rpc::GearPhysicsControl &>(list[i]));
  }
  self.SetForwardGears(gears);
}

static auto GetTorqueCurve(const carla::rpc::VehiclePhysicsControl &self) {
  const std::vector<carla::geom::Vector2D> &torque_curve = self.GetTorqueCurve();
  boost::python::object get_iter = boost::python::iterator<const std::vector<carla::geom::Vector2D>>();
  boost::python::object iter = get_iter(torque_curve);
  return boost::python::list(iter);
}

static void SetTorqueCurve(carla::rpc::VehiclePhysicsControl &self, const boost::python::list &list) {
  self.torque_curve = GetVectorOfVector2DFromList(list);
}

static auto GetSteeringCurve(const carla::rpc::VehiclePhysicsControl &self) {
  const std::vector<carla::geom::Vector2D> &steering_curve = self.GetSteeringCurve();
  boost::python::object get_iter = boost::python::iterator<const std::vector<carla::geom::Vector2D>>();
  boost::python::object iter = get_iter(steering_curve);
  return boost::python::list(iter);
}

static void SetSteeringCurve(carla::rpc::VehiclePhysicsControl &self, const boost::python::list &list) {
  self.steering_curve = GetVectorOfVector2DFromList(list);
}

boost::python::object VehiclePhysicsControl_init(boost::python::tuple args, boost::python::dict kwargs) {
  // Args names
  const uint32_t NUM_ARGUMENTS = 17;
  const char *args_names[NUM_ARGUMENTS] = {
    "torque_curve",
    "max_rpm",
    "moi",
    "damping_rate_full_throttle",
    "damping_rate_zero_throttle_clutch_engaged",
    "damping_rate_zero_throttle_clutch_disengaged",

    "use_gear_autobox",
    "gear_switch_time",
    "clutch_strength",
    "final_ratio",
    "forward_gears",

    "mass",
    "drag_coefficient",

    "center_of_mass",
    "steering_curve",
    "wheels",
    "use_sweep_wheel_collision",
  };

  boost::python::object self = args[0];
  args = boost::python::tuple(args.slice(1, boost::python::_));

  auto res = self.attr("__init__")();
  if (len(args) > 0) {
    for (unsigned int i = 0; i < len(args); ++i) {
      self.attr(args_names[i]) = args[i];
    }
  }

  for (unsigned int i = 0; i < NUM_ARGUMENTS; ++i) {
    if (kwargs.contains(args_names[i])) {
      self.attr(args_names[i]) = kwargs[args_names[i]];
    }
  }

  return res;
}

static auto GetWheelsTelemetry(const carla::rpc::VehicleTelemetryData &self) {
  const auto &wheels = self.GetWheels();
  boost::python::object get_iter = boost::python::iterator<std::vector<carla::rpc::WheelTelemetryData>>();
  boost::python::object iter = get_iter(wheels);
  return boost::python::list(iter);
}

static void SetWheelsTelemetry(carla::rpc::VehicleTelemetryData &self, const boost::python::list &list) {
  std::vector<carla::rpc::WheelTelemetryData> wheels;
  auto length = boost::python::len(list);
  for (auto i = 0u; i < length; ++i) {
    wheels.push_back(boost::python::extract<carla::rpc::WheelTelemetryData &>(list[i]));
  }
  self.SetWheels(wheels);
}

boost::python::object VehicleTelemetryData_init(boost::python::tuple args, boost::python::dict kwargs) {
  // Args names
  const uint32_t NUM_ARGUMENTS = 7;
  const char *args_names[NUM_ARGUMENTS] = {
    "speed",
    "steer",
    "throttle",
    "brake",
    "engine_rpm",
    "gear",
    "wheels"
  };

  boost::python::object self = args[0];
  args = boost::python::tuple(args.slice(1, boost::python::_));

  auto res = self.attr("__init__")();
  if (len(args) > 0) {
    for (unsigned int i = 0; i < len(args); ++i) {
      self.attr(args_names[i]) = args[i];
    }
  }

  for (unsigned int i = 0; i < NUM_ARGUMENTS; ++i) {
    if (kwargs.contains(args_names[i])) {
      self.attr(args_names[i]) = kwargs[args_names[i]];
    }
  }

  return res;
}

static auto GetBonesTransform(const carla::rpc::WalkerBoneControlIn &self) {
  const std::vector<carla::rpc::BoneTransformDataIn> &bone_transform_data = self.bone_transforms;
  boost::python::object get_iter =
      boost::python::iterator<const std::vector<carla::rpc::BoneTransformDataIn>>();
  boost::python::object iter = get_iter(bone_transform_data);
  return boost::python::list(iter);
}

static void SetBonesTransform(carla::rpc::WalkerBoneControlIn &self, const boost::python::list &list) {
  self.bone_transforms = GetVectorOfBoneTransformFromList(list);
}

static auto GetBonesTransformOut(const carla::rpc::WalkerBoneControlOut &self) {
  const std::vector<carla::rpc::BoneTransformDataOut> &bone_transform_data = self.bone_transforms;
  boost::python::object get_iter =
      boost::python::iterator<const std::vector<carla::rpc::BoneTransformDataOut>>();
  boost::python::object iter = get_iter(bone_transform_data);
  return boost::python::list(iter);
}

boost::python::object WalkerBoneControl_init(boost::python::tuple args, boost::python::dict kwargs) {
  // Args names
  const uint32_t NUM_ARGUMENTS = 1;
  const char *args_names[NUM_ARGUMENTS] = {
    "bone_transforms"
  };

  boost::python::object self = args[0];
  args = boost::python::tuple(args.slice(1, boost::python::_));

  auto res = self.attr("__init__")();
  if (len(args) > 0) {
    for (unsigned int i = 0; i < len(args); ++i) {
      self.attr(args_names[i]) = args[i];
    }
  }

  for (unsigned int i = 0; i < NUM_ARGUMENTS; ++i) {
    if (kwargs.contains(args_names[i])) {
      self.attr(args_names[i]) = kwargs[args_names[i]];
    }
  }

  return res;
}

void export_control() {
  using namespace boost::python;
  namespace cg = carla::geom;
  namespace cr = carla::rpc;

  class_<cr::VehicleControl>("VehicleControl")
    .def(init<float, float, float, bool, bool, bool, int>(
      (arg("throttle") = 0.0f,
      arg("steer") = 0.0f,
      arg("brake") = 0.0f,
      arg("hand_brake") = false,
      arg("reverse") = false,
      arg("manual_gear_shift") = false,
      arg("gear") = 0)))
    .def_readwrite("throttle", &cr::VehicleControl::throttle)
    .def_readwrite("steer", &cr::VehicleControl::steer)
    .def_readwrite("brake", &cr::VehicleControl::brake)
    .def_readwrite("hand_brake", &cr::VehicleControl::hand_brake)
    .def_readwrite("reverse", &cr::VehicleControl::reverse)
    .def_readwrite("manual_gear_shift", &cr::VehicleControl::manual_gear_shift)
    .def_readwrite("gear", &cr::VehicleControl::gear)
    .def("__eq__", &cr::VehicleControl::operator==)
    .def("__ne__", &cr::VehicleControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::VehicleAckermannControl>("VehicleAckermannControl")
    .def(init<float, float, float, float, float>(
      (arg("steer") = 0.0f,
      arg("steer_speed") = 0.0f,
      arg("speed") = 0.0f,
      arg("acceleration") = 0.0f,
      arg("jerk") = 0.0f)))
    .def_readwrite("steer", &cr::VehicleAckermannControl::steer)
    .def_readwrite("steer_speed", &cr::VehicleAckermannControl::steer_speed)
    .def_readwrite("speed", &cr::VehicleAckermannControl::speed)
    .def_readwrite("acceleration", &cr::VehicleAckermannControl::acceleration)
    .def_readwrite("jerk", &cr::VehicleAckermannControl::jerk)
    .def("__eq__", &cr::VehicleAckermannControl::operator==)
    .def("__ne__", &cr::VehicleAckermannControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::AckermannControllerSettings>("AckermannControllerSettings")
    .def(init<float, float, float, float, float, float>(
      (arg("speed_kp") = 0.0f,
      arg("speed_ki") = 0.0f,
      arg("speed_kd") = 0.0f,
      arg("accel_kp") = 0.0f,
      arg("accel_ki") = 0.0f,
      arg("accel_kd") = 0.0f)))
    .def_readwrite("speed_kp", &cr::AckermannControllerSettings::speed_kp)
    .def_readwrite("speed_ki", &cr::AckermannControllerSettings::speed_ki)
    .def_readwrite("speed_kd", &cr::AckermannControllerSettings::speed_kd)
    .def_readwrite("accel_kp", &cr::AckermannControllerSettings::accel_kp)
    .def_readwrite("accel_ki", &cr::AckermannControllerSettings::accel_ki)
    .def_readwrite("accel_kd", &cr::AckermannControllerSettings::accel_kd)
    .def("__eq__", &cr::AckermannControllerSettings::operator==)
    .def("__ne__", &cr::AckermannControllerSettings::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::WalkerControl>("WalkerControl")
    .def(init<cg::Vector3D, float, bool>(
       (arg("direction") = cg::Vector3D{1.0f, 0.0f, 0.0f},
       arg("speed") = 0.0f,
       arg("jump") = false)))
    .def_readwrite("direction", &cr::WalkerControl::direction)
    .def_readwrite("speed", &cr::WalkerControl::speed)
    .def_readwrite("jump", &cr::WalkerControl::jump)
    .def("__eq__", &cr::WalkerControl::operator==)
    .def("__ne__", &cr::WalkerControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::BoneTransformDataIn>("bone_transform")
    .def(init<>())
    .def_readwrite("name", &std::pair<std::string, cg::Transform>::first)
    .def_readwrite("transform", &std::pair<std::string, cg::Transform>::second)
    .def(self_ns::str(self_ns::self))
  ;

  class_<std::vector<cr::BoneTransformDataIn>>("vector_of_bones")
    .def(init<>())
    .def(boost::python::vector_indexing_suite<std::vector<cr::BoneTransformDataIn>>())
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::BoneTransformDataOut>("bone_transform_out")
    .def(init<>())
    .def_readwrite("name", &cr::BoneTransformDataOut::bone_name)
    .def_readwrite("world", &cr::BoneTransformDataOut::world)
    .def_readwrite("component", &cr::BoneTransformDataOut::component)
    .def_readwrite("relative", &cr::BoneTransformDataOut::relative)
    .def(self_ns::str(self_ns::self))
    .def("__eq__", &cr::BoneTransformDataOut::operator==)
    .def("__ne__", &cr::BoneTransformDataOut::operator!=)
  ;

  class_<std::vector<cr::BoneTransformDataOut>>("vector_of_bones_out")
    .def(init<>())
    .def(boost::python::vector_indexing_suite<std::vector<cr::BoneTransformDataOut>>())
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::WalkerBoneControlIn>("WalkerBoneControlIn")
    .def("__init__", raw_function(WalkerBoneControl_init))
    .def(init<>())
    .add_property("bone_transforms", &GetBonesTransform, &SetBonesTransform)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::WalkerBoneControlOut>("WalkerBoneControlOut")
    .def("__init__", raw_function(WalkerBoneControl_init))
    .def(init<>())
    // .add_property("bone_transforms", &GetBonesTransformOut, &SetBonesTransformOut)
    .add_property("bone_transforms", &GetBonesTransformOut)
    .def(self_ns::str(self_ns::self))
  ;

  class_<std::vector<cr::GearPhysicsControl>>("vector_of_gears")
      .def(boost::python::vector_indexing_suite<std::vector<cr::GearPhysicsControl>>())
      .def(self_ns::str(self_ns::self))
  ;

  class_<cr::GearPhysicsControl>("GearPhysicsControl")
    .def(init<float, float, float>(
        (arg("ratio")=1.0f,
         arg("down_ratio")=0.5f,
         arg("up_ratio")=0.65f)))
    .def_readwrite("ratio", &cr::GearPhysicsControl::ratio)
    .def_readwrite("down_ratio", &cr::GearPhysicsControl::down_ratio)
    .def_readwrite("up_ratio", &cr::GearPhysicsControl::up_ratio)
    .def("__eq__", &cr::GearPhysicsControl::operator==)
    .def("__ne__", &cr::GearPhysicsControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<std::vector<cr::WheelPhysicsControl>>("vector_of_wheels")
    .def(boost::python::vector_indexing_suite<std::vector<cr::WheelPhysicsControl>>())
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::WheelPhysicsControl>("WheelPhysicsControl")
    .def(init<float, float, float, float, float, float, float, float, float, cg::Vector3D>(
        (arg("tire_friction")=2.0f,
         arg("damping_rate")=0.25f,
         arg("max_steer_angle")=70.0f,
         arg("radius")=30.0f,
         arg("max_brake_torque")=1500.0f,
         arg("max_handbrake_torque")=3000.0f,
         arg("lat_stiff_max_load")=2.0f,
         arg("lat_stiff_value")=17.0f,
         arg("long_stiff_value")=1000.0f,
         arg("position")=cg::Vector3D{0.0f, 0.0f, 0.0f})))
    .def_readwrite("tire_friction", &cr::WheelPhysicsControl::tire_friction)
    .def_readwrite("damping_rate", &cr::WheelPhysicsControl::damping_rate)
    .def_readwrite("max_steer_angle", &cr::WheelPhysicsControl::max_steer_angle)
    .def_readwrite("radius", &cr::WheelPhysicsControl::radius)
    .def_readwrite("max_brake_torque", &cr::WheelPhysicsControl::max_brake_torque)
    .def_readwrite("max_handbrake_torque", &cr::WheelPhysicsControl::max_handbrake_torque)
    .def_readwrite("lat_stiff_max_load", &cr::WheelPhysicsControl::lat_stiff_max_load)
    .def_readwrite("lat_stiff_value", &cr::WheelPhysicsControl::lat_stiff_value)
    .def_readwrite("long_stiff_value", &cr::WheelPhysicsControl::long_stiff_value)
    .def_readwrite("position", &cr::WheelPhysicsControl::position)
    .def("__eq__", &cr::WheelPhysicsControl::operator==)
    .def("__ne__", &cr::WheelPhysicsControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::VehiclePhysicsControl>("VehiclePhysicsControl", no_init)
    .def("__init__", raw_function(VehiclePhysicsControl_init))
    .def(init<>())
    .add_property("torque_curve", &GetTorqueCurve, &SetTorqueCurve)
    .def_readwrite("max_rpm", &cr::VehiclePhysicsControl::max_rpm)
    .def_readwrite("moi", &cr::VehiclePhysicsControl::moi)
    .def_readwrite("damping_rate_full_throttle",
        &cr::VehiclePhysicsControl::damping_rate_full_throttle)
    .def_readwrite("damping_rate_zero_throttle_clutch_engaged",
        &cr::VehiclePhysicsControl::damping_rate_zero_throttle_clutch_engaged)
    .def_readwrite("damping_rate_zero_throttle_clutch_disengaged",
        &cr::VehiclePhysicsControl::damping_rate_zero_throttle_clutch_disengaged)
    .def_readwrite("use_gear_autobox", &cr::VehiclePhysicsControl::use_gear_autobox)
    .def_readwrite("gear_switch_time", &cr::VehiclePhysicsControl::gear_switch_time)
    .def_readwrite("clutch_strength", &cr::VehiclePhysicsControl::clutch_strength)
    .def_readwrite("final_ratio", &cr::VehiclePhysicsControl::final_ratio)
    .add_property("forward_gears", &GetForwardGears, &SetForwardGears)
    .def_readwrite("mass", &cr::VehiclePhysicsControl::mass)
    .def_readwrite("drag_coefficient", &cr::VehiclePhysicsControl::drag_coefficient)
    .def_readwrite("center_of_mass", &cr::VehiclePhysicsControl::center_of_mass)
    .add_property("steering_curve", &GetSteeringCurve, &SetSteeringCurve)
    .add_property("wheels", &GetWheels, &SetWheels)
    .def_readwrite("use_sweep_wheel_collision", &cr::VehiclePhysicsControl::use_sweep_wheel_collision)
    .def("__eq__", &cr::VehiclePhysicsControl::operator==)
    .def("__ne__", &cr::VehiclePhysicsControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::WheelTelemetryData>("WheelTelemetryData")
    .def(init<float, float, float, float, float, float, float, float, float, float, float>(
        (arg("tire_friction")=0.0f,
         arg("lat_slip")=0.0f,
         arg("long_slip")=0.0f,
         arg("omega")=0.0f,
         arg("tire_load")=0.0f,
         arg("normalized_tire_load")=0.0f,
         arg("torque")=0.0f,
         arg("long_force")=0.0f,
         arg("lat_force")=0.0f,
         arg("normalized_long_force")=0.0f,
         arg("normalized_lat_force")=0.0f)))
    .def_readwrite("tire_friction", &cr::WheelTelemetryData::tire_friction)
    .def_readwrite("lat_slip", &cr::WheelTelemetryData::lat_slip)
    .def_readwrite("long_slip", &cr::WheelTelemetryData::long_slip)
    .def_readwrite("omega", &cr::WheelTelemetryData::omega)
    .def_readwrite("tire_load", &cr::WheelTelemetryData::tire_load)
    .def_readwrite("normalized_tire_load", &cr::WheelTelemetryData::normalized_tire_load)
    .def_readwrite("torque", &cr::WheelTelemetryData::torque)
    .def_readwrite("long_force", &cr::WheelTelemetryData::long_force)
    .def_readwrite("lat_force", &cr::WheelTelemetryData::lat_force)
    .def_readwrite("normalized_long_force", &cr::WheelTelemetryData::normalized_long_force)
    .def_readwrite("normalized_lat_force", &cr::WheelTelemetryData::normalized_lat_force)
    .def("__eq__", &cr::WheelTelemetryData::operator==)
    .def("__ne__", &cr::WheelTelemetryData::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::VehicleTelemetryData>("VehicleTelemetryData", no_init)
    .def("__init__", raw_function(VehicleTelemetryData_init))
    .def(init<>())
    .def_readwrite("speed", &cr::VehicleTelemetryData::speed)
    .def_readwrite("steer", &cr::VehicleTelemetryData::steer)
    .def_readwrite("throttle", &cr::VehicleTelemetryData::throttle)
    .def_readwrite("brake", &cr::VehicleTelemetryData::brake)
    .def_readwrite("engine_rpm", &cr::VehicleTelemetryData::engine_rpm)
    .def_readwrite("gear", &cr::VehicleTelemetryData::gear)
    .def_readwrite("drag", &cr::VehicleTelemetryData::drag)
    .add_property("wheels", &GetWheelsTelemetry, &SetWheelsTelemetry)
    .def("__eq__", &cr::VehicleTelemetryData::operator==)
    .def("__ne__", &cr::VehicleTelemetryData::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

}
