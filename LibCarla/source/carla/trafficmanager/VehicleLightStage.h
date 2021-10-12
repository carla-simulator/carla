
#pragma once

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
  const SimulationState &simulation_state;
  const BufferMap &buffer_map;
  const cc::World &world;
  ControlFrame& control_frame;
  /// All vehicle light states
  rpc::VehicleLightStateList all_light_states;
  /// Current weather parameters
  rpc::WeatherParameters weather;

public:
  VehicleLightStage(const SimulationState &simulation_state,
                    const BufferMap &buffer_map,
                    const cc::World &world,
                    ControlFrame& control_frame);

  void ClearCycleCache();

  void Update(const unsigned long index) override;

  void RemoveActor(const ActorId actor_id) override;

  void Reset() override;
};

} // namespace traffic_manager
} // namespace carla