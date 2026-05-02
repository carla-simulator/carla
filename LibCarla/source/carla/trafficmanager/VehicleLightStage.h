
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
  bool is_weather_enabled;

public:
  VehicleLightStage(const std::vector<ActorId> &vehicle_id_list,
                    const BufferMap &buffer_map,
                    const Parameters &parameters,
                    const cc::World &world,
                    ControlFrame& control_frame);

  void UpdateWorldInfo();

  void Update(const unsigned long index) override;

  void RemoveActor(const ActorId actor_id) override;

  void Reset() override;
};

} // namespace traffic_manager
} // namespace carla
