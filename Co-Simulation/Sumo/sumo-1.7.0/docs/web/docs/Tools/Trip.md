---
title: Tools/Trip
permalink: /Tools/Trip/
---

# randomTrips.py

"randomTrips.py" generates a set of random trips for a given network
(option **-n**). It does so by choosing source and destination edge either
uniformly at random or with a modified distribution as described below.
The resulting trips are stored in an XML file (option **-o**, default
trips.trips.xml) suitable for [duarouter](../duarouter.md) which is
called automatically if the  option (with a filename for the resulting
route file) is given. The trips are distributed evenly in an interval
defined by begin (option **-b**, default 0) and end time (option **-e**, default
3600) in seconds. The number of trips is defined by the repetition rate
(option **-p**, default 1) in seconds. Every trip has an id consisting of a
prefix (option **--prefix**, default "") and a running number. Example call:

```
<SUMO_HOME>/tools/randomTrips.py -n input_net.net.xml -e 50
```

The script does not check whether the chosen destination may be reached
from the source. This task is performed by the router. If the network is
not fully connected some of the trips may be discarded.

The option **--min-distance** {{DT_FLOAT}} ensures a minimum straight-line distance (in meter) between
start and end edges of a trip. The script will keep sampling from the
edge distribution until enough trips with sufficient distance are found.

## Randomization

When running *randomTrips.py* twice with the same parameters, different
output files will be created due to randomness. The option **--seed** {{DT_INT}} can be used
to get repeatable pseudo-randomness.

## Edge Probabilities

The option **--fringe-factor** {{DT_FLOAT}} increases the probability that trips will start/end at the
fringe of the network. If the value *10* is given, edges that have no
successor or no predecessor will be 10 times more likely to be chosen as
start- or endpoint of a trip. This is useful when modelling
through-traffic which starts and ends at the outside of the simulated
area.

The probabilities for selecting an edge may also be weighted by

- edge length (option **-l**),
- by number of lanes (option **-L**)
- edge speed (exponentially, by option **--speed-exponent**)
- [generic edge parameter](../Simulation/GenericParameters.md) (option **--edge-param**)
- direction of travel (option **--angle-factor** and **--angle**)

For additional ways to influence edge probabilities call

```
<SUMO_HOME>/tools/randomTrips.py --help
```

## Arrival rate

The arrival rate is controlled by option **--period** {{DT_FLOAT}} (*default 1*). By default this
generates vehicles with a constant period and arrival rate of (1/period)
per second. By using values below 1, multiple arrivals per second can be
achieved.

When adding option **--binomial** {{DT_INT}} the arrivals will be randomized using a binomial
distribution where *n* (the maximum number of simultaneous arrivals) is
given by the argument to **--binomial** and the expected arrival rate is 1/period
*(this option is not yet available in version 0.23.0)*.

### Example

To let *n* vehicles depart between times *t0* and *t1* set the options

```sh
-b t0 -e t1 -p ((t1 - t0) / n)
```

!!! note
    The actual number of departures may be lower if the road capacity is insufficient to accommodate that number of vehicles or if the network is not fully connected (in this case some of the generated trips will be invalid).

## Validated routes and trips

When using the option **--route-file**, an output file with valid vehicle routes will be
generated. This works by automatically calling
[duarouter](../duarouter.md) in the background to turn the random
trips into routes and automatically discard disconnected trips. It may
be necessary to increase the number of generated random trips to account
for a fraction disconnected, discarded trips.

!!! caution
    When using the option **--vehicle-class** the same value should be set for option **--edge-permission**

Sometimes it is desirable to obtain validated trips rather than routes
(i.e. to make use of [one-shot route
assignment](../Demand/Dynamic_User_Assignment.md#oneshot-assignment).
In this case the additional option **--validate** may be used to generate validated
trips (by first generating valid routes and then converting them back
into trips).

## Generating vehicles with additional parameters

With the option **--trip-attributes** {{DT_STR}}, additional parameters can be given to the generated
vehicles (note, usage of the quoting characters).

```
<SUMO_HOME>/tools/randomTrips.py -n input_net.net.xml 
  --trip-attributes="departLane=\"best\" departSpeed=\"max\" departPos=\"random\""
```

This would make the random vehicles be distributed randomly on their
starting edges and inserted with high speed on a reasonable lane.

!!! caution
    Quoting of trip attributes on Linux must use the style **--trip-attributes 'departLane="best" departSpeed="max" departPos="random"'**

### Setting a vehicle type from an external file

If the generated vehicles should have a specific vehicle type, an {{AdditionalFile}} needs
to be prepared:

```
<additional>
  <vType id="myType" maxSpeed="27" vClass="passenger"/>
</additional>
```

Then load this file (assume it was saved as *type.add.xml*) with the
option --additional-file

```
<SUMO_HOME>/tools/randomTrips.py -n input_net.net.xml --trip-attributes="type=\"myType\"" --additional-file type.add.xml
   --edge-permission passenger
```

Note the use of the option **--edge-permission** (deprecated alias: **--vclass**) which ensures that
random start- and arrival-edges allow a specific vehicle class.

To generate random pedestrian traffic instead of vehicular traffic, the
option **--pedestrians** may be used. It is recommended to combined this with the option **--max-distance**
to avoid walks of excessive length. See
[Simulation/Pedestrians](../Simulation/Pedestrians.md) for addition
information on the simulation of pedestrians.

Note that the option **--vehicle-class** should only be used as a quick shorthand to
generate trips for the standard type of the given vehicle class since it
places a standard vType definition in the generated trips file.

### Automatically generating a vehicle type

By setting the option **--vehicle-class** a vehicle type definition that specifies vehicle
class will be added to the output files. I.e.

```
randomTrips.py --vehicle-class bus ...
```

will add

```
<vType id="bus" vClass="bus"/>
```

Any **--trip-attributes** that are applicable to a vehicle type rather than a vehicle will be
placed in the generated `vType` definition automatically:

```
randomTrips.py --vehicle-class bus --trip-attributes="maxSpeed=\"27.8\""
```

will add

```
<vType id="bus" vClass="bus" maxSpeed="random"/>
```

Alternatively, after *randomTrips.py* has finished, the created `<vType>`-element
can be edited to specify additional parameters. The downside of manual
editing is that it must be repeated when running *randomTrips.py* again.

## Generating different modes of traffic

- Using the option **--pedestrians** will generated pedestrians instead of vehicles.
- Using the option **--persontrips** will generated persons with `<persontrip>` definitions. This
allows to specify the available traffic modes and thus use
[IntermodalRouting](../IntermodalRouting.md) to decided whether
they use public transport, a personal car or walking.
  - walking or public transport: **--trip-attributes "modes=\"public\""**
  - walking, public transport or car **--trip-attributes "modes=\"public car\""**

!!! caution
    Quoting of trip attributes on Linux must use the style **--trip-attributes 'modes="public"'**

## Intermediate Way Points

To generate longer trips within a network, intermediate way points may
be generated using the option **--intermediate** {{DT_INT}}. This will add the given number of
[via-edges](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#incomplete_routes_trips_and_flows)
to the trip definitions.

## Customized Weights

### Saving

Using option **weights-output-prefix** {{DT_STR}} will cause three *weight-files* with the given prefix to
be generated (*<prefix\>.src.xml, <prefix\>.dst.xml, <prefix\>.via.xml*)
which contain the used edge probabilities.

- *.src.xml* contains the probabilities for an edge to be selected as `from`-edge
- *.dst.xml* contains the probabilities for an edge to be selected as `to`-edge
- *.via.xml* contains the probabilities for an edge to be selected as `via`-edge (only used when option **--intermediate** is set).

### Visualization

Any of these files can be loaded in [sumo-gui for
visualization](../sumo-gui.md#visualizing_edge-related_data)

### Loading

Files in this format can be used to set customized weights for
generating random trips by using the option **--weights-prefix** {{DT_STR}} with the prefix value as
argument. The randomTrips script will attempt to load weights for all
edges and all three file extensions (*.src.xml, .dst.xml, .via.xml*) but
will use the following defaults if same values are missing:

- If a loaded weight file does not contain all edges, probability 0
  will be assumed for the missing edges
- If a file is missing, the probabilities according to the regular
  options are used when sampling that type of edge (i.e. missing
  *<prefix\>.dst.xml* will result in probabilities as described in
  section [\#Edge_Probabilities](#edge_probabilities) will
  be used when sampling destination edges)

!!! note
    Probabilities loaded from the *weight-files* are automatically normalized. Therefore it does not matter whether probabilities are specified as fractions or percentage values.

### Usage Example

To obtain trips from two specific locations (edges *a*, and *b*) to
random destinations, use

```
randomTrips.py --weights-prefix example  ...<other options>...
```

and define only the file *example.src.xml* as follows:

```
<edgedata>
  <interval begin="0" end="10"/>
    <edge id="a" value="0.5"/>
    <edge id="b" value="0.5"/>
  </interval>
</edgedata>
```

# route2trips.py

This script generates a trip file from a route file by stripping all
route information except for start and end edge. It has a single
parameter which is the route file and prints the trip file to stdout.
Example:

```
<SUMO_HOME>/tools/route2trips.py input_routes.rou.xml
```
