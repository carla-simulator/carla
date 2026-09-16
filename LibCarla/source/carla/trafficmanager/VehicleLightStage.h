
#pragma once

#include <limits>
#include <unordered_set>

#include "carla/trafficmanager/DataStructures.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/RandomGenerator.h"
#include "carla/trafficmanager/SimulationState.h"
#include "carla/trafficmanager/Stage.h"

namespace carla {
namespace traffic_manager {

/// This class has functionality for turning on/off the vehicle lights
/// according to the current vehicle state and its surrounding environment.
class VehicleLightStage: Stage {
private:
  const std::vector<ActorId> &vehicle_id_list;
  const BufferMap &buffer_map;
  const Parameters &parameters;
  const cc::World &world;
  ControlFrame& control_frame;
  /// All vehicle light states
  rpc::VehicleLightStateList all_light_states;
  /// Current weather parameters
  rpc::WeatherParameters weather;
  /// Weather enabled
  bool is_weather_enabled {false};
  /// Simulation time of the last refresh of each cached world query.
  double last_light_states_update {-std::numeric_limits<double>::infinity()};
  double last_weather_update {-std::numeric_limits<double>::infinity()};
  /// Whether all_light_states was read from the server on the current step.
  bool light_states_refreshed {false};
  /// Vehicles the server itself left out of the list it last returned. Dropped
  /// whenever the list is read again, so it only holds vehicles that were
  /// checked against a list this stage did not have to guess at.
  std::unordered_set<ActorId> missing_from_last_refresh;

  /// Keeps the commands issued by this stage visible in the cached list until
  /// it is read from the server again.
  void SetCachedLightState(const ActorId actor_id,
                           const rpc::VehicleLightState::flag_type light_state);

public:
  VehicleLightStage(const std::vector<ActorId> &vehicle_id_list,
                    const BufferMap &buffer_map,
                    const Parameters &parameters,
                    const cc::World &world,
                    ControlFrame& control_frame);

  /// @a current_time is the elapsed simulation time of the frame being
  /// processed, which paces the refresh of each cached query. Synchronous mode
  /// refreshes every step, as it did before the caching was introduced.
  void UpdateWorldInfo(const double current_time, const bool synchronous_mode);

  void Update(const unsigned long index) override;

  void RemoveActor(const ActorId actor_id) override;

  void Reset() override;
};

} // namespace traffic_manager
} // namespace carla
