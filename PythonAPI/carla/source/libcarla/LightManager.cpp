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
namespace cr = carla::rpc;
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
  const cr::LightState::LightGroup light_group) {

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

  std::vector<cr::LightState::LightGroup> light_groups {
    boost::python::stl_input_iterator<cr::LightState::LightGroup>(py_light_group),
    boost::python::stl_input_iterator<cr::LightState::LightGroup>()
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

    enum_<cr::LightState::LightGroup>("LightGroup")
      .value("None", cr::LightState::LightGroup::None)
      .value("Vehicle", cr::LightState::LightGroup::Vehicle)
      .value("Street", cr::LightState::LightGroup::Street)
      .value("Building", cr::LightState::LightGroup::Building)
      .value("Other", cr::LightState::LightGroup::Other)
    ;

    class_<cc::LightState>("LightState")
      .def(init<float, csd::Color, cr::LightState::LightGroup, bool>((arg("intensity")=0.0f, arg("color")=csd::Color(), arg("group")=cr::LightState::LightGroup::None, arg("active")=false ), "@DocString(LightState.__init__)"))
      .def_readwrite("intensity", &cc::LightState::_intensity, "@DocString(LightState.intensity)")
      .def_readwrite("color", &cc::LightState::_color, "@DocString(LightState.color)")
      .def_readwrite("group", &cc::LightState::_group, "@DocString(LightState.group)")
      .def_readwrite("active", &cc::LightState::_active, "@DocString(LightState.active)")
    ;

    class_<cc::Light, boost::shared_ptr<cc::Light>>("Light", no_init)
      .add_property("color", &cc::Light::GetColor, "@DocString(Light.color)")
      .add_property("id", &cc::Light::GetId, "@DocString(Light.id)")
      .add_property("intensity", &cc::Light::GetIntensity, "@DocString(Light.intensity)")
      .add_property("is_on", &cc::Light::IsOn, "@DocString(Light.is_on)")
      .add_property("location", &cc::Light::GetLocation, "@DocString(Light.location)")
      .add_property("light_group", &cc::Light::GetLightGroup, "@DocString(Light.light_group)")
      .add_property("light_state", &cc::Light::GetLightState, "@DocString(Light.light_state)")
      .def("set_color", &cc::Light::SetColor, (arg("color")), "@DocString(Light.set_color)")
      .def("set_intensity", &cc::Light::SetIntensity, (arg("intensity")), "@DocString(Light.set_intensity)")
      .def("set_light_group", &cc::Light::SetLightGroup, (arg("light_group")), "@DocString(Light.set_light_group)")
      .def("set_light_state", &cc::Light::SetLightState, (arg("light_state")), "@DocString(Light.set_light_state)")
      .def("turn_on", &cc::Light::TurnOn, "@DocString(Light.turn_on)")
      .def("turn_off", &cc::Light::TurnOff, "@DocString(Light.turn_off)")
    ;

    class_<cc::LightManager, boost::shared_ptr<cc::LightManager>>("LightManager", no_init)
      .def("get_all_lights", CALL_RETURNING_LIST_1(cc::LightManager, GetAllLights, cr::LightState::LightGroup), (args("light_group") = cr::LightState::LightGroup::None ), "@DocString(LightManager.get_all_lights)")
      .def("turn_on", &LightManagerTurnOn, (arg("lights")), "@DocString(LightManager.turn_on)")
      .def("turn_off", &LightManagerTurnOff, (arg("lights")), "@DocString(LightManager.turn_off)")
      .def("set_active", &LightManagerSetActive, (arg("lights"), arg("active")), "@DocString(LightManager.set_active)")
      .def("is_active", &LightManagerIsActive, (arg("lights")), "@DocString(LightManager.is_active)")
      .def("get_turned_on_lights", CALL_RETURNING_LIST_1(cc::LightManager, GetTurnedOnLights, cr::LightState::LightGroup), (args("light_group") = cr::LightState::LightGroup::None ), "@DocString(LightManager.get_turned_on_lights)")
      .def("get_turned_off_lights", CALL_RETURNING_LIST_1(cc::LightManager, GetTurnedOffLights, cr::LightState::LightGroup), (args("light_group") = cr::LightState::LightGroup::None ), "@DocString(LightManager.get_turned_off_lights)")
      .def("set_color", &LightManagerSetColor, (arg("lights"), arg("color")), "@DocString(LightManager.set_color)")
      .def("set_colors", &LightManagerSetVectorColor, (arg("lights"), arg("colors")), "@DocString(LightManager.set_colors)")
      .def("get_color", &LightManagerGetColor, (arg("lights")), "@DocString(LightManager.get_color)")
      .def("set_intensity", &LightManagerSetIntensity, (arg("lights"), arg("intensity")), "@DocString(LightManager.set_intensity)")
      .def("set_intensities", &LightManagerSetVectorIntensity, (arg("lights"), arg("intensities")), "@DocString(LightManager.set_intensities)")
      .def("get_intensity", &LightManagerGetIntensity, (arg("lights")), "@DocString(LightManager.get_intensity)")
      .def("set_light_group", &LightManagerSetLightGroup, (arg("lights"), arg("light_group")), "@DocString(LightManager.set_light_group)")
      .def("set_light_groups", &LightManagerSetVectorLightGroup, (arg("lights"), arg("light_groups")), "@DocString(LightManager.set_light_groups)")
      .def("get_light_group", &LightManagerGetLightGroup, (arg("lights")), "@DocString(LightManager.get_light_group)")
      .def("set_light_state", &LightManagerSetLightState, (arg("lights"), arg("light_state")), "@DocString(LightManager.set_light_state)")
      .def("set_light_states", &LightManagerSetVectorLightState, (arg("lights"), arg("light_states")), "@DocString(LightManager.set_light_states)")
      .def("get_light_state", &LightManagerGetLightState, (arg("lights")), "@DocString(LightManager.get_light_state)")
    ;

}
