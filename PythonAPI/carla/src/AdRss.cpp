// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/rss/RssRestrictor.h>
#include <carla/rss/RssSensor.h>
#include <carla/sensor/data/RssResponse.h>

#include <ad/physics/python/AdPhysicsPython.hpp>
#include <ad/map/python/AdMapAccessPython.hpp>
#include <ad/rss/python/AdRssPython.hpp>
#include <ad/rss/python/AdRssMapIntegrationPython.hpp>
#include <ad/rss/world/RssDynamics.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

namespace carla {
namespace rss {

std::ostream &operator<<(std::ostream &out, const RssRestrictor &) {
  out << "RssRestrictor()";
  return out;
}

}  // namespace rss

namespace sensor {
namespace data {

std::ostream &operator<<(std::ostream &out, const RssResponse &resp) {
  out << "RssResponse(frame=" << resp.GetFrame() << ", timestamp=" << resp.GetTimestamp()
      << ", valid=" << resp.GetResponseValid() << ", proper_response=" << resp.GetProperResponse()
      << ", rss_state_snapshot=" << resp.GetRssStateSnapshot() << ", situation_snapshot=" << resp.GetSituationSnapshot()
      << ", world_model=" << resp.GetWorldModel() << ", ego_dynamics_on_route=" << resp.GetEgoDynamicsOnRoute() << ')';
  return out;
}

}  // namespace data
}  // namespace sensor
}  // namespace carla

static auto GetEgoVehicleDynamics(const carla::client::RssSensor &self) {
  ad::rss::world::RssDynamics ego_dynamics(self.GetEgoVehicleDynamics());
  return ego_dynamics;
}

static auto GetOtherVehicleDynamics(const carla::client::RssSensor &self) {
  ad::rss::world::RssDynamics other_dynamics(self.GetOtherVehicleDynamics());
  return other_dynamics;
}

static auto GetPedestrianDynamics(const carla::client::RssSensor &self) {
  ad::rss::world::RssDynamics pedestrian_dynamics(self.GetPedestrianDynamics());
  return pedestrian_dynamics;
}

static auto GetRoadBoundariesMode(const carla::client::RssSensor &self) {
  carla::rss::RoadBoundariesMode road_boundaries_mode(self.GetRoadBoundariesMode());
  return road_boundaries_mode;
}

static auto GetRoutingTargets(const carla::client::RssSensor &self) {
  std::vector<carla::geom::Transform> routing_targets(self.GetRoutingTargets());
  return routing_targets;
}

static void RegisterActorConstellationCallback(carla::client::RssSensor &self, boost::python::object callback) {
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
  auto callback_function = [callback = std::move(callback_ptr)](
                               carla::SharedPtr<::carla::rss::ActorConstellationData> actor_constellation_data)
                               ->::carla::rss::ActorConstellationResult {
    carla::PythonUtil::AcquireGIL lock;
    ::carla::rss::ActorConstellationResult actor_constellation_result;
    try {
      actor_constellation_result =
          py::call<::carla::rss::ActorConstellationResult>(callback->ptr(), py::object(actor_constellation_data));
    } catch (const py::error_already_set &) {
      PyErr_Print();
    }
    return actor_constellation_result;
  };
  self.RegisterActorConstellationCallback(callback_function);
}

static void Stop(carla::client::RssSensor &self) {
  // ensure the RssSensor is stopped with GIL released to sync on processing lock
  carla::PythonUtil::ReleaseGIL unlock;
  self.Stop();
}

void export_ad() {
  using namespace boost::python;

  // create/import the ad module scope
  object ad_module(handle<>(borrowed(PyImport_AddModule("ad"))));
  scope().attr("ad") = ad_module;
  scope submodule_scope = ad_module;
  scope().attr("__doc__") = "Python binding of ad namespace C++ code";
  scope().attr("__copyright__") = "Copyright (C) 2019-2020 Intel Corporation";

  export_ad_physics_python();
  export_ad_map_access_python();
  export_ad_rss_python();
  export_ad_rss_map_integration_python();
}

void export_ad_rss() {

  export_ad();

  using namespace boost::python;
  namespace cc = carla::client;
  namespace cs = carla::sensor;
  namespace csd = carla::sensor::data;

  class_<carla::rss::EgoDynamicsOnRoute>("RssEgoDynamicsOnRoute")
      .def_readwrite("timestamp", &carla::rss::EgoDynamicsOnRoute::timestamp)
      .def_readwrite("time_since_epoch_check_start_ms",
                     &carla::rss::EgoDynamicsOnRoute::time_since_epoch_check_start_ms)
      .def_readwrite("time_since_epoch_check_end_ms", &carla::rss::EgoDynamicsOnRoute::time_since_epoch_check_end_ms)
      .def_readwrite("ego_speed", &carla::rss::EgoDynamicsOnRoute::ego_speed)
      .def_readwrite("min_stopping_distance", &carla::rss::EgoDynamicsOnRoute::min_stopping_distance)
      .def_readwrite("ego_center", &carla::rss::EgoDynamicsOnRoute::ego_center)
      .def_readwrite("ego_heading", &carla::rss::EgoDynamicsOnRoute::ego_heading)
      .def_readwrite("ego_center_within_route", &carla::rss::EgoDynamicsOnRoute::ego_center_within_route)
      .def_readwrite("crossing_border", &carla::rss::EgoDynamicsOnRoute::crossing_border)
      .def_readwrite("route_heading", &carla::rss::EgoDynamicsOnRoute::route_heading)
      .def_readwrite("route_nominal_center", &carla::rss::EgoDynamicsOnRoute::route_nominal_center)
      .def_readwrite("heading_diff", &carla::rss::EgoDynamicsOnRoute::heading_diff)
      .def_readwrite("route_speed_lat", &carla::rss::EgoDynamicsOnRoute::route_speed_lat)
      .def_readwrite("route_speed_lon", &carla::rss::EgoDynamicsOnRoute::route_speed_lon)
      .def_readwrite("route_accel_lat", &carla::rss::EgoDynamicsOnRoute::route_accel_lat)
      .def_readwrite("route_accel_lon", &carla::rss::EgoDynamicsOnRoute::route_accel_lon)
      .def_readwrite("avg_route_accel_lat", &carla::rss::EgoDynamicsOnRoute::avg_route_accel_lat)
      .def_readwrite("avg_route_accel_lon", &carla::rss::EgoDynamicsOnRoute::avg_route_accel_lon)
      .def(self_ns::str(self_ns::self));

  class_<carla::rss::ActorConstellationResult>("RssActorConstellationResult")
      .def_readwrite("rss_calculation_mode", &carla::rss::ActorConstellationResult::rss_calculation_mode)
      .def_readwrite("restrict_speed_limit_mode", &carla::rss::ActorConstellationResult::restrict_speed_limit_mode)
      .def_readwrite("ego_vehicle_dynamics", &carla::rss::ActorConstellationResult::ego_vehicle_dynamics)
      .def_readwrite("actor_object_type", &carla::rss::ActorConstellationResult::actor_object_type)
      .def_readwrite("actor_dynamics", &carla::rss::ActorConstellationResult::actor_dynamics)
      .def(self_ns::str(self_ns::self));

  class_<carla::rss::ActorConstellationData, boost::noncopyable, std::shared_ptr<carla::rss::ActorConstellationData>>(
      "RssActorConstellationData", no_init)
      .def_readonly("ego_match_object", &carla::rss::ActorConstellationData::ego_match_object)
      .def_readonly("ego_route", &carla::rss::ActorConstellationData::ego_route)
      .def_readonly("ego_dynamics_on_route", &carla::rss::ActorConstellationData::ego_dynamics_on_route)
      .def_readonly("other_match_object", &carla::rss::ActorConstellationData::other_match_object)
      .def_readonly("other_actor", &carla::rss::ActorConstellationData::other_actor)
      .def(self_ns::str(self_ns::self));

  enum_<spdlog::level::level_enum>("RssLogLevel")
      .value("trace", spdlog::level::trace)
      .value("debug", spdlog::level::debug)
      .value("info", spdlog::level::info)
      .value("warn", spdlog::level::warn)
      .value("err", spdlog::level::err)
      .value("critical", spdlog::level::critical)
      .value("off", spdlog::level::off);

  enum_<carla::rss::RoadBoundariesMode>("RssRoadBoundariesMode")
      .value("Off", carla::rss::RoadBoundariesMode::Off)
      .value("On", carla::rss::RoadBoundariesMode::On);

  class_<csd::RssResponse, bases<cs::SensorData>, boost::noncopyable, std::shared_ptr<csd::RssResponse>>(
      "RssResponse", no_init)
      .add_property("response_valid", &csd::RssResponse::GetResponseValid)
      .add_property("proper_response", CALL_RETURNING_COPY(csd::RssResponse, GetProperResponse))
      .add_property("rss_state_snapshot", CALL_RETURNING_COPY(csd::RssResponse, GetRssStateSnapshot))
      .add_property("situation_snapshot", CALL_RETURNING_COPY(csd::RssResponse, GetSituationSnapshot))
      .add_property("world_model", CALL_RETURNING_COPY(csd::RssResponse, GetWorldModel))
      .add_property("ego_dynamics_on_route", CALL_RETURNING_COPY(csd::RssResponse, GetEgoDynamicsOnRoute))
      .def(self_ns::str(self_ns::self));

  class_<cc::RssSensor, bases<cc::Sensor>, boost::noncopyable, std::shared_ptr<cc::RssSensor>>("RssSensor", no_init)
      .add_property("ego_vehicle_dynamics", &GetEgoVehicleDynamics, &cc::RssSensor::SetEgoVehicleDynamics)
      .add_property("other_vehicle_dynamics", &GetOtherVehicleDynamics, &cc::RssSensor::SetOtherVehicleDynamics)
      .add_property("pedestrian_dynamics", &GetPedestrianDynamics, &cc::RssSensor::SetPedestrianDynamics)
      .add_property("road_boundaries_mode", &GetRoadBoundariesMode, &cc::RssSensor::SetRoadBoundariesMode)
      .add_property("routing_targets", &GetRoutingTargets)
      .def("stop", &Stop)
      .def("register_actor_constellation_callback", &RegisterActorConstellationCallback, (arg("callback")))
      .def("append_routing_target", &cc::RssSensor::AppendRoutingTarget, (arg("routing_target")))
      .def("reset_routing_targets", &cc::RssSensor::ResetRoutingTargets)
      .def("drop_route", &cc::RssSensor::DropRoute)
      .def("set_log_level", &cc::RssSensor::SetLogLevel, (arg("log_level")))
      .def("set_map_log_level", &cc::RssSensor::SetMapLogLevel, (arg("map_log_level")))
      .def(self_ns::str(self_ns::self));

  class_<carla::rss::RssRestrictor, boost::noncopyable, std::shared_ptr<carla::rss::RssRestrictor>>("RssRestrictor",
                                                                                                      no_init)
      .def(init<>())
      .def("restrict_vehicle_control", &carla::rss::RssRestrictor::RestrictVehicleControl,
           (arg("vehicle_control"), arg("proper_response"), arg("ego_dynamics_on_route"), arg("vehicle_physics")))
      .def("set_log_level", &carla::rss::RssRestrictor::SetLogLevel, (arg("log_level")))
      .def(self_ns::str(self_ns::self));
}
