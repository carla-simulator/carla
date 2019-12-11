// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/rss/RssSensor.h>
#include <carla/rss/RssRestrictor.h>
#include <carla/sensor/data/RssResponse.h>

#include "ad_rss/world/RssDynamics.hpp"

namespace ad_rss {
namespace physics {
  std::ostream &operator<<(std::ostream &out, const Acceleration &accel) {
    out << "Acceleration(value=" << std::to_string(static_cast<double>(accel))
        << ")";
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const Distance &d) {
    out << "Distance(value=" << std::to_string(static_cast<double>(d))
        << ")";
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const Duration &d) {
    out << "Duration(value=" << std::to_string(static_cast<double>(d))
        << ")";
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const Speed &speed) {
    out << "Speed(value=" << std::to_string(static_cast<double>(speed))
        << ")";
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const AccelerationRange &range) {
    out << "AccelerationRestriction(min=" << std::to_string(static_cast<double>(range.minimum))
        << ", max= " << std::to_string(static_cast<double>(range.maximum))
        << ")";
    return out;
  }
} // namespace physics

namespace world {
  std::ostream &operator<<(std::ostream &out, const AccelerationRestriction &restr) {
    out << "AccelerationRestriction(longitudinal_range=" << restr.longitudinalRange
        << ", lateral_left_range=" << restr.lateralLeftRange
        << ", lateral_right_range=" << restr.lateralRightRange
        << ")";
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const LateralRssAccelerationValues &values) {
    out << "LateralRssAccelerationValues(accel_max=" << values.accelMax
        << ", brake_min=" << values.brakeMin
        << ")";
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const LongitudinalRssAccelerationValues &values) {
    out << "LongitudinalRssAccelerationValues(accel_max=" << values.accelMax
        << ", brake_max=" << values.brakeMax
        << ", brake_min=" << values.brakeMin
        << ", brake_min_correct=" << values.brakeMinCorrect
        << ")";
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const RssDynamics &dyn) {
    out << "RssDynamics(alpha_lon=" << dyn.alphaLon
        << ", alpha_lat=" << dyn.alphaLat
        << ", lateral_fluctuation_margin=" << std::to_string(static_cast<double>(dyn.lateralFluctuationMargin))
        << ", response_time=" << std::to_string(static_cast<double>(dyn.responseTime))
        << ")";
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const Velocity &vel) {
    out << "Velocity(speed_lon=" << vel.speedLon
        << ", speed_lat=" << vel.speedLat
        << ")";
    return out;
  }
} // namespace world
} // namespace ad_rss

namespace carla {
namespace sensor {
namespace data {
  std::ostream &operator<<(std::ostream &out, const RssResponse &resp) {
    out << "RssResponse(frame=" << std::to_string(resp.GetFrame())
        << ", timestamp=" << std::to_string(resp.GetTimestamp())
        << ", valid=" << resp.GetResponseValid()
        << ')';
    return out;
  }

} // namespace data
} // namespace sensor

namespace rss {
std::ostream &operator<<(std::ostream &out, const RssRestrictor &) {
  out << "RssRestrictor()";
  return out;
} // namespace rss
} // namespace carla



namespace carla {
}
}

static auto GetEgoVehicleDynamics(const carla::client::RssSensor &self) {
  ad_rss::world::RssDynamics dynamics(self.GetEgoVehicleDynamics());
  return dynamics;
}

static auto GetOtherVehicleDynamics(const carla::client::RssSensor &self) {
  ad_rss::world::RssDynamics dynamics(self.GetOtherVehicleDynamics());
  return dynamics;
}

void export_ad_rss() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cs = carla::sensor;
  namespace csd = carla::sensor::data;

  enum_<csd::LateralResponse>("LateralResponse")
    .value("None", csd::LateralResponse::None)
    .value("BrakeMin", csd::LateralResponse::BrakeMin)
  ;

  enum_<csd::LongitudinalResponse>("LongitudinalResponse")
    .value("None", csd::LongitudinalResponse::None)
    .value("BrakeMinCorrect", csd::LongitudinalResponse::BrakeMinCorrect)
    .value("BrakeMin", csd::LongitudinalResponse::BrakeMin)
  ;

  class_<ad_rss::physics::Acceleration>("Acceleration", init<double>())
    .def_readonly("value", &ad_rss::physics::Acceleration::operator double)
    .def(self_ns::str(self_ns::self))
  ;

  class_<ad_rss::physics::Distance>("Distance", init<double>())
    .def_readonly("value", &ad_rss::physics::Distance::operator double)
    .def(self_ns::str(self_ns::self))
  ;

  class_<ad_rss::physics::Duration>("Duration", init<double>())
    .def_readonly("value", &ad_rss::physics::Duration::operator double)
    .def(self_ns::str(self_ns::self))
  ;

  class_<ad_rss::physics::AccelerationRange>("AccelerationRange")
    .def_readwrite("minimum", &ad_rss::physics::AccelerationRange::minimum)
    .def_readwrite("maximum", &ad_rss::physics::AccelerationRange::maximum)
    .def(self_ns::str(self_ns::self))
  ;

  class_<ad_rss::world::AccelerationRestriction>("AccelerationRestriction")
    .def_readwrite("longitudinal_range", &ad_rss::world::AccelerationRestriction::longitudinalRange)
    .def_readwrite("lateral_left_range", &ad_rss::world::AccelerationRestriction::lateralLeftRange)
    .def_readwrite("lateral_right_range", &ad_rss::world::AccelerationRestriction::lateralRightRange)
    .def(self_ns::str(self_ns::self))
  ;

  class_<ad_rss::world::LateralRssAccelerationValues>("LateralRssAccelerationValues")
    .def_readwrite("accel_max", &ad_rss::world::LateralRssAccelerationValues::accelMax)
    .def_readwrite("brake_min", &ad_rss::world::LateralRssAccelerationValues::brakeMin)
    .def(self_ns::str(self_ns::self))
  ;

  class_<ad_rss::world::LongitudinalRssAccelerationValues>("LongitudinalRssAccelerationValues")
    .def_readwrite("accel_max", &ad_rss::world::LongitudinalRssAccelerationValues::accelMax)
    .def_readwrite("brake_max", &ad_rss::world::LongitudinalRssAccelerationValues::brakeMax)
    .def_readwrite("brake_min", &ad_rss::world::LongitudinalRssAccelerationValues::brakeMin)
    .def_readwrite("brake_min_correct", &ad_rss::world::LongitudinalRssAccelerationValues::brakeMinCorrect)
    .def(self_ns::str(self_ns::self))
  ;

  class_<ad_rss::world::RssDynamics>("RssDynamics")
    .def_readwrite("alpha_lon", &ad_rss::world::RssDynamics::alphaLon)
    .def_readwrite("alpha_lat", &ad_rss::world::RssDynamics::alphaLat)
    .def_readwrite("lateral_fluctuation_margin", &ad_rss::world::RssDynamics::lateralFluctuationMargin)
    .def_readwrite("response_time", &ad_rss::world::RssDynamics::responseTime)
    .def(self_ns::str(self_ns::self))
  ;

  class_<ad_rss::physics::Speed>("Speed", init<double>())
    .def_readonly("value", &ad_rss::physics::Speed::operator double)
    .def(self_ns::str(self_ns::self))
  ;

  class_<ad_rss::world::Velocity>("Velocity")
    .def_readwrite("speed_lon", &ad_rss::world::Velocity::speedLon)
    .def_readwrite("speed_lat", &ad_rss::world::Velocity::speedLat)
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::RssResponse, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::RssResponse>>("RssResponse", no_init)
    .add_property("response_valid", &csd::RssResponse::GetResponseValid)
    .add_property("longitudinal_response", CALL_RETURNING_COPY(csd::RssResponse, GetLongitudinalResponse))
    .add_property("lateral_response_right", CALL_RETURNING_COPY(csd::RssResponse, GetLateralResponseRight))
    .add_property("lateral_response_left", CALL_RETURNING_COPY(csd::RssResponse, GetLateralResponseLeft))
    .add_property("acceleration_restriction", CALL_RETURNING_COPY(csd::RssResponse, GetAccelerationRestriction))
    .add_property("ego_velocity", CALL_RETURNING_COPY(csd::RssResponse, GetEgoVelocity))
    .def(self_ns::str(self_ns::self))
  ;

  class_<cc::RssSensor, bases<cc::Sensor>, boost::noncopyable, boost::shared_ptr<cc::RssSensor>>
      ("RssSensor", no_init)
    .def_readwrite("visualize_results", &cc::RssSensor::visualize_results)
    .add_property("ego_vehicle_dynamics", &GetEgoVehicleDynamics, &cc::RssSensor::SetEgoVehicleDynamics)
    .add_property("other_vehicle_dynamics", &GetOtherVehicleDynamics, &cc::RssSensor::SetOtherVehicleDynamics)
    .def(self_ns::str(self_ns::self))
  ;

  class_<carla::rss::RssRestrictor, boost::noncopyable, boost::shared_ptr<carla::rss::RssRestrictor>>
      ("RssRestrictor", no_init)
    .def(init<>())
    .def("restrictVehicleControl", &carla::rss::RssRestrictor::restrictVehicleControl, (arg("restriction"), arg("vehicleControl")))
    .def(self_ns::str(self_ns::self))
  ;
}


