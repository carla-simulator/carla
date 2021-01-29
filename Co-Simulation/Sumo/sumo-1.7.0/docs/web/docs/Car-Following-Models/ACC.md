---
title: Car-Following-Models/ACC
permalink: /Car-Following-Models/ACC/
---

## Overview

The integrated ACC car-following model is based on the work of Milanés &
Shladover [\[1\]](#references) and Xiao, Wang & van Arem [\[2\]](#references), wherein the developed
control law in the ACC control algorithm is explicitly divided into
three modes: (i) speed (or cruising) control, (ii) gap-closing control
and (iii) gap control. A fourth mode (i.e. collision avoidance mode) has
been introduced within the project [TransAID](https://www.transaid.eu).

## Speed control mode

The speed control mode is designed to maintain the pre-defined by the
driver desired speed and is activated when there are no preceding
vehicles in the range covered by the sensors or preceding vehicles exist
in a spacing larger of 120 m.

## Gap control mode

The gap control mode aims to maintain a constant time gap between the
ACC-equipped vehicle and its predecessor and is activated when the gap
and speed deviations (with respect to the preceding vehicle) are
concurrently smaller than 0.2 m and 0.1 m/s, respectively.

## Gap-closing control mode

The gap-closing controller enables the smooth transition from speed
control mode to gap control mode and is triggered when the spacing to
the preceding vehicle is smaller than 100 m. If the spacing is between
100 m and 120 m, the ACC-equipped vehicle retains the previous control
strategy to provide hysteresis in the control loop and perform a smooth
transfer between the two strategies.

## Collision avoidance control mode

The collision avoidance mode prevents rear-end collisions when safety
critical conditions prevail. This mode is activated when the spacing to
the preceding vehicle is smaller than 100 m and the gap deviation is
negative.

## Notes

- The implemented model can be found in [{{SUMO}}/src/microsim/cfmodels/MSCFModel_ACC.cpp]({{Source}}src/microsim/cfmodels/MSCFModel_ACC.cpp).
- Literature on the developed ACC driving model and its implementation
  can be found
  [here](https://www.transaid.eu/wp-content/uploads/2017/Deliverables/WP3/TransAID_D3.1_Modelling-simulation-and-assessment-of-vehicle-automations.pdf).
- This part of SUMO was developed and extended within the project
  [TransAID](https://www.transaid.eu).
- The model is primarily intended for use in specific traffic
  situations.

## References

1.  Milanés, V., & Shladover, S. E. (2014). Modeling cooperative and
    autonomous adaptive cruise control dynamic responses using
    experimental data. Transportation Research Part C: Emerging
    Technologies, 48, pp. 285–300.
    <https://doi.org/10.1016/j.trc.2014.09.001>
2.  Xiao, L., Wang, M., & van Arem, B. (2017). Realistic Car-Following
    Models for Microscopic Simulation of Adaptive and Cooperative
    Adaptive Cruise Control Vehicles. Transportation Research Record:
    Journal of the Transportation Research Board, 2623, pp. 1–9.
    <https://doi.org/10.3141/2623-01>