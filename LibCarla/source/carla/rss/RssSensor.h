// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <mutex>
#include <vector>
#include "carla/client/Sensor.h"

namespace ad {
namespace rss {
namespace world {

/// forward declaration of the RssDynamics struct
struct RssDynamics;

}  // namespace world
}  // namespace rss
}  // namespace ad

namespace carla {

namespace rss {

/// forward declaration of the RoadBoundariesMode
enum class RoadBoundariesMode;
/// forward declaration of the RssCheck class
class RssCheck;
/// forward declaration of the ActorContellationResult struct
struct ActorConstellationResult;
/// forward declaration of the ActorContellationData struct
struct ActorConstellationData;
}  // namespace rss

namespace client {

/// The RSS Sensor class implementing the carla::client::Sensor interface
/// This class is a proxy to the RssCheck class
class RssSensor : public Sensor {
public:
  using Sensor::Sensor;

  using ActorConstellationCallbackFunctionType =
      std::function<::carla::rss::ActorConstellationResult(carla::SharedPtr<::carla::rss::ActorConstellationData>)>;

  /// @brief constructor
  explicit RssSensor(ActorInitializer init);

  /// @brief destructor
  ~RssSensor();

  /// Register a @a callback to be executed for each actor within each measurement to be processed
  /// to decide on the operation of the RSS sensor in respect to the ego vehicle to actor constellation
  void RegisterActorConstellationCallback(ActorConstellationCallbackFunctionType callback);

  /// Register a @a callback to be executed each time a new measurement is
  /// received.
  ///
  /// @warning Calling this function on a sensor that is already listening
  /// steals the data stream from the previously set callback. Note that
  /// several instances of Sensor (even in different processes) may point to
  /// the same sensor in the simulator.
  void Listen(CallbackFunctionType callback) override;

  /// Stop listening for new measurements.
  void Stop() override;

  /// Return whether this Sensor instance is currently listening to the
  /// associated sensor in the simulator.
  bool IsListening() const override {
    return _on_tick_register_id != 0u;
  }

  /// @brief sets the current log level
  void SetLogLevel(const uint8_t &log_level);

  /// @brief sets the current map log level
  void SetMapLogLevel(const uint8_t &map_log_level);

  /// @returns the currently used dynamics of the ego vehicle (@see also
  /// RssCheck::GetEgoVehicleDynamics())
  const ::ad::rss::world::RssDynamics &GetEgoVehicleDynamics() const;
  /// @brief sets the ego vehicle dynamics to be used by the ego vehicle (@see
  /// also RssCheck::SetEgoVehicleDynamics())
  void SetEgoVehicleDynamics(const ::ad::rss::world::RssDynamics &ego_dynamics);

  /// @returns the currently used dynamics of other vehicles (@see also
  /// RssCheck::GetOtherVehicleDynamics())
  const ::ad::rss::world::RssDynamics &GetOtherVehicleDynamics() const;
  /// @brief sets the ego vehicle dynamics to be used by other vehicles (@see
  /// also RssCheck::SetOtherVehicleDynamics())
  void SetOtherVehicleDynamics(const ::ad::rss::world::RssDynamics &other_vehicle_dynamics);

  /// @returns the currently used dynamics of pedestrians (@see also
  /// RssCheck::GetPedestrianDynamics())
  const ::ad::rss::world::RssDynamics &GetPedestrianDynamics() const;
  /// @brief sets the ego vehicle dynamics to be used by pedestrians (@see
  /// also RssCheck::SetPedestrianDynamics())
  void SetPedestrianDynamics(const ::ad::rss::world::RssDynamics &pedestrian_dynamics);

  /// @returns the current mode for respecting the road boundaries (@see also
  /// RssCheck::GetRoadBoundariesMode())
  const ::carla::rss::RoadBoundariesMode &GetRoadBoundariesMode() const;
  /// @brief sets the current mode for respecting the road boundaries (@see also
  /// RssCheck::SetRoadBoundariesMode())
  void SetRoadBoundariesMode(const ::carla::rss::RoadBoundariesMode &road_boundaries_mode);

  /// @returns the current routing targets (@see also
  /// RssCheck::GetRoutingTargets())
  const std::vector<::carla::geom::Transform> GetRoutingTargets() const;
  /// @brief appends a routing target to the current routing target list (@see
  /// also RssCheck::AppendRoutingTarget())
  void AppendRoutingTarget(const ::carla::geom::Transform &routing_target);
  /// @brief resets the current routing target (@see also
  /// RssCheck::ResetRoutingTargets())
  void ResetRoutingTargets();

  /// @brief drop the current route (@see also RssCheck::DropRoute())
  void DropRoute();

private:
  /// the acutal sensor tick callback function
  SharedPtr<sensor::SensorData> TickRssSensor(const Timestamp &timestamp);

  /// the id got when registering for the on tick event
  std::size_t _on_tick_register_id;

  /// the mutex to protect the actual RSS processing and in case it takes too long to process ever frame
  std::mutex _processing_lock;

  //// the object actually performing the RSS processing
  std::shared_ptr<::carla::rss::RssCheck> _rss_check;

  //// the rss actor constellation callback function
  ActorConstellationCallbackFunctionType _rss_actor_constellation_callback;

  /// reqired to store DropRoute() requests until next sensor tick
  bool _drop_route;

  /// last processed frame
  std::size_t _last_processed_frame;
};

}  // namespace client
}  // namespace carla
