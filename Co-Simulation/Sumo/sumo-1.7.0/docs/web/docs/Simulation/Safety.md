---
title: Simulation/Safety
permalink: /Simulation/Safety/
---

# Collisions

[sumo](../sumo.md) tracks gaps between vehicles that are on the
same edge either fully or partially. By default, whenever these gaps are
reduced to below a vehicles *minGap* a collision is registered (default
2.5m). This (potentially) surprising behavior is used to detect issues
with the default car-following model that is supposed to never reduce
the gap below the minGap. By setting the option **--collision.mingap-factor** this threshold can be
reduced. When setting **--collision.mingap-factor 0**, only *physical collisions* (i.e. front and back
bumper meet or overlap) are registered.

When setting the option **--collisions.check-junctions**, collisions between vehicles on the same
intersection are also checked by detecting overlap of the vehicles
shapes.

!!! note
    Junction collisions are never registered on junctions of type *unregulated*.

The consequence of a collision is configured using the option **--collision.action** using one
of the following keywords:

- **teleport**: (the default): the follower vehicle is moved
  (teleported) to the next edge on its route
- **warn**: a warning is issued
- **none**: no action is taken
- **remove**: both vehicles are removed from the simulation

Additionally there is the possibility of stopping vehicles for a fixed
time before the *collision action* takes place. This allows for pile-ups
and traffic disturbance. To enable stopping, the option **--collision.stoptime** {{DT_TIME}} must be set
with the stopping time in seconds.

!!! note
    The pedestrian model *striping* detects collisions between pedestrians. This only serves to detect issues with the model.

## Deliberately causing collisions

To force collisions at a set time, TraCI must be used. Besides setting
the speed, it is also necessary to disable safety checks using [the
commands speedMode and
laneChangeMode](../TraCI/Change_Vehicle_State.md).

### Collisions during car-following
Rear-end collisiosn during normal driving may be caused by any of the following:

- vehicles with a value of *tau* that is lower than the simulation step
size (default 1s) when using the default Krauss model.
- vehicles with a value of *tau* that is lower than their *actionStepLength*
- vehicles with an *apparentDecel* parameter that is lower than their *decel* parameter (causing other drivers to misjudge their deceleration capabilities)
- [driver imperfection modelled with the
  *driverstate*-device](../Driver_State.md)

### Collisions related to lane-changing
Collisions from lane-changing can be caused by unsafe lateral movements (side collisions) and by changing lanes in a way that creates unsafe following situations (rear-end collisions).

Side collosions can be caused by
- configuring lateral imperfection with vType parameter *lcSigma*
- allowing lateral encroachment with vType parameter *lcPushy* (but this parameter itself will not cause collisions, only reduce lateral gaps in some situations, requires the sublane model)
- *lcImpatience* (growing impatience permits lower lateral gaps when using the sublane model)

Unsafe changing can be caused by configuringlower gap acceptance with parameter
- *lcAssertive* (the acceptable gap is computed by dividing the safe gap by lcAssertive)

### Collisions at Intersections
Collisions at intersections may be caused by any of the following:

- Unsafe [junction model
parameters](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#junction_model_parameters)
  - *jmDriveAfterRedTime* \> 0 (ignoring a red light)
  - *jmIgnoreFoeProb* and *jmIgnoreFoeSpeed* (ignore foes below the
    given speed with the given probability)
  - *jmTimegapMinor* < 1 (safety time gap when passing an
    intersection without priority)
- yellow phases which are too short in relation to the vehicle speed
(giving insufficient time to come to a stop). By default this causes
strong braking (*Warning: emergency braking*) potentially followed
by rear-end collisions
- Green phases that allow conflicting streams to drive at the same
time. Collision beyond the intersection due to this are always
detected but collisions on the intersection are only registered when
setting the option **--collision.check-junctions**.
- [Double
connections](../Networks/PlainXML.md#multiple_connections_from_the_same_edge_to_the_same_target_lane)
from a traffic-light-controlled edge: If two lanes from the same
edge with the same target lane get the green light at the same time,
collisions are likely
- If all [Double
connections](../Networks/PlainXML.md#multiple_connections_from_the_same_edge_to_the_same_target_lane)
at an edge are configured with `pass="true"`
- If connections from the minor road of a priority junction are
configured with `pass="true"`
- [Imperfect driver behavior](../Driver_State.md)

To simulate the **effects** of an accident see
[FAQ\#How_to_simulate_an_accident](../FAQ.md#how_to_simulate_an_accident).

# Reaction Times

In Reality, drivers take actions with some delay in regard to the
evolving traffic situation. This reaction time is modelled in the
simulation in various ways. Reaction time should not be confused with
the **tau** parameter as this models the *desired* time headway. Keeping
sufficient headway is necessary for safe driving with delayed reaction.
However, this headway can be set independently of the actual reaction
time.

## Simulation Step Length

All simulation vehicles select their speed simultaneously with regard to
the traffic state from the previous simulation step. This means, there
is at least a reaction time of the simulation step-length.

In contrast, lane-changing within an edge happens sequentially (in the
upstream direction) within each step . Consequently, two vehicles that
are driving almost parallel on a 3-lane road may both change their lanes
in parallel within a single simulation step because the vehicle further
behind already takes into account the lateral movement of the vehicle
ahead.

## Action Step Length

By default, vehicles recompute their speed and lane-changing decisions
every simulation step. When running with sub-second time resolution (i.e.
**--step-length 0.1**), this also gives very low effective reaction times.

To decouple the decision interval from the simulation step length, the
vehicle *action-step-length* can be defined as the duration between
subsequent vehicle decisions. Decision-making starts directly after
insertion which means vehicles inserted at different times may take
decisions during different simulation steps. During simulation steps
without decision-making, vehicle positions are updated according to the
previously computed acceleration. Lateral dynamics (when using the
sublane model) use a dynamic acceleration curve that to complete the
current lateral driving manoeuvre.

The action step length can be defined in any of the following ways:

- setting the option **default.action-step-length** {{DT_FLOAT}} which sets the default action step length for
  all vehicles
- setting the `<vType>` attribute *actionStepLength*
- calling *traci.vehicle.setActionStepLength*

  !!! note
      the action step length must be a multiple of the simulation step length.

# Safety-related parameters

Simulation of unsafe behavior is a developing subject. Expect model
additions and extensions. The following [vehicle type
parameters](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_types)
are safety related:

## car-following model

- **speedFactor**: higher value causes speeding (driving above the
  speed limit)
- **accel**, **decel**: higher values lead to abrupt speed changes
- **tau**: lower values lead to reduced gaps to the leader vehicle.
  When setting tau lower than the simulation **--step-length** parameter (default 1s),
  resp. the vehicle's action step length (**--default.action-step-length** or vehicle parameter
  `actionStepLength`), collisions may occur
- **emergencyDecel**: this is the maximum deceleration a vehicle can
  take to avoid an accident. By default this takes the same value as
  **decel**
- **apparentDecel**: this is the deceleration value that will be
  assumed by other vehicles of the configured vehicle. By default this
  takes the same value as **decel**
- [driver imperfection modelled with the
  *driverstate*-device](../Driver_State.md)

## Lane-Changing Model

- lcCooperative: lower values cause less cooperation during lane
  changing
- lcSpeedGain: higher values cause more overtaking for speed
- lcKeepRight: lower values cause less driving on the right
- lcPushy: setting this to values between 0 and 1 causes aggressive
  lateral encroachment (only when using the [Sublane
  Model](../Simulation/SublaneModel.md))
- lcImpatience: Repeated failure to change lanes causes lower lateral gaps to be accepted when using the sublane model
- lcAssertive: setting this values above 1 cause acceptance of smaller
  longitudinal gaps in proportion to driver impatience(only when using
  the [Sublane Model](../Simulation/SublaneModel.md))
- lcSigma: models random lateral variations (lane keeping imperfection)

## Junction Model

There are further parameters which affect safety-related junction
behavior. For a description see
[Definition_of_Vehicles,_Vehicle_Types,_and_Routes\#Junction_Model_Parameters](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#junction_model_parameters).

- impatience: higher values lead to acceptance of smaller time gaps at
  intersections. These are still safe in the sense of collision
  avoidance.
- jmCrossingGap: lower values lead to more aggressive driving at
  pedestrian intersections (safe)
- jmDriveAfterRedTime: Violate red lights some time after the switch
  (safety decreases with growing values)
- jmDriveRedSpeed: Speed when violating red lights (higher values
  decrease safety)
- jmIgnoreFoeProb: Probability to ignore foes with right-of-way
  (higher values decrease safety)
- jmIgnoreFoeSpeed: Foe speed threshold which prevents fast vehicles
  from being ignored (Higher values decrease safety)
- jmSigmaMinor: When planning to pass a minor link, optimum
  acceleration is assumed. (higher values cause reduced acceleration
  and thus decrease time gaps and safety)
- jmTimegapMinor: Minimum time gap when passing a minor link ahead of
  a prioritzed vehicle (lower values decrease safety)

# Safety-Related Outputs

- [Surrogate Safety Measures (SSM)](../Simulation/Output/SSM_Device.md)
  (headway, brake rates, time to collision etc.)
- The [Instant Induction
  Loop](../Simulation/Output/Instantaneous_Induction_Loops_Detectors.md)
  output records the *gap* attribute which gives the time gap between
  successive vehicles with sub-second precision.
- The [Lanechange output](../Simulation/Output/Lanechange.md)
  generates a record of all lane-changes that take place in the
  simulation. It includes the longitudinal gap to the leader and
  follower vehicle on the target lane (both the actual gap and the one
  that would have been required to ensure stringent deceleration
  bounds under all circumstances). When using the [sublane
  model](../Simulation/SublaneModel.md), the attribute *latGap*
  records the lateral gap to the neighboring vehicle in the direction
  of the change if such a vehicle exists.