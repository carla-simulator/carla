---
title: Simulation/Output
permalink: /Simulation/Output/
---

# Introduction

[sumo](../sumo.md) allows to generate a large number of different
measures. All write the values they collect into files or a socket
connection following the common rules for [writing files](../Basics/Using_the_Command_Line_Applications.md#writing_files).
Per default, all are disabled, and have to be triggered individually.
Some of the available outputs ([raw vehicle positions
dump](../Simulation/Output/RawDump.md), [trip
information](../Simulation/Output/TripInfo.md), [vehicle routes
information](../Simulation/Output/VehRoutes.md), and [simulation
state statistics](../Simulation/Output/Summary.md)) are triggered
using command line options, the others have to be defined within {{AdditionalFiles}}.

## Converting Outputs

All output files written by SUMO are in XML-format by default. However,
with the python tool [xml2csv.py](../Tools/Xml.md#xml2csvpy) you
can convert any of them to a flat-file (CSV) format which can be opened
with most spread-sheet software. If you need a more compressed but still
"standardized" binary version, you can use
[xml2protobuf.py](../Tools/Xml.md#xml2protobufpy). Furthermore all
files can use a [custom binary
format](../Simulation/Output/Binary.md) which is triggered by the
file extension .sbx.

## Separating outputs of repeated runs

To keep the outputs of multiple simulation runs separate, the option **--output-prefix** {{DT_STR}}
can be used to prefix **all** output file names. When setting **--output-prefix TIME** all
outputs will be prefixed using the time at which the simulation was
started which keeps them separated automatically.

# Available Output Files

Below, the available outputs are listed, joined into groups of
topic/aggregation type. Further information about each output can be
found by following its link.

## vehicle-based information, disaggregated

- [raw vehicle positions dump](../Simulation/Output/RawDump.md):
  all vehicle positions over time
  *contains*: positions and speeds for all vehicles for all simulated
  time steps
  *used for*: obtaining movements of nodes (V2V, for ns-2)
- [emission output](../Simulation/Output/EmissionOutput.md):
  emission values of all vehicles for every simulation step
- [full output](../Simulation/Output/FullOutput.md): various
  informations for all edges, lanes and vehicles (good for
  visualisation purposes)
- [vtk output](../Simulation/Output/VTKOutput.md): generates
  Files in the well known [VTK](http://www.vtk.org/) (Visualization
  Toolkit) format, to show the positions the speed value for every
  vehicle
- [fcd output](../Simulation/Output/FCDOutput.md): Floating Car
  Data includes name, position, angle and type for every vehicle
- [trajectories output](../Simulation/Output/AmitranOutput.md):
  Trajectory Data following includes name, position, speed and
  acceleration for every vehicle following the Amitran standard
- [lanechange output](../Simulation/Output/Lanechange.md): Lane
  changing events with the associated motivation for changing for
  every vehicle
- [surrogate safety measures
  (SSM)](../Simulation/Output/SSM_Device.md): Output of safety
  related measures, headway, brake rates, etc
- [vehicle type probe](../Simulation/Output/VTypeProbe.md):
  positions of vehicles over time for a certain vehicle type
  (deprecated, use vType filters with FCD-output instead)

## simulated detectors

- [Inductive loop detectors (E1)](../Simulation/Output/Induction_Loops_Detectors_(E1).md):
  simulated induction loops
- [Instant induction loops](../Simulation/Output/Instantaneous_Induction_Loops_Detectors.md):
  simulated unaggregated induction loops
- [Lane area detectors (E2)](../Simulation/Output/Lanearea_Detectors_(E2).md):
  detectors that capture a lane segment (i.e. simulated vehicle
  tracking cameras)
- [Multi-Entry-Exit detectors (E3)](../Simulation/Output/Multi-Entry-Exit_Detectors_(E3).md):
  simulators that track traffic in an area by detecting entry and exit
  events at defined locations
- [Route Detectors](../Simulation/Output/RouteProbe.md): detector
  for sampling route distributions

## values for edges or lanes

- [edgelane traffic](../Simulation/Output/Lane-_or_Edge-based_Traffic_Measures.md):
  edge/lane-based network performance measures
- [aggregated Amitran measures](../Simulation/Output/Amitran_Traffic_Measures.md):
  edge/lane-based network performance measures following the Amitran
  standard
- [edgelane emissions](../Simulation/Output/Lane-_or_Edge-based_Emissions_Measures.md):
  edge/lane-based vehicular pollutant emission
- [edgelane noise](../Simulation/Output/Lane-_or_Edge-based_Noise_Measures.md):
  edge/lane-based vehicular noise emission; based on Harmonoise
- [queue output](../Simulation/Output/QueueOutput.md): lane-based
  calculation of the actual tailback in front of a junction

## values for junctions
There is no dedicated output format for traffic at junctions. Instead junction related traffic can be measured by placing detectors that measure traffic at the intersection.

- [Tools/Output\#generateTLSE1Detectors.py](../Tools/Output.md#generatetlse1detectorspy)
  script for generating induction loop detectors around all
  TLS-controlled intersections (point-based detected on individual lanes)
- [Tools/Output\#generateTLSE2Detectors.py](../Tools/Output.md#generatetlse2detectorspy)
  script for generating lane-area detectors around all TLS-controlled
  intersections (area-based detection on individual lanes)
- [Tools/Output\#generateTLSE3Detectors.py](../Tools/Output.md#generatetlse3detectorspy)
  script for generating multi-entry-exit detectors around all
  TLS-controlled intersections or for an arbitrary list of
  intersections. The detectors can be configured to either aggregate
  or separate the approaching edges and to include or exclude the
  junction interior. (area-based detection on edges)

Alternatively, the [values for edges or lanes](Output.md#values_for_edges_or_lanes) can be manually aggregated to obtain the flow at at a junction.

## vehicle-based information

- [trip information](../Simulation/Output/TripInfo.md):
  aggregated information about each vehicle's journey (optionally with
  emission data)
- [vehicle routes
  information](../Simulation/Output/VehRoutes.md): information
  about each vehicle's routes over simulation run
- [stop output](../Simulation/Output/StopOutput.md): information
  about vehicle
  [stops](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#stops)
  and loading/unloading of persons and containers
- [battery usage](../Models/Electric.md#battery-output):
  information about battery state for electric vehicles

## simulation(network)-based information

- [simulation state statistics](../Simulation/Output/Summary.md):
  information about the current state of the simulation (vehicle count etc.)
- [simulation state person statistics](../Simulation/Output/PersonSummary.md):
  information about the current state of persons the simulation (person count etc.)

## [traffic lights-based information](../Simulation/Output/Traffic_Lights.md)

- [traffic light states](../Simulation/Output/Traffic_Lights.md#tls_states):
  information about the state (lights) of a traffic light
- [stream-based traffic light switches](../Simulation/Output/Traffic_Lights.md#tls_switches):
  information about the switches of a traffic light signal responsible
  for a certain link
- [traffic light states, by switch](../Simulation/Output/Traffic_Lights.md#tls_switch_states):
  information about the states (lights) of a traffic light signal,
  written only when changed
- [areal detectors coupled to tls](../Simulation/Output/Traffic_Lights.md#coupled_areal_detectors):
  simulated vehicle tracking cameras triggered by tls

## Additional Debugging Outputs

- The option **--link-output** {{DT_FILE}} saves debugging data for the intersection model. This
data reveals how long each vehicle intends to occupy an upcoming
intersection.
- The option **--movereminder-output** {{DT_FILE}} saves debugging data for the interaction between vehicle
devices, lanes and output facilities. It is only available when
compiling [sumo](../sumo.md) with debug flags.
- The option **--railsignal-block-output** {{DT_FILE}} saves information about rail signal blocks. For each
controlled railSignal link the following information is generated:
  - **forwardBlock**: all lanes that are reached from the signalized
    link in forward direction up to the next rail signal
  - **bidiBlock**: all lanes that make up the reverse-direction
    track encountered within the forward block and followed up to
    the next railSignal beyond a railway switch which allows passing
  - **backwardBlock**: all lanes that enter the forwardBlock or
    bidiBlock from outside followed upstream to the entry rail
    signal
  - **conflictLinks**: all controlled links that enter the conflict
    area (forwardBlock, bidiBlock, backwardBlock) from outside,
    encoded as *<SIGNALID\>_<LINKINDEX\>*

# Commandline Output (verbose)

When running the simulation with option **--verbose** (short **-v**) the following data
will be printed (unless explicitly disabled with option **--duration-log false**):

## Vehicle Counts

- Inserted: number vehicles that entered the simulation network
- Loaded: number of vehicles that were loaded from route files. This
may differ from emitted for two reason:
  - Running with option **--scale** with a value less than 1.0
  - Having a congested network where not all vehicles could be
    inserted before the simulation time ended
- Running: number of vehicles currently active in the network at
simulation end
- Waiting: number of vehicles which could not yet be inserted into the
network due to congestion
- Teleports: number of of times that vehicles were teleported for any
of the following reasons (These reasons are given whenever a
teleport warning is issued)
  - [Collision](../Simulation/Why_Vehicles_are_teleporting.md#collisions):
    a vehicle violated its minGap requirement in relation to its
    leader vehicle
  - [Timeout](../Simulation/Why_Vehicles_are_teleporting.md#waiting_too_long_aka_grid-locks):
    a vehicle was unable to move for --time-to-teleport seconds
    (default 300)
    - wrong lane: a vehicle was unable to move because it could
      not continue its route on the current lane and was unable to
      change to the correct lane
    - yield: a vehicle was unable to cross an intersection where
      it did not have priority
    - jam: a vehicle could not continue because there was no space
      on the next lane

If the simulation contained persons the following output will be added:

- Inserted: number of persons that were loaded from route files
- Running: number of persons active in the network at simulation end
- Jammed: number of times a person was jammed

## Timing Data

- Duration: The amount of elapsed time (as measure by a clock hanging
  on the wall) while computing the simulation.
- "Real time factor": The quotient of *simulated time* / *computation
  time*. If one hour is simulated in 360 seconds the real time factor
  is 10.
- UPS: (updates per second). The number of vehicle updates that were
  performed on average per second of computation time. If a single 
  vehicle update takes on average one millisecond, this will be 1000.

If routing took place in the simulation, Each routing algorithm instance
will report

- The number of routing queries
- The number of edges that were checked in order to find the best
  route
- The total time spend routing and the average time per routing call

## Aggregated Traffic Measures

When setting the option **--duration-log.statistics**, (shortcut **-t**) verbose output is automatically enabled
(unless explicitly set to *false*) and the following averages for all
vehicle trips will be printed:

- RouteLength: average route length
- Duration: average trip duration
- WaitingTime: average time spent standing (involuntarily)
- TimeLoss: average time lost due to driving slower than desired
  (includes WaitingTime). The desired speed takes the vehicles
  [speedFactor](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#speed_distributions)
  into account.
- DepartDelay: average time vehicle departures were [delayed due to
  lack of road space](../Simulation/VehicleInsertion.md)

  !!! note
      By default, only vehicles that have arrived are included in these statistics. If the option **--tripinfo-output.write-unfinished** is set, running vehicles will be included as well.

- DepartDelayWaiting: average waiting time of vehicles which could not
  be inserted due to lack of road space by the end of the simulation.
  (only present if the option **--tripinfo-output.write-unfinished** is set)

If the simulation contained pedestrians (walking persons) the following
output will be added:

- walks: the number of distinct `<walk>`-elements in the input
- RouteLength: average walk length
- Duration: average walk duration

If the simulation contained passengers (persons riding in vehicles) the
following output will be added:

- Rides: the number of distinct `<ride>`-elements in the input
- Duration: average ride duration
- Bus rides: number of rides with a public transport vehicle driving
  on roads (public transport is identified by having the
  *line*-attribute set).
- Train rides number of rides with a public transport vehicle driving
  on rails
- Bike rides: number of rides with vehicle class *bicycle*
- Aborted rides: rides that could not be completed because no suitable
  vehicle was available

When setting this option and using [sumo-gui](../sumo-gui.md), the
network parameter dialog will also show a running average for these
traffic measures (The dialog is accessible by right-clicking on the
network background).