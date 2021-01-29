---
title: Tools/Misc
permalink: /Tools/Misc/
---

# createVehTypeDistribution.py

Creates a vehicle type distribution by sampling from configurable value
distributions for the [desired `vType`-parameters](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_types).

Example use

```
<SUMO_HOME>/tools/createVehTypeDistribution.py config.txt
```

The only required parameter is the configuration file in the format
shown below (*example config.txt*):

```
tau; normal(0.8,0.1)
sigma; normal(0.5,0.2)
length; normal(4.9,0.2); [3.5,5.5]
param; myCustomParameter; normal(5, 2); [0, 12]
vClass; passenger
carFollowModel; Krauss
```

In the config file, one line is used per vehicle type attribute. The
syntax is: \[param; \] <AtrributeOrParameterName\>; <ValueOrDistribution\>
\[; <limits\>\]

If the prefix *param* is given at the beginning of a line, it is assumed
that the values of a vehicle *parameter* (given as a `param` child
element of the `vehicle` element) are to be sampled. Otherwise, values
of an *attribute* of the `vehicle` element are sampled.
ValueOrDistribution can be a string, a scalar value or a distribution
definition. Available distributions and its syntax are: "normal(mu,sd)"
with mu and sd being floating numbers: Normal distribution with mean mu
and standard deviation sd. "uniform(a,b)" with limits a and b being
floating numbers: Uniform distribution between a and b.
"gamma(alpha,beta)" with parameters alpha and beta: Gamma distribution.

Limits are optional and defined as the allowed interval: e.g. "\[0,1\]"
or "\[3.5,5.0\]". By default, no negative values are accepted but may be
enabled by setting a negative lower limit.

Additional options:

- **--output-file** configures the name of the output file to be written
- **--name** Name of the created distribution
- **--size** Number of s to be sampled for filling the distribution
- **--seed** Set the seed for the random number generator

## Retrieving parameters from measurements of individual vehicles

To obtain mean and deviation a number of values must be obtained from
the data set. The following is recommenced:

- **accel**: the maximum (or high percentile) acceleration for each
  vehicle
- **deccel**: the maximum (or high percentile) deceleration for each
  vehicle
- **speedFactor**: the maximum (or high percentile) quotient of
  speed/speedLimit for each vehicle


# extractTest.py

This scripts extracts test scenarios if you like to run a simulation scenario which is included in the test folder <SUMO_HOME>/tests. In order to do so you can either download the complete sumo package or use the online test extraction. I In order to do so you can either [download the complete sumo package](../Downloads.md#all-inclusive-tarball) or use the [online test extraction](https://sumo.dlr.de/extractTest.php). In the online tool you enter the path to the test you like (e.g. [{{SUMO}}/tests/sumo/extended/rerouter/use_routing_device](https://github.com/eclipse/sumo/blob/master/tests/sumo/extended/rerouter/use_routing_device) into the form and get a zip containing all the files.

# generateParkingLots.py

This script generates parking lots.

Example use

```
<SUMO_HOME>/tools/generateParkingLots.py -b <xmin, ymin, xmax, ymax> -c <connecting edge>
 [-i <parking-id> -n <number of parking spaces> -l <space-length> -a <space-angle> ...]
```

or

```
<SUMO_HOME>/tools/generateParkingLots.py -x <x-pos> -y <y-pos> -c <connecting edge>
 [-i <parking-id> -n <number of parking spaces> -l <space-length> -a <space-angle> ...]
```

The required parameter are the shape (--bounding-box) or the position
(--x-axis and --y-axis) of the parking lot and the connecting edge
(--connecting-edge). More options can be obtained by calling
<SUMO_HOME\>/tools/generateParkingLots.py --help.

Additional options:

- **--parking-id** defines the name/id of the parking lot
- **--parking-spaces** defines the number of the parking spaces
- **--start-position** defines the begin position of the entrance/exit of the parking lot
- **--end-position** defines the end position of the entrance/exit of the parking lot
- **--space-length** defines the length of each parking space
- **--space-angle** defines the angle of the parking spaces
- **--x-space-distance** defines the lateral distance (x-direction) between the locations of
  two parking spaces
- **--y-space-distance** defines the longitudinal distance (y-direction) between the
  locations of two parking spaces
- **--rotation-degree** defines the rotation degree of the parking lot
- **--adjustrate-x** defines the modification rate of x-axis if the rotation exists
- **--adjustrate-y** defines the modification rate of y-axis if the rotation exists
- **--output-suffix** output suffix
- **--fullname** full name of parking area
- **--verbose** tell me what you are doing

# generateStationEdges.py

This script generates a pedestrian edge for each public transport stop
(in the form of *.nod.xml* and *.edg.xml* files. The output is suitable
for extending rail-only networks with the bare minimum of pedestrian
infrastructure for departing, changing trains and arriving.

Example use

```
python <SUMO_HOME>/tools/generateStationEdges.py rail.net.xml stops.xml
 netconvert -s rail.net.xml -e stops.access.edg.xml -n stops.access.nod.xml --ptstop-files stops.xml -o railForPersons.net.xml --ptstop-output stopsWithAccess.xml
```

# generateContinuousRerouters.py

This script generates rerouter definitions for a continuously running simulation. Rerouters are placed ahead of each intersection with routes leading up to the next intersection and configurable turning ratios. Vehicles that enter the simulation will circulate continuously (unless hitting a dead-end).

Example use
```
python <SUMO_HOME>/tools/generateContinuousRerouters.py -n net.net.xml -o rerouter.add.xml
```

# averageRuns.py

This script runs a given sumo configuration multiple times with
different random seeds and averages [trip statistics](../Simulation/Output.md#aggregated_traffic_measures).

Example use

```
python <SUMO_HOME>/tools/averageRuns.py example.sumocfg -n 100
```

# tileGet.py

This script retrieves background images from ESRI ArcGIS tile servers and other imaging APIs
such as Google Maps and MapQuest. The simplest usage is to call it with a SUMO
network file only. It will generate a settings file containing the coordinates which
can be loaded with sumo-gui or netedit. The most useful options are -t for the
(maximum) number of tiles to retrieve and -u to give the URL of the tile server.

Example use (retrieving data from the public ArcGIS online instance)

```
python <SUMO_HOME>/tools/tileGet.py -n test.net.xml -t 10
sumo-gui -n test.net.xml -g settings.xml
```

Retrieving satellite data from Google or MapQuest (Requires obtaining an API-key first):
```
python <SUMO_HOME>/tools/tileGet.py -n test.net.xml -t 10 --url maps.googleapis.com/maps/api/staticmap --key YOURKEY
python <SUMO_HOME>/tools/tileGet.py -n test.net.xml -t 10 --url open.mapquestapi.com/staticmap/v4/getmap --key YOURKEY
```