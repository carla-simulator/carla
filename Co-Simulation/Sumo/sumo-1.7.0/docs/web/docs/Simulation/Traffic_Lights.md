---
title: Simulation/Traffic Lights
permalink: /Simulation/Traffic_Lights/
---

Normally, [netconvert](../netconvert.md) and
[netgenerate](../netgenerate.md) generate traffic lights and
programs for junctions during the computation of the networks. Still,
these computed programs quite often differ from those found in reality.
To feed the simulation with real traffic light programs, it is possible
to run [sumo](../sumo.md)/[sumo-gui](../sumo-gui.md) with
additional program definitions. Also,
[sumo](../sumo.md)/[sumo-gui](../sumo-gui.md) allow loading
definitions which describe when and how a set of traffic lights can
switch from one program to another. Both will be discussed in the
following subchapters. Another possiblity is to edit traffic light plans
visually in [netedit](../netedit.md#traffic_lights).

# Automatically Generated TLS-Programs

- All traffic lights are generated with a fixed cycle and a default
  cycle time of 90s. This can be changed with the option **--tls.cycle.time**.
- The green time is split equally between the main phases
- All green phases are followed by a yellow phase. The length of the
  yellow phase is computed from the maximum speed of the incoming
  roads but may be customized with the option **--tls.yellow.time**
- If the speeds at the intersection are below the threshold of 70km/h
  (configurable via option **tls.minor-left.max-speed**), left-turns are allowed at the same time
  as oncoming straight traffic but have to yield. This is called
  *green minor* and indicated with a lower-case **g** in the state
  definition. Otherwise, the left-turning stream must use a protected
  left-turn phase (see below). If no such phase can be built because
  there is no dedicated turning lane, *green minor* is allowed anyway
  but a warning is issued.
- If a green phase allows for partially conflicting streams (i.e.
  straight going and left-turning from the opposite direction) and
  there is a dedicated turning lane, it is succeeded by another green
  phase with full priority to the partially conflicted streams (this
  would typically be a left-turning phase). The duration of this phase
  defaults to 6s and can be customized (or disabled) by setting the
  option **--tls.left-green.time**.
- The generated cycle starts at time 0 by default with a green phase for the first main direction 
  (sorted by road priority, lane count and speed). This can be influenced for a specified list of traffic light ids using the options **--tls.half-offset TLS1,TLS2,..** and **--tls.quarter-offset TLS3,TLS4,...**. (shifting the start of the first phase by the indicated fraction of the cycle time).
- In reality there are often phases where all streams have red to
  allow clearing an intersection. SUMO does not build these phases by
  default. To have each green phase preceded by an all-red phase, the
  option **--tls.allred.time** can be used.
- It is also possible to generate
  [\#Actuated_Traffic_Lights](#actuated_traffic_lights)
  by setting the option **--tls.default-type actuated**. This will generated the same signal plans as
  above but with green phases that have a variable length of 5s-50s
  (both values can be set using the options **--tls.min-dur, --tls.max-dur**).
- The generated phase layout can be selected setting option **--tls.layout** to
  *opposites* (default) or *incoming*.
- The generated phase layout is also influenced by the [node
  type](../Networks/PlainXML.md#node_types) which may be either
  *traffic_light* or *traffic_light_right_on_red* (explained
  below)

## Default 4-arm intersection (layout *opposites*)

By default, programs are generated with 4 green phases:

1.  a straight phase
2.  a left-turning phase (only if there is a dedicated left-turn lane)
3.  a straight phase for the direction orthogonal to the first one
4.  a left-turning phase for the direction direction orthogonal to the
    first one (only if there is a dedicated left-turn lane)

Due to the default timings explained above, the green phases usually
have a duration of 31s.

If the node type is set to *traffic_light_right_on_red* rather than
*traffic_light*, then right turns from all sides are permitted in all
phases (after coming to a stop before entering the traffic stream that
currently has the green light.

## Intersection layout *incoming*

- each incoming edge of the intersection gets a separate green phase
where all directions of movement are allowed
- If the node type is set to *traffic_light_right_on_red* rather
than *traffic_light*, then compatible right-turns are allowed
simultaneously

## Other Intersections

- If there are more than 4 roads meeting at an intersection,
additional green phases are generated
- If the roads meeting at an intersection have the highest road
priorities on a turning stream, then layout *incoming* may be
generated instead of layout *opposites*.
- If a traffic-light junction does not have any conflicting roads
(i.e. it models a pedestrian crossing) it will not get a red phase
by default. A single red phase may be generated by setting the
option **--tls.red.time**.

## Improving Generated programs with knowledge about traffic demand

- To get traffic lights that adapt to demand dynamically, built the
network with option **--tls.default-type actuated**. This will automatically generate [actuated
traffic lights](#actuated_traffic_lights). To convert an
existing .net.xml file so that all traffic lights are actuated,
perform the following steps:
  - netconvert -s orig.net.xml --plain-output-prefix plain
  - netconvert -e plain.edg.xml -n plain.nod.xml -x plain.con.xml -o
    updated.net.xml --ignore-errors.edge-type --tls.default-type
    actuated
- The tool
[tlsCycleAdaptation.py](../Tools/tls.md#tlscycleadaptationpy)
can be used to modify green phase durations to accommodate a given
traffic demand.
- The tool [tlsCoordinator.py](../Tools/tls.md#tlscoordinatorpy)
can be used to modify program offsets to generated green waves for a
given traffic demand.

# Defining New TLS-Programs

You can load new definitions for traffic lights as a part of an {{AdditionalFile}}. When
loaded, the last program will be used. Switching between programs is
possible via WAUTs and/or TraCI. Also, one can switch between them using
the GUI context menu. A definition of a traffic light program within an {{AdditionalFile}}
looks like this:

```
<additional>
  <tlLogic id="0" programID="my_program" offset="0" type="static">
    <phase duration="31" state="GGggrrrrGGggrrrr"/>
    <phase duration="5"  state="yyggrrrryyggrrrr"/>
    <phase duration="6"  state="rrGGrrrrrrGGrrrr"/>
    <phase duration="5"  state="rryyrrrrrryyrrrr"/>
    <phase duration="31" state="rrrrGGggrrrrGGgg"/>
    <phase duration="5"  state="rrrryyggrrrryygg"/>
    <phase duration="6"  state="rrrrrrGGrrrrrrGG"/>
    <phase duration="5"  state="rrrrrryyrrrrrryy"/>
  </tlLogic>
</additional>
```

!!! note
    To get started you can copy the **tlLogic** elements for all traffic lights you wish to edit from a .net.xml file and put them into a new file. Then you only need to change the programID attribute and the program is ready to be modified and loaded

## <tlLogic\> Attributes

The following attributes/elements are used within the tlLogic element:

| Attribute Name | Value Type                            | Description      |
| -------------- | ------------------------------------- | ---------------- |
| **id**         | id (string)                           | The id of the traffic light. This must be an existing traffic light id in the .net.xml file. Typically the id for a traffic light is identical with the junction id. The name may be obtained by right-clicking the red/green bars in front of a controlled intersection. |
| **type**       | enum (static, actuated, delay_based) | The type of the traffic light (fixed phase durations, phase prolongation based on time gaps between vehicles (actuated), or on accumulated time loss of queued vehicles (delay_based) )                                                                                  |
| **programID**  | id (string)                           | The id of the traffic light program; This must be a new program name for the traffic light id. Please note that "off" is reserved, see below.                                                                                                                             |
| **offset**     | int                                   | The initial time offset of the program |

## <phase\> Attributes

Each phase is defined using the following attributes:

| Attribute Name | Value Type            | Description                |
| -------------- | --------------------- | -------------------------- |
| **duration**   | time (int)            | The duration of the phase                                                                                                                                    |
| **state**      | list of signal states | The traffic light states for this phase, see below                                                                                                           |
| minDur         | time (int)            | The minimum duration of the phase when using type **actuated**. Optional, defaults to duration.                                                              |
| maxDur         | time (int)            | The maximum duration of the phase when using type **actuated**. Optional, defaults to duration.                                                              |
| name           | string                | An optional description for the phase. This can be used to establish the correspondence between SUMO-phase-indexing and traffic engineering phase names.     |
| next           | list of phase indices (int ...)           | The next phase in the cycle after the current. This is useful when adding extra transition phases to a traffic light plan which are not part of every cycle. Traffic lights of type 'actuated' can make use of a list of indices for selecting among alternative successor phases. |

!!! caution
    In a SUMO-TLS definition, time is on the vertical axis and each phase describes all signal states that last for a fixed duration. This differs from typical traffic engineering diagrams where time is on the horizontal axis and each row describes the states for one signal. Another crucial difference is, that in SUMO a new phase is introduced whenever at least one signal changes its state. This means that transitions between green phases can be made up of multiple intermediate phases.

## Signal state definitions

Each character within a phase's state describes the state of one signal
of the traffic light. Please note, that a single lane may contain
several signals - for example one for vehicles turning left and one for
vehicles which move straight. This means that a signal does not control
lanes, but links - each connecting a lane which is incoming into a
junction and one which is outgoing from this junction. In SUMO, a
one-to-n dependency between signals and links is implemented, this means
each signal may control more than a single link - though networks
generated by [netconvert](../netconvert.md) or
[netgenerate](../netgenerate.md) usually use one signal per link.
Please note also, that a traffic light may control lanes incoming into
different junctions. The information about which link is controlled by
which traffic light signal may be obtained using the "show link tls
index" option within [sumo-gui](../sumo-gui.md)'s visualisation
settings or from the according `linkIndex`
attribute of the `<connection>` elements in the *.net.xml* file.

The following signal colors are used:

| Character | GUI Color                                                  | Description          |
| --------- | ---------------------------------------------------------- | -------------------- |
| r         | <span style="color:#FF0000; background:#FF0000">FOO</span> | 'red light' for a signal - vehicles must stop     |
| y         | <span style="color:#FFFF00; background:#FFFF00">FOO</span> | 'amber (yellow) light' for a signal - vehicles will start to decelerate if far away from the junction, otherwise they pass    |
| g         | <span style="color:#00B300; background:#00B300">FOO</span> | 'green light' for a signal, no priority - vehicles may pass the junction if no vehicle uses a higher priorised foe stream, otherwise they decelerate for letting it pass. They always decelerate on approach until they are within the configured [visibility distance](../Networks/PlainXML.md#explicitly_setting_which_edge_lane_is_connected_to_which) |
| G         | <span style="color:#00FF00; background:#00FF00">FOO</span> | 'green light' for a signal, priority - vehicles may pass the junction                                                                                                                                                                                                                                                                                                                                 |
| s         | <span style="color:#800080; background:#800080">FOO</span> | 'green right-turn arrow' requires stopping - vehicles may pass the junction if no vehicle uses a higher priorised foe stream. They always stop before passing. This is only generated for junction type *traffic_light_right_on_red*.                                                                                                                                                             |
| u         | <span style="color:#FF8000; background:#FF8000">FOO</span> | 'red+yellow light' for a signal, may be used to indicate upcoming green phase but vehicles may not drive yet (shown as orange in the gui)                                                                                                                                                                                                                                                             |
| o         | <span style="color:#804000; background:#804000">FOO</span> | 'off - blinking' signal is switched off, blinking light indicates vehicles have to yield                                                                                                                                                                                                                                                                                                              |
| O         | <span style="color:#00FFFF; background:#00FFFF">FOO</span> | 'off - no signal' signal is switched off, vehicles have the right of way|


![traci_tutorial_tls.png](../images/Traci_tutorial_tls.png
"traci_tutorial_tls.png")

Example: traffic light with the current state
**"GrGr"**. The leftmost letter "G" encodes the green light for link 0,
followed by red for link 1, green for link 2 and red for link 3. The
link numbers are enabled via [sumo-gui settings](../sumo-gui.md) by
activating *show link tls index*.

### Default link indices

For traffic lights that control a single intersection, the default
[indices generated generated by
netconvert](../Networks/SUMO_Road_Networks.md#indices_of_a_connection)
are numbered in a clockwise pattern starting with 0 at 12 o'clock with
right-turns ordered before straight connections and left turns.
[Pedestrian
crossings](../Networks/PlainXML.md#pedestrian_crossings) are always
assigned at the very end, also in a clockwise manner.

If traffic lights are joined so that a single program controls multiple
intersections, the ordering for each intersection stays the same but the
indices are increased according the order of the controlled junctions in
the input file.

### Accessing indices programmatically

TLS Link indices can be access using either

- [sumolib](../Tools/Sumolib.md) using
  [tls.getConnections()](http://sumo.dlr.de/daily/pydoc/sumolib.net.html#TLS)
- or [TraCI](../TraCI.md) using
  [traci.trafficlight.getControlledLinks()](../TraCI/Traffic_Lights_Value_Retrieval.md#structure_of_compound_object_controlled_links)

## Viewing TLS-Programs graphically

In the sumo-gui you can click on a red/green stop bar and select show
phases.

## Actuated Traffic Lights

### Based on Time Gaps

SUMO supports *gap-based* actuated traffic control This control scheme
is common in Germany and works by prolonging traffic phases whenever a
continuous stream of traffic is detected. It switches to the next phase
after detecting a sufficient time gap between successive vehicles. This
allows for better distribution of green-time among phases and also
affects cycle duration in response to dynamic traffic conditions.

To use this type of control, the `<tlLogic>`-element needs to receive the attribute
`type="actuated"`. It also requires the usage of the `phase`-attributes `minDur` and `maxDur` instead of `duration` to
define the allowed range of time durations for each phase (if these
values are equal or only `duration` is given, that phase will have constant
duration). Additional parameters may be used to configure the control
algorithm further. These may be given within the `<tlLogic>`-Element as follows:

#### Detectors
The time gaps which determine the phase extensions are collected by induction loop detectors.
These detectors are placed automatically at a configurable distance (see below). If the incoming lanes are too short and there is a sequence of unique predecessor lanes, the detector will be placed on a predecessor lane at the computed distance instead.

Each lane incoming to the traffic light will receive a detector. However, not all detectors can be used in all phases.
In the current implementation, detectors for actuation are only used if all connections from the detector lane gets the unconditional green light ('G') in a particular phase.
This is done to prevent useless phase extensions when the first vehicle on a given lane is not allowed to drive.
Sumo will issue a warning a phase or link index does not have usable detectors.


#### Example

```
<tlLogic id="0" programID="my_program" offset="0" type="actuated">
  <param key="max-gap" value="3.0"/>
  <param key="detector-gap" value="2.0"/>
  <param key="passing-time" value="2.0"/>
  <param key="vTypes" value=""/>
  <param key="show-detectors" value="false"/>
  <param key="file" value="NULL"/>
  <param key="freq" value="300"/>

  <phase duration="31" minDur="5" maxDur="45" state="GGggrrrrGGggrrrr"/>
  ...
</tlLogic>
```
#### Parameters
Several optional parameters can be used to control the behavior of actuated traffic lights. The examples values in the previous section are the default values for these parameters and their meaning is given below:

- **max-gap**: the maximum time gap between successive vehicle that will cause the current phase to be prolonged
(within maxDur limit)
- **detector-gap**: determines the time distance between the (automatically generated) detector and the stop line in seconds (at
each lanes maximum speed). 
- **passing-time**: estimates the headway between vehicles when passing the stop line. This sets an uppper bound on the distance between detector and stop line according to the formula `(minDur / passingTime + 0.5) / 7.5`. The intent of this bound is to allow all vehicles between the detector and the stop line to pass the intersection within the minDur time. A warning will be issued if the minDur gives insufficient clearing time.
- **linkMaxDur:X** (where X is a traffic light index): This sets an additional maximum duration criterion based on individual signals rather than phase duration.
- **show-detectors** controls whether generated detectors will be visible or hidden in [sumo-gui](../sumo-gui.md). The default for all traffic lights can be set with option **--tls.actuated.show-detectors**. It is also possible to toggle this value from within the GUI by right-clicking on a traffic light.
- parameters **vTypes**, **file** and **freq** have the same meaning as for [regular
induction loop detectors](../Simulation/Output/Induction_Loops_Detectors_(E1).md).

Some parameters are only used when a signal plan with [dynamic phase selection](#dynamic_phase_selection_phase_skipping) is active:

- **inactive-threshold** (default 180): The parameter sets the time in s after which an inactive phase will be entered preferentially.
- **linkMinDur:X** (where X is a traffic light index): This sets an additional minimum duration criterion based on individual signals rather than phase duration

#### Custom Detectors
To use custom detectors (i.e. for custom placement or output) additional parameters can be defined where KEY is a lane that is incoming to the traffic light and VALUE is a user-defined inductionLoop (that could also lie on another upstream lane).
```
   <param key="gneE42_2" value="customDetector1"/>
```

!!! caution
    Custom detectors only work when the 'tlLogic' is loaded from an additional file.

#### Dynamic Phase Selection (Phase Skipping)
When a phase uses attribute 'next' with a list of indices. The next phase is chosen dynamically based on the detector status of all candidate phases according to the following algorithm:

- compute the priority for each phase given in 'next'. Priority is primarily given by the number of active detectors for that phase. Active means either of:
  - with detection gap below threshold
  - with a detection since the last time where the signal after this detector was green
- the current phase is implicitly available for continuation as long as it's maxDur is not reached. Detectors of the current phase get a bonus priority
- the phase with the highest priority is used with phases coming earlier in the next list given precedence over those coming later
- if there is no traffic, the phases will run through a default cycle defined by the first value in the 'next' attribute
- if a particular phase should remain active indefinitely in the absence of traffic it must have its own index in the 'next' list as well as a high maxDur value
- if an active detector was not served for a given time threshold (param **inactive-threshold**), this detector receives bonus priority according the time it was not served. This can be used to prevent starvation if other phases are consistently preferred due to serving more traffic 

Examples for this type of traffic light logic can be found in [{{SUMO}}/tests/sumo/basic/tls/actuated/dualring_simple]({{Source}}tests/sumo/basic/tls/actuated/dualring_simple).

The helper script [tls_buildTransitions.py] can be used to generate such logics from simplified definitions.


#### Visualization
By setting the sumo option **--tls.actuated.show-detectors** the default visibility of detectors can be
set. Also, in [sumo-gui](../sumo-gui.md) detectors can be
shown/hidden by right-clicking on an actuated traffic light and
selecting the corresponding menu entry.

The detectors used by an actuated traffic light will be colored to indicate their status:
- green color indicates that the detector is used to determine the length of the current phase
- white color indicates that the detector is not used in the current phase
- red color indicates that a vehicle was detected since the last time at which the controlled links at that lane had a green light (only if these links are currently red)

### Based on Time Loss

Similar to the control by time gaps between vehicles, a phase
prolongation can also be triggered by the presence of vehicles with time
loss. A TLS with this actuation type can be defined as follows:

```
<tlLogic id="0" programID="my_program" offset="0" type="delay_based">
  <param key="detectorRange" value="100" />
  <param key="minTimeLoss" value="1" />
  <param key="file" value="NULL"/>
  <param key="freq" value="300"/>
  <param key="show-detectors" value="false"/>

  <phase duration="31" minDur="5" maxDur="45" state="GGggrrrrGGggrrrr"/>
  ...
</tlLogic>
```

Here, the `detectorRange` specifies the upstream detection range in meters measured
from the stop line. Per default (if the parameter is left undefined) the
underlying [E2
detector](../Simulation/Output/Lanearea_Detectors_(E2).md) is
assumed to cover the first approaching lanes completely. The time loss
for a vehicle is accumulated as soon as it enters the detector range. If
its accumulated time loss exceeds the value of `minTimeLoss` (current default is one
second) seconds a prolongation of the corresponding green phase is
requested if it is active. The instantaneous time loss of a vehicle is
defined as `1 - v/v_max`, where `v` is its current velocity and `v_max`
the allowed maximal velocity. See \[Oertel, Robert, and Peter Wagner.
"Delay-time actuated traffic signal control for an isolated
intersection." Transportation Research Board 2011 (90th Annual Meeting).
2011.\] for details.

#### Custom Detectors
To use custom detectors (i.e. for custom placement or output) additional parameters can be defined where KEY is a lane that is incoming to the traffic light and VALUE is a user-defined laneAreaDetector.
```
   <param key="gneE42_2" value="customDetector1"/>
```
!!! caution
    Custom detectors only work when the 'tlLogic' is loaded from an additional file.


## Interaction between signal plans and right-of-way rules

Every traffic light intersection has the right-of-way rules of a
priority intersection underneath of it. This becomes obvious when
switching a traffic light off (either in [sumo-gui](../sumo-gui.md)
with a right-click on the traffic light or by [loading the "off"
program](#loading_a_new_program)).

The right-of-way rules of this priority intersection come into play when
defining signal plans with simultaneous conflicting green streams (by
using *g* and *G* states). These signal plans only work correctly if the
right-of-way rules force vehicles from the *g* stream to yield to the
*G* stream. In most cases, this relationship was set correctly by the
default signal plan.

However, when introducing new *g*/*G* relationships, correctness is only
ensured by loading the network and the new signal plan into
[netconvert](../netconvert.md) and thus updating the right-of-way
rules.

# Loading a new Program

After having defined a tls program as above, it can be loaded as an {{AdditionalFile}}; of
course, a single {{AdditionalFile}} may contain several programs. It is possible to load
several programs for a single tls into the simulation. The program
loaded last will be used (unless not defined differently using a WAUT
description). All subkeys of the additional programs must differ if they
describe the same tls.

Assuming the program as defined above is put in a file called
*tls.add.xml* it can be loaded in
[sumo](../sumo.md)/[sumo-gui](../sumo-gui.md) like this

```
sumo -a tls.add.xml ...<other options for network and routes>
```

It is also possible to load a program which switches the tls off by
giving the `programID` the value
"`off`".

```
<tlLogic id="0" type="static" programID="off"/>
```

# Tools for Importing TLS Programs

Description from real-world traffic light systems do not arrive in form
of SUMO-traffic light descriptions normally. The main difference in
presentation comes from the fact that SUMO defines a new `<phase>` whenever any
of the controlled signals changes its state, whereas traffic engineers
differentiate between phases and phase transitions.

For an easier import than editing the XML by hand, some tools exists in
{{SUMO}}/tools/tls.

- [tls_csv2SUMO.py](../Tools/tls.md#tls_csv2sumopy): this tool
  simplifies descriptions because it allows to define the duration of
  phases for each controlled edge-to-edge connection. Splitting into
  smaller SUMO-phases because other signals change is done
  automatically.
- [tls_csvSignalGroup.py](../Tools/tls.md#tls_csvsignalgrouppy):
  this tool further simplifies descriptions because it allows to
  define the start and end times of green-phases per signal group (up
  to 2 green phases, actually) and the transitions (yellow,
  red-yellow) are added automatically. The splitting into smaller
  SUMO-phases is also done automatically.

Alternatively [netedit](../netedit.md) can be used to edit programs
using a graphical user interface.

# Modifying Existing TLS-Programs

To modify the program of a traffic light it is generally necessary to
[load a new program](#loading_new_tls-programs). However, in
the special case that only the offset shall be modified it is also
possible to specify a new offset for an existing traffic light id and
programID:

```
<additional>
  <tlLogic id="0" programID="0" offset="42"/>
</additional>
```

# Controlling multiple Junctions with the same controller

In SUMO, a traffic light controller can control an arbitrary number of
junctions simultaneously. This is accomplished by one of the following
methods:

- Defining the same *tl* attribute (the controller ID) for a set of
  nodes in *.nod.xml* file when building the network with
  [netconvert](../netconvert.md)
- Setting the same *tl* attribute for multiple nodes in
  [netedit](../netedit.md)
- Setting the option **--tls.join** when building the network with
  [netconvert](../netconvert.md). This will automatically join
  the traffic lights of nodes in close proximity within a default
  distance of 20m (customizable by setting option **tls.join-dist** {{DT_FLOAT}}).

Note, that in this case the state vector for each phase will be as long
as the total number of controlled intersections. Also, the tls indices
for the connections will differ from the link indices (as the latter
starts with 0 for each intersection whereas the tls indices are unique
within a controller).

!!! caution
    The generated TLS program will attempt to identify the main directions but usually this needs some manual corrections.

# Defining Signal Groups

In SUMO every lane-to-lane connection at a controlled intersection is
assigned an index called *link tls index*. This index is used to
determine the state for each phase by looking up the character code at
that index in the state vector. The link index can be shown in
[sumo-gui](../sumo-gui.md) by setting the junction visualization
option *show link tls index*. By default link indices are unique for
each connection and assigned in a clockwise manner starting at the north
(the same as the junction link index which is used for defining
right-of-way rules). When [defining joint
TLS](#controlling_multiple_junctions_with_the_same_controller),
the indices continue their numbering in the order of the controlled
junctions.

The tls index for each connection can be [freely assigned in a
*.tll.file*](../Networks/PlainXML.md#controlled_connections)
or by setting attribute *linkIndex* in [netedit](../netedit.md). By
assigning the **same** index to multiple connection they form a signal
group and always show the same state (simply because they reference the
same state index). This allows shortening and thus simplifying the state
vector.

# Defining Program Switch Times and Procedure

In practice, a tls often uses different programs during a day and maybe
also for weekdays and for the weekend days. It is possible to load a
definition of switch times between the programs using a WAUT (short for
"Wochenschaltautomatik" \~ weekly switch automatism).

Given a tls which knows four programs - two for weekdays and two for
weekend days where from 22:00 till 6:00 the night plan shall be used and
from 6:00 till 22:00 the day plan, and already defined programs, named
"weekday_night", "weekday_day", "weekend_night", "weekend_day". To
describe the switch process, we have to describe the switch at first,
assuming our simulation runs from monday 0.00 (second 0) to monday 0.00
(second 604800):

```
<WAUT refTime="0" id="myWAUT" startProg="weekday_night">
    <wautSwitch time="21600" to="weekday_day"/>    <!-- monday, 6.00 -->
    <wautSwitch time="79200" to="weekday_night"/>  <!-- monday, 22.00 -->
    <wautSwitch time="108000" to="weekday_day"/>   <!-- tuesday, 6.00 -->
... further weekdays ...
    <wautSwitch time="453600" to="weekend_day"/>   <!-- saturday, 6.00 -->
... the weekend days ...
</WAUT>
```

The fields in WAUT have the following meanings:

| Attribute Name | Value Type | Description                                                                                      |
| -------------- | ---------- | ------------------------------------------------------------------------------------------------ |
| **id**         | string id  | The name of the defined WAUT                                   |
| **startProg**  | string id  | The program that will be used at the simulation's begin     |
| refTime   | time   | A reference time which is used as offset to the switch times given later (in simulation seconds or D:H:M:S) |
| period  | time  | The period for repeating switch times. Disabled when set to <= 0, default 0     |

and the fields in wautSwitch:

| Attribute Name | Value Type | Description                                              |
| -------------- | ---------- | -------------------------------------------------------- |
| **time**       | int        | The time the switch will take place                      |
| **to**         | string id  | The name of the program the assigned tls shall switch to |

Of course, programs with the used names must be defined before this
definition is read. Also, the switch steps must be sorted by their
execution time.

Additionally, a definition about which tls shall be switched by the WAUT
must be given, as following:

```
<wautJunction wautID="myWAUT" junctionID="RCAS" [procedure="Stretch"] [synchron="t"]/>
```

Here, the attributes have the following meaning:

| Attribute Name | Value Type  | Description                                                                                                     |
| -------------- | ----------- | --------------------------------------------------------------------------------------------------------------- |
| **wautID**     | string id   | The id of the WAUT the tls shall be switched by                                                                 |
| **junctionID** | string id   | The name of the tls to assign to the WAUT                                                                       |
| **procedure**  | string enum | The switching algorithm to use ("GSP" or "Stretch"). If not set, the programs will switch immediately (default) |
| **synchron**   | bool        | Additional information whether the switch shall be done synchron (default: false)                               |

It is possible to assign several tls to a single WAUT. It is also
possible to assign several WAUTs to a single junction in theory, but
this is not done in reality.

A complete definition within an {{AdditionalFile}} is shown below. It would trigger
switching between programs **S1** and **S2** for traffic light logic
**X** with an initial program called **0**.

```
<additional>

  <tlLogic id="X" type="static" programID="S1" offset="0">
    <phase duration="50" state="Gr"/>
    <phase duration="50" state="rG"/>
  </tlLogic>

  <tlLogic id="X" type="static" programID="S2" offset="0">
    <phase duration="30" state="Gr"/>
    <phase duration="80" state="rG"/>
  </tlLogic>


  <WAUT startProg="0" refTime="100" id="w1">
    <wautSwitch to="S1" time="300"></wautSwitch>
    <wautSwitch to="SS" time="800"></wautSwitch>
  </WAUT>

  <wautJunction junctionID="X" wautID="w1"></wautJunction>

<additional>
```

!!! note
    If a traffic light program called "online" is loaded, this program will interrupt WAUT switching at that traffic light. This can be used to override WAUT behavior via TraCI.

# Evaluation of Traffic Lights Performance

## Tools For Automatic Detector Generation

Some tools are available which help generating detector definitions for
the evaluation of traffic lights. All are located in {{SUMO}}/tools/output.

!!! note
    The actuated traffic lights do not require detector definitions to be added as they generate their own detectors for internal use.

- ***generateTLSE2Detectors.py*** generates a file which includes
  areal detectors. All lanes incoming into an intersection are covered
  with these detectors. The offset from the intersection may be given
  using the option **--distance-to-TLS** {{DT_FLOAT}} (or **-d** {{DT_FLOAT}}), the default is .1m. The generated detectors
  end either after a given length, defined using **--detector-length** {{DT_FLOAT}} (or **-l** {{DT_FLOAT}}) where the
  default is 250m, or at the lane's end if the lane is shorter than
  this length.
- ***generateTLSE3Detectors.py*** generates a file which includes
  multi-entry/multi-exit detectors. Detectors are built for each edge
  incoming to the traffic light. All lanes of each of these edges are
  covered with exit points. These point's offset from the intersection
  may be given using the option **--distance-to-TLS** {{DT_FLOAT}} (or **-d** {{DT_FLOAT}}), the default is .1m. The
  incoming edges are followed upstream, either until a given length,
  defined using **--detector-length** {{DT_FLOAT}} (or **-l** {{DT_FLOAT}}) where the default is 250m, or another traffic
  light is reached or no further upstream edge exists. Entry points
  are generated at these points.

In both cases, the network must be given using the option **--net-file** {{DT_FILE}} (or **-n** {{DT_FILE}}). The
file including the detector definitions to generate may be given using
the option **--output** {{DT_FILE}} (or **-o** {{DT_FILE}}), default is "e2.add.xml" for areal detectors, and
"e3.add.xml" for multi-entry/multi-exit detectors. Per default, the
areal detectors generated by ***generateTLSE2Detectors.py*** are writing
their measures to "e2output.xml", the multi-entry/multi-exit detectors
generated by ***generateTLSE2Detectors.py*** to "e3output.xml". The
output file name can be changed for both scripts using the option **--results-file** {{DT_FILE}} (or **-r** {{DT_FILE}}). The frequency of generated reports is 60s per default. It can be
changed using the option **--frequency** {{DT_INT}} (or **-f** {{DT_INT}}).

# Controlling traffic lights via TraCI

[TraCI provides various functions for controlling traffic
lights](../TraCI/Change_Traffic_Lights_State.md). And the basic
approaches are outline below

## Setting the phase

A common pattern for implementing adaptive control via TraCI is to load
a program (usually from an
[additional-file](#defining_new_tls-programs)) where all
green phases have a long duration (i.e. 1000s) to avoid switching by
SUMO and then use the **setPhase** function to switch to the next phase
(typically a yellow phase) whenever the green phase should end. The
nice thing about this approach is, that yellow phase and all-red phases
can still be handled automatically by SUMO and the control script simply
determines the duration of the current green phase.

To implement controllers with branching phase transitions, provide
multiple transitions to subsequent green phases and when calling
setPhase, select the yellow phase that starts the transition to the
target green phase.

These additional transition phases can be added to the end of the
program and the optional phase attribute "next" can be used to indicated the
next phase after the transition ends.

Tutorials for controlling traffic lights in this way can be found at
[Tutorials\#TraCI_Tutorials](../Tutorials.md#traci_tutorials).

## Setting the duration

Using the function **setPhaseDuration**, the remaining duration for the
current phase can be modified. Note, that this does not take effect when
encountering the same phase again in the next phase cycle.

## Setting the state

Another method for controlling the traffic light is to set the signal
state for all links directly using the function
*setRedYellowGreenState*. After using this function, SUMO will not
modify the state anymore (until switching to another program using
*setProgram*). Consequently, the script must handle all phases and
transitions.

## Setting the complete Program

Using the method **setCompleteRedYellowGreenDefinition**, a static
signal plan can be loaded. Since this method requires a complex data
structure as argument, it is recommend to first obtain a data structure
using **getCompleteRedYellowGreenDefinition** and then modify it

## Switching between pre-defined Programs

SUMO can load multiple traffic light programs from the *.net.mxl* file
and from [additional files](#defining_new_tls-programs). Using
the TraCI function **setProgram**, a script can switch between them.
