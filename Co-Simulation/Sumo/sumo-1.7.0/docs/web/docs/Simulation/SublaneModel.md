---
title: Simulation/SublaneModel
permalink: /Simulation/SublaneModel/
---

# Introduction

By default, there is at most one vehicle per lane and lane-changes are
performed instantly. To changes this, one of the two models described
below may be activated.

# Sublane-Model

This page describes simulations with increased lateral resolution. This
model is useful to simulate the following:

- multiple 2-wheeled vehicles driving in parallel on a single lane
- vehicles overtaking a bicycle on a single lane
- formation of virtual lanes in dense traffic (i.e. 3 vehicles driving
in parallel on 2 lanes)
- virtual lane formation for emergency traffic
- lateral dynamics
  - lateral acceleration and speed
  - blocking of 2 lanes during overtaking
  - bicycles drive on the rightmost side of a lane
  - lateral encroachment by aggressive drivers
  - dynamic longitudinal gap acceptance during lane changing

This model is activated using the option **--lateral-resolution** {{DT_FLOAT}}. The model is described in
*Simulation framework for testing ADAS in Chinese traffic situations*
[in proceedings of
SUMO2016](http://elib.dlr.de/106342/1/SUMOconference_proceedings_2016.pdf)

!!! caution
    The sublane model is a developing subject. It is not compatible with [opposite direction driving](../Simulation/OppositeDirectionDriving.md) so far.

## Lateral Resolution and Vehicle Position
When the sublane model is activated, the lateral vehicle position (measured as offset from the centerline of the lane) can take on any value between [-laneWidth/2, laneWidth/2] regardless of the lateral resolution value.

The resolution which implicitly divides a lane into one or more sublanes, defines the granularity of decision making and collision detection. Each vehicle may occupy multiple sublanes (according to it's width).
During normal simulation behavior two vehicles will never occupy the same stripe while having an overlap in the longitudinal direction of the lane. Consequently, the value of **--lateral-resoluton** should be low enough to account for the space requirements of the least wide traffic member.
If three bicycles should be able to ride side by side on a 3.6m wide lane, the lateral resolution must not be higher than 1.2m.

!!! note
    Vehicles move **continously** between the sublanes. This means, even when the lateral resolution is set equal to the lane width, vehicles will occupy many intermediate positions while changing between lanes (whenever it takes multiple simulation steps to achieve the desired maneuver).

!!! note
    It is recommended to set the lateral resolution to a value that divides the lane width evenly to avoid artefacts from varyings stripe width (stripes end at the lane border).

!!! caution
    The smaller the value of **--lateral-resolution**, the higher the running time for computing the simulation state.

## New Parameters

The vehicle behavior is subject to model-specific [vType attributes
(maxSpeedLat, minGapLat,
latAlignment)](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_types)
and [lane-changing-modell attributes (lcSublane,
lcPushy)](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#lane-changing_models).

When describing the state of a vehicle in
[sumo-gui](../sumo-gui.md)-dialogs or in the
[netstate-output](../Simulation/Output/RawDump.md), additional
attributes are used:

- lateral position (posLat): offset from the center of the current
  lane in meters
- shadow lane: Each vehicle in the sublane model occupies 1 or 2
  lanes. The lane that contains the center of the
  front-bumper is refered to as its *lane*. If the vehicle
  front-bumper also reaches into another lane, this is called the
  *shadow lane*
- target lane: if the vehicle has started a lane changing manoeuvre to
  another lane, this is the target lane
- lateral speed: the lateral velocity in the current simulation step
- lane change maneuver distance: the absolute lateral distance to be
  covered in the current lane change maneuver in m (overtaking another
  vehicle is considered to consist of two maneuvers: 1. leaving the
  current lane, 2. re-entering the lane after overtaking)
- right side on edge: offset of the right vehicle side from the right
  side of the current road edge in m
- left side on edge: offset of the left vehicle side from the right
  side of the current road edge in m
- right edge sublane: the rightmost sublane that the vehicle occupies
  (at least partially) when counting all sublanes of the current edge
  beginning at 0 on the right side of the edge
- left edge sublane: the leftmost sublane that the vehicle occupies
  (at least partially) when counting all sublanes of the current edge
  beginning at 0 on the right side of the edge

## Model Details

The regular lanes of the road network are divided into sublanes with a
minimum width of the given resolution (**--lateral-resolution**). If the lane width is not a
multiple of the given value, the leftmost sublane has a reduced with.
The default lane-width of SUMO is 3.2m so a lateral resolution of 0.8
will created exactly 4 sublanes of that width per lane. A resolution of
1.0 will create three sublanes of 1.0m width and on more lane of 0.2m
width. It is recommended use a resolution that is at least as small as
the least wide vehicle being simulated (i.e. motorcycles).

### Car-Following

Vehicles occupy one or more sublanes and perform car-following
calculations for all vehicles that are being followed on at least one
sublane.

### Lane-Changing

The lane-changing model *SL2015* is automatically used when enabling the
sublane model. Lane-changing takes place at the sublane level and
potentially uses the whole width of the road according to the width of
the vehicles. Besides changing for route-following, cooperation,
obligation (keepRight) or speed gain, vehicles also perform lateral
movement to achieve a certain lateral alignment. This preference for
either staying in the middle of a lane or to one of its sides is
configured with the vType attribute `latAlignment`.

In addition to these motivation, an additional behavioral layer is
responsible for maintaining safe lateral gaps. The desired gap can be
set using the vType attribute `minGapLat`. Distances keeping is only performed in
regard to vehicles that are not too far behind the ego vehicle. If the
front bumper of a neighbouring vehicle is behind the longitudinal
midpoint of the ego vehicle, that neighbour is ignored.

!!! note
    the lateral extend of neighbouring vehicles is only computed within the set **--lateral-resolution**

The model *SL2015* supports these additional parameters:

- **lcSublane**: The eagerness for using the configured lateral
alignment within the lane. Higher values result in increased
willingness to sacrifice speed for alignment. default: 1.0, range
\[0-inf\]
- **lcPushy**: Willingness to encroach laterally on other drivers.
default: 0, range \[0-1\] If this is set, vehicles will start to
change laterally even though their target sublanes(s) are still
occupied. For urgent (strategic) lane-changes this produces behavior
where the ego vehicle violates the lateral minimum gap of its
neighbor and thereby triggers evasive lateral movement (pushing).
- **lcAssertive**: Willingness to accept lower front and rear gaps on
the target lane. default: 0, range 0 to 1
- **lcImpatience**: dynamic factor for modifying lcAssertive and
lcPushy. default: 0 (no effect) range -1 to 1. Impatience acts as a
multiplier. At -1 the multiplier is 0.5 and at 1 the multiplier is
1.5
- **lcTimeToImpatience**: Time to reach maximum impatience (of 1).
Impatience grows whenever a lane-change manoeuvre is blocked.
- **lcAccelLat**: maximum lateral acceleration per second. Together
with *maxSpeedLat* this constrains lateral movement speed.
- **latAlignment**: prefered lateral alignment within a lane.
  - right: stay on the right side of the lane
  - center: stay in the center of the lane
  - left: stay on the left side of the lane
  - arbitrary: keep the current lateral alignment. (Vehicle will
    only change laterally in response to surrounding vehicles or
    changing lane width
  - compact: stay close to the neighboring vehicle on the right
  - nice: align with the nearest sublane boundary to the right. (to
    avoid using more sublanes than necessary)

### Misc

Vehicles can be located laterally anywhere on the lane(s). Vehicles
change lanes incrementally depending on their *maxSpeedLat*
vType-attribute. The width of sublanes affects the fidelity of car
following in regard to the acceptance of lateral gaps and also
determines the number of candidate movements that are evaluated during
lane-changing.

When [controlling vehicles via-TraCI using the vehicle command
moveToXY](../TraCI/Change_Vehicle_State.md#move_to_xy_0xb4),
vehicles will be placed at the exact longitudinal and lateral position
to match the specified coordinates. This allows for full control of
sublane-placement.

# Simple Continous lane-change model

The sublane model described above allows simulating a wide range of
phenomena related to lateral vehicle dynamics at the price of increased
computational load.

One important aspect of lateral dynamics is the fact that a lane-change
manoeuvre generally takes longer than a single simulation step where is
lane-changing is instantaneous without activating the sublane model.

If only the non-instantaneous aspect of lane-changing needs to be
modelled, a simplified (and thus faster) model may be used as an
alternative to the sublane model.

The *Simple continous lane-change model* is activated by setting the
option **--lanechange.duration** {{DT_FLOAT}} which specifies the default time for changing between adjacent
lanes in seconds (instead of setting option **--lateral-resolution**).

When using this model, vehicles will move with constant lateral speed to
complete a manoeuvre in the specified time. Lane-changing decisions take
the extra time required into account.

## Additional Vehicle Parameters

- when setting the `<vType>`-attribute *maxSpeedLat*, the lateral speed
  computed from the default duration is replaced by the configured
  type-specific speed.
