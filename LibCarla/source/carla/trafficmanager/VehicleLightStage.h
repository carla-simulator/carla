
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
  bool light_states_refreshed {false};
  /// Vehicles the server left out of the list it last returned.
  std::unordered_set<ActorId> missing_from_last_refresh;

  void SetCachedLightState(const ActorId actor_id,
                           const rpc::VehicleLightState::flag_type light_state);

public:
  VehicleLightStage(const std::vector<ActorId> &vehicle_id_list,
                    const BufferMap &buffer_map,
                    const Parameters &parameters,
                    const cc::World &world,
                    ControlFrame& control_frame);

  /// Synchronous mode refreshes every step.
  void UpdateWorldInfo(const double current_time, const bool synchronous_mode);

  void Update(const unsigned long index) override;

  void RemoveActor(const ActorId actor_id) override;

  void Reset() override;
};

} // namespace traffic_manager
} // namespace carla
