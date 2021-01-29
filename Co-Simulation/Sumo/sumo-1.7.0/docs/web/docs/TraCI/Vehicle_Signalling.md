---
title: TraCI/Vehicle Signalling
permalink: /TraCI/Vehicle_Signalling/
---

# Signaling

A vehicle's signals are encoded in an integer, each by one bit, encoding
whether the according signal/... is on or off. Most signals are rather
meant to be used by an external application. However, a few signals are
computed in each step.

# Computed Signals

The following signals are computed in each time step. The value set via
TraCI is only retained for the current time step.

- **VEH_SIGNAL_BRAKELIGHT**: The brake light is set whenever the
vehicle is standing (not stopped) or when the vehicle decelerates
beyond a threshold value. The signal value is used by the vehicles
when [trying to avoid jamming on an intersection](../Simulation/Intersections.md#junction_blocking).
- **VEH_SIGNAL_BLINKER_RIGHT** and **VEH_SIGNAL_BLINKER_LEFT**:
The blinker is activated in either of the following conditions
  - The vehicle is about to take a turn at an intersection
    (activated 7 seconds before reaching the intersection)
  - Either of the [continuous lane-change models](../Simulation/SublaneModel.md) is active
  - A desired lane change cannot be executed due to neighboring
    vehicles.
- **VEH_SIGNAL_EMERGENCY_BLUE**: when switched on, a blinking blue
light is shown in the GUI for vehicles with vClass="emergency" and
shapeClass="emergency"

# Defined Signals

The following table shows the defined signals.

| Name                            | Bit |
| ------------------------------- | --- |
| VEH_SIGNAL_BLINKER_RIGHT     | 0   |
| VEH_SIGNAL_BLINKER_LEFT      | 1   |
| VEH_SIGNAL_BLINKER_EMERGENCY | 2   |
| VEH_SIGNAL_BRAKELIGHT         | 3   |
| VEH_SIGNAL_FRONTLIGHT         | 4   |
| VEH_SIGNAL_FOGLIGHT           | 5   |
| VEH_SIGNAL_HIGHBEAM           | 6   |
| VEH_SIGNAL_BACKDRIVE          | 7   |
| VEH_SIGNAL_WIPER              | 8   |
| VEH_SIGNAL_DOOR_OPEN_LEFT   | 9   |
| VEH_SIGNAL_DOOR_OPEN_RIGHT  | 10  |
| VEH_SIGNAL_EMERGENCY_BLUE    | 11  |
| VEH_SIGNAL_EMERGENCY_RED     | 12  |
| VEH_SIGNAL_EMERGENCY_YELLOW  | 13  |

!!! note
    Sending a negative signal value immediately resets all signals to their computed values.

# See Also

- [Vehicle Value Retrieval](../TraCI/Vehicle_Value_Retrieval.md)
- [Change Vehicle State](../TraCI/Change_Vehicle_State.md)
- [Accessing signal state without TraCI using **--fcd-output**](../Simulation/Output/FCDOutput.md#further_options)