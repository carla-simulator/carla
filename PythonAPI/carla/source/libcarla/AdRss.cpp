// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/rss/RssRestrictor.h>
#include <carla/rss/RssSensor.h>
#include <carla/sensor/data/RssResponse.h>

#include <ad/rss/world/RssDynamics.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#ifdef LIBCARLA_PYTHON_MAJOR_2
extern "C" {
void initlibad_physics_python();
void initlibad_rss_python();
void initlibad_map_access_python();
void initlibad_rss_map_integration_python();
}
#endif

#ifdef LIBCARLA_PYTHON_MAJOR_3
extern "C" {
void PyInit_libad_physics_python();
void PyInit_libad_rss_python();
void PyInit_libad_map_access_python();
void PyInit_libad_rss_map_integration_python();
}
#endif

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
      << ", valid=" << resp.GetResponseValid() << ')';
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

static auto GetRoadBoundariesMode(const carla::client::RssSensor &self) {
  carla::rss::RoadBoundariesMode road_boundaries_mode(self.GetRoadBoundariesMode());
  return road_boundaries_mode;
}

static auto GetRoutingTargets(const carla::client::RssSensor &self) {
  std::vector<carla::geom::Transform> routing_targets(self.GetRoutingTargets());
  return routing_targets;
}

static auto GetVisualizationMode(const carla::client::RssSensor &self) {
  carla::rss::VisualizationMode visualization_mode(self.GetVisualizationMode());
  return visualization_mode;
}

void export_ad_rss() {
#ifdef LIBCARLA_PYTHON_MAJOR_2
  initlibad_physics_python();
  initlibad_rss_python();
  initlibad_map_access_python();
  initlibad_rss_map_integration_python();
#endif

#ifdef LIBCARLA_PYTHON_MAJOR_3
  PyInit_libad_physics_python();
  PyInit_libad_rss_python();
  PyInit_libad_map_access_python();
  PyInit_libad_rss_map_integration_python();
#endif

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

  enum_<carla::rss::RoadBoundariesMode>("RssRoadBoundariesMode")
      .value("Off", carla::rss::RoadBoundariesMode::Off)
      .value("On", carla::rss::RoadBoundariesMode::On);

  enum_<carla::rss::VisualizationMode>("RssVisualizationMode")
      .value("Off", carla::rss::VisualizationMode::Off)
      .value("RouteOnly", carla::rss::VisualizationMode::RouteOnly)
      .value("VehicleStateOnly", carla::rss::VisualizationMode::VehicleStateOnly)
      .value("VehicleStateAndRoute", carla::rss::VisualizationMode::VehicleStateAndRoute)
      .value("All", carla::rss::VisualizationMode::All);

  class_<csd::RssResponse, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::RssResponse>>(
      "RssResponse", no_init)
      .add_property("response_valid", &csd::RssResponse::GetResponseValid)
      .add_property("proper_response", CALL_RETURNING_COPY(csd::RssResponse, GetProperResponse))
      .add_property("acceleration_restriction", CALL_RETURNING_COPY(csd::RssResponse, GetAccelerationRestriction))
      .add_property("rss_state_snapshot", CALL_RETURNING_COPY(csd::RssResponse, GetRssStateSnapshot))
      .add_property("ego_dynamics_on_route", CALL_RETURNING_COPY(csd::RssResponse, GetEgoDynamicsOnRoute))
      .def(self_ns::str(self_ns::self));

  class_<cc::RssSensor, bases<cc::Sensor>, boost::noncopyable, boost::shared_ptr<cc::RssSensor>>("RssSensor", no_init)
      .add_property("ego_vehicle_dynamics", &GetEgoVehicleDynamics, &cc::RssSensor::SetEgoVehicleDynamics)
      .add_property("other_vehicle_dynamics", &GetOtherVehicleDynamics, &cc::RssSensor::SetOtherVehicleDynamics)
      .add_property("road_boundaries_mode", &GetRoadBoundariesMode, &cc::RssSensor::SetRoadBoundariesMode)
      .add_property("visualization_mode", &GetVisualizationMode, &cc::RssSensor::SetVisualizationMode)
      .add_property("routing_targets", &GetRoutingTargets)
      .def("append_routing_target", &cc::RssSensor::AppendRoutingTarget, (arg("routing_target")))
      .def("reset_routing_targets", &cc::RssSensor::ResetRoutingTargets)
      .def("drop_route", &cc::RssSensor::DropRoute)
      .def(self_ns::str(self_ns::self));

  class_<carla::rss::RssRestrictor, boost::noncopyable, boost::shared_ptr<carla::rss::RssRestrictor>>("RssRestrictor",
                                                                                                      no_init)
      .def(init<>())
      .def("restrict_vehicle_control", &carla::rss::RssRestrictor::RestrictVehicleControl,
           (arg("vehicle_control"), arg("restriction"), arg("ego_dynamics_on_route"), arg("vehicle_physics")))
      .def(self_ns::str(self_ns::self));
}
