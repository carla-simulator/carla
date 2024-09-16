// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <PythonAPI.h>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

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

static auto GetForwardGearRatios(const carla::rpc::VehiclePhysicsControl &self) {
  const auto &gears = self.GetForwardGearRatios();
  boost::python::object get_iter = boost::python::iterator<std::vector<float>>();
  boost::python::object iter = get_iter(gears);
  return boost::python::list(iter);
}

static void SetForwardGearRatios(carla::rpc::VehiclePhysicsControl &self, const boost::python::list &list) {
  std::vector<float> gears;
  auto length = boost::python::len(list);
  for (auto i = 0u; i < length; ++i) {
    gears.push_back(boost::python::extract<float &>(list[i]));
  }
  self.SetForwardGearRatios(gears);
}

static auto GetReverseGearRatios(const carla::rpc::VehiclePhysicsControl &self) {
  const auto &gears = self.GetReverseGearRatios();
  boost::python::object get_iter = boost::python::iterator<std::vector<float>>();
  boost::python::object iter = get_iter(gears);
  return boost::python::list(iter);
}

static void SetReverseGears(carla::rpc::VehiclePhysicsControl &self, const boost::python::list &list) {
  std::vector<float> gears;
  auto length = boost::python::len(list);
  for (auto i = 0u; i < length; ++i) {
    gears.push_back(boost::python::extract<float &>(list[i]));
  }
  self.SetReverseGears(gears);
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
  const uint32_t NUM_ARGUMENTS = 21;
  const char *args_names[NUM_ARGUMENTS] = {
    "torque_curve",
    "max_torque",
    "max_rpm",
    "idle_rpm",
    "brake_effect",
    "rev_up_moi",
    "rev_down_rate",
    "differential_type",
    "front_rear_split",
    "use_automatic_gears",
    "gear_change_time",
    "final_ratio",
    "forward_gear_ratios",
    "reverse_gear_ratios",
    "change_up_rpm",
    "change_down_rpm",
    "transmission_efficiency",
    "mass",
    "drag_coefficient",
    "center_of_mass",
    "chassis_width",
    "chassis_height",
    "downforce_coefficient",
    "drag_area",
    "inertia_tensor_scale",
    "sleep_threshold",
    "sleep_slope_limit",
    "steering_curve",
    "wheels",
    "use_sweep_wheel_collision"
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

  class_<std::vector<cr::WheelPhysicsControl>>("vector_of_wheels")
    .def(boost::python::vector_indexing_suite<std::vector<cr::WheelPhysicsControl>>())
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::WheelPhysicsControl>("WheelPhysicsControl")
    .def(init<float, float, float, float, bool, bool, float, float, cg::Vector3D>(
        (arg("tire_friction")=3.0f,
         arg("max_steer_angle")=70.0f,
         arg("radius")=30.0f,
         arg("cornering_stiffness")=1000.0f,
         arg("abs")=false,
         arg("traction_control")=false,
         arg("max_brake_torque")=1500.0f,
         arg("max_handbrake_torque")=3000.0f,
         arg("position")=cg::Vector3D{0.0f, 0.0f, 0.0f})))
    .def_readwrite("tire_friction", &cr::WheelPhysicsControl::tire_friction)
    .def_readwrite("max_steer_angle", &cr::WheelPhysicsControl::max_steer_angle)
    .def_readwrite("radius", &cr::WheelPhysicsControl::radius)
    .def_readwrite("cornering_stiffness", &cr::WheelPhysicsControl::cornering_stiffness)
    .def_readwrite("abs", &cr::WheelPhysicsControl::abs)
    .def_readwrite("traction_control", &cr::WheelPhysicsControl::traction_control)
    .def_readwrite("max_brake_torque", &cr::WheelPhysicsControl::max_brake_torque)
    .def_readwrite("max_handbrake_torque", &cr::WheelPhysicsControl::max_handbrake_torque)
    .def_readwrite("position", &cr::WheelPhysicsControl::position)
    .def("__eq__", &cr::WheelPhysicsControl::operator==)
    .def("__ne__", &cr::WheelPhysicsControl::operator!=)
    .def(self_ns::str(self_ns::self))
  ;

  class_<cr::VehiclePhysicsControl>("VehiclePhysicsControl", no_init)
    .def("__init__", raw_function(VehiclePhysicsControl_init))
    .def(init<>())
    .add_property("torque_curve", &GetTorqueCurve, &SetTorqueCurve)
    .def_readwrite("max_torque", &cr::VehiclePhysicsControl::max_torque)
    .def_readwrite("max_rpm", &cr::VehiclePhysicsControl::max_rpm)
    .def_readwrite("rev_up_moi", &cr::VehiclePhysicsControl::rev_up_moi)
    .def_readwrite("rev_down_rate", &cr::VehiclePhysicsControl::rev_down_rate)
    .def_readwrite("differential_type", &cr::VehiclePhysicsControl::differential_type)
    .def_readwrite("front_rear_split", &cr::VehiclePhysicsControl::front_rear_split)
    .def_readwrite("use_automatic_gears", &cr::VehiclePhysicsControl::use_automatic_gears)
    .def_readwrite("gear_change_time", &cr::VehiclePhysicsControl::gear_change_time)
    .def_readwrite("final_ratio", &cr::VehiclePhysicsControl::final_ratio)
    .add_property("forward_gear_ratios", &GetForwardGearRatios, &SetForwardGearRatios)
    .add_property("reverse_gear_ratios", &GetReverseGearRatios, &SetReverseGears)
    .def_readwrite("change_up_rpm", &cr::VehiclePhysicsControl::change_up_rpm)
    .def_readwrite("change_down_rpm", &cr::VehiclePhysicsControl::change_down_rpm)
    .def_readwrite("transmission_efficiency", &cr::VehiclePhysicsControl::mass)
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
}
