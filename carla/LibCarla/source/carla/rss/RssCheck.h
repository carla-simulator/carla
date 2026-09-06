// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <spdlog/spdlog.h>
#include <ad/map/landmark/LandmarkIdSet.hpp>
#include <ad/map/match/Object.hpp>
#include <ad/map/route/FullRoute.hpp>
#include <ad/rss/core/RssCheck.hpp>
#include <ad/rss/map/RssSceneCreation.hpp>
#include <ad/rss/situation/SituationSnapshot.hpp>
#include <ad/rss/state/ProperResponse.hpp>
#include <ad/rss/state/RssStateSnapshot.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include "carla/client/ActorList.h"
#include "carla/client/Vehicle.h"
#include "carla/road/Map.h"

namespace carla {
namespace rss {

/// @brief struct defining the different supported handling of road boundaries
enum class RoadBoundariesMode {
  Off,  /// No road boundaries considered by RSS check
  On    /// The road boundaries of the current route are considered by RSS check
};

/// @brief struct defining the ego vehicles current dynamics in respect to the
/// current route
///
/// Especially the velocity of the vehicle is split into lateral and
/// longitudinal components
/// according to the current route
///
struct EgoDynamicsOnRoute {
  /// @brief constructor
  EgoDynamicsOnRoute();

  /// @brief the carla timestamp of the last calculation
  carla::client::Timestamp timestamp;
  /// @brief the time since epoch in ms at start of the checkObjects call
  double time_since_epoch_check_start_ms;
  /// @brief the time since epoch in ms at the end of the checkObjects call
  double time_since_epoch_check_end_ms;
  /// @brief the ego speed
  ::ad::physics::Speed ego_speed;
  /// @brief the current minimum stopping distance
  ::ad::physics::Distance min_stopping_distance;
  /// @brief the considered enu position of the ego vehicle
  ::ad::map::point::ENUPoint ego_center;
  /// @brief the considered heading of the ego vehicle
  ::ad::map::point::ENUHeading ego_heading;
  /// @brief the considered heading change of the ego vehicle
  ::ad::physics::AngularVelocity ego_heading_change;
  /// @brief the considered steering angle of the ego vehicle
  ::ad::physics::Angle ego_steering_angle;
  /// @brief check if the ego center is within route
  bool ego_center_within_route;
  /// @brief flag indicating if the current state is already crossing one of the
  /// borders
  /// this is only evaluated if the border checks are active!
  /// It is a hint to oversteer a bit on countersteering
  bool crossing_border;
  /// @brief the considered heading of the route
  ::ad::map::point::ENUHeading route_heading;
  /// @brief the considered nominal center of the current route
  ::ad::map::point::ENUPoint route_nominal_center;
  /// @brief the considered heading diff towards the route
  ::ad::map::point::ENUHeading heading_diff;
  /// @brief the ego speed component lat in respect to a route
  ::ad::physics::Speed route_speed_lat;
  /// @brief the ego speed component lon in respect to a route
  ::ad::physics::Speed route_speed_lon;
  /// @brief the ego acceleration component lat in respect to a route
  ::ad::physics::Acceleration route_accel_lat;
  /// @brief the ego acceleration component lon in respect to a route
  ::ad::physics::Acceleration route_accel_lon;
  /// @brief the ego acceleration component lat in respect to a route
  /// smoothened by an average filter
  ::ad::physics::Acceleration avg_route_accel_lat;
  /// @brief the ego acceleration component lon in respect to a route
  /// smoothened by an average filter
  ::ad::physics::Acceleration avg_route_accel_lon;
};

/// @brief Struct defining the configuration for RSS processing of a given actor
///
/// The RssSensor implementation allows to configure the actors individually
/// for every frame
///
struct ActorConstellationResult {
  /// The calculation mode to be applied with the actor
  ::ad::rss::map::RssMode rss_calculation_mode{::ad::rss::map::RssMode::NotRelevant};

  /// The mode for restricting speed limit
  ::ad::rss::map::RssSceneCreation::RestrictSpeedLimitMode restrict_speed_limit_mode{
      ::ad::rss::map::RssSceneCreation::RestrictSpeedLimitMode::None};

  /// The Rss dynamics to be applied for the ego vehicle
  ::ad::rss::world::RssDynamics ego_vehicle_dynamics;

  /// The Rss object type to be used for the actor
  ::ad::rss::world::ObjectType actor_object_type;

  /// The Rss dynamics to be applied for the actor
  ::ad::rss::world::RssDynamics actor_dynamics;
};

struct ActorConstellationData {
  /// @brief the ego map matched information
  ::ad::map::match::Object ego_match_object;

  /// @brief the ego route
  ::ad::map::route::FullRoute ego_route;

  /// @brief the ego dynamics on the route
  EgoDynamicsOnRoute ego_dynamics_on_route;

  /// @brief the other object's map matched information
  ::ad::map::match::Object other_match_object;

  carla::SharedPtr<carla::client::Actor> other_actor;
};

/// @brief class implementing the actual RSS checks based on CARLA world
/// description
class RssCheck {
public:
  using ActorConstellationCallbackFunctionType =
      std::function<::carla::rss::ActorConstellationResult(carla::SharedPtr<ActorConstellationData>)>;

  /// @brief default constructor with default internal default actor constellation callback
  RssCheck(float max_steering_angle);

  /// @brief constructor with actor constellation callback
  RssCheck(float max_steering_angle, ActorConstellationCallbackFunctionType rss_actor_constellation_callback,
           carla::SharedPtr<carla::client::Actor> const &carla_ego_actor);

  /// @brief destructor
  ~RssCheck();

  /// @brief get the logger of this
  std::shared_ptr<spdlog::logger> GetLogger() {
    return _logger;
  }

  /// @brief main function to trigger the RSS check at a certain point in time
  ///
  /// This function has to be called cyclic with increasing timestamps to ensure
  /// proper RSS evaluation.
  ///
  bool CheckObjects(carla::client::Timestamp const &timestamp, carla::SharedPtr<carla::client::ActorList> const &actors,
                    carla::SharedPtr<carla::client::Actor> const &carla_ego_actor,
                    ::ad::rss::state::ProperResponse &output_response,
                    ::ad::rss::state::RssStateSnapshot &output_rss_state_snapshot,
                    ::ad::rss::situation::SituationSnapshot &output_situation_snapshot,
                    ::ad::rss::world::WorldModel &output_world_model,
                    EgoDynamicsOnRoute &output_rss_ego_dynamics_on_route);

  /// @returns the used vehicle dynamics for ego vehicle
  const ::ad::rss::world::RssDynamics &GetDefaultActorConstellationCallbackEgoVehicleDynamics() const;
  /// @brief sets the vehicle dynamics to be used for the ego vehicle
  void SetDefaultActorConstellationCallbackEgoVehicleDynamics(
      const ::ad::rss::world::RssDynamics &ego_vehicle_dynamics);
  /// @returns the used vehicle dynamics for other vehicles
  const ::ad::rss::world::RssDynamics &GetDefaultActorConstellationCallbackOtherVehicleDynamics() const;
  /// @brief sets the vehicle dynamics to be used for other vehicles
  void SetDefaultActorConstellationCallbackOtherVehicleDynamics(
      const ::ad::rss::world::RssDynamics &other_vehicle_dynamics);
  /// @returns the used vehicle dynamics for pedestrians
  const ::ad::rss::world::RssDynamics &GetDefaultActorConstellationCallbackPedestrianDynamics() const;
  /// @brief sets the dynamics to be used for pedestrians
  void SetDefaultActorConstellationCallbackPedestrianDynamics(const ::ad::rss::world::RssDynamics &pedestrian_dynamics);

  /// @brief sets the current log level
  void SetLogLevel(const spdlog::level::level_enum &log_level);

  /// @brief sets the current log level
  void SetMapLogLevel(const spdlog::level::level_enum &map_log_level);

  /// @returns the current mode for respecting the road boundaries (@see also
  /// RssSensor::GetRoadBoundariesMode())
  const ::carla::rss::RoadBoundariesMode &GetRoadBoundariesMode() const;
  /// @brief sets the current mode for respecting the road boundaries (@see also
  /// RssSensor::SetRoadBoundariesMode())
  void SetRoadBoundariesMode(const ::carla::rss::RoadBoundariesMode &road_boundaries_mode);

  /// @returns the current routing targets (@see also
  /// RssSensor::GetRoutingTargets())
  const std::vector<::carla::geom::Transform> GetRoutingTargets() const;
  /// @brief appends a routing target to the current routing target list (@see
  /// also RssSensor::AppendRoutingTargets())
  void AppendRoutingTarget(const ::carla::geom::Transform &routing_target);
  /// @brief resets the current routing targets (@see also
  /// RssSensor::ResetRoutingTargets())
  void ResetRoutingTargets();

  /// @brief drop the current route
  ///
  /// Afterwards a new route is selected randomly (if multiple routes are
  /// possible).
  ///
  void DropRoute();

  /// @returns the default vehicle dynamics
  static ::ad::rss::world::RssDynamics GetDefaultVehicleDynamics();

  /// @returns the default pedestrian dynamics
  static ::ad::rss::world::RssDynamics GetDefaultPedestrianDynamics();

  /// @returns the default road boundaries mode
  static RoadBoundariesMode GetDefaultRoadBoundariesMode() {
    return RoadBoundariesMode::Off;
  }

private:
  /// @brief standard logger
  std::shared_ptr<spdlog::logger> _logger;
  /// @brief logger for timing log messages
  std::shared_ptr<spdlog::logger> _timing_logger;

  /// @brief maximum steering angle
  float _maximum_steering_angle;

  /// @brief current used vehicle dynamics for ego vehicle by the default actor constellation callback
  ::ad::rss::world::RssDynamics _default_actor_constellation_callback_ego_vehicle_dynamics;
  /// @brief current used vehicle dynamics for other vehicle by the default actor constellation callback
  ::ad::rss::world::RssDynamics _default_actor_constellation_callback_other_vehicle_dynamics;
  /// @brief current used vehicle dynamics for pedestrians by the default actor constellation callback
  ::ad::rss::world::RssDynamics _default_actor_constellation_callback_pedestrian_dynamics;

  /// @brief the current actor constellation callback
  ActorConstellationCallbackFunctionType _actor_constellation_callback;

  /// @brief current used road boundaries mode
  ::carla::rss::RoadBoundariesMode _road_boundaries_mode;
  /// @brief current used routing targets
  std::vector<::ad::map::point::ENUPoint> _routing_targets;
  /// @brief routing targets to be appended next run
  std::vector<::ad::map::point::ENUPoint> _routing_targets_to_append;

  /// @brief struct collecting the rss states required
  struct CarlaRssState {
    /// @brief the actual RSS checker object
    ::ad::rss::core::RssCheck rss_check;

    /// @brief the ego map matched information
    ::ad::map::match::Object ego_match_object;

    /// @brief the ego route
    ::ad::map::route::FullRoute ego_route;

    /// @brief the ego dynamics on the route
    EgoDynamicsOnRoute ego_dynamics_on_route;

    /// @brief current used default vehicle dynamics for ego vehicle
    ::ad::rss::world::RssDynamics default_ego_vehicle_dynamics;

    /// @brief check input: the RSS world model
    ::ad::rss::world::WorldModel world_model;

    /// @brief check result: the situation snapshot
    ::ad::rss::situation::SituationSnapshot situation_snapshot;
    /// @brief check result: the rss state snapshot
    ::ad::rss::state::RssStateSnapshot rss_state_snapshot;
    /// @brief check result: the proper response
    ::ad::rss::state::ProperResponse proper_response;
    /// @brief flag indicating if the current state is overall dangerous
    bool dangerous_state;
    /// @brief flag indicating if the current state is dangerous because of a
    /// vehicle
    bool dangerous_vehicle;
    /// @brief flag indicating if the current state is dangerous because of an
    /// opposite vehicle
    bool dangerous_opposite_state;
  };

  class RssObjectChecker {
  public:
    RssObjectChecker(RssCheck const &rss_check, ::ad::rss::map::RssSceneCreation &scene_creation,
                     carla::client::Vehicle const &carla_ego_vehicle, CarlaRssState const &carla_rss_state,
                     ::ad::map::landmark::LandmarkIdSet const &green_traffic_lights);
    void operator()(const carla::SharedPtr<carla::client::Actor> other_traffic_participant) const;

  private:
    RssCheck const &_rss_check;
    ::ad::rss::map::RssSceneCreation &_scene_creation;
    carla::client::Vehicle const &_carla_ego_vehicle;
    CarlaRssState const &_carla_rss_state;
    ::ad::map::landmark::LandmarkIdSet const &_green_traffic_lights;
  };

  friend class RssObjectChecker;

  /// @brief the current state of the ego vehicle
  CarlaRssState _carla_rss_state;

  /// @brief calculate the map matched object from the actor
  ::ad::map::match::Object GetMatchObject(carla::SharedPtr<carla::client::Actor> const &actor,
                                          ::ad::physics::Distance const &sampling_distance) const;

  /// @brief calculate the speed from the actor
  ::ad::physics::Speed GetSpeed(carla::client::Actor const &actor) const;

  /// @brief calculate the heading change from the actor
  ::ad::physics::AngularVelocity GetHeadingChange(carla::client::Actor const &actor) const;

  /// @brief calculate the steering angle from the actor
  ::ad::physics::Angle GetSteeringAngle(carla::client::Vehicle const &actor) const;

  /// @brief update the desired ego vehicle route
  void UpdateRoute(CarlaRssState &carla_rss_state);

  /// @brief calculate ego vehicle dynamics on the route
  EgoDynamicsOnRoute CalculateEgoDynamicsOnRoute(carla::client::Timestamp const &current_timestamp,
                                                 double const &time_since_epoch_check_start_ms,
                                                 carla::client::Vehicle const &carla_vehicle,
                                                 ::ad::map::match::Object match_object,
                                                 ::ad::map::route::FullRoute const &route,
                                                 ::ad::rss::world::RssDynamics const &default_ego_vehicle_dynamics,
                                                 EgoDynamicsOnRoute const &last_dynamics) const;

  void UpdateDefaultRssDynamics(CarlaRssState &carla_rss_state);

  /// @brief collect the green traffic lights on the current route
  ::ad::map::landmark::LandmarkIdSet GetGreenTrafficLightsOnRoute(
      std::vector<SharedPtr<carla::client::TrafficLight>> const &traffic_lights,
      ::ad::map::route::FullRoute const &route) const;

  /// @brief Create the RSS world model
  void CreateWorldModel(carla::client::Timestamp const &timestamp, carla::client::ActorList const &actors,
                        carla::client::Vehicle const &carla_ego_vehicle, CarlaRssState &carla_rss_state) const;

  /// @brief Perform the actual RSS check
  bool PerformCheck(CarlaRssState &carla_rss_state) const;

  /// @brief Analyse the RSS check results
  void AnalyseCheckResults(CarlaRssState &carla_rss_state) const;
};

}  // namespace rss
}  // namespace carla

namespace std {
/**
 * \brief standard ostream operator
 *
 * \param[in/out] os The output stream to write to
 * \param[in] ego_dynamics_on_route the ego dynamics on route to stream out
 *
 * \returns The stream object.
 *
 */
inline std::ostream &operator<<(std::ostream &out, const ::carla::rss::EgoDynamicsOnRoute &ego_dynamics_on_route) {
  out << "EgoDynamicsOnRoute(timestamp=" << ego_dynamics_on_route.timestamp
      << ", time_since_epoch_check_start_ms=" << ego_dynamics_on_route.time_since_epoch_check_start_ms
      << ", time_since_epoch_check_end_ms=" << ego_dynamics_on_route.time_since_epoch_check_end_ms
      << ", ego_speed=" << ego_dynamics_on_route.ego_speed
      << ", min_stopping_distance=" << ego_dynamics_on_route.min_stopping_distance
      << ", ego_center=" << ego_dynamics_on_route.ego_center << ", ego_heading=" << ego_dynamics_on_route.ego_heading
      << ", ego_heading_change=" << ego_dynamics_on_route.ego_heading_change
      << ", ego_steering_angle=" << ego_dynamics_on_route.ego_steering_angle
      << ", ego_center_within_route=" << ego_dynamics_on_route.ego_center_within_route
      << ", crossing_border=" << ego_dynamics_on_route.crossing_border
      << ", route_heading=" << ego_dynamics_on_route.route_heading
      << ", route_nominal_center=" << ego_dynamics_on_route.route_nominal_center
      << ", heading_diff=" << ego_dynamics_on_route.heading_diff
      << ", route_speed_lat=" << ego_dynamics_on_route.route_speed_lat
      << ", route_speed_lon=" << ego_dynamics_on_route.route_speed_lon
      << ", route_accel_lat=" << ego_dynamics_on_route.route_accel_lat
      << ", route_accel_lon=" << ego_dynamics_on_route.route_accel_lon
      << ", avg_route_accel_lat=" << ego_dynamics_on_route.avg_route_accel_lat
      << ", avg_route_accel_lon=" << ego_dynamics_on_route.avg_route_accel_lon << ")";
  return out;
}

/**
 * \brief standard ostream operator
 *
 * \param[in/out] os The output stream to write to
 * \param[in] actor_constellation_result the actor constellation result to stream out
 *
 * \returns The stream object.
 *
 */
inline std::ostream &operator<<(std::ostream &out,
                                const ::carla::rss::ActorConstellationResult &actor_constellation_result) {
  out << "ActorConstellationResult(rss_calculation_mode=" << actor_constellation_result.rss_calculation_mode
      << ", restrict_speed_limit_mode=" << actor_constellation_result.restrict_speed_limit_mode
      << ", ego_vehicle_dynamics=" << actor_constellation_result.ego_vehicle_dynamics
      << ", actor_object_type=" << actor_constellation_result.actor_object_type
      << ", actor_dynamics=" << actor_constellation_result.actor_dynamics << ")";
  return out;
}

/**
 * \brief standard ostream operator
 *
 * \param[in/out] os The output stream to write to
 * \param[in] actor_constellation_result the actor constellation result to stream out
 *
 * \returns The stream object.
 *
 */
inline std::ostream &operator<<(std::ostream &out,
                                const ::carla::rss::ActorConstellationData &actor_constellation_data) {
  out << "ActorConstellationData(";
  if (actor_constellation_data.other_actor != nullptr) {
    out << "actor_id=" << actor_constellation_data.other_actor->GetId()
        << ", actor_dynamics=" << actor_constellation_data.other_match_object << ", ";
  }
  out << "ego_match_object=" << actor_constellation_data.ego_match_object
      << ", ego_route=" << actor_constellation_data.ego_route
      << ", ego_dynamics_on_route=" << actor_constellation_data.ego_dynamics_on_route << ")";
  return out;
}

}  // namespace std
