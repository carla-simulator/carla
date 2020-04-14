// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <chrono>
#include <memory>

#include "carla/PythonUtil.h"
#include "boost/python/suite/indexing/vector_indexing_suite.hpp"

#include "carla/client/LightManager.h"

namespace cc = carla::client;
namespace csd = carla::sensor::data;

/****** ACTIVE ******/

static void LightManagerTurnOn(
  cc::LightManager& self,
  const boost::python::object& py_lights) {

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  self.TurnOn(lights);
}

static void LightManagerTurnOff(
  cc::LightManager& self,
  const boost::python::object& py_lights) {

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  self.TurnOff(lights);
}

static void LightManagerSetActive(
  cc::LightManager& self,
  const boost::python::object& py_lights,
  const boost::python::object& py_active) {

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  std::vector<bool> active {
    boost::python::stl_input_iterator<bool>(py_active),
    boost::python::stl_input_iterator<bool>()
  };

  self.SetActive(lights, active);
}

static boost::python::list LightManagerIsActive(
    cc::LightManager& self,
    const boost::python::object& py_lights) {

  boost::python::list result;

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  for (auto &&item : self.IsActive(lights)) {
    // Avoiding bit_ref conversion
    result.append(static_cast<bool>(item));
  }

  return result;
}

/*******************/

/****** COLOR ******/

static void LightManagerSetColor(
  cc::LightManager& self,
  const boost::python::object& py_lights,
  const csd::Color color) {

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  self.SetColor(lights, color);
}

static void LightManagerSetVectorColor(
  cc::LightManager& self,
  const boost::python::object& py_lights,
  const boost::python::object& py_colors) {

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  std::vector<csd::Color> colors {
    boost::python::stl_input_iterator<csd::Color>(py_colors),
    boost::python::stl_input_iterator<csd::Color>()
  };

  self.SetColor(lights, colors);
}

static boost::python::list LightManagerGetColor(
    cc::LightManager& self,
    const boost::python::object& py_lights) {

  boost::python::list result;

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  for (auto &&item : self.GetColor(lights)) {
    result.append(item);
  }

  return result;
}

/*******************/

/****** INTENSITY ******/
static void LightManagerSetIntensity(
  cc::LightManager& self,
  const boost::python::object& py_lights,
  const float intensity) {

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  self.SetIntensity(lights, intensity);
}

static void LightManagerSetVectorIntensity(
  cc::LightManager& self,
  const boost::python::object& py_lights,
  const boost::python::object& py_intensities) {

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  std::vector<float> intensities {
    boost::python::stl_input_iterator<float>(py_intensities),
    boost::python::stl_input_iterator<float>()
  };

  self.SetIntensity(lights, intensities);
}

static boost::python::list LightManagerGetIntensity(
    cc::LightManager& self,
    const boost::python::object& py_lights) {

  boost::python::list result;

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  for (auto &&item : self.GetIntensity(lights)) {
    result.append(item);
  }

  return result;
}

/*******************/

/****** LIGHT GROUP ******/
static void LightManagerSetLightGroup(
  cc::LightManager& self,
  const boost::python::object& py_lights,
  const cc::LightGroup light_group) {

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  self.SetLightGroup(lights, light_group);
}

static void LightManagerSetVectorLightGroup(
  cc::LightManager& self,
  const boost::python::object& py_lights,
  const boost::python::object& py_light_group) {

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  std::vector<cc::LightGroup> light_groups {
    boost::python::stl_input_iterator<cc::LightGroup>(py_light_group),
    boost::python::stl_input_iterator<cc::LightGroup>()
  };

  self.SetLightGroup(lights, light_groups);
}

static boost::python::list LightManagerGetLightGroup(
    cc::LightManager& self,
    const boost::python::object& py_lights) {

  boost::python::list result;

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  for (auto &&item : self.GetLightGroup(lights)) {
    result.append(item);
  }

  return result;
}

/*******************/

/****** LIGHT STATE ******/
static void LightManagerSetLightState(
  cc::LightManager& self,
  const boost::python::object& py_lights,
  cc::LightState& light_state) {

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  self.SetLightState(lights, light_state);
}

static void LightManagerSetVectorLightState(
  cc::LightManager& self,
  const boost::python::object& py_lights,
  const boost::python::object& py_light_state) {

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  std::vector<cc::LightState> light_states {
    boost::python::stl_input_iterator<cc::LightState>(py_light_state),
    boost::python::stl_input_iterator<cc::LightState>()
  };

  self.SetLightState(lights, light_states);
}

static boost::python::list LightManagerGetLightState(
    cc::LightManager& self,
    const boost::python::object& py_lights) {

  boost::python::list result;

  std::vector<cc::Light> lights {
    boost::python::stl_input_iterator<cc::Light>(py_lights),
    boost::python::stl_input_iterator<cc::Light>()
  };

  for (auto &&item : self.GetLightState(lights)) {
    result.append(item);
  }

  return result;
}

/*******************/

void export_lightmanager() {
    using namespace boost::python;

    enum_<cc::LightGroup>("LightGroup")
      .value("None", cc::LightGroup::None)
      .value("Vehicle", cc::LightGroup::Vehicle)
      .value("Street", cc::LightGroup::Street)
      .value("Building", cc::LightGroup::Building)
      .value("Other", cc::LightGroup::Other)
    ;

    class_<cc::LightState>("LightState")
      .def(init<float, csd::Color, cc::LightGroup, bool>((arg("intensity")=0.0f, arg("color")=csd::Color(), arg("group")=cc::LightGroup::None, arg("active")=false )))
      .def_readwrite("intensity", &cc::LightState::_intensity)
      .def_readwrite("color", &cc::LightState::_color)
      .def_readwrite("group", &cc::LightState::_group)
      .def_readwrite("active", &cc::LightState::_active)
      .def("Reset", &cc::LightState::Reset)
    ;

    class_<cc::Light, boost::shared_ptr<cc::Light>>("Light", no_init)
      .add_property("color", &cc::Light::GetColor)
      .add_property("id", &cc::Light::GetId)
      .add_property("intensity", &cc::Light::GetIntensity)
      .add_property("is_on", &cc::Light::IsOn)
      .add_property("is_off", &cc::Light::IsOff)
      .add_property("location", &cc::Light::GetLocation)
      .add_property("rotation", &cc::Light::GetRotation)
      .add_property("light_group", &cc::Light::GetLightGroup)
      .add_property("light_state", &cc::Light::GetLightState)
      .def("set_color", &cc::Light::SetColor, (arg("color")))
      .def("set_intensity", &cc::Light::SetIntensity, (arg("intensity")))
      .def("set_light_group", &cc::Light::SetLightGroup, (arg("light_group")))
      .def("set_light_state", &cc::Light::SetLightState, (arg("light_state")))
      .def("turn_on", &cc::Light::TurnOn)
      .def("turn_off", &cc::Light::TurnOff)
    ;

    class_<cc::LightManager, boost::shared_ptr<cc::LightManager>>("LightManager", no_init)
      .def("get_all_lights", CALL_RETURNING_LIST_1(cc::LightManager, GetAllLights, cc::LightGroup), (args("light_group") = cc::LightGroup::None ))
      .def("turn_on", &LightManagerTurnOn, (arg("lights")))
      .def("turn_off", &LightManagerTurnOff, (arg("lights")))
      .def("set_active", &LightManagerSetActive, (arg("lights"), arg("active")))
      .def("is_active", &LightManagerIsActive, (arg("lights")))
      .def("get_turned_on_lights", CALL_RETURNING_LIST_1(cc::LightManager, GetTurnedOnLights, cc::LightGroup), (args("light_group") = cc::LightGroup::None ))
      .def("get_turned_off_lights", CALL_RETURNING_LIST_1(cc::LightManager, GetTurnedOffLights, cc::LightGroup), (args("light_group") = cc::LightGroup::None ))
      .def("set_color", &LightManagerSetColor, (arg("lights"), arg("color")))
      .def("set_color", &LightManagerSetVectorColor, (arg("lights"), arg("color")))
      .def("get_color", &LightManagerGetColor, (arg("lights")))
      .def("set_intensity", &LightManagerSetIntensity, (arg("lights"), arg("intensity")))
      .def("set_intensity", &LightManagerSetVectorIntensity, (arg("lights"), arg("intensity")))
      .def("get_intensity", &LightManagerGetIntensity, (arg("lights")))
      .def("set_light_group", &LightManagerSetLightGroup, (arg("lights"), arg("light_group")))
      .def("set_light_group", &LightManagerSetVectorLightGroup, (arg("lights"), arg("light_group")))
      .def("get_light_group", &LightManagerGetLightGroup, (arg("lights")))
      .def("set_light_state", &LightManagerSetLightState, (arg("lights"), arg("light_state")))
      .def("set_light_state", &LightManagerSetVectorLightState, (arg("lights"), arg("light_state")))
      .def("get_light_state", &LightManagerGetLightState, (arg("lights")))
    ;
}
