
#pragma once

#include <vector>

#include "SharedData.hpp"


namespace traffic_manager {

    struct ActuationSignal {
        float throttle;
        float brake;
        float steer;
    };

    class PIDController {
        /*
        This class calculates PID actuation signals to control the vehicle
        such that the vehicle maintains a target velocity and aligns its orientation
        along it's trajectory.
        */

        public:

        PIDController();

        /*
        This method calculates the present state of the vehicle including
        the accumulated integral component of the PID system.
        */
        StateEntry stateUpdate(
            StateEntry previous_state,
            float current_velocity,
            float target_velocity,
            float angular_deviation,
            TimeInstance current_time
        );

        /*
        This method calculates the actuation signals based on the resent state change
        of the vehicle to minimize PID error.
        */
        ActuationSignal runStep(
            StateEntry present_state,
            StateEntry previous_state,
            std::vector<float> longitudinal_parameters,
            std::vector<float> lateral_parameters
        );
    };
}
