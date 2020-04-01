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

/// @brief struct defining the different supported handling of debug
/// visualization
enum class VisualizationMode {
  Off,                               /// Nothing is visualized
  RouteOnly,                         /// Only the route is visualized
  DebugRouteOrientationObjectRoute,  /// Only the route section around the
                                     /// object
                                     /// is visualized
  DebugRouteOrientationBorders,      /// Only the route section transformed into
                                     /// borders around the object is visualized
  DebugRouteOrientationBoth,         /// The route section around the object and the
                                     /// transformed borders are visualized
  VehicleStateOnly,                  /// Only the states of the vehicles is visualized
  VehicleStateAndRoute,              /// The states of the vehicles and the route is
                                     /// visualized
  All                                /// All (except debug) is visualized
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

/// @brief class implementing the actual RSS checks based on CARLA world
/// description
class RssCheck {
public:
  /// @brief constructor
  RssCheck();

  /// @brief destructor
  ~RssCheck();

  /// @brief main function to trigger the RSS check at a certain point in time
  ///
  /// This function has to be called cyclic with increasing timestamps to ensure
  /// proper RSS evaluation.
  ///
  bool CheckObjects(carla::client::Timestamp const &timestamp, carla::SharedPtr<carla::client::ActorList> const &actors,
                    carla::SharedPtr<carla::client::Actor> const &carla_ego_actor,
                    ::ad::rss::state::ProperResponse &output_response,
                    ::ad::rss::world::AccelerationRestriction &output_acceleration_restriction,
                    ::ad::rss::state::RssStateSnapshot &output_rss_state_snapshot,
                    EgoDynamicsOnRoute &output_rss_ego_dynamics_on_route);

  /// @brief function to visualize the RSS check results
  ///
  void VisualizeResults(carla::client::World &world,
                        carla::SharedPtr<carla::client::Actor> const &carla_ego_actor) const;

  /// @returns the used vehicle dynamics for ego vehcile
  const ::ad::rss::world::RssDynamics &GetEgoVehicleDynamics() const;
  /// @brief sets the vehicle dynamics to be used for the ego vehcile
  void SetEgoVehicleDynamics(const ::ad::rss::world::RssDynamics &ego_vehicle_dynamics);
  /// @returns the used vehicle dynamics for other vehciles
  const ::ad::rss::world::RssDynamics &GetOtherVehicleDynamics() const;
  /// @brief sets the vehicle dynamics to be used for other vehciles
  void SetOtherVehicleDynamics(const ::ad::rss::world::RssDynamics &other_vehicle_dynamics);

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

  /// @brief sets the visualization mode (@see also
  /// ::carla::rss::VisualizationMode)
  void SetVisualizationMode(const ::carla::rss::VisualizationMode &visualization_mode);
  /// @returns get the current visualization mode (@see also
  /// ::carla::rss::VisualizationMode)
  const ::carla::rss::VisualizationMode &GetVisualizationMode() const;

  /// @brief drop the current route
  ///
  /// Afterwards a new route is selected randomly (if multiple routes are
  /// possible).
  ///
  void DropRoute();

  /// @returns the default vehicle dynamics
  static ::ad::rss::world::RssDynamics GetDefaultVehicleDynamics();

  /// @returns the default road boundaries mode
  static RoadBoundariesMode GetDefaultRoadBoundariesMode() {
    return RoadBoundariesMode::Off;
  }

  /// @returns the default visualization mode
  static VisualizationMode GetDefaultVisualizationMode() {
    return VisualizationMode::All;
  }

private:
  /// @brief standard logger
  std::shared_ptr<spdlog::logger> _logger;
  /// @brief logger for timing log messages
  std::shared_ptr<spdlog::logger> _timing_logger;
  /// @brief current used vehicle dynamics for ego vehcile
  ::ad::rss::world::RssDynamics _ego_vehicle_dynamics;
  /// @brief current used vehicle dynamics for other vehciles
  ::ad::rss::world::RssDynamics _other_vehicle_dynamics;
  /// @brief current used road boundaries mode
  ::carla::rss::RoadBoundariesMode _road_boundaries_mode;
  /// @brief current used routing targets
  std::vector<::ad::map::point::ENUPoint> _routing_targets;
  /// @brief routing targets to be appended next run
  std::vector<::ad::map::point::ENUPoint> _routing_targets_to_append;
  /// @brief current used routing target
  ::carla::rss::VisualizationMode _visualization_mode;

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

    /// @brief check input: the RSS world model
    ::ad::rss::world::WorldModel world_model;

    /// @brief check result: the situation snapshot
    ::ad::rss::situation::SituationSnapshot situation_snapshot;
    /// @brief check result: the rss state snapshot
    ::ad::rss::state::RssStateSnapshot rss_state_snapshot;
    /// @brief check result: the proper response
    ::ad::rss::state::ProperResponse proper_response;
    /// @brief check result: the acceleration restrictions
    ::ad::rss::world::AccelerationRestriction acceleration_restriction;
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
    void operator()(const carla::SharedPtr<carla::client::Vehicle> vehicle) const;

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

  /// @brief calculate the map matched object from the carla_vehicle
  ::ad::map::match::Object GetMatchObject(carla::client::Vehicle const &carla_vehicle,
                                          ::ad::physics::Distance const &match_distance) const;

  /// @brief calculate the speed from the carla_vehicle
  ::ad::physics::Speed GetSpeed(carla::client::Vehicle const &carla_vehicle) const;

  /// @brief update the desired ego vehicle route
  void UpdateRoute(CarlaRssState &carla_rss_state);

  /// @brief calculate ego vehicle dynamics on the route
  EgoDynamicsOnRoute CalculateEgoDynamicsOnRoute(carla::client::Timestamp const &current_timestamp,
                                                 double const &time_since_epoch_check_start_ms,
                                                 carla::client::Vehicle const &carla_vehicle,
                                                 ::ad::map::match::Object match_object,
                                                 ::ad::map::route::FullRoute const &route,
                                                 EgoDynamicsOnRoute const &last_dynamics) const;

  /// @brief collect the green traffic lights on the current route
  ::ad::map::landmark::LandmarkIdSet GetGreenTrafficLightsOnRoute(
      std::vector<SharedPtr<carla::client::TrafficLight>> const &traffic_lights,
      ::ad::map::route::FullRoute const &route) const;

  /// @brief Create the RSS world model
  void CreateWorldModel(carla::client::Timestamp const &timestamp, carla::client::ActorList const &actors,
                        carla::client::Vehicle const &carla_ego_vehicle, CarlaRssState &carla_rss_state) const;

  /// @brief Perform the actual RSS check
  void PerformCheck(CarlaRssState &carla_rss_state) const;

  /// @brief Analyse the RSS check results
  void AnalyseCheckResults(CarlaRssState &carla_rss_state) const;

  ///
  /// visualization
  ///
  void StoreVisualizationResults(CarlaRssState const &carla_rss_state);

  void StoreDebugVisualization(::ad::map::route::FullRoute const &debug_route,
                               std::vector<::ad::map::lane::ENUBorder> const &enu_border) const;

  /// mutex to protect the visualization content
  mutable std::mutex _visualization_mutex;
  /// the RssStateSnapshot to be visualized
  ::ad::rss::state::RssStateSnapshot _visualization_state_snapshot;
  void VisualizeRssResultsLocked(carla::client::DebugHelper &dh, carla::client::World &world,
                                 carla::SharedPtr<carla::client::Actor> const &carla_ego_actor,
                                 ::ad::rss::state::RssStateSnapshot state_snapshot) const;

  /// the FullRoute to be visualized
  std::pair<::ad::map::route::FullRoute, bool> _visualization_route;
  void VisualizeRouteLocked(carla::client::DebugHelper &dh, ::ad::map::route::FullRoute const &route,
                            carla::SharedPtr<carla::client::Actor> const &carla_ego_actor, bool dangerous) const;

  void VisualizeENUEdgeLocked(carla::client::DebugHelper &dh, ::ad::map::point::ENUEdge const &edge,
                              carla::sensor::data::Color const &color, float const z_offset) const;

  /// the EgoDynamicsOnRoute to be visualized
  EgoDynamicsOnRoute _visualization_ego_dynamics;
  void VisualizeEgoDynamics(carla::client::DebugHelper &dh,
                            carla::SharedPtr<carla::client::Actor> const &carla_ego_actor,
                            EgoDynamicsOnRoute const &ego_dynamics_on_route) const;

  mutable ::ad::map::route::FullRoute _visualization_debug_route;
  mutable std::vector<::ad::map::lane::ENUBorder> _visualization_debug_enu_border;
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
      << ", avg_route_accel_lon=" << ego_dynamics_on_route.avg_route_accel_lon << ')';
  return out;
}
}  // namespace std
