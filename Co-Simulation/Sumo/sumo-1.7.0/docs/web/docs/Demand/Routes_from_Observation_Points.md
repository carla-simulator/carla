---
title: Demand/Routes from Observation Points
permalink: /Demand/Routes_from_Observation_Points/
---
# Introduction
Traffic counts are a common form of traffic data. This data may be available from automated counting devices such as induction loops or radar detectors or it may be obtained from manual counts. The counts apply to a specific time range and the data may cover multiple time slices. It is also possible to distinguish counts for different types of vehicles.

SUMO provides several tools to generate traffic demand from such counting data. The generated traffic demand typically describes vehicles and their routes through the network along with their departure time. Sometimes vehicles that use the same route are grouped into `<flow>`-definitions. 

The generated traffic should obviously match the counting data but this requirement alone does not define a unique solution. The provided SUMO tools differ in their algorithm to resolve the ambiguity and arrive at a specific set of routes and vehicles. The tools can also be distinguished by the type of counting data they consume:

- [dfrouter](../dfrouter.md) uses edge based counts
- [flowrouter](../Tools/Detector.md#flowrouterpy) uses edge based counts
- [jtcrouter](../Tools/Turns.md#jtcrouterpy) uses turn-counts
- [routeSampler](../Tools/Turns.md#routesamplerpy) uses turn-counts and edge counts (and also origin-destination counts)


## Chosing the right tool
The algorithms listed above where developed to solve different problems and may work badly when used on the wrong kind of problem.

- dfrouter requires that all edges which are used as sources and sinks of traffic are provided with traffic count data. In contrast, flowrouter can infer traffic on those edges from measurements at intermediate locations.
- dfrouter and jtcrouter have no capability for calibrating generated routes among the set of all routes that fit the measurement data. They can provide good results on motorway networks but produce implausible routes in highly meshed networks (i.e. cities).
- flowrouter can use a blacklist to avoid implausible routes. The tool [implausibleRoutes.py](../Tools/Routes.md#implausibleroutespy) can be used to generate restrictions for routes that are implausible according to a configurable heuristic. When the set of implausible routes is very large (which is often the case due to a combinatorial explosion of possible routes), creating such a blacklist may be infeasible.
- routeSampler uses a whitelist to restrict the set of routes that can be used to construct a solution. Generating a sufficient set of plausible routes is easier than listing all implausible routes.
- routeSampler is the only tool that can use edge-count data together with turn-count data

# dfrouter
Since version 0.9.5, the SUMO-package contains a routing module named
[dfrouter](../dfrouter.md). The idea behind this router is that
nowadays, most highways are well equipped with induction loops,
measuring each of the highways' entering and leaving flows. Given this
information one may assume that the flows on the highway are completely
known. [dfrouter](../dfrouter.md) uses directly the information
collected from induction loops to rebuild the vehicle amounts and
routes. This is done in several steps, being mainly:

1.  Computing (and optionally saving) the detector types in the means
    that each induction is set to be a source detector, a sink detector
    or an in-between detector
2.  Computing (and optionally saving) the routes between the detectors
3.  Computing the flow amounts between the detectors
4.  Saving the flow amounts and further control structures



## Computing Detector Types

The idea behind the [dfrouter](../dfrouter.md) assumes that a
network is completely covered by detectors, meaning that all off- and
on-ramps have an induction loop placed on them. Such an information
whether an induction loop is a pure source or sink or whether it is
placed between such is but not given initially. It must be computed. To
do this, the [dfrouter](../dfrouter.md) needs the underlying
network as well as a list of detector definitions where each describes
the position of an induction loop. The network, being a previously built
SUMO-network, is supplied to the [dfrouter](../dfrouter.md) as
usual using the **--net-file <SUMO_NET_FILE\>** (**-n**) option, the list of induction loops using **--detector-files <DETECTOR_FILE\>[,<DETECTOR_FILE\>]+** (**-d** for
short). A detector file should look as follows:

```
<detectors>
    <detectorDefinition id="<DETECTOR_ID>" lane="<LANE_ID>" pos="<POS>"/>
... further detectors ...
</detectors>
```

This means that each detector is initially described using its id, a
lane it is placed on, and a position on the lane. To be exact:

- id: A string holding the id of the detector
- lane: The id of the lane the detector lies on. Must be a lane within
  the network.
- pos: The position on the lane the detector shall be laid on in
  meters. The position must be a value between -1\*lane's length and
  the lane's length. In the case of a negative value, the position
  will be computed backward from the lane's end (the position the
  vehicles drive towards).

Given a network and the list of detectors,
[dfrouter](../dfrouter.md) assigns types to detectors and saves the
so extended list into a file if the option **--detectors-output <DETECTOR_OUTPUT_FILE\>** is given. This list looks
like the input described above except that an additional attribute is
given for each detector, "type", which may have one of the following
values: "source", "sink", "between", and "discarded". You can also
generate a list of points of interests (POIs) which can be read by
[sumo-gui](../sumo-gui.md) where each POI represents a detector and
is colored by the detector type: green for source detectors, red for
sink detectors, blue for in-between detectors, and black for discarded
detectors. To force [dfrouter](../dfrouter.md) to do this, use **--detectors-poi-output <POI_FILENAME\>**.

When wished, if for example other parameters change, the extended
<DETECTOR_OUTPUT_FILE\> can be fed back again into
[dfrouter](../dfrouter.md) instead of the previous <DETECTOR_FILE\>.
In this case the detector types do not have to be computed again. To
force [dfrouter](../dfrouter.md) to recompute the types, use **--revalidate-detectors**.

## Computing Routes

Now that we do know where vehicles enter and where they leave the
network, we may compute routes for each of the pairs. The
[dfrouter](../dfrouter.md) is told to build and save routes using **--routes-output <ROUTE_OUTPUT_FILE\>**
where <ROUTE_OUTPUT_FILE\> is the name of the file the computed routes
shall be written to. The generated file only contains routes, no vehicle
type definitions and no vehicles.

Normally, only routes starting at source detectors and ending at sink
detectors are computed. Using the option **--routes-for-all** you can force
[dfrouter](../dfrouter.md) to also build routes that start at
in-between detectors. The option **--all-end-follower** will make the routes not end at the
edge the source detector is placed on, but on all edges that follow this
edge. **--keep-unfinished-routes** will also keep those routes where a sink detector could not be
found for what may be the case if the network is not completely covered
with induction loops.

## Computing Flows

The next step is to use the computed routes and flow amounts from the
real-world detectors to compute flows across the modelled network. The
flows are given to dfrouter using **--measure-files** {{DT_STR}} (or **-f <DETECTOR_FLOWS\>[,<DETECTOR_FLOWS\>]+**. They are assumed to be stored
in CSV-format using ';' as dividing character. The file should look as
follows:

```
Detector;Time;qPKW;qLKW;vPKW;vLKW
myDet1;0;10;2;100;80
... further entries ...
```

This means the first time has to name the entries (columns). Their order
is not of importance, but at least the following columns must be
included:

- Detector: A string holding the id of the detector this line
  describes; should be one of the ids used in <DETECTOR_FILE\>
- Time: The time period begin that this entry describes (in minutes)
- qPKW: The number of passenger cars that drove over the detector
  within this time period
- vPKW: The average speed of passenger cars that drove over the
  detector within this time period in km/h

The following columns may optionally be included:

- qLKW: The number of transport vehicles that drove over the detector
  within this time period
- vLKW: The average speed of transport vehicles that drove over the
  detector within this time period in km/h

These are not quite the values to be found in induction loop output. We
had to constrain the <DETECTOR_FLOWS\> files this way because dfrouter is
meant to read very many of such definitions and to do this as fast as
possible.

Because in some cases one reads detector flow definitions starting at a
certain time but wants the simulation to begin at another, it is
possible to add a time offset using **--time-offset** {{DT_INT}} which is the number of seconds to
subtracted from the read times.

### Algorithm Properties

[dfrouter](../dfrouter.md) works best when all possible source and
sink edges are supplied with detector values and these values are evenly
spaced in time. The algorithm works under the assumption that the sum of
source flows matches the sum of sink flows for every measurement
interval (thus somewhat ignoring travel time). Some properties of
[Vehicle generation](#generating_vehicles):

- The number of generated vehicles for each measurement interval is
determined by source detectors alone.
  - Excess flow at sink detectors is ignored
  - If sink detectors measure less flow, it will nevertheless arrive
    there in proportion to the measured flow
  - If sink detectors measure no flow at all, all vehicles will
    drive to one (arbitrary) sink

The number of will be determined by

## Generating Vehicles

If flow definitions were supplied, we can let the dfrouter save the
computed vehicles together with their routes. Vehicles will be generated
at the source detectors which are placed at certain positions of the
networks' lanes. dfrouter generates vehicle with the option **--emitters-output <EMITTER_OUTPUT_FILE\>**. This file
will contain vehicle (emitter) declarations for each of the source
detectors. If no value is given, vehicles will not be written.
Accompanying, there will be `routeDistributions` each with the same name as a detector.
These reflect the distribution of routes at the detector with the same
ID.

By default vehicles are inserted with even spacing over each measurement
interval. This can be changed by adding the option **--randomize-flows**.

The generated file contains individual vehicles which are assigned
routes from the [routes-output file](#computing_routes).

!!! note
    The emitters output also contains a `<routeDistribution>` for each source detector but this distribution is not used by the vehicles. It is generated for information purposes and shows the route distribution aggregated over the whole simulation time (this distribution assumes that source flows data matches sink flow data).

## Further Outputs

### Variable Speed Signs

As some approaches use a speed limit to avoid open-end boundary
problems, the dfrouter can generate a list of speed triggers (see
"Variable Speed Signs (VSS)") placed on the positions of sink detectors.
The name to save the declaration of these speed triggers into is given
using the option **--variable-speed-sign-output <VSS_OUTPUT_FILE\>**. The according variable speed sign definitions will be
written into files named "vss_<DETECTOR_ID\>.def.xml" where
<DETECTOR_ID\> is the name of the according sink detector.

### Rerouters

In order not to end vehicle routes on off-ramps, it is possible to place
rerouters (see "Rerouter") at the positions of the sink detectors, too.
Giving the option **--end-reroute-output <REROUTER_OUTPUT_FILE\>** will generate a list of rerouter declarations. Please
remark that in this case, no rerouter definitions are written, because
the dfrouter has no further information about possible routes beyond the
area covered by the detectors.

### Validation Detectors

It's quite nice to have the possibility to check whether the simulation
does what one wants. To validate whether the same flows are found within
the simulation as within the reality, the option **--validation-output <SUMO_DETECTORS_OUTPUT\>** may be helpful. It
generates a list of detector definitions (see "[inductive loop
detectors](../Simulation/Output/Induction_Loops_Detectors_(E1).md)")
placed at the positions of sink and in-between detectors. Their output
will be saved into files named "validation_det_<DETECTOR_ID\>.xml" and
should be easily comparable to the detector flows previously fed to the
router. The option **--validation-output.add-sources** will let dfrouter also build inductive loop
detectors for source detectors which are place 1m behind the real-life
detector's position.

## How to include the files

The [dfrouter](../dfrouter.md) is unique among the **SUMO**-routing
applications in that it outputs routes and vehicles separately. [You
need to make sure that the list of input files is in the correct order
for resolving references](../sumo.md#loading_order_of_input_files).
Furthermore, [dfrouter](../dfrouter.md) currently returns unsorted
vehicles in its emitters-output. Assuming that
[dfrouter](../dfrouter.md) was called with the options

```
dfrouter --net-file net.net.xml --routes-output routes.rou.xml --emitters-output vehicles.xml ...
```

SUMO must be called in the following way:

```
sumo --net-file net.net.xml --additional-files routes.rou.xml,emitters.rou.xml
```

If you run the tool
[Tools/Routes\#sort_routes.py](../Tools/Routes.md#sort_routespy)
to sort the emitters, either of the following will work:

```
sumo --net-file net.net.xml --route-files routes.rou.xml,sorted_emitters.rou.xml
sumo --net-file net.net.xml --route-files sorted_emitters.rou.xml --additional-files routes.rou.xml
```

# flowrouter.py

The [flowrouter](../Tools/Detector.md#flowrouterpy) tool [improves on dfrouter](https://doi.org/10.29007/rjj7) by dealing better with missing data and finding a set of routes that maximize the total flow (within the limits given by the count data). It provides several options to restrict the set of generated routes and thus allows for calibrating the generated traffic.

# jtcrouter.py

The [jtcrouter.py](../Tools/Turns.md#jtcrouterpy) tool (available since version 1.5.0) can build a traffic demand from turn-count data. It does so by transforming the counts into flows and turn-ratios and then passing these files to [jtrrouter](../jtrrouter.md). 

# routeSampler.py
The [routeSampler.py](../Tools/Turns.md#routesamplerpy) tool (available since version 1.5.0) builds traffic demand from turn-count data as well as edge-count data. It uses a route file as input and then repeatedly selects from this set of routes to fulfill the given count data.

A suitable route file for a given network can be generated with [randomTrips.py](../Tools/Trip.md#randomtripspy). Such a route file covers the network with shortest-path routes. Routes that deviate from the shortest path can also be generated with the help of [duarouter](../duarouter.md) option **--weights.random-factor**.

By changing the route file used as input, the generated traffic can be calibrated.
