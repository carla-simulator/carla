---
title: marouter
permalink: /marouter/
---

# From 30.000 feet

**marouter** computes a macroscopic [user
assignment](Demand/Dynamic_User_Assignment.md#introduction)
(routes) to be loaded into [sumo](sumo.md) from various inputs
(Origin-Destination-Matrices, trip files or route files).

- **Purpose:** Macroscopic route assignment
- **System:** portable (Linux/Windows is tested); runs on command line
- **Input (mandatory):**
  - A) a road network as generated via
      [netconvert](netconvert.md) or
      [netgenerate](netgenerate.md), see [Building
      Networks](index.md#network_building)
  - B) O/D matrix
- **Output:** [Definition of Vehicles, Vehicle Types, and
  Routes](Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md)
  usable by [sumo](sumo.md)
- **Programming Language:** C++

# Usage Description

marouter computes a user assignment (routes) with a configurable
assignment method (using option **assignment-method**):

- **incremental**
- **UE**: user equilibrium

  !!! caution
      Not yet implemented

- **SUE**: stochastic user equilibrium

marouter uses a hard-coded capacity-constraint function based on speed
limit, lane number and edge priority to compute traveltimes and flows
based on density. For details, see functions
*capacityConstraintFunction* and *getCapacity* in [{{SUMO}}/src/marouter/ROMAAssignments.cpp]({{Source}}src/marouter/ROMAAssignments.cpp).

marouter reads OD matrices and creates route files. These route files
contain route distributions between traffic assignment zones. Each route
in the distribution is a fully specified list of network edges.

## Microscopic Outputs

One of the main usages of marouter is to compute a
[Demand/Dynamic_User_Assignment](Demand/Dynamic_User_Assignment.md).
To this end it generates `<flow>` definitions with fully specified routes that
can be loaded into [sumo](sumo.md) and
[sumo-gui](sumo-gui.md).

## Macroscopic Outputs

marouter can create macroscopic traffic data for all network edges by
using the option **--netload-output**. This data contains *travelTime, speed*, *entered*
(the number of vehicles that entered the edge) and *flowCapacityRatio*
(the proportion of total capacity used). These values can either be
written aggregated for the whole day (default) or split into intervals
according to the time-granularity of the input data (**--aggregation-interval**).

This output can [visualized in
sumo-gui](sumo-gui.md#visualizing_edge-related_data) to
understand how the different traffic measures change over time.

## Options

You may use a XML schema definition file for setting up a marouter
configuration:
[marouterConfiguration.xsd](http://sumo.dlr.de/xsd/marouterConfiguration.xsd).

### Configuration

| Option | Description |
|--------|-------------|
| **-c** {{DT_FILE}}<br> **--configuration-file** {{DT_FILE}} | Loads the named config on startup |
| **-C** {{DT_FILE}}<br> **--save-configuration** {{DT_FILE}} | Saves current configuration into FILE |
| **--save-template** {{DT_FILE}} | Saves a configuration template (empty) into FILE |
| **--save-schema** {{DT_FILE}} | Saves the configuration schema into FILE |
| **--save-commented** {{DT_BOOL}} | Adds comments to saved template, configuration, or schema; *default:* **false** |

### Input

| Option | Description |
|--------|-------------|
| **-n** {{DT_FILE}}<br> **--net-file** {{DT_FILE}} | Use FILE as SUMO-network to route on |
| **-d** {{DT_FILE}}<br> **--additional-files** {{DT_FILE}} | Read additional network data (districts, bus stops) from FILE |
| **-m** {{DT_FILE}}<br> **--od-matrix-files** {{DT_FILE}} | Loads O/D-files from FILE(s) |
| **--od-amitran-files** {{DT_FILE}} | Loads O/D-matrix in Amitran format from FILE(s) |
| **-r** {{DT_FILE}}<br> **--route-files** {{DT_FILE}} | Read sumo-routes or trips from FILE(s) |
| **-w** {{DT_FILE}}<br> **--weight-files** {{DT_FILE}} | Read network weights from FILE(s) |
| **--lane-weight-files** {{DT_FILE}} | Read lane-based network weights from FILE(s) |
| **-x** {{DT_STR}}<br> **--weight-attribute** {{DT_STR}} | Name of the xml attribute which gives the edge weight; *default:* **traveltime** |
| **--weight-adaption** {{DT_FLOAT}} | The travel time influence of prior intervals; *default:* **0** |
| **--taz-param** {{DT_STR[]}} | Parameter key(s) defining source (and sink) taz |
| **--junction-taz** {{DT_BOOL}} | Initialize a TAZ for every junction to use attributes toJunction and fromJunction; *default:* **false** |

### Output

| Option | Description |
|--------|-------------|
| **-o** {{DT_FILE}}<br> **--output-file** {{DT_FILE}} | Write flow definitions with route distributions to FILE |
| **--vtype-output** {{DT_FILE}} | Write used vehicle types into separate FILE |
| **--ignore-vehicle-type** {{DT_BOOL}} | Does not save vtype information; *default:* **false** |
| **--netload-output** {{DT_FILE}} | Writes edge loads and final costs into FILE |
| **--all-pairs-output** {{DT_FILE}} | Writes complete distance matrix into FILE |
| **--write-license** {{DT_BOOL}} | Include license info into every output file; *default:* **false** |
| **--output-prefix** {{DT_STR}} | Prefix which is applied to all output files. The special string 'TIME' is replaced by the current time. |
| **--precision** {{DT_INT}} | Defines the number of digits after the comma for floating point output; *default:* **2** |
| **--precision.geo** {{DT_INT}} | Defines the number of digits after the comma for lon,lat output; *default:* **6** |
| **-H** {{DT_BOOL}}<br> **--human-readable-time** {{DT_BOOL}} | Write time values as hour:minute:second or day:hour:minute:second rather than seconds; *default:* **false** |
| **--exit-times** {{DT_BOOL}} | Write exit times (weights) for each edge; *default:* **false** |

### Processing

| Option | Description |
|--------|-------------|
| **--aggregation-interval** {{DT_TIME}} | Defines the time interval when aggregating single vehicle input; Defaults to one hour; *default:* **3600** |
| **--max-alternatives** {{DT_INT}} | Prune the number of alternatives to INT; *default:* **5** |
| **--weights.interpolate** {{DT_BOOL}} | Interpolate edge weights at interval boundaries; *default:* **false** |
| **--weights.expand** {{DT_BOOL}} | Expand weights behind the simulation's end; *default:* **false** |
| **--weights.priority-factor** {{DT_FLOAT}} | Consider edge priorities in addition to travel times, weighted by factor; *default:* **0** |
| **--routing-algorithm** {{DT_STR}} | Select among routing algorithms ['dijkstra', 'astar', 'CH', 'CHWrapper']; *default:* **dijkstra** |
| **--bulk-routing.vtypes** {{DT_BOOL}} | Aggregate routing queries with the same origin for different vehicle types; *default:* **false** |
| **--routing-threads** {{DT_INT}} | The number of parallel execution threads used for routing; *default:* **0** |
| **--weight-period** {{DT_TIME}} | Aggregation period for the given weight files; triggers rebuilding of Contraction Hierarchy; *default:* **3600** |
| **-s** {{DT_FLOAT}}<br> **--scale** {{DT_FLOAT}} | Scales the loaded flows by FLOAT; *default:* **1** |
| **--vtype** {{DT_STR}} | Defines the name of the vehicle type to use |
| **--prefix** {{DT_STR}} | Defines the prefix for vehicle flow names |
| **--timeline** {{DT_STR[]}} | Uses STR[] as a timeline definition |
| **--timeline.day-in-hours** {{DT_BOOL}} | Uses STR as a 24h-timeline definition; *default:* **false** |
| **--additive-traffic** {{DT_BOOL}} | Keep traffic flows of all time slots in the net; *default:* **false** |
| **--assignment-method** {{DT_STR}} | Choose a assignment method: incremental, UE or SUE; *default:* **incremental** |
| **--tolerance** {{DT_FLOAT}} | Use FLOAT as tolerance when checking for SUE stability; *default:* **0.001** |
| **--left-turn-penalty** {{DT_FLOAT}} | Use left-turn penalty FLOAT to calculate link travel time when searching routes; *default:* **0** |
| **--paths** {{DT_INT}} | Use INTEGER as the number of paths needed to be searched for each OD pair at each iteration; *default:* **1** |
| **--paths.penalty** {{DT_FLOAT}} | Penalize existing routes with FLOAT to find secondary routes; *default:* **1** |
| **--upperbound** {{DT_FLOAT}} | Use FLOAT as the upper bound to determine auxiliary link cost; *default:* **0.5** |
| **--lowerbound** {{DT_FLOAT}} | Use FLOAT as the lower bound to determine auxiliary link cost; *default:* **0.15** |
| **-i** {{DT_INT}}<br> **--max-iterations** {{DT_INT}} | maximal number of iterations for new route searching in incremental and stochastic user assignment; *default:* **20** |
| **--max-inner-iterations** {{DT_INT}} | maximal number of inner iterations for user equilibrium calculation in the stochastic user assignment; *default:* **1000** |
| **--route-choice-method** {{DT_STR}} | Choose a route choice method: gawron, logit, or lohse; *default:* **logit** |
| **--gawron.beta** {{DT_FLOAT}} | Use FLOAT as Gawron's beta; *default:* **0.3** |
| **--gawron.a** {{DT_FLOAT}} | Use FLOAT as Gawron's a; *default:* **0.05** |
| **--keep-all-routes** {{DT_BOOL}} | Save routes with near zero probability; *default:* **false** |
| **--skip-new-routes** {{DT_BOOL}} | Only reuse routes from input, do not calculate new ones; *default:* **false** |
| **--logit.beta** {{DT_FLOAT}} | Use FLOAT as (c-)logit's beta for the commonality factor; *default:* **0.15** |
| **--logit.gamma** {{DT_FLOAT}} | Use FLOAT as (c-)logit's gamma for the commonality factor; *default:* **1** |
| **--logit.theta** {{DT_FLOAT}} | Use FLOAT as (c-)logit's theta; *default:* **0.01** |

### Defaults

| Option | Description |
|--------|-------------|
| **--flow-output.departlane** {{DT_STR}} | Assigns a default depart lane; *default:* **free** |
| **--flow-output.departpos** {{DT_STR}} | Assigns a default depart position |
| **--flow-output.departspeed** {{DT_STR}} | Assigns a default depart speed; *default:* **max** |
| **--flow-output.arrivallane** {{DT_STR}} | Assigns a default arrival lane |
| **--flow-output.arrivalpos** {{DT_STR}} | Assigns a default arrival position |
| **--flow-output.arrivalspeed** {{DT_STR}} | Assigns a default arrival speed |

### Time

| Option | Description |
|--------|-------------|
| **-b** {{DT_TIME}}<br> **--begin** {{DT_TIME}} | Defines the begin time; Previous trips will be discarded; *default:* **0** |
| **-e** {{DT_TIME}}<br> **--end** {{DT_TIME}} | Defines the end time; Later trips will be discarded; Defaults to the maximum time that SUMO can represent; *default:* **9223372036854774** |

### Report

| Option | Description |
|--------|-------------|
| **-v** {{DT_BOOL}}<br> **--verbose** {{DT_BOOL}} | Switches to verbose output; *default:* **false** |
| **--print-options** {{DT_BOOL}} | Prints option values before processing; *default:* **false** |
| **-?** {{DT_BOOL}}<br> **--help** {{DT_BOOL}} | Prints this screen or selected topics; *default:* **false** |
| **-V** {{DT_BOOL}}<br> **--version** {{DT_BOOL}} | Prints the current version; *default:* **false** |
| **-X** {{DT_STR}}<br> **--xml-validation** {{DT_STR}} | Set schema validation scheme of XML inputs ("never", "auto" or "always"); *default:* **auto** |
| **--xml-validation.net** {{DT_STR}} | Set schema validation scheme of SUMO network inputs ("never", "auto" or "always"); *default:* **never** |
| **--xml-validation.routes** {{DT_STR}} | Set schema validation scheme of SUMO route inputs ("never", "auto" or "always"); *default:* **auto** |
| **-W** {{DT_BOOL}}<br> **--no-warnings** {{DT_BOOL}} | Disables output of warnings; *default:* **false** |
| **--aggregate-warnings** {{DT_INT}} | Aggregate warnings of the same type whenever more than INT occur; *default:* **-1** |
| **-l** {{DT_FILE}}<br> **--log** {{DT_FILE}} | Writes all messages to FILE (implies verbose) |
| **--message-log** {{DT_FILE}} | Writes all non-error messages to FILE (implies verbose) |
| **--error-log** {{DT_FILE}} | Writes all warnings and errors to FILE |
| **--ignore-errors** {{DT_BOOL}} | Continue if a route could not be build; *default:* **false** |

### Random Number

| Option | Description |
|--------|-------------|
| **--random** {{DT_BOOL}} | Initialises the random number generator with the current system time; *default:* **false** |
| **--seed** {{DT_INT}} | Initialises the random number generator with the given value; *default:* **23423** |


