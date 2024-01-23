// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/TrafficSign.h"
#include "carla/rpc/TrafficLightState.h"
#include "carla/client/Waypoint.h"
#include "carla/client/Map.h"
#include "carla/geom/BoundingBox.h"

namespace carla {
namespace client {

  class TrafficLight : public TrafficSign {

  public:

    explicit TrafficLight(ActorInitializer init) : TrafficSign(std::move(init)) {}

    void SetState(rpc::TrafficLightState state);

    /// Return the current state of the traffic light.
    ///
    /// @note This function does not call the simulator, it returns the data
    /// received in the last tick.
    rpc::TrafficLightState GetState() const;

    void SetGreenTime(float green_time);

    /// @note This function does not call the simulator, it returns the data
    /// received in the last tick.
    float GetGreenTime() const;

    void SetYellowTime(float yellow_time);

    /// @note This function does not call the simulator, it returns the data
    /// received in the last tick.
    float GetYellowTime() const;

    void SetRedTime(float red_time);

    /// @note This function does not call the simulator, it returns the data
    /// received in the last tick.
    float GetRedTime() const;

    /// @note This function does not call the simulator, it returns the data
    /// received in the last tick.
    float GetElapsedTime() const;

    void Freeze(bool freeze);

    /// @note This function does not call the simulator, it returns the data
    /// received in the last tick.
    bool IsFrozen() const;

    /// Returns the index of the pole in the traffic light group
    uint32_t GetPoleIndex();

    /// Return all traffic lights in the group this one belongs to.
    ///
    /// @note This function calls the simulator
    std::vector<SharedPtr<TrafficLight>> GetGroupTrafficLights();

    // resets the timers and states of all groups
    void ResetGroup();

    std::vector<SharedPtr<Waypoint>> GetAffectedLaneWaypoints() const;

    std::vector<geom::BoundingBox> GetLightBoxes() const;

    road::SignId GetOpenDRIVEID() const;

    std::vector<SharedPtr<Waypoint>> GetStopWaypoints() const;

  };

} // namespace client
} // namespace carla
