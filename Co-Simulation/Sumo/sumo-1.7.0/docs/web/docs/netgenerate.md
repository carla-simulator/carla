---
title: netgenerate
permalink: /netgenerate/
---

# From 30.000 feet

**netgenerate** generates abstract road networks that may be used by
other SUMO-applications.

- **Purpose:** Abstract road network generation
- **System:** portable (Linux/Windows is tested); runs on command line
- **Input (mandatory):** Command line parameter
- **Output:** A generated SUMO-road network; optionally also other
outputs
- **Programming Language:** C++

# Usage Description

The usage is described at
[Networks/Abstract_Network_Generation](Networks/Abstract_Network_Generation.md)

## Options

You may use a XML schema definition file for setting up a netgenerate
configuration:
[netgenerateConfiguration.xsd](http://sumo.dlr.de/xsd/netgenerateConfiguration.xsd).

### Configuration

All applications of the **SUMO**-suite handle configuration options the
same way. These options are discussed at [Basics/Using the Command Line
Applications\#Configuration
Files](Basics/Using_the_Command_Line_Applications.md#configuration_files).

| Option | Description |
|--------|-------------|
| **-c** {{DT_FILE}}<br> **--configuration-file** {{DT_FILE}} | Loads the named config on startup |
| **-C** {{DT_FILE}}<br> **--save-configuration** {{DT_FILE}} | Saves current configuration into FILE |
| **--save-template** {{DT_FILE}} | Saves a configuration template (empty) into FILE |
| **--save-schema** {{DT_FILE}} | Saves the configuration schema into FILE |
| **--save-commented** {{DT_BOOL}} | Adds comments to saved template, configuration, or schema; *default:* **false** |

### Grid Network

| Option | Description |
|--------|-------------|
| **-g** {{DT_BOOL}}<br> **--grid** {{DT_BOOL}} | Forces NETGEN to build a grid-like network; *default:* **false** |
| **--grid.number** {{DT_INT}} | The number of junctions in both dirs; *default:* **5** |
| **--grid.length** {{DT_FLOAT}} | The length of streets in both dirs; *default:* **100** |
| **--grid.x-number** {{DT_INT}} | The number of junctions in x-dir; Overrides --grid-number; *default:* **5** |
| **--grid.y-number** {{DT_INT}} | The number of junctions in y-dir; Overrides --grid-number; *default:* **5** |
| **--grid.x-length** {{DT_FLOAT}} | The length of horizontal streets; Overrides --grid-length; *default:* **100** |
| **--grid.y-length** {{DT_FLOAT}} | The length of vertical streets; Overrides --grid-length; *default:* **100** |
| **--grid.attach-length** {{DT_FLOAT}} | The length of streets attached at the boundary; 0 means no streets are attached; *default:* **0** |

### Spider Network

| Option | Description |
|--------|-------------|
| **-s** {{DT_BOOL}}<br> **--spider** {{DT_BOOL}} | Forces NETGEN to build a spider-net-like network; *default:* **false** |
| **--spider.arm-number** {{DT_INT}} | The number of axes within the net; *default:* **13** |
| **--spider.circle-number** {{DT_INT}} | The number of circles of the net; *default:* **20** |
| **--spider.space-radius** {{DT_FLOAT}} | The distances between the circles; *default:* **100** |
| **--spider.omit-center** {{DT_BOOL}} | Omit the central node of the network; *default:* **false** |

### Random Network

!!! note
    It is not recommended to set **--rand.connectivity** to 1 as the algorithm may fail to terminate in this case.

| Option | Description |
|--------|-------------|
| **-r** {{DT_BOOL}}<br> **--rand** {{DT_BOOL}} | Forces NETGEN to build a random network; *default:* **false** |
| **--rand.iterations** {{DT_INT}} | Describes how many times an edge shall be added to the net; *default:* **2000** |
| **--rand.bidi-probability** {{DT_FLOAT}} | Defines the probability to build a reverse edge; *default:* **1** |
| **--rand.max-distance** {{DT_FLOAT}} | The maximum distance for each edge; *default:* **250** |
| **--rand.min-distance** {{DT_FLOAT}} | The minimum distance for each edge; *default:* **100** |
| **--rand.min-angle** {{DT_FLOAT}} | The minimum angle for each pair of (bidirectional) roads in DEGREES; *default:* **45** |
| **--rand.num-tries** {{DT_INT}} | The number of tries for creating each node; *default:* **50** |
| **--rand.connectivity** {{DT_FLOAT}} | Probability for roads to continue at each node; *default:* **0.95** |
| **--rand.neighbor-dist1** {{DT_FLOAT}} | Probability for a node having exactly 1 neighbor; *default:* **0** |
| **--rand.neighbor-dist2** {{DT_FLOAT}} | Probability for a node having exactly 2 neighbors; *default:* **0** |
| **--rand.neighbor-dist3** {{DT_FLOAT}} | Probability for a node having exactly 3 neighbors; *default:* **10** |
| **--rand.neighbor-dist4** {{DT_FLOAT}} | Probability for a node having exactly 4 neighbors; *default:* **10** |
| **--rand.neighbor-dist5** {{DT_FLOAT}} | Probability for a node having exactly 5 neighbors; *default:* **2** |
| **--rand.neighbor-dist6** {{DT_FLOAT}} | Probability for a node having exactly 6 neighbors; *default:* **1** |
| **--rand.random-lanenumber** {{DT_BOOL}} | Draw lane numbers randomly from [1,default.lanenumber]; *default:* **false** |
| **--rand.random-priority** {{DT_BOOL}} | Draw edge priority randomly from [1,default.priority]; *default:* **false** |
| **--rand.grid** {{DT_BOOL}} | Place nodes on a regular grid with spacing rand.min-distance; *default:* **false** |

### Input
| Option | Description |
|--------|-------------|
| **-t** {{DT_FILE}}<br> **--type-files** {{DT_FILE}} | Read edge-type defs from FILE |

### Output

| Option | Description |
|--------|-------------|
| **--write-license** {{DT_BOOL}} | Include license info into every output file; *default:* **false** |
| **--output-prefix** {{DT_STR}} | Prefix which is applied to all output files. The special string 'TIME' is replaced by the current time. |
| **--precision** {{DT_INT}} | Defines the number of digits after the comma for floating point output; *default:* **2** |
| **--precision.geo** {{DT_INT}} | Defines the number of digits after the comma for lon,lat output; *default:* **6** |
| **-H** {{DT_BOOL}}<br> **--human-readable-time** {{DT_BOOL}} | Write time values as hour:minute:second or day:hour:minute:second rather than seconds; *default:* **false** |
| **--alphanumerical-ids** {{DT_BOOL}} | The Ids of generated nodes use an alphanumerical code for easier readability when possible; *default:* **true** |
| **-o** {{DT_FILE}}<br> **--output-file** {{DT_FILE}} | The generated net will be written to FILE |
| **--plain-output-prefix** {{DT_FILE}} | Prefix of files to write plain xml nodes, edges and connections to |
| **--junctions.join-output** {{DT_FILE}} | Writes information about joined junctions to FILE (can be loaded as additional node-file to reproduce joins |
| **--prefix** {{DT_STR}} | Defines a prefix for edge and junction names |
| **--amitran-output** {{DT_FILE}} | The generated net will be written to FILE using Amitran format |
| **--matsim-output** {{DT_FILE}} | The generated net will be written to FILE using MATsim format |
| **--opendrive-output** {{DT_FILE}} | The generated net will be written to FILE using OpenDRIVE format |
| **--dlr-navteq-output** {{DT_FILE}} | The generated net will be written to dlr-navteq files with the given PREFIX |
| **--dlr-navteq.precision** {{DT_INT}} | The network coordinates are written with the specified level of output precision; *default:* **2** |
| **--output.street-names** {{DT_BOOL}} | Street names will be included in the output (if available); *default:* **false** |
| **--output.original-names** {{DT_BOOL}} | Writes original names, if given, as parameter; *default:* **false** |
| **--street-sign-output** {{DT_FILE}} | Writes street signs as POIs to FILE |
| **--opendrive-output.straight-threshold** {{DT_FLOAT}} | Builds parameterized curves whenever the angular change  between straight segments exceeds FLOAT degrees; *default:* **1e-08** |

### Processing

Normally, both [netconvert](netconvert.md) and
[netgenerate](netgenerate.md) translate the read network so that
the left- and down-most node are at coordinate (0,0). The options
--offset.x and --offset.y allow to disable this and to apply different
offsets for both the x- and the y-axis. If there are explicit offsets
given, the normalization is disabled automatically (thus there is no
need to give --offset.disable-normalization if there is at least one of
the offsets given).

| Option | Description |
|--------|-------------|
| **--turn-lanes** {{DT_INT}} | Generate INT left-turn lanes; *default:* **0** |
| **--turn-lanes.length** {{DT_FLOAT}} | Set the length of generated turning lanes to FLOAT; *default:* **20** |
| **--perturb-x** {{DT_STR}} | Apply random spatial perturbation in x direction according the the given distribution; *default:* **0** |
| **--perturb-y** {{DT_STR}} | Apply random spatial perturbation in y direction according the the given distribution; *default:* **0** |
| **--perturb-z** {{DT_STR}} | Apply random spatial perturbation in z direction according the the given distribution; *default:* **0** |
| **--numerical-ids** {{DT_BOOL}} | Remaps alphanumerical IDs of nodes and edges to ensure that all IDs are integers; *default:* **false** |
| **--numerical-ids.node-start** {{DT_INT}} | Remaps IDs of nodes to integers starting at INT; *default:* **2147483647** |
| **--numerical-ids.edge-start** {{DT_INT}} | Remaps IDs of edges to integers starting at INT; *default:* **2147483647** |
| **--reserved-ids** {{DT_FILE}} | Ensures that generated ids do not included any of the typed IDs from FILE (sumo-gui selection file format) |
| **--geometry.max-grade** {{DT_FLOAT}} | Warn about edge geometries with a grade in % above FLOAT.; *default:* **10** |
| **--geometry.max-grade.fix** {{DT_BOOL}} | Smooth edge edge geometries with a grade in above the warning threshold.; *default:* **true** |
| **--offset.disable-normalization** {{DT_BOOL}} | Turn off normalizing node positions; *default:* **false** |
| **--offset.x** {{DT_FLOAT}} | Adds FLOAT to net x-positions; *default:* **0** |
| **--offset.y** {{DT_FLOAT}} | Adds FLOAT to net y-positions; *default:* **0** |
| **--offset.z** {{DT_FLOAT}} | Adds FLOAT to net z-positions; *default:* **0** |
| **--flip-y-axis** {{DT_BOOL}} | Flips the y-coordinate along zero; *default:* **false** |
| **--roundabouts.guess** {{DT_BOOL}} | Enable roundabout-guessing; *default:* **true** |
| **--roundabouts.visibility-distance** {{DT_FLOAT}} | Default visibility when approaching a roundabout; *default:* **9** |
| **--opposites.guess** {{DT_BOOL}} | Enable guessing of opposite direction lanes usable for overtaking; *default:* **false** |
| **--opposites.guess.fix-lengths** {{DT_BOOL}} | Ensure that opposite edges have the same length; *default:* **false** |
| **--fringe.guess** {{DT_BOOL}} | Enable guessing of network fringe nodes; *default:* **false** |
| **--lefthand** {{DT_BOOL}} | Assumes left-hand traffic on the network; *default:* **false** |
| **--edges.join** {{DT_BOOL}} | Merges edges which connect the same nodes and are close to each other (recommended for VISSIM import); *default:* **false** |

### Building Defaults

| Option | Description |
|--------|-------------|
| **-L** {{DT_INT}}<br> **--default.lanenumber** {{DT_INT}} | The default number of lanes in an edge; *default:* **1** |
| **--default.lanewidth** {{DT_FLOAT}} | The default width of lanes; *default:* **-1** |
| **--default.spreadtype** {{DT_STR}} | The default method for computing lane shapes from edge shapes; *default:* **right** |
| **-S** {{DT_FLOAT}}<br> **--default.speed** {{DT_FLOAT}} | The default speed on an edge (in m/s); *default:* **13.89** |
| **-P** {{DT_INT}}<br> **--default.priority** {{DT_INT}} | The default priority of an edge; *default:* **-1** |
| **--default.type** {{DT_STR}} | The default edge type |
| **--default.sidewalk-width** {{DT_FLOAT}} | The default width of added sidewalks; *default:* **2** |
| **--default.bikelane-width** {{DT_FLOAT}} | The default width of added bike lanes; *default:* **1** |
| **--default.crossing-width** {{DT_FLOAT}} | The default width of a pedestrian crossing; *default:* **4** |
| **--default.disallow** {{DT_STR}} | The default for disallowed vehicle classes |
| **--default.junctions.keep-clear** {{DT_BOOL}} | Whether junctions should be kept clear by default; *default:* **true** |
| **--default.junctions.radius** {{DT_FLOAT}} | The default turning radius of intersections; *default:* **4** |
| **--default.connection-length** {{DT_FLOAT}} | The default length when overriding connection lengths; *default:* **-1** |
| **--default.right-of-way** {{DT_STR}} | The default algorithm for computing right of way rules ('default', 'edgePriority'); *default:* **default** |
| **-j** {{DT_STR}}<br> **--default-junction-type** {{DT_STR}} | [traffic_light|priority|right_before_left|traffic_light_right_on_red|priority_stop|allway_stop|...] Determines junction type (see wiki/Networks/PlainXML#Node_types) |

### Tls Building

| Option | Description |
|--------|-------------|
| **--tls.set** {{DT_STR[]}} | Interprets STR[] as list of junctions to be controlled by TLS |
| **--tls.unset** {{DT_STR[]}} | Interprets STR[] as list of junctions to be not controlled by TLS |
| **--tls.guess** {{DT_BOOL}} | Turns on TLS guessing; *default:* **false** |
| **--tls.guess.threshold** {{DT_FLOAT}} | Sets minimum value for the sum of all incoming lane speeds when guessing TLS; *default:* **69.4444** |
| **--tls.guess.joining** {{DT_BOOL}} | Includes node clusters into guess; *default:* **false** |
| **--tls.join** {{DT_BOOL}} | Tries to cluster tls-controlled nodes; *default:* **false** |
| **--tls.join-dist** {{DT_FLOAT}} | Determines the maximal distance for joining traffic lights (defaults to 20); *default:* **20** |
| **--tls.uncontrolled-within** {{DT_BOOL}} | Do not control edges that lie fully within a joined traffic light. This may cause collisions but allows old traffic light plans to be used; *default:* **false** |
| **--tls.ignore-internal-junction-jam** {{DT_BOOL}} | Do not build mutually conflicting response matrix, potentially ignoring vehicles that are stuck at an internal junction when their phase has ended; *default:* **false** |
| **--tls.cycle.time** {{DT_INT}} | Use INT as cycle duration; *default:* **90** |
| **--tls.green.time** {{DT_INT}} | Use INT as green phase duration; *default:* **31** |
| **-D** {{DT_FLOAT}}<br> **--tls.yellow.min-decel** {{DT_FLOAT}} | Defines smallest vehicle deceleration; *default:* **3** |
| **--tls.yellow.patch-small** {{DT_BOOL}} | Given yellow times are patched even if being too short; *default:* **false** |
| **--tls.yellow.time** {{DT_INT}} | Set INT as fixed time for yellow phase durations; *default:* **-1** |
| **--tls.red.time** {{DT_INT}} | Set INT as fixed time for red phase duration at traffic lights that do not have a conflicting flow; *default:* **5** |
| **--tls.allred.time** {{DT_INT}} | Set INT as fixed time for intermediate red phase after every switch; *default:* **0** |
| **--tls.minor-left.max-speed** {{DT_FLOAT}} | Use FLOAT as threshold for allowing left-turning vehicles to move in the same phase as oncoming straight-going vehicles; *default:* **19.44** |
| **--tls.left-green.time** {{DT_INT}} | Use INT as green phase duration for left turns (s). Setting this value to 0 disables additional left-turning phases; *default:* **6** |
| **--tls.crossing-min.time** {{DT_INT}} | Use INT as minimum green duration for pedestrian crossings (s).; *default:* **4** |
| **--tls.crossing-clearance.time** {{DT_INT}} | Use INT as clearance time for pedestrian crossings (s).; *default:* **5** |
| **--tls.scramble.time** {{DT_INT}} | Use INT as green phase duration for pedestrian scramble phase (s).; *default:* **5** |
| **--tls.half-offset** {{DT_STR[]}} | TLSs in STR[] will be shifted by half-phase |
| **--tls.quarter-offset** {{DT_STR[]}} | TLSs in STR[] will be shifted by quarter-phase |
| **--tls.default-type** {{DT_STR}} | TLSs with unspecified type will use STR as their algorithm; *default:* **static** |
| **--tls.layout** {{DT_STR}} | Set phase layout four grouping opposite directions or grouping all movements for one incoming edge ['opposites', 'incoming']; *default:* **opposites** |
| **--tls.min-dur** {{DT_INT}} | Default minimum phase duration for traffic lights with variable phase length; *default:* **5** |
| **--tls.max-dur** {{DT_INT}} | Default maximum phase duration for traffic lights with variable phase length; *default:* **50** |
| **--tls.group-signals** {{DT_BOOL}} | Assign the same tls link index to connections that share the same states; *default:* **false** |
| **--tls.ungroup-signals** {{DT_BOOL}} | Assign a distinct tls link index to every connection; *default:* **false** |

### Edge Removal

| Option | Description |
|--------|-------------|
| **--keep-edges.min-speed** {{DT_FLOAT}} | Only keep edges with speed in meters/second > FLOAT; *default:* **-1** |
| **--remove-edges.explicit** {{DT_STR[]}} | Remove edges in STR[] |
| **--keep-edges.explicit** {{DT_STR[]}} | Only keep edges in STR[] or those which are kept due to other keep-edges or remove-edges options |
| **--keep-edges.input-file** {{DT_FILE}} | Only keep edges in FILE (Each id on a single line. Selection files from sumo-gui are also supported) or those which are kept due to other keep-edges or remove-edges options |
| **--remove-edges.input-file** {{DT_FILE}} | Remove edges in FILE. (Each id on a single line. Selection files from sumo-gui are also supported) |
| **--keep-edges.in-boundary** {{DT_STR[]}} | Only keep edges which are located within the given boundary (given either as CARTESIAN corner coordinates <xmin,ymin,xmax,ymax> or as polygon <x0,y0,x1,y1,...>) |
| **--keep-edges.in-geo-boundary** {{DT_STR[]}} | Only keep edges which are located within the given boundary (given either as GEODETIC corner coordinates <lon-min,lat-min,lon-max,lat-max> or as polygon <lon0,lat0,lon1,lat1,...>) |

### Unregulated Nodes

| Option | Description |
|--------|-------------|
| **--keep-nodes-unregulated** {{DT_BOOL}} | All nodes will be unregulated; *default:* **false** |
| **--keep-nodes-unregulated.explicit** {{DT_STR[]}} | Do not regulate nodes in STR[] |
| **--keep-nodes-unregulated.district-nodes** {{DT_BOOL}} | Do not regulate district nodes; *default:* **false** |

### Junctions

| Option | Description |
|--------|-------------|
| **--junctions.right-before-left.speed-threshold** {{DT_FLOAT}} | Allow building right-before-left junctions when the incoming edge speeds are below FLOAT (m/s); *default:* **13.6111** |
| **--no-internal-links** {{DT_BOOL}} | Omits internal links; *default:* **false** |
| **--no-turnarounds** {{DT_BOOL}} | Disables building turnarounds; *default:* **false** |
| **--no-turnarounds.tls** {{DT_BOOL}} | Disables building turnarounds at tls-controlled junctions; *default:* **false** |
| **--no-turnarounds.geometry** {{DT_BOOL}} | Disables building turnarounds at geometry-like junctions; *default:* **true** |
| **--no-turnarounds.except-deadend** {{DT_BOOL}} | Disables building turnarounds except at dead end junctions; *default:* **false** |
| **--no-turnarounds.except-turnlane** {{DT_BOOL}} | Disables building turnarounds except at at junctions with a dedicated turning lane; *default:* **false** |
| **--no-turnarounds.fringe** {{DT_BOOL}} | Disables building turnarounds at fringe junctions; *default:* **false** |
| **--no-left-connections** {{DT_BOOL}} | Disables building connections to left; *default:* **false** |
| **--junctions.join** {{DT_BOOL}} | Joins junctions that are close to each other (recommended for OSM import); *default:* **false** |
| **--junctions.join-dist** {{DT_FLOAT}} | Determines the maximal distance for joining junctions (defaults to 10); *default:* **10** |
| **--junctions.corner-detail** {{DT_INT}} | Generate INT intermediate points to smooth out intersection corners; *default:* **5** |
| **--junctions.internal-link-detail** {{DT_INT}} | Generate INT intermediate points to smooth out lanes within the intersection; *default:* **5** |
| **--junctions.scurve-stretch** {{DT_FLOAT}} | Generate longer intersections to allow for smooth s-curves when the number of lanes changes; *default:* **0** |
| **--junctions.join-turns** {{DT_BOOL}} | Builds common edges for turning connections with common from- and to-edge. This causes discrepancies between geometrical length and assigned length due to averaging but enables lane-changing while turning; *default:* **false** |
| **--junctions.limit-turn-speed** {{DT_FLOAT}} | Limits speed on junctions to an average lateral acceleration of at most FLOAT m/s^2); *default:* **5.5** |
| **--junctions.limit-turn-speed.min-angle** {{DT_FLOAT}} | Do not limit turn speed for angular changes below FLOAT (degrees). The value is subtracted from the geometric angle before computing the turning radius.; *default:* **15** |
| **--junctions.limit-turn-speed.min-angle.railway** {{DT_FLOAT}} | Do not limit turn speed for angular changes below FLOAT (degrees) on railway edges. The value is subtracted from the geometric angle before computing the turning radius.; *default:* **35** |
| **--junctions.limit-turn-speed.warn.straight** {{DT_FLOAT}} | Warn about turn speed limits that reduce the speed of straight connections by more than FLOAT; *default:* **5** |
| **--junctions.limit-turn-speed.warn.turn** {{DT_FLOAT}} | Warn about turn speed limits that reduce the speed of turning connections (no u-turns) by more than FLOAT; *default:* **22** |
| **--junctions.small-radius** {{DT_FLOAT}} | Default radius for junctions that do not require wide vehicle turns; *default:* **1.5** |
| **--rectangular-lane-cut** {{DT_BOOL}} | Forces rectangular cuts between lanes and intersections; *default:* **false** |
| **--check-lane-foes.roundabout** {{DT_BOOL}} | Allow driving onto a multi-lane road if there are foes on other lanes (at roundabouts); *default:* **true** |
| **--check-lane-foes.all** {{DT_BOOL}} | Allow driving onto a multi-lane road if there are foes on other lanes (everywhere); *default:* **false** |

### Pedestrian

| Option | Description |
|--------|-------------|
| **--sidewalks.guess** {{DT_BOOL}} | Guess pedestrian sidewalks based on edge speed; *default:* **false** |
| **--sidewalks.guess.max-speed** {{DT_FLOAT}} | Add sidewalks for edges with a speed equal or below the given limit; *default:* **13.89** |
| **--sidewalks.guess.min-speed** {{DT_FLOAT}} | Add sidewalks for edges with a speed above the given limit; *default:* **5.8** |
| **--sidewalks.guess.from-permissions** {{DT_BOOL}} | Add sidewalks for edges that allow pedestrians on any of their lanes regardless of speed; *default:* **false** |
| **--sidewalks.guess.exclude** {{DT_STR[]}} | Do not guess sidewalks for the given list of edges |
| **--crossings.guess** {{DT_BOOL}} | Guess pedestrian crossings based on the presence of sidewalks; *default:* **false** |
| **--crossings.guess.speed-threshold** {{DT_FLOAT}} | At uncontrolled nodes, do not build crossings across edges with a speed above the threshold; *default:* **13.89** |
| **--walkingareas** {{DT_BOOL}} | Always build walking areas even if there are no crossings; *default:* **false** |
| **--walkingareas.join-dist** {{DT_FLOAT}} | Do not create a walkingarea between sidewalks that are connected by a pedestrian junction within FLOAT; *default:* **15** |

### Bicycle

| Option | Description |
|--------|-------------|
| **--bikelanes.guess** {{DT_BOOL}} | Guess bike lanes based on edge speed; *default:* **false** |
| **--bikelanes.guess.max-speed** {{DT_FLOAT}} | Add bike lanes for edges with a speed equal or below the given limit; *default:* **22.22** |
| **--bikelanes.guess.min-speed** {{DT_FLOAT}} | Add bike lanes for edges with a speed above the given limit; *default:* **5.8** |
| **--bikelanes.guess.from-permissions** {{DT_BOOL}} | Add bike lanes for edges that allow bicycles on any of their lanes regardless of speed; *default:* **false** |
| **--bikelanes.guess.exclude** {{DT_STR[]}} | Do not guess bikelanes for the given list of edges |

### Report

All applications of the **SUMO**-suite handle most of the reporting
options the same way. These options are discussed at [Basics/Using the
Command Line Applications\#Reporting
Options](Basics/Using_the_Command_Line_Applications.md#reporting_options).

| Option | Description |
|--------|-------------|
| **-v** {{DT_BOOL}}<br> **--verbose** {{DT_BOOL}} | Switches to verbose output; *default:* **false** |
| **--print-options** {{DT_BOOL}} | Prints option values before processing; *default:* **false** |
| **-?** {{DT_BOOL}}<br> **--help** {{DT_BOOL}} | Prints this screen or selected topics; *default:* **false** |
| **-V** {{DT_BOOL}}<br> **--version** {{DT_BOOL}} | Prints the current version; *default:* **false** |
| **-X** {{DT_STR}}<br> **--xml-validation** {{DT_STR}} | Set schema validation scheme of XML inputs ("never", "auto" or "always"); *default:* **auto** |
| **-W** {{DT_BOOL}}<br> **--no-warnings** {{DT_BOOL}} | Disables output of warnings; *default:* **false** |
| **--aggregate-warnings** {{DT_INT}} | Aggregate warnings of the same type whenever more than INT occur; *default:* **-1** |
| **-l** {{DT_FILE}}<br> **--log** {{DT_FILE}} | Writes all messages to FILE (implies verbose) |
| **--message-log** {{DT_FILE}} | Writes all non-error messages to FILE (implies verbose) |
| **--error-log** {{DT_FILE}} | Writes all warnings and errors to FILE |

### Random Number

All applications of the **SUMO**-suite handle randomisation options the
same way. These options are discussed at [Basics/Using the Command Line
Applications\#Random Number
Options](Basics/Using_the_Command_Line_Applications.md#random_number_options).

| Option | Description |
|--------|-------------|
| **--random** {{DT_BOOL}} | Initialises the random number generator with the current system time; *default:* **false** |
| **--seed** {{DT_INT}} | Initialises the random number generator with the given value; *default:* **23423** |


