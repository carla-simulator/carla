# Scenic for CARLA — the constructs that work

Detail layer for `create-scenic-scenario`. Everything here is confirmed against
the scenarios that run on this build; anything absent from the CARLA world model
is called out.

## File skeleton

```
"""Docstring — what the scenario is."""

param map = localPath('.../Town05.xodr')   # builds the road network
param carla_map = 'Town05'                 # what CARLA loads
param timestep = 0.1                       # simulation step, seconds
model srunner.scenic.models.model          # or scenic.simulators.carla.model

<constants>
<behavior definitions>
<geometry selection>
<object placement>
require ...
terminate when ...
```

`param` lines must precede `model`, because the model reads them as it loads.

`map` and `carla_map` must name the same *road network*, but not necessarily the
same string: on this build towns 1-9 ship as `TownXX_Opt` while their OpenDrive
assets are named without the suffix, so `carla_map = 'Town05_Opt'` pairs with
`map = .../Town05.xodr`. Get them out of sync and the scene is sampled on one
network and simulated on another, with no error.

## Object classes

From `scenic.domains.driving.model` plus the CARLA model's conveniences:

| Class | Notes |
|---|---|
| `Car`, `Truck`, `Bus`, `Van` | vehicles; sensible default blueprints per class |
| `Pedestrian` | needs `regionContainedIn None` off the sidewalk |
| `Bicycle`, `Motorcycle` | **category may be empty on this client version** — name a blueprint |
| `Debris`, `Cone`, `TrashCan`, `VendingMachine`, `Container` | props; also need containment waived |

Override the blueprint on any of them: `with blueprint "vehicle.dodge.charger"`.
A class picks from Scenic's category table; an explicit blueprint bypasses it,
which is the way to use an id the table omits.

## Geometry selection

`network` is the loaded road network. What scenarios select on:

```python
lane = Uniform(*network.lanes)                     # any lane
intersections = network.intersections              # .is3Way .is4Way .isSignalized
lane.centerline                                    # a region to place on
lane.sections                                      # only sections know neighbours
sec._laneToLeft / sec._laneToRight                 # adjacent lane, or None
intersec.incomingLanes                             # arms
lane.maneuvers                                     # .type, .startLane, .connectingLane, .endLane
maneuver.conflictingManeuvers                      # crossing traffic
ManeuverType.STRAIGHT / LEFT_TURN / RIGHT_TURN
```

**Adjacent lanes live on sections, not lanes.** To find a lane with a right
neighbour you must loop over `lane.sections` and test `sec._laneToRight`; there is
no `lane.laneToRight`.

**`filter()` over a network attribute is deterministic** — safe to `assert` on.
**Anything derived from `Uniform()` is random** — comparing it raises
`RandomControlFlowError`. This is the single most common authoring error.

## Placement operators

```python
new OrientedPoint on lane.centerline               # random point, road heading
new OrientedPoint in maneuver.startLane.centerline
new Car at pt
new Car following roadDirection from pt for Range(10, 30)     # ahead
new Car following roadDirection from pt for -Range(10, 30)    # behind
new Pedestrian right of pt by 3
new Car left of pt by 3
with heading 90 deg relative to pt.heading
```

`Range(a, b)` is a uniform continuous distribution; `Uniform(*xs)` is discrete
over a list. Both are resampled on every rejection.

## Behaviors

From the driving domain, usable as-is:

```python
do FollowLaneBehavior(target_speed=10)
do FollowTrajectoryBehavior(target_speed=10, trajectory=[startLane, connectingLane, endLane])
do CrossingBehavior(ego, min_speed, threshold)     # pedestrians
take SetBrakeAction(1.0)
take SetThrottleAction(0.6)
```

Composition:

```python
behavior EgoBehavior(speed):
    try:
        do FollowLaneBehavior(speed)
    interrupt when withinDistanceToAnyObjs(self, 15):
        take SetBrakeAction(1.0)
```

`interrupt when` is how reactive behaviour is expressed — a plain `if` on a
simulation value will not work, because behaviours are coroutines evaluated per
tick. Useful guards: `withinDistanceToAnyObjs`, `withinDistanceToObjsInLane`.

`FollowLaneBehavior` applies to vehicles. A pedestrian given it will not move.

## Requirements and termination

```python
require (distance to intersection) > 50            # hard constraint, resampled
require 15 <= (distance to intersec) <= 25
require (distance from adversary to intersec) > 10
terminate when (distance to ego_spawn) > 70
terminate after 30 seconds
```

Every `require` multiplies rejection cost. Two tight distance requirements on a
small map is the usual cause of `failed to generate scenario in N iterations` —
widen one before suspecting anything else.

Termination bounds the *scenario*; `--time` bounds the *run*. Pass both: a
`terminate when` that never fires leaves the simulation running forever.

## What is not available

- **3D geometry.** These scenarios are written for `--2d`. Without it Scenic wants
  real meshes and the CARLA model does not define them.
- **Scenario composition across files.** `--scenario NAME` selects a named
  `scenario` block within one file, not across files.
- **Traffic-light state control** beyond what the CARLA model exposes; the
  driving domain models signalization as an intersection property, not a
  controllable actor.
