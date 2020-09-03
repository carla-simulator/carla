// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rss/RssCheck.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#ifdef RSS_USE_TBB
#include <tbb/tbb.h>
#endif
#include <ad/map/access/Logging.hpp>
#include <ad/map/access/Operation.hpp>
#include <ad/map/intersection/Intersection.hpp>
#include <ad/map/lane/Operation.hpp>
#include <ad/map/match/AdMapMatching.hpp>
#include <ad/map/match/MapMatchedOperation.hpp>
#include <ad/map/route/LaneIntervalOperation.hpp>
#include <ad/map/route/Operation.hpp>
#include <ad/map/route/Planning.hpp>
#include <ad/rss/map/Logging.hpp>
#include <ad/rss/map/RssObjectConversion.hpp>
#include <ad/rss/map/RssObjectData.hpp>
#include <ad/rss/map/RssSceneCreator.hpp>
#include <ad/rss/state/RssStateOperation.hpp>
#include <chrono>
#include <tuple>

#include "carla/client/Map.h"
#include "carla/client/TrafficLight.h"
#include "carla/client/Vehicle.h"
#include "carla/client/Walker.h"
#include "carla/client/Waypoint.h"

#define DEBUG_TIMING 0

namespace carla {
namespace rss {

void printRoute(std::string const &route_descr, ::ad::map::route::FullRoute const &route) {
  std::cout << route_descr << std::endl;
  for (auto road_segment : route.roadSegments) {
    for (auto lane_segment : road_segment.drivableLaneSegments) {
      std::cout << "(" << static_cast<uint64_t>(lane_segment.laneInterval.laneId) << " | " << std::setprecision(2)
                << static_cast<double>(lane_segment.laneInterval.start) << ":"
                << static_cast<double>(lane_segment.laneInterval.end) << ")    ";
    }
    std::cout << std::endl;
  }
}

// constants for deg-> rad conversion PI / 180
constexpr float to_radians = static_cast<float>(M_PI) / 180.0f;

EgoDynamicsOnRoute::EgoDynamicsOnRoute()
  : time_since_epoch_check_start_ms(0.),
    time_since_epoch_check_end_ms(0.),
    ego_speed(0.),
    min_stopping_distance(0.),
    ego_center({0., 0., 0.}),
    ego_heading(0.),
    ego_heading_change(0.),
    ego_steering_angle(0.),
    ego_center_within_route(false),
    crossing_border(false),
    route_heading(0.),
    route_nominal_center({0., 0., 0.}),
    heading_diff(0.),
    route_speed_lat(0.),
    route_speed_lon(0.),
    route_accel_lat(0.),
    route_accel_lon(0.),
    avg_route_accel_lat(0.),
    avg_route_accel_lon(0.) {
  timestamp.elapsed_seconds = 0.;
}

std::shared_ptr<spdlog::logger> getLogger() {
  static auto logger = spdlog::stdout_color_mt("RssCheck");
  return logger;
}

std::shared_ptr<spdlog::logger> getTimingLogger() {
  static auto logger = spdlog::stdout_color_mt("RssCheckTiming");
  return logger;
}

::ad::rss::world::RssDynamics RssCheck::GetDefaultVehicleDynamics() {
  ::ad::rss::world::RssDynamics default_ego_vehicle_dynamics;
  default_ego_vehicle_dynamics.alphaLon.accelMax = ::ad::physics::Acceleration(3.5);
  default_ego_vehicle_dynamics.alphaLon.brakeMax = ::ad::physics::Acceleration(-8.);
  default_ego_vehicle_dynamics.alphaLon.brakeMin = ::ad::physics::Acceleration(-4.);
  default_ego_vehicle_dynamics.alphaLon.brakeMinCorrect = ::ad::physics::Acceleration(-3);
  default_ego_vehicle_dynamics.alphaLat.accelMax = ::ad::physics::Acceleration(0.2);
  default_ego_vehicle_dynamics.alphaLat.brakeMin = ::ad::physics::Acceleration(-0.8);
  default_ego_vehicle_dynamics.lateralFluctuationMargin = ::ad::physics::Distance(0.1);
  default_ego_vehicle_dynamics.responseTime = ::ad::physics::Duration(1.0);
  default_ego_vehicle_dynamics.maxSpeedOnAcceleration = ::ad::physics::Speed(100.);
  default_ego_vehicle_dynamics.unstructuredSettings.pedestrianTurningRadius = ad::physics::Distance(2.0);
  default_ego_vehicle_dynamics.unstructuredSettings.driveAwayMaxAngle = ad::physics::Angle(2.4);
  default_ego_vehicle_dynamics.unstructuredSettings.vehicleYawRateChange = ad::physics::AngularAcceleration(0.3);
  default_ego_vehicle_dynamics.unstructuredSettings.vehicleMinRadius = ad::physics::Distance(3.5);
  default_ego_vehicle_dynamics.unstructuredSettings.vehicleTrajectoryCalculationStep = ad::physics::Duration(0.2);
  return default_ego_vehicle_dynamics;
}

::ad::rss::world::RssDynamics RssCheck::GetDefaultPedestrianDynamics() {
  ::ad::rss::world::RssDynamics default_pedestrian_dynamics;
  default_pedestrian_dynamics.alphaLon.accelMax = ::ad::physics::Acceleration(2.0);
  default_pedestrian_dynamics.alphaLon.brakeMax = ::ad::physics::Acceleration(-4.);
  default_pedestrian_dynamics.alphaLon.brakeMin = ::ad::physics::Acceleration(-2.);
  default_pedestrian_dynamics.alphaLon.brakeMinCorrect = ::ad::physics::Acceleration(-2.);
  default_pedestrian_dynamics.alphaLat.accelMax = ::ad::physics::Acceleration(0.001);
  default_pedestrian_dynamics.alphaLat.brakeMin = ::ad::physics::Acceleration(-0.001);
  default_pedestrian_dynamics.lateralFluctuationMargin = ::ad::physics::Distance(0.1);
  default_pedestrian_dynamics.responseTime = ::ad::physics::Duration(0.5);
  default_pedestrian_dynamics.maxSpeedOnAcceleration = ::ad::physics::Speed(10.);
  default_pedestrian_dynamics.unstructuredSettings.pedestrianTurningRadius = ad::physics::Distance(2.0);
  default_pedestrian_dynamics.unstructuredSettings.driveAwayMaxAngle = ad::physics::Angle(2.4);
  default_pedestrian_dynamics.unstructuredSettings.vehicleYawRateChange = ad::physics::AngularAcceleration(0.3);
  default_pedestrian_dynamics.unstructuredSettings.vehicleMinRadius = ad::physics::Distance(3.5);
  default_pedestrian_dynamics.unstructuredSettings.vehicleTrajectoryCalculationStep = ad::physics::Duration(0.2);

  return default_pedestrian_dynamics;
}

RssCheck::RssCheck(float maximum_steering_angle)
  : _maximum_steering_angle(maximum_steering_angle), _road_boundaries_mode(GetDefaultRoadBoundariesMode()) {
  _logger = getLogger();
  _timing_logger = getTimingLogger();
  _timing_logger->set_level(spdlog::level::off);

  SetLogLevel(spdlog::level::warn);
  SetMapLogLevel(spdlog::level::warn);

  _default_actor_constellation_callback_ego_vehicle_dynamics = GetDefaultVehicleDynamics();
  _default_actor_constellation_callback_other_vehicle_dynamics = GetDefaultVehicleDynamics();
  // set the response time of others vehicles to 2 seconds; the rest stays the same
  _default_actor_constellation_callback_other_vehicle_dynamics.responseTime = ::ad::physics::Duration(2.0);
  _default_actor_constellation_callback_pedestrian_dynamics = GetDefaultPedestrianDynamics();

  // Create a default callback.
  _actor_constellation_callback =
      [this](carla::SharedPtr<::carla::rss::ActorConstellationData> actor_constellation_data)
      -> ::carla::rss::ActorConstellationResult {
    ::carla::rss::ActorConstellationResult actor_constellation_result;

    actor_constellation_result.rss_calculation_mode = ::ad::rss::map::RssMode::NotRelevant;
    actor_constellation_result.restrict_speed_limit_mode =
        ::ad::rss::map::RssSceneCreation::RestrictSpeedLimitMode::IncreasedSpeedLimit10;
    actor_constellation_result.actor_object_type = ad::rss::world::ObjectType::Invalid;
    actor_constellation_result.ego_vehicle_dynamics = this->_default_actor_constellation_callback_ego_vehicle_dynamics;
    actor_constellation_result.actor_dynamics = this->_default_actor_constellation_callback_other_vehicle_dynamics;

    if (actor_constellation_data->other_actor != nullptr) {
      if (boost::dynamic_pointer_cast<carla::client::Walker>(actor_constellation_data->other_actor) != nullptr) {
        actor_constellation_result.rss_calculation_mode = ::ad::rss::map::RssMode::Unstructured;
        actor_constellation_result.actor_object_type = ad::rss::world::ObjectType::Pedestrian;
        actor_constellation_result.actor_dynamics = this->_default_actor_constellation_callback_pedestrian_dynamics;
      } else if (boost::dynamic_pointer_cast<carla::client::Vehicle>(actor_constellation_data->other_actor) !=
                 nullptr) {
        actor_constellation_result.rss_calculation_mode = ::ad::rss::map::RssMode::Structured;
        actor_constellation_result.actor_object_type = ad::rss::world::ObjectType::OtherVehicle;

        if (GetSpeed(*actor_constellation_data->other_actor) == ::ad::physics::Speed(0.)) {
          /*
             special handling for vehicles standing still
             to cope with not yet implemented lateral intersection checks in core RSS implementation
             if the other is standing still, we don't assume that he will accelerate
             otherwise if standing at the intersection the acceleration within reaction time
             will allow to enter the intersection which current RSS implementation will immediately consider
             as dangerous
           */
          actor_constellation_result.actor_dynamics.alphaLon.accelMax = ::ad::physics::Acceleration(0.);
        }
      }
    }

    /* since the ego vehicle is controlled manually, it is easy possible that the ego vehicle
      accelerates far more in lateral direction than the ego_dynamics indicate
      in an automated vehicle this would be considered by the low-level controller when the RSS restriction
      is taken into account properly
      but the simple RSS restrictor within CARLA is not able to do so...
      So we should at least tell RSS about the fact that we acceleration more than this
      to be able to react on this
      */
    auto const abs_avg_route_accel_lat = std::fabs(actor_constellation_data->ego_dynamics_on_route.avg_route_accel_lat);
    if (abs_avg_route_accel_lat > actor_constellation_result.ego_vehicle_dynamics.alphaLat.accelMax) {
      actor_constellation_result.ego_vehicle_dynamics.alphaLat.accelMax =
          std::min(ad::physics::Acceleration(20.), abs_avg_route_accel_lat);
    }
    return actor_constellation_result;
  };

  // set the default dynamics
  _carla_rss_state.default_ego_vehicle_dynamics = _default_actor_constellation_callback_ego_vehicle_dynamics;

  _logger->debug("RssCheck with default actor constellation callback created");
}

RssCheck::RssCheck(float maximum_steering_angle,
                   ActorConstellationCallbackFunctionType rss_actor_constellation_callback,
                   carla::SharedPtr<carla::client::Actor> const &carla_ego_actor)
  : _maximum_steering_angle(maximum_steering_angle),
    _actor_constellation_callback(rss_actor_constellation_callback),
    _road_boundaries_mode(GetDefaultRoadBoundariesMode()) {
  _logger = getLogger();
  _timing_logger = getTimingLogger();
  _timing_logger->set_level(spdlog::level::off);

  SetLogLevel(spdlog::level::warn);
  SetMapLogLevel(spdlog::level::warn);

  _carla_rss_state.ego_match_object = GetMatchObject(carla_ego_actor, ::ad::physics::Distance(2.0));
  UpdateDefaultRssDynamics(_carla_rss_state);

  _logger->debug("RssCheck created");
}

RssCheck::~RssCheck() {}

void RssCheck::SetLogLevel(const spdlog::level::level_enum &log_level) {
  spdlog::set_level(log_level);
  _logger->set_level(log_level);
}

void RssCheck::SetMapLogLevel(const spdlog::level::level_enum &map_log_level) {
  ::ad::map::access::getLogger()->set_level(map_log_level);
  ::ad::rss::map::getLogger()->set_level(map_log_level);
}

const ::ad::rss::world::RssDynamics &RssCheck::GetDefaultActorConstellationCallbackEgoVehicleDynamics() const {
  return _default_actor_constellation_callback_ego_vehicle_dynamics;
}

void RssCheck::SetDefaultActorConstellationCallbackEgoVehicleDynamics(
    const ::ad::rss::world::RssDynamics &ego_vehicle_dynamics) {
  _default_actor_constellation_callback_ego_vehicle_dynamics = ego_vehicle_dynamics;
}

const ::ad::rss::world::RssDynamics &RssCheck::GetDefaultActorConstellationCallbackOtherVehicleDynamics() const {
  return _default_actor_constellation_callback_other_vehicle_dynamics;
}

void RssCheck::SetDefaultActorConstellationCallbackOtherVehicleDynamics(
    const ::ad::rss::world::RssDynamics &other_vehicle_dynamics) {
  _default_actor_constellation_callback_other_vehicle_dynamics = other_vehicle_dynamics;
}

const ::ad::rss::world::RssDynamics &RssCheck::GetDefaultActorConstellationCallbackPedestrianDynamics() const {
  return _default_actor_constellation_callback_pedestrian_dynamics;
}

void RssCheck::SetDefaultActorConstellationCallbackPedestrianDynamics(
    const ::ad::rss::world::RssDynamics &pedestrian_dynamics) {
  _default_actor_constellation_callback_pedestrian_dynamics = pedestrian_dynamics;
}

const ::carla::rss::RoadBoundariesMode &RssCheck::GetRoadBoundariesMode() const {
  return _road_boundaries_mode;
}

void RssCheck::SetRoadBoundariesMode(const ::carla::rss::RoadBoundariesMode &road_boundaries_mode) {
  _road_boundaries_mode = road_boundaries_mode;
}

void RssCheck::AppendRoutingTarget(::carla::geom::Transform const &routing_target) {
  _routing_targets_to_append.push_back(
      ::ad::map::point::createENUPoint(routing_target.location.x, -1. * routing_target.location.y, 0.));
}

const std::vector<::carla::geom::Transform> RssCheck::GetRoutingTargets() const {
  std::vector<::carla::geom::Transform> routing_targets;
  if (withinValidInputRange(_routing_targets)) {
    for (auto const &target : _routing_targets) {
      ::carla::geom::Transform routing_target;
      routing_target.location.x = static_cast<float>(target.x);
      routing_target.location.y = static_cast<float>(-target.y);
      routing_target.location.z = 0.f;
      routing_targets.push_back(routing_target);
    }
  }
  return routing_targets;
}

void RssCheck::ResetRoutingTargets() {
  _routing_targets.clear();
  _routing_targets_to_append.clear();
}

void RssCheck::DropRoute() {
  _logger->debug("Dropping Route:: {}", _carla_rss_state.ego_route);
  _carla_rss_state.ego_route = ::ad::map::route::FullRoute();
}

bool RssCheck::CheckObjects(carla::client::Timestamp const &timestamp,
                            carla::SharedPtr<carla::client::ActorList> const &actors,
                            carla::SharedPtr<carla::client::Actor> const &carla_ego_actor,
                            ::ad::rss::state::ProperResponse &output_response,
                            ::ad::rss::state::RssStateSnapshot &output_rss_state_snapshot,
                            ::ad::rss::situation::SituationSnapshot &output_situation_snapshot,
                            ::ad::rss::world::WorldModel &output_world_model,
                            EgoDynamicsOnRoute &output_rss_ego_dynamics_on_route) {
  bool result = false;
  try {
    double const time_since_epoch_check_start_ms =
        std::chrono::duration<double, std::milli>(std::chrono::system_clock::now().time_since_epoch()).count();
#if DEBUG_TIMING
    std::cout << "--- time: " << timestamp.frame << ", " << timestamp.elapsed_seconds << std::endl;
    auto t_start = std::chrono::high_resolution_clock::now();
    auto t_end = std::chrono::high_resolution_clock::now();
    std::cout << "-> SC " << std::chrono::duration<double, std::milli>(t_end - t_start).count() << " start checkObjects"
              << std::endl;
#endif

    const auto carla_ego_vehicle = boost::dynamic_pointer_cast<carla::client::Vehicle>(carla_ego_actor);
    if (carla_ego_vehicle == nullptr) {
      _logger->error("RSS Sensor only support vehicles as ego.");
    }

#if DEBUG_TIMING
    t_end = std::chrono::high_resolution_clock::now();
    std::cout << "-> ME " << std::chrono::duration<double, std::milli>(t_end - t_start).count()
              << " before  MapMatching" << std::endl;
#endif

    // allow the vehicle to be at least 2.0 m away form the route to not lose
    // the contact to the route
    auto const ego_match_object = GetMatchObject(carla_ego_actor, ::ad::physics::Distance(2.0));

    if (::ad::map::point::isValid(_carla_rss_state.ego_match_object.enuPosition.centerPoint, false)) {
      // check for bigger position jumps of the ego vehicle
      auto const travelled_distance = ::ad::map::point::distance(
          _carla_rss_state.ego_match_object.enuPosition.centerPoint, ego_match_object.enuPosition.centerPoint);
      if (travelled_distance > ::ad::physics::Distance(10.)) {
        _logger->warn("Jump in ego vehicle position detected {} -> {}! Force reroute!",
                      _carla_rss_state.ego_match_object.enuPosition.centerPoint,
                      ego_match_object.enuPosition.centerPoint);
        DropRoute();
      }
    }

    _carla_rss_state.ego_match_object = ego_match_object;

    _logger->trace("MapMatch:: {}", _carla_rss_state.ego_match_object);

#if DEBUG_TIMING
    t_end = std::chrono::high_resolution_clock::now();
    std::cout << "-> ME " << std::chrono::duration<double, std::milli>(t_end - t_start).count()
              << " after ego MapMatching" << std::endl;
#endif

    UpdateRoute(_carla_rss_state);

#if DEBUG_TIMING
    t_end = std::chrono::high_resolution_clock::now();
    std::cout << "-> RU " << std::chrono::duration<double, std::milli>(t_end - t_start).count()
              << " after route update " << std::endl;
#endif

    _carla_rss_state.ego_dynamics_on_route = CalculateEgoDynamicsOnRoute(
        timestamp, time_since_epoch_check_start_ms, *carla_ego_vehicle, _carla_rss_state.ego_match_object,
        _carla_rss_state.ego_route, _carla_rss_state.default_ego_vehicle_dynamics,
        _carla_rss_state.ego_dynamics_on_route);

    UpdateDefaultRssDynamics(_carla_rss_state);

    CreateWorldModel(timestamp, *actors, *carla_ego_vehicle, _carla_rss_state);

#if DEBUG_TIMING
    t_end = std::chrono::high_resolution_clock::now();
    std::cout << "-> WM " << std::chrono::duration<double, std::milli>(t_end - t_start).count()
              << " after create world model " << std::endl;
#endif

    result = PerformCheck(_carla_rss_state);

#if DEBUG_TIMING
    t_end = std::chrono::high_resolution_clock::now();
    std::cout << "-> CH " << std::chrono::duration<double, std::milli>(t_end - t_start).count() << " end RSS check"
              << std::endl;
#endif

    AnalyseCheckResults(_carla_rss_state);

#if DEBUG_TIMING
    t_end = std::chrono::high_resolution_clock::now();
    std::cout << "-> AN " << std::chrono::duration<double, std::milli>(t_end - t_start).count()
              << " end analyze results" << std::endl;
#endif

    _carla_rss_state.ego_dynamics_on_route.time_since_epoch_check_end_ms =
        std::chrono::duration<double, std::milli>(std::chrono::system_clock::now().time_since_epoch()).count();

    // store result
    output_response = _carla_rss_state.proper_response;
    output_rss_state_snapshot = _carla_rss_state.rss_state_snapshot;
    output_situation_snapshot = _carla_rss_state.situation_snapshot;
    output_world_model = _carla_rss_state.world_model;
    output_rss_ego_dynamics_on_route = _carla_rss_state.ego_dynamics_on_route;
    if (_carla_rss_state.dangerous_state) {
      _logger->debug("===== ROUTE NOT SAFE =====");
    } else {
      _logger->debug("===== ROUTE SAFE =====");
    }

#if DEBUG_TIMING
    t_end = std::chrono::high_resolution_clock::now();
    std::cout << "-> EC " << std::chrono::duration<double, std::milli>(t_end - t_start).count() << " end check objects"
              << std::endl;
#endif
  } catch (...) {
    _logger->error("Exception -> Check failed");
  }
  return result;
}

::ad::map::match::Object RssCheck::GetMatchObject(carla::SharedPtr<carla::client::Actor> const &actor,
                                                  ::ad::physics::Distance const &sampling_distance) const {
  ::ad::map::match::Object match_object;

  auto const vehicle_transform = actor->GetTransform();
  match_object.enuPosition.centerPoint.x = ::ad::map::point::ENUCoordinate(vehicle_transform.location.x);
  match_object.enuPosition.centerPoint.y = ::ad::map::point::ENUCoordinate(-1. * vehicle_transform.location.y);
  match_object.enuPosition.centerPoint.z = ::ad::map::point::ENUCoordinate(0.);  // vehicle_transform.location.z;
  match_object.enuPosition.heading =
      ::ad::map::point::createENUHeading(-1 * vehicle_transform.rotation.yaw * to_radians);

  auto const vehicle = boost::dynamic_pointer_cast<carla::client::Vehicle>(actor);
  auto const walker = boost::dynamic_pointer_cast<carla::client::Walker>(actor);
  if (vehicle != nullptr) {
    const auto &bounding_box = vehicle->GetBoundingBox();
    match_object.enuPosition.dimension.length = ::ad::physics::Distance(2 * bounding_box.extent.x);
    match_object.enuPosition.dimension.width = ::ad::physics::Distance(2 * bounding_box.extent.y);
    match_object.enuPosition.dimension.height = ::ad::physics::Distance(2 * bounding_box.extent.z);
  } else if (walker != nullptr) {
    const auto &bounding_box = walker->GetBoundingBox();
    match_object.enuPosition.dimension.length = ::ad::physics::Distance(2 * bounding_box.extent.x);
    match_object.enuPosition.dimension.width = ::ad::physics::Distance(2 * bounding_box.extent.y);
    match_object.enuPosition.dimension.height = ::ad::physics::Distance(2 * bounding_box.extent.z);
  } else {
    _logger->error("Could not get bounding box of actor {}", actor->GetId());
  }
  match_object.enuPosition.enuReferencePoint = ::ad::map::access::getENUReferencePoint();

  ::ad::map::match::AdMapMatching map_matching;
  match_object.mapMatchedBoundingBox =
      map_matching.getMapMatchedBoundingBox(match_object.enuPosition, sampling_distance);

  return match_object;
}

::ad::physics::Speed RssCheck::GetSpeed(carla::client::Actor const &actor) const {
  auto velocity = actor.GetVelocity();
  auto const actor_transform = actor.GetTransform();
  actor_transform.rotation.InverseRotateVector(velocity);

  ::ad::physics::Speed speed(std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y));
  if (velocity.x < 0.) {
    speed = -speed;
  }

  return speed;
}

::ad::physics::AngularVelocity RssCheck::GetHeadingChange(carla::client::Actor const &actor) const {
  auto const angular_velocity = actor.GetAngularVelocity();
  ::ad::physics::AngularVelocity heading_change(-1. * angular_velocity.z * to_radians);
  return heading_change;
}

::ad::physics::Angle RssCheck::GetSteeringAngle(carla::client::Vehicle const &actor) const {
  auto const steer_ratio = actor.GetControl().steer;
  ::ad::physics::Angle steering_angle(-1 * _maximum_steering_angle * steer_ratio);
  return steering_angle;
}

void RssCheck::UpdateRoute(CarlaRssState &carla_rss_state) {
  _logger->trace("Update route start: {}", carla_rss_state.ego_route);

  // remove the parts of the route already taken, try to prepend route sections
  // (i.e. when driving backwards)
  // try to ensure that the back of the vehicle is still within the route to
  // support orientation calculation
  ::ad::map::point::ParaPointList all_lane_matches;
  for (auto reference_point :
       {::ad::map::match::ObjectReferencePoints::RearRight, ::ad::map::match::ObjectReferencePoints::RearLeft}) {
    auto const &reference_position =
        carla_rss_state.ego_match_object.mapMatchedBoundingBox.referencePointPositions[size_t(reference_point)];
    auto const para_points = ::ad::map::match::getParaPoints(reference_position);
    all_lane_matches.insert(all_lane_matches.end(), para_points.begin(), para_points.end());
  }

  auto shorten_route_result = ::ad::map::route::shortenRoute(
      all_lane_matches, carla_rss_state.ego_route,
      ::ad::map::route::ShortenRouteMode::DontCutIntersectionAndPrependIfSucceededBeforeRoute);
  if (shorten_route_result == ::ad::map::route::ShortenRouteResult::SucceededIntersectionNotCut) {
    shorten_route_result = ::ad::map::route::ShortenRouteResult::Succeeded;
  }

  bool routing_target_check_finished = false;
  while ((!_routing_targets.empty()) && (!routing_target_check_finished)) {
    auto const next_target = _routing_targets.front();
    auto const &distance_to_next_target =
        ::ad::map::point::distance(next_target, carla_rss_state.ego_match_object.enuPosition.centerPoint);
    if (distance_to_next_target < ::ad::physics::Distance(3.)) {
      _routing_targets.erase(_routing_targets.begin());
      _logger->debug("Next target reached: {}; remaining targets: {}", next_target, _routing_targets);
    } else {
      routing_target_check_finished = true;
    }
  }

  bool reroute_required = false;
  if (!_routing_targets_to_append.empty()) {
    reroute_required = true;
    _routing_targets.insert(_routing_targets.end(), _routing_targets_to_append.begin(),
                            _routing_targets_to_append.end());
    _logger->debug("Appending new routing targets: {}; resulting targets: {}", _routing_targets_to_append,
                   _routing_targets);
    _routing_targets_to_append.clear();
  }

  ::ad::physics::Distance const route_target_length(50.);

  if ((!reroute_required) && (shorten_route_result == ::ad::map::route::ShortenRouteResult::Succeeded)) {
    std::vector<::ad::map::route::FullRoute> additional_routes;
    auto const route_valid =
        ::ad::map::route::extendRouteToDistance(carla_rss_state.ego_route, route_target_length, additional_routes);

    if (route_valid) {
      if (additional_routes.size() > 0u) {
        // take a random extension to the route
        std::size_t route_index = static_cast<std::size_t>(std::rand()) % (additional_routes.size() + 1);
        if (route_index < additional_routes.size()) {
          // we decided for one of the additional routes
          _logger->debug("Additional Routes: {}->{}", additional_routes.size(), route_index);
          carla_rss_state.ego_route = additional_routes[route_index];
        } else {
          // we decided for the extension within route, nothing to be done
          _logger->debug("Additional Routes: expand current");
        }
      }
    } else {
      reroute_required = true;
    }
  } else {
    // on all other results we recreate the route
    reroute_required = true;
  }

  // create the route if required
  if (reroute_required) {
    // try to create routes
    std::vector<::ad::map::route::FullRoute> all_new_routes;
    for (const auto &position :
         carla_rss_state.ego_match_object.mapMatchedBoundingBox
             .referencePointPositions[int32_t(::ad::map::match::ObjectReferencePoints::Center)]) {
      auto start_point = position.lanePoint.paraPoint;
      auto projected_start_point = start_point;
      if (!::ad::map::lane::isHeadingInLaneDirection(start_point,
                                                     carla_rss_state.ego_match_object.enuPosition.heading)) {
        _logger->debug("EgoVehicle heading in opposite lane direction");
        if (::ad::map::lane::projectPositionToLaneInHeadingDirection(
                start_point, carla_rss_state.ego_match_object.enuPosition.heading, projected_start_point)) {
          _logger->debug("Projected to lane {}", projected_start_point.laneId);
        }
      }
      _logger->debug("Route start_point: {}, projected_start_point: {}", start_point, projected_start_point);
      auto routing_start_point = ::ad::map::route::planning::createRoutingPoint(
          projected_start_point, carla_rss_state.ego_match_object.enuPosition.heading);
      if (!_routing_targets.empty() && ::ad::map::point::isValid(_routing_targets)) {
        auto new_route = ::ad::map::route::planning::planRoute(routing_start_point, _routing_targets,
                                                               ::ad::map::route::RouteCreationMode::AllRoutableLanes);
        all_new_routes.push_back(new_route);
      } else {
        auto new_routes = ::ad::map::route::planning::predictRoutesOnDistance(
            routing_start_point, route_target_length, ::ad::map::route::RouteCreationMode::AllRoutableLanes);

        for (const auto &new_route : new_routes) {
          // extend route with all lanes
          all_new_routes.push_back(new_route);
        }
      }
    }

    _logger->debug("New routes: {}", all_new_routes.size());

    if (!all_new_routes.empty()) {
      // take a random route
      std::size_t route_index = static_cast<std::size_t>(std::rand()) % (all_new_routes.size());
      carla_rss_state.ego_route = all_new_routes[route_index];
    }
  }

  _logger->trace("Update route result: {}", carla_rss_state.ego_route);
}

EgoDynamicsOnRoute RssCheck::CalculateEgoDynamicsOnRoute(
    carla::client::Timestamp const &current_timestamp, double const &time_since_epoch_check_start_ms,
    carla::client::Vehicle const &carla_vehicle, ::ad::map::match::Object match_object,
    ::ad::map::route::FullRoute const &route, ::ad::rss::world::RssDynamics const &default_ego_vehicle_dynamics,
    EgoDynamicsOnRoute const &last_dynamics) const {
  EgoDynamicsOnRoute new_dynamics;
  new_dynamics.timestamp = current_timestamp;
  new_dynamics.time_since_epoch_check_start_ms = time_since_epoch_check_start_ms;
  new_dynamics.ego_speed = GetSpeed(carla_vehicle);
  new_dynamics.ego_center = match_object.enuPosition.centerPoint;
  new_dynamics.ego_heading = match_object.enuPosition.heading;
  new_dynamics.ego_heading_change = GetHeadingChange(carla_vehicle);
  new_dynamics.ego_steering_angle = GetSteeringAngle(carla_vehicle);

  auto object_route =
      ::ad::map::route::getRouteSection(match_object, route, ::ad::map::route::RouteSectionCreationMode::AllRouteLanes);
  auto border = ::ad::map::route::getENUBorderOfRoute(object_route);
  new_dynamics.route_heading = ::ad::map::lane::getENUHeading(border, match_object.enuPosition.centerPoint);

  auto const object_center = ::ad::map::route::findCenterWaypoint(match_object, object_route);
  if (object_center.isValid()) {
    auto lane_center_point = object_center.queryPosition;
    auto lane_center_point_enu = ::ad::map::lane::getENULanePoint(lane_center_point);
    if (std::fabs(new_dynamics.route_heading) > ::ad::map::point::ENUHeading(M_PI)) {
      // if the actual center point is already outside, try to use this extended
      // object center for the route heading calculation
      new_dynamics.route_heading = ::ad::map::lane::getENUHeading(border, lane_center_point_enu);
    }

    if (object_center.laneSegmentIterator->laneInterval.wrongWay) {
      // driving on the wrong lane, so we have to project to nominal route
      // direction
      ::ad::map::lane::projectPositionToLaneInHeadingDirection(lane_center_point, new_dynamics.route_heading,
                                                               lane_center_point);
      lane_center_point_enu = ::ad::map::lane::getENULanePoint(lane_center_point);
    }
    new_dynamics.route_nominal_center = lane_center_point_enu;

  } else {
    // the ego vehicle is completely outside the route, so we can't update the
    // values
    new_dynamics.route_nominal_center = last_dynamics.route_nominal_center;
    new_dynamics.route_heading = last_dynamics.route_heading;
  }

  new_dynamics.heading_diff =
      ::ad::map::point::normalizeENUHeading(new_dynamics.route_heading - new_dynamics.ego_heading);
  new_dynamics.route_speed_lon =
      std::fabs(std::cos(static_cast<double>(new_dynamics.heading_diff))) * new_dynamics.ego_speed;
  new_dynamics.route_speed_lat = std::sin(static_cast<double>(new_dynamics.heading_diff)) * new_dynamics.ego_speed;

  bool keep_last_acceleration = true;
  if (last_dynamics.timestamp.elapsed_seconds > 0.) {
    ::ad::physics::Duration const delta_time(current_timestamp.elapsed_seconds -
                                             last_dynamics.timestamp.elapsed_seconds);
    if (delta_time > ::ad::physics::Duration(0.0001)) {
      try {
        new_dynamics.route_accel_lat = (new_dynamics.route_speed_lat - last_dynamics.route_speed_lat) / delta_time;
        new_dynamics.avg_route_accel_lat =
            ((last_dynamics.avg_route_accel_lat * 2.) + new_dynamics.route_accel_lat) / 3.;
        new_dynamics.route_accel_lon = (new_dynamics.route_speed_lon - last_dynamics.route_speed_lon) / delta_time;
        new_dynamics.avg_route_accel_lon =
            ((last_dynamics.avg_route_accel_lon * 2.) + new_dynamics.route_accel_lon) / 3.;

        if (new_dynamics.avg_route_accel_lat == ::ad::physics::Acceleration(0.)) {
          // prevent from underrun
          new_dynamics.avg_route_accel_lat = ::ad::physics::Acceleration(0.);
        }
        if (new_dynamics.avg_route_accel_lon == ::ad::physics::Acceleration(0.)) {
          // prevent from underrun
          new_dynamics.avg_route_accel_lon = ::ad::physics::Acceleration(0.);
        }
        keep_last_acceleration = false;
      } catch (...) {
      }
    }
  }

  if (keep_last_acceleration) {
    new_dynamics.route_accel_lat = last_dynamics.route_accel_lat;
    new_dynamics.avg_route_accel_lat = last_dynamics.avg_route_accel_lat;
    new_dynamics.route_accel_lon = last_dynamics.route_accel_lon;
    new_dynamics.avg_route_accel_lon = last_dynamics.avg_route_accel_lon;
  }

  // check if the center point (and only the center point) is still found on the
  // route
  ::ad::map::point::ParaPointList in_lane_matches;
  for (auto &match_position : match_object.mapMatchedBoundingBox
                                  .referencePointPositions[int32_t(::ad::map::match::ObjectReferencePoints::Center)]) {
    if (match_position.type == ::ad::map::match::MapMatchedPositionType::LANE_IN) {
      in_lane_matches.push_back(match_position.lanePoint.paraPoint);
    }
  }
  auto const object_in_lane_center = ::ad::map::route::findNearestWaypoint(in_lane_matches, route);
  new_dynamics.ego_center_within_route = object_in_lane_center.isValid();
  // evaluated by AnalyseResults
  new_dynamics.crossing_border = false;

  // calculate the ego stopping distance, to be able to reduce the effort

  ::ad::rss::map::RssObjectData ego_object_data;
  ego_object_data.id = ::ad::rss::world::ObjectId(0u);
  ego_object_data.type = ::ad::rss::world::ObjectType::EgoVehicle;
  ego_object_data.matchObject = match_object;
  ego_object_data.speed = new_dynamics.ego_speed;
  ego_object_data.yawRate = new_dynamics.ego_heading_change;
  ego_object_data.steeringAngle = new_dynamics.ego_steering_angle;
  ego_object_data.rssDynamics = default_ego_vehicle_dynamics;

  ad::rss::map::RssObjectConversion object_conversion(ego_object_data);
  if (!object_conversion.calculateMinStoppingDistance(new_dynamics.min_stopping_distance)) {
    _logger->error(
        "CalculateEgoDynamicsOnRoute: calculation of min stopping distance "
        "failed. Setting to 100. ({} {} {} {})",
        match_object, new_dynamics.ego_speed, new_dynamics.ego_speed, new_dynamics.ego_heading_change,
        default_ego_vehicle_dynamics);
    new_dynamics.min_stopping_distance = ::ad::physics::Distance(100.);
  }

  _logger->trace("CalculateEgoDynamicsOnRoute: route-section {} -> dynamics: {}", object_route, new_dynamics);
  return new_dynamics;
}

void RssCheck::UpdateDefaultRssDynamics(CarlaRssState &carla_rss_state) {
  ::ad::map::match::Object other_match_object;
  carla::SharedPtr<ActorConstellationData> default_constellation_data{
      new ActorConstellationData{carla_rss_state.ego_match_object, carla_rss_state.ego_route,
                                 carla_rss_state.ego_dynamics_on_route, other_match_object, nullptr}};
  auto const default_constellation_result = _actor_constellation_callback(default_constellation_data);
  carla_rss_state.default_ego_vehicle_dynamics = default_constellation_result.ego_vehicle_dynamics;
}

::ad::map::landmark::LandmarkIdSet RssCheck::GetGreenTrafficLightsOnRoute(
    std::vector<SharedPtr<carla::client::TrafficLight>> const &traffic_lights,
    ::ad::map::route::FullRoute const &route) const {
  ::ad::map::landmark::LandmarkIdSet green_traffic_lights;

  auto next_intersection = ::ad::map::intersection::Intersection::getNextIntersectionOnRoute(route);
  if (next_intersection &&
      (next_intersection->intersectionType() == ::ad::map::intersection::IntersectionType::TrafficLight)) {
    // try to guess the the relevant traffic light with the rule: nearest
    // traffic light in respect to the incoming lane.
    // @todo: when OpenDrive maps have the traffic lights incorporated, we only
    // have to fill all green traffic lights into the green_traffic_lights list
    auto incoming_lanes = next_intersection->incomingLanesOnRoute();
    // since our route spans the whole street, we have to filter out the
    // incoming lanes with wrong way flag
    auto incoming_lanes_iter = incoming_lanes.begin();
    while (incoming_lanes_iter != incoming_lanes.end()) {
      auto find_waypoint = ::ad::map::route::findWaypoint(*incoming_lanes_iter, route);
      if (find_waypoint.isValid() && find_waypoint.laneSegmentIterator->laneInterval.wrongWay) {
        incoming_lanes_iter = incoming_lanes.erase(incoming_lanes_iter);
      } else {
        incoming_lanes_iter++;
      }
    }

    ::ad::map::match::AdMapMatching traffic_light_map_matching;
    bool found_relevant_traffic_light = false;
    for (const auto &traffic_light : traffic_lights) {
      auto traffic_light_state = traffic_light->GetState();
      carla::geom::BoundingBox trigger_bounding_box = traffic_light->GetTriggerVolume();

      auto traffic_light_transform = traffic_light->GetTransform();
      auto trigger_box_location = trigger_bounding_box.location;
      traffic_light_transform.TransformPoint(trigger_box_location);

      ::ad::map::point::ENUPoint trigger_box_position;
      trigger_box_position.x = ::ad::map::point::ENUCoordinate(trigger_box_location.x);
      trigger_box_position.y = ::ad::map::point::ENUCoordinate(-1 * trigger_box_location.y);
      trigger_box_position.z = ::ad::map::point::ENUCoordinate(0.);

      _logger->trace("traffic light[{}] Position: {}", traffic_light->GetId(), trigger_box_position);
      auto traffic_light_map_matched_positions = traffic_light_map_matching.getMapMatchedPositions(
          trigger_box_position, ::ad::physics::Distance(0.25), ::ad::physics::Probability(0.1));

      _logger->trace("traffic light[{}] Map Matched Position: {}", traffic_light->GetId(),
                     traffic_light_map_matched_positions);

      for (auto matched_position : traffic_light_map_matched_positions) {
        if (incoming_lanes.find(matched_position.lanePoint.paraPoint.laneId) != incoming_lanes.end()) {
          if (found_relevant_traffic_light &&
              (green_traffic_lights.empty() && (traffic_light_state == carla::rpc::TrafficLightState::Green))) {
            _logger->warn("found another relevant traffic light on lane {}; {} state {}",
                          matched_position.lanePoint.paraPoint.laneId, traffic_light->GetId(),
                          (traffic_light_state == carla::rpc::TrafficLightState::Green) ? "green" : "not green");
          } else {
            _logger->debug("found relevant traffic light on lane {}; {} state {}",
                           matched_position.lanePoint.paraPoint.laneId, traffic_light->GetId(),
                           (traffic_light_state == carla::rpc::TrafficLightState::Green) ? "green" : "not green");
          }

          found_relevant_traffic_light = true;

          // found matching traffic light
          if (traffic_light_state == carla::rpc::TrafficLightState::Green) {
            // @todo: currently there is only this workaround because of missign
            // OpenDrive map support for actual traffic light ids
            green_traffic_lights.insert(::ad::map::landmark::LandmarkId::getMax());
          } else {
            // if the light is not green, we don't have priority
            green_traffic_lights.clear();
          }
          break;
        }
      }
    }
  }
  return green_traffic_lights;
}

RssCheck::RssObjectChecker::RssObjectChecker(RssCheck const &rss_check,
                                             ::ad::rss::map::RssSceneCreation &scene_creation,
                                             carla::client::Vehicle const &carla_ego_vehicle,
                                             CarlaRssState const &carla_rss_state,
                                             ::ad::map::landmark::LandmarkIdSet const &green_traffic_lights)
  : _rss_check(rss_check),
    _scene_creation(scene_creation),
    _carla_ego_vehicle(carla_ego_vehicle),
    _carla_rss_state(carla_rss_state),
    _green_traffic_lights(green_traffic_lights) {}

void RssCheck::RssObjectChecker::operator()(
    const carla::SharedPtr<carla::client::Actor> other_traffic_participant) const {
  try {
    auto other_match_object = _rss_check.GetMatchObject(other_traffic_participant, ::ad::physics::Distance(2.0));

    _rss_check._logger->trace("OtherVehicleMapMatching: {} {}", other_traffic_participant->GetId(),
                              other_match_object.mapMatchedBoundingBox);

    carla::SharedPtr<ActorConstellationData> actor_constellation_data{new ActorConstellationData{
        _carla_rss_state.ego_match_object, _carla_rss_state.ego_route, _carla_rss_state.ego_dynamics_on_route,
        other_match_object, other_traffic_participant}};
    auto const actor_constellation_result = _rss_check._actor_constellation_callback(actor_constellation_data);

    auto other_speed = _rss_check.GetSpeed(*other_traffic_participant);
    auto other_heading_change = _rss_check.GetHeadingChange(*other_traffic_participant);
    auto other_steering_angle = ::ad::physics::Angle(0.);

    ::ad::rss::map::RssObjectData ego_object_data;
    ego_object_data.id = _carla_ego_vehicle.GetId();
    ego_object_data.type = ::ad::rss::world::ObjectType::EgoVehicle;
    ego_object_data.matchObject = _carla_rss_state.ego_match_object;
    ego_object_data.speed = _carla_rss_state.ego_dynamics_on_route.ego_speed;
    ego_object_data.yawRate = _carla_rss_state.ego_dynamics_on_route.ego_heading_change;
    ego_object_data.steeringAngle = _carla_rss_state.ego_dynamics_on_route.ego_steering_angle;
    ego_object_data.rssDynamics = actor_constellation_result.ego_vehicle_dynamics;

    ::ad::rss::map::RssObjectData other_object_data;
    other_object_data.id = ::ad::rss::world::ObjectId(other_traffic_participant->GetId());
    other_object_data.type = actor_constellation_result.actor_object_type;
    other_object_data.matchObject = other_match_object;
    other_object_data.speed = other_speed;
    other_object_data.yawRate = other_heading_change;
    other_object_data.steeringAngle = other_steering_angle;
    other_object_data.rssDynamics = actor_constellation_result.actor_dynamics;

    _scene_creation.appendScenes(ego_object_data, _carla_rss_state.ego_route, other_object_data,
                                 actor_constellation_result.restrict_speed_limit_mode, _green_traffic_lights,
                                 actor_constellation_result.rss_calculation_mode);

  } catch (...) {
    _rss_check._logger->error("Exception processing other traffic participant {} -> Ignoring it",
                              other_traffic_participant->GetId());
  }
}

void RssCheck::CreateWorldModel(carla::client::Timestamp const &timestamp, carla::client::ActorList const &actors,
                                carla::client::Vehicle const &carla_ego_vehicle, CarlaRssState &carla_rss_state) const {
  // only loop once over the actors since always the respective objects are created
  std::vector<SharedPtr<carla::client::TrafficLight>> traffic_lights;
  std::vector<SharedPtr<carla::client::Actor>> other_traffic_participants;
  for (const auto &actor : actors) {
    const auto traffic_light = boost::dynamic_pointer_cast<carla::client::TrafficLight>(actor);
    if (traffic_light != nullptr) {
      traffic_lights.push_back(traffic_light);
      continue;
    }

    if ((boost::dynamic_pointer_cast<carla::client::Vehicle>(actor) != nullptr) ||
        (boost::dynamic_pointer_cast<carla::client::Walker>(actor) != nullptr)) {
      if (actor->GetId() == carla_ego_vehicle.GetId()) {
        continue;
      }
      auto const relevant_distance =
          std::max(static_cast<double>(carla_rss_state.ego_dynamics_on_route.min_stopping_distance), 100.);
      if (actor->GetTransform().location.Distance(carla_ego_vehicle.GetTransform().location) < relevant_distance) {
        other_traffic_participants.push_back(actor);
      }
    }
  }

  ::ad::map::landmark::LandmarkIdSet green_traffic_lights =
      GetGreenTrafficLightsOnRoute(traffic_lights, carla_rss_state.ego_route);

  ::ad::rss::map::RssSceneCreation scene_creation(timestamp.frame, carla_rss_state.default_ego_vehicle_dynamics);

#ifdef RSS_USE_TBB
  tbb::parallel_for_each(
      other_traffic_participants.begin(), other_traffic_participants.end(),
      RssObjectChecker(*this, scene_creation, carla_ego_vehicle, carla_rss_state, green_traffic_lights));
#else
  for (auto const traffic_participant : other_traffic_participants) {
    auto checker = RssObjectChecker(*this, scene_creation, carla_ego_vehicle, carla_rss_state, green_traffic_lights);
    checker(traffic_participant);
  }
#endif

  if (_road_boundaries_mode != RoadBoundariesMode::Off) {
    // add artifical objects on the road boundaries for "stay-on-road" feature
    // use 'smart' dynamics
    auto ego_vehicle_dynamics = carla_rss_state.default_ego_vehicle_dynamics;
    ego_vehicle_dynamics.alphaLat.accelMax = ::ad::physics::Acceleration(0.);

    ::ad::rss::map::RssObjectData ego_object_data;
    ego_object_data.id = carla_ego_vehicle.GetId();
    ego_object_data.type = ::ad::rss::world::ObjectType::EgoVehicle;
    ego_object_data.matchObject = _carla_rss_state.ego_match_object;
    ego_object_data.speed = _carla_rss_state.ego_dynamics_on_route.ego_speed;
    ego_object_data.yawRate = _carla_rss_state.ego_dynamics_on_route.ego_heading_change;
    ego_object_data.steeringAngle = _carla_rss_state.ego_dynamics_on_route.ego_steering_angle;
    ego_object_data.rssDynamics = ego_vehicle_dynamics;
    scene_creation.appendRoadBoundaries(ego_object_data, carla_rss_state.ego_route,
                                        // since the route always expanded, route isn't required to expand any
                                        // more
                                        ::ad::rss::map::RssSceneCreation::AppendRoadBoundariesMode::RouteOnly);
  }

  carla_rss_state.world_model = scene_creation.getWorldModel();
}

bool RssCheck::PerformCheck(CarlaRssState &carla_rss_state) const {
  bool result = carla_rss_state.rss_check.calculateProperResponse(
      carla_rss_state.world_model, carla_rss_state.situation_snapshot, carla_rss_state.rss_state_snapshot,
      carla_rss_state.proper_response);

  if (!result) {
    _logger->warn("calculateProperResponse failed!");
  }
  else if (!carla_rss_state.proper_response.isSafe) {
    _logger->info("Unsafe route: {}", carla_rss_state.proper_response);
  }
  return result;
}

void RssCheck::AnalyseCheckResults(CarlaRssState &carla_rss_state) const {
  carla_rss_state.dangerous_state = false;
  carla_rss_state.dangerous_vehicle = false;
  carla_rss_state.dangerous_opposite_state = false;
  bool left_border_is_dangerous = false;
  bool right_border_is_dangerous = false;
  bool vehicle_triggered_left_response = false;
  bool vehicle_triggered_right_response = false;
  bool vehicle_triggered_longitudinal_response = false;
  for (auto const state : carla_rss_state.rss_state_snapshot.individualResponses) {
    if (::ad::rss::state::isDangerous(state)) {
      carla_rss_state.dangerous_state = true;
      _logger->trace("DangerousState: {}", state);
      auto dangerous_sitation_iter = std::find_if(carla_rss_state.situation_snapshot.situations.begin(),
                                                  carla_rss_state.situation_snapshot.situations.end(),
                                                  [&state](::ad::rss::situation::Situation const &situation) {
                                                    return situation.situationId == state.situationId;
                                                  });
      if (dangerous_sitation_iter != carla_rss_state.situation_snapshot.situations.end()) {
        _logger->trace("Situation: {}", *dangerous_sitation_iter);
        if (dangerous_sitation_iter->objectId == ::ad::rss::map::RssSceneCreator::getRightBorderObjectId()) {
          right_border_is_dangerous = true;
        } else if (dangerous_sitation_iter->objectId == ::ad::rss::map::RssSceneCreator::getLeftBorderObjectId()) {
          left_border_is_dangerous = true;
        } else {
          carla_rss_state.dangerous_vehicle = true;
          if (state.longitudinalState.response != ::ad::rss::state::LongitudinalResponse::None) {
            vehicle_triggered_longitudinal_response = true;
          }
          if (state.lateralStateLeft.response != ::ad::rss::state::LateralResponse::None) {
            vehicle_triggered_left_response = true;
          }
          if (state.lateralStateRight.response != ::ad::rss::state::LateralResponse::None) {
            vehicle_triggered_right_response = true;
          }
        }
        if (dangerous_sitation_iter->situationType == ::ad::rss::situation::SituationType::OppositeDirection) {
          carla_rss_state.dangerous_opposite_state = true;
        }
      }
    }
  }

  // border are restricting potentially too much, fix this
  if (!vehicle_triggered_longitudinal_response &&
      (carla_rss_state.proper_response.longitudinalResponse != ::ad::rss::state::LongitudinalResponse::None)) {
    _logger->debug("!! longitudinalResponse only triggered by borders: ignore !!");
    carla_rss_state.proper_response.longitudinalResponse = ::ad::rss::state::LongitudinalResponse::None;
    carla_rss_state.proper_response.accelerationRestrictions.longitudinalRange.maximum =
        carla_rss_state.default_ego_vehicle_dynamics.alphaLon.accelMax;
  }
  if (!vehicle_triggered_left_response && !left_border_is_dangerous &&
      (carla_rss_state.proper_response.lateralResponseLeft != ::ad::rss::state::LateralResponse::None)) {
    _logger->debug("!! lateralResponseLeft only triggered by right border: ignore !!");
    carla_rss_state.proper_response.lateralResponseLeft = ::ad::rss::state::LateralResponse::None;
    carla_rss_state.proper_response.accelerationRestrictions.lateralLeftRange.maximum =
        carla_rss_state.default_ego_vehicle_dynamics.alphaLat.accelMax;
    carla_rss_state.ego_dynamics_on_route.crossing_border = true;
  }
  if (!vehicle_triggered_right_response && !right_border_is_dangerous &&
      (carla_rss_state.proper_response.lateralResponseRight != ::ad::rss::state::LateralResponse::None)) {
    _logger->debug("!! lateralResponseRight only triggered by left border: ignore !!");
    carla_rss_state.proper_response.lateralResponseRight = ::ad::rss::state::LateralResponse::None;
    carla_rss_state.proper_response.accelerationRestrictions.lateralRightRange.maximum =
        carla_rss_state.default_ego_vehicle_dynamics.alphaLat.accelMax;
    carla_rss_state.ego_dynamics_on_route.crossing_border = true;
  }

  _logger->trace("RouteResponse: {}", carla_rss_state.proper_response);
}

}  // namespace rss
}  // namespace carla
