---
title: netconvert
permalink: /netconvert/
---

# From 30.000 feet

**netconvert** imports digital road networks from different sources and
generates road networks that can be used by other tools from the
package.

- **Purpose:** Road networks import and conversion
- **System:** portable (Linux/Windows is tested); runs on command line
- **Input (mandatory):** Definition of a road network
- **Output:** A generated SUMO-road network; optionally also other
outputs
- **Programming Language:** C++

# Usage Description

netconvert is a command line application. It
assumes at least one parameter - the combination of the name of the file
type to import as parameter name and the name of the file to import as
parameter value. So, for importing a network from OpenStreetMap one
could simply write:

```
netconvert --osm my_osm_net.xml
```

and for importing a VISUM-network:

```
netconvert --visum my_visum_net.net
```

In both cases, as no output name is given, the SUMO network generated
from the imported data is written into the file "net.net.xml". To write
the network into a different file, use the option **-o** {{DT_FILE}}. If you want to save
the imported VISUM-network into a file named "my_sumo_net.net.xml",
write:

```
netconvert --visum my_visum_net.net -o my_sumo_net.net.xml
```

Many further parameter steer how the network is imported and how the
resulting SUMO-network is generated.

# Supported Network Formats

## Import

netconvert is able to import road networks from
the following formats:

- ["SUMO native" XML
  descriptions](Networks/PlainXML.md)
  (\*.edg.xml, \*.nod.xml, \*.con.xml, \*.tll.xml)
- [OpenStreetMap](OpenStreetMap_file.md) (\*.osm.xml),
  including shapes (see [OpenStreetMap
  import](Networks/Import/OpenStreetMap.md))
- [VISUM](Networks/Import/VISUM.md), including shapes and
  demands
- [Vissim](Networks/Import/Vissim.md), including demands
- [OpenDRIVE](Networks/Import/OpenDRIVE.md)
- [MATsim](Networks/Import/MATsim.md)
- [SUMO](Networks/Import/SUMO_Road_Networks.md) (\*.net.xml)
- [Shapefiles](Networks/Import/ArcView.md) (.shp, .shx, .dbf),
  e.g. ArcView and newer Tiger networks
- [Robocup Rescue League](Networks/Import/RoboCup.md),
  including shapes
- [a DLR internal variant of Navteq's
  GDF](Networks/Import/DlrNavteq.md) (Elmar format)

## Export

netconvert is able to export road networks in
the following formats:

- [SUMO](Networks/Export.md#sumo) (\*.net.xml)
- ["SUMO native" XML descriptions](Networks/Export.md#plain)
  (\*.edg.xml, \*.nod.xml, \*.con.xml, \*.tll.xml)
- [OpenDRIVE](Networks/Export.md#opendrive)
- [MATsim](Networks/Export.md#matsim)
- [a DLR internal variant of Navteq's
  GDF](Networks/Export.md#dlrnavteq) (Elmar format)
- [a basic network view developed in the Amitran
  project](Networks/Export.md#amitran)

Using python tools, conversion of .net.xml files into further formats is supported:

- [KML](Tools/Net.md#net2kmlpy)
- [GeoJSON](Tools/Net.md#net2geojsonpy)

# Further supported Data Formats

## Import

- [Public transport
  stops](Simulation/Public_Transport.md#bus_stops). The main
  use of importing stop data is to update/add
  [access](Simulation/Public_Transport.md#access_lanes)
  information.

## Export

The following list ouf output is explained in more detail at
[Networks/Further_Outputs](Networks/Further_Outputs.md)

- [Public transport
  stops](Simulation/Public_Transport.md#bus_stops). Currently,
  this can only be imported from
  [OSM](Tutorials/PT_from_OpenStreetMap.md#initial_network_and_public_transit_information_extraction)
- [Public transport
  lines](Tutorials/PT_from_OpenStreetMap.md#initial_network_and_public_transit_information_extraction)
- Information on joined junctions
- Street Signs (as POIs)
- Street Names (embedded within the *.net.xml* output
- [Parking Areas](Simulation/ParkingArea.md) (currently, only
  road-side parking can be imported from OSM)
- Railway topology output: A file for analyzing the topology of
  railway networks in regard to bi-directional track usage

# Options

You may use a XML schema definition file for setting up a netconvert
configuration:
[netconvertConfiguration.xsd](https://sumo.dlr.de/xsd/netconvertConfiguration.xsd).

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

### Input

| Option | Description |
|--------|-------------|
| **-s** {{DT_FILE}}<br> **--sumo-net-file** {{DT_FILE}} | Read SUMO-net from FILE |
| **-n** {{DT_FILE}}<br> **--node-files** {{DT_FILE}} | Read XML-node defs from FILE |
| **-e** {{DT_FILE}}<br> **--edge-files** {{DT_FILE}} | Read XML-edge defs from FILE |
| **-x** {{DT_FILE}}<br> **--connection-files** {{DT_FILE}} | Read XML-connection defs from FILE |
| **-i** {{DT_FILE}}<br> **--tllogic-files** {{DT_FILE}} | Read XML-traffic light defs from FILE |
| **-t** {{DT_FILE}}<br> **--type-files** {{DT_FILE}} | Read XML-type defs from FILE |
| **--ptstop-files** {{DT_FILE}} | Reads public transport stops from FILE |
| **--ptline-files** {{DT_FILE}} | Reads public transport lines from FILE |
| **--polygon-files** {{DT_FILE}} | Reads polygons from FILE for embedding in network where applicable |
| **--shapefile-prefix** {{DT_FILE}} | Read shapefiles (ArcView, Tiger, ...) from files starting with 'FILE' |
| **--dlr-navteq-prefix** {{DT_FILE}} | Read converted Navteq GDF data (unsplitted Elmar-network) from path 'FILE' |
| **--osm-files** {{DT_FILE}} | Read OSM-network from path 'FILE(s)' |
| **--opendrive-files** {{DT_FILE}} | Read OpenDRIVE-network from FILE |
| **--visum-file** {{DT_FILE}} | Read VISUM-net from FILE |
| **--vissim-file** {{DT_FILE}} | Read VISSIM-net from FILE |
| **--robocup-dir** {{DT_FILE}} | Read RoboCup-net from DIR |
| **--matsim-files** {{DT_FILE}} | Read MATsim-net from FILE |
| **--itsumo-files** {{DT_FILE}} | Read ITSUMO-net from FILE |
| **--heightmap.shapefiles** {{DT_FILE}} | Read heightmap from ArcGIS shapefile |
| **--heightmap.geotiff** {{DT_FILE}} | Read heightmap from GeoTIFF |

### Output

| Option | Description |
|--------|-------------|
| **--write-license** {{DT_BOOL}} | Include license info into every output file; *default:* **false** |
| **--output-prefix** {{DT_STR}} | Prefix which is applied to all output files. The special string 'TIME' is replaced by the current time. |
| **--precision** {{DT_INT}} | Defines the number of digits after the comma for floating point output; *default:* **2** |
| **--precision.geo** {{DT_INT}} | Defines the number of digits after the comma for lon,lat output; *default:* **6** |
| **-H** {{DT_BOOL}}<br> **--human-readable-time** {{DT_BOOL}} | Write time values as hour:minute:second or day:hour:minute:second rather than seconds; *default:* **false** |
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
| **--ptstop-output** {{DT_FILE}} | Writes public transport stops to FILE |
| **--ptline-output** {{DT_FILE}} | Writes public transport lines to FILE |
| **--ptline-clean-up** {{DT_BOOL}} | Clean-up pt stops that are not served by any line; *default:* **false** |
| **--parking-output** {{DT_FILE}} | Writes parking areas to FILE |
| **--railway.topology.output** {{DT_FILE}} | Analyze topology of the railway network |
| **--polygon-output** {{DT_FILE}} | Write shapes that are embedded in the network input and that are not supported by polyconvert (OpenDRIVE) |
| **--opendrive-output.straight-threshold** {{DT_FLOAT}} | Builds parameterized curves whenever the angular change  between straight segments exceeds FLOAT degrees; *default:* **1e-08** |

### Projection

| Option | Description |
|--------|-------------|
| **--simple-projection** {{DT_BOOL}} | Uses a simple method for projection; *default:* **false** |
| **--proj.scale** {{DT_FLOAT}} | Scaling factor for input coordinates; *default:* **1** |
| **--proj.rotate** {{DT_FLOAT}} | Rotation (clockwise degrees) for input coordinates; *default:* **0** |
| **--proj.utm** {{DT_BOOL}} | Determine the UTM zone (for a universal transversal mercator projection based on the WGS84 ellipsoid); *default:* **false** |
| **--proj.dhdn** {{DT_BOOL}} | Determine the DHDN zone (for a transversal mercator projection based on the bessel ellipsoid, "Gauss-Krueger"); *default:* **false** |
| **--proj** {{DT_STR}} | Uses STR as proj.4 definition for projection; *default:* **!** |
| **--proj.inverse** {{DT_BOOL}} | Inverses projection; *default:* **false** |
| **--proj.dhdnutm** {{DT_BOOL}} | Convert from Gauss-Krueger to UTM; *default:* **false** |
| **--proj.plain-geo** {{DT_BOOL}} | Write geo coordinates in plain-xml; *default:* **false** |

### Processing

| Option | Description |
|--------|-------------|
| **--speed-in-kmh** {{DT_BOOL}} | vmax is parsed as given in km/h (some); *default:* **false** |
| **--construction-date** {{DT_STR}} | Use YYYY-MM-DD date to determine the readiness of features under construction |
| **--flatten** {{DT_BOOL}} | Remove all z-data; *default:* **false** |
| **--plain.extend-edge-shape** {{DT_BOOL}} | If edge shapes do not end at the node positions, extend them; *default:* **false** |
| **--numerical-ids** {{DT_BOOL}} | Remaps alphanumerical IDs of nodes and edges to ensure that all IDs are integers; *default:* **false** |
| **--numerical-ids.node-start** {{DT_INT}} | Remaps IDs of nodes to integers starting at INT; *default:* **2147483647** |
| **--numerical-ids.edge-start** {{DT_INT}} | Remaps IDs of edges to integers starting at INT; *default:* **2147483647** |
| **--reserved-ids** {{DT_FILE}} | Ensures that generated ids do not included any of the typed IDs from FILE (sumo-gui selection file format) |
| **--dismiss-vclasses** {{DT_BOOL}} | Removes vehicle class restrictions from imported edges; *default:* **false** |
| **--geometry.split** {{DT_BOOL}} | Splits edges across geometry nodes; *default:* **false** |
| **-R** {{DT_BOOL}}<br> **--geometry.remove** {{DT_BOOL}} | Replace nodes which only define edge geometry by geometry points (joins edges); *default:* **false** |
| **--geometry.remove.keep-edges.explicit** {{DT_STR[]}} | Ensure that the given list of edges is not modified |
| **--geometry.remove.keep-edges.input-file** {{DT_FILE}} | Ensure that the edges in FILE are not modified (Each id on a single line. Selection files from sumo-gui are also supported) |
| **--geometry.remove.min-length** {{DT_FLOAT}} | Allow merging edges with differing attributes when their length is below min-length; *default:* **0** |
| **--geometry.remove.width-tolerance** {{DT_FLOAT}} | Allow merging edges with differing lane widths if the difference is below FLOAT; *default:* **0** |
| **--geometry.max-segment-length** {{DT_FLOAT}} | splits geometry to restrict segment length; *default:* **0** |
| **--geometry.min-dist** {{DT_FLOAT}} | reduces too similar geometry points; *default:* **-1** |
| **--geometry.max-angle** {{DT_FLOAT}} | Warn about edge geometries with an angle above DEGREES in successive segments; *default:* **99** |
| **--geometry.min-radius** {{DT_FLOAT}} | Warn about edge geometries with a turning radius less than METERS at the start or end; *default:* **9** |
| **--geometry.min-radius.fix** {{DT_BOOL}} | Straighten edge geometries to avoid turning radii less than geometry.min-radius; *default:* **false** |
| **--geometry.min-radius.fix.railways** {{DT_BOOL}} | Straighten edge geometries to avoid turning radii less than geometry.min-radius (only railways); *default:* **true** |
| **--geometry.junction-mismatch-threshold** {{DT_FLOAT}} | Warn if the junction shape is to far away from the original node position; *default:* **20** |
| **--geometry.check-overlap** {{DT_FLOAT}} | Warn if edges overlap by more than the given threshold value; *default:* **0** |
| **--geometry.check-overlap.vertical-threshold** {{DT_FLOAT}} | Ignore overlapping edges if they are separated vertically by the given threshold.; *default:* **4** |
| **--geometry.avoid-overlap** {{DT_BOOL}} | Modify edge geometries to avoid overlap at junctions; *default:* **true** |
| **--join-lanes** {{DT_BOOL}} | join adjacent lanes that have the same permissions and which do not admit lane-changing (sidewalks and disallowed lanes); *default:* **false** |
| **--ptline.match-dist** {{DT_FLOAT}} | Matches stops outside the road network to the referencing pt line when below the given distance; *default:* **100** |
| **--ptstop-output.no-bidi** {{DT_BOOL}} | Skips automatic generation of stops on the bidi-edge of a loaded stop; *default:* **false** |
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
| **--speed.offset** {{DT_FLOAT}} | Modifies all edge speeds by adding FLOAT; *default:* **0** |
| **--speed.factor** {{DT_FLOAT}} | Modifies all edge speeds by multiplying by FLOAT; *default:* **1** |
| **--speed.minimum** {{DT_FLOAT}} | Modifies all edge speeds to at least FLOAT; *default:* **0** |
| **--edges.join-tram-dist** {{DT_FLOAT}} | Joins tram edges into road lanes with similar geometry (within FLOAT distance); *default:* **-1** |

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

### Tls Building

| Option | Description |
|--------|-------------|
| **--tls.discard-loaded** {{DT_BOOL}} | Does not instatiate traffic lights loaded from other formats than plain-XML; *default:* **false** |
| **--tls.discard-simple** {{DT_BOOL}} | Does not instatiate traffic lights at geometry-like nodes loaded from other formats than plain-XML; *default:* **false** |
| **--tls.set** {{DT_STR[]}} | Interprets STR[] as list of junctions to be controlled by TLS |
| **--tls.unset** {{DT_STR[]}} | Interprets STR[] as list of junctions to be not controlled by TLS |
| **--tls.guess** {{DT_BOOL}} | Turns on TLS guessing; *default:* **false** |
| **--tls.guess.threshold** {{DT_FLOAT}} | Sets minimum value for the sum of all incoming lane speeds when guessing TLS; *default:* **69.4444** |
| **--tls.taz-nodes** {{DT_BOOL}} | Sets district nodes as tls-controlled; *default:* **false** |
| **--tls.guess.joining** {{DT_BOOL}} | Includes node clusters into guess; *default:* **false** |
| **--tls.join** {{DT_BOOL}} | Tries to cluster tls-controlled nodes; *default:* **false** |
| **--tls.join-dist** {{DT_FLOAT}} | Determines the maximal distance for joining traffic lights (defaults to 20); *default:* **20** |
| **--tls.uncontrolled-within** {{DT_BOOL}} | Do not control edges that lie fully within a joined traffic light. This may cause collisions but allows old traffic light plans to be used; *default:* **false** |
| **--tls.ignore-internal-junction-jam** {{DT_BOOL}} | Do not build mutually conflicting response matrix, potentially ignoring vehicles that are stuck at an internal junction when their phase has ended; *default:* **false** |
| **--tls.guess-signals** {{DT_BOOL}} | Interprets tls nodes surrounding an intersection as signal positions for a larger TLS. This is typical pattern for OSM-derived networks; *default:* **false** |
| **--tls.guess-signals.dist** {{DT_FLOAT}} | Distance for interpreting nodes as signal locations; *default:* **25** |
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

### Ramp Guessing

| Option | Description |
|--------|-------------|
| **--ramps.guess** {{DT_BOOL}} | Enable ramp-guessing; *default:* **false** |
| **--ramps.guess-acceleration-lanes** {{DT_BOOL}} | Guess on-ramps and mark acceleration lanes if they exist but do not add new lanes; *default:* **true** |
| **--ramps.max-ramp-speed** {{DT_FLOAT}} | Treat edges with speed > FLOAT as no ramps; *default:* **-1** |
| **--ramps.min-highway-speed** {{DT_FLOAT}} | Treat edges with speed < FLOAT as no highways; *default:* **21.9444** |
| **--ramps.ramp-length** {{DT_FLOAT}} | Use FLOAT as ramp-length; *default:* **100** |
| **--ramps.min-weave-length** {{DT_FLOAT}} | Use FLOAT as minimum ramp-length; *default:* **50** |
| **--ramps.set** {{DT_STR[]}} | Tries to handle the given edges as ramps |
| **--ramps.unset** {{DT_STR[]}} | Do not consider the given edges as ramps |
| **--ramps.no-split** {{DT_BOOL}} | Avoids edge splitting; *default:* **false** |

### Edge Removal

| Option | Description |
|--------|-------------|
| **--keep-edges.min-speed** {{DT_FLOAT}} | Only keep edges with speed in meters/second > FLOAT; *default:* **-1** |
| **--remove-edges.explicit** {{DT_STR[]}} | Remove edges in STR[] |
| **--keep-edges.explicit** {{DT_STR[]}} | Only keep edges in STR[] or those which are kept due to other keep-edges or remove-edges options |
| **--keep-edges.input-file** {{DT_FILE}} | Only keep edges in FILE (Each id on a single line. Selection files from sumo-gui are also supported) or those which are kept due to other keep-edges or remove-edges options |
| **--remove-edges.input-file** {{DT_FILE}} | Remove edges in FILE. (Each id on a single line. Selection files from sumo-gui are also supported) |
| **--keep-edges.postload** {{DT_BOOL}} | Remove edges after joining; *default:* **false** |
| **--keep-edges.in-boundary** {{DT_STR[]}} | Only keep edges which are located within the given boundary (given either as CARTESIAN corner coordinates <xmin,ymin,xmax,ymax> or as polygon <x0,y0,x1,y1,...>) |
| **--keep-edges.in-geo-boundary** {{DT_STR[]}} | Only keep edges which are located within the given boundary (given either as GEODETIC corner coordinates <lon-min,lat-min,lon-max,lat-max> or as polygon <lon0,lat0,lon1,lat1,...>) |
| **--keep-edges.by-vclass** {{DT_STR[]}} | Only keep edges which allow one of the vclasss in STR[] |
| **--remove-edges.by-vclass** {{DT_STR[]}} | Remove edges which allow only vclasses from STR[] |
| **--keep-edges.by-type** {{DT_STR[]}} | Only keep edges where type is in STR[] |
| **--keep-edges.components** {{DT_INT}} | Only keep the INT largest weakly connected components; *default:* **0** |
| **--remove-edges.by-type** {{DT_STR[]}} | Remove edges where type is in STR[] |
| **--remove-edges.isolated** {{DT_BOOL}} | Removes isolated edges; *default:* **false** |

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
| **--junctions.join-exclude** {{DT_STR[]}} | Interprets STR[] as list of junctions to exclude from joining |
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

### Railway

| Option | Description |
|--------|-------------|
| **--railway.signals.discard** {{DT_BOOL}} | Discard all railway signal information loaded from other formats than plain-xml; *default:* **false** |
| **--railway.topology.repair** {{DT_BOOL}} | Repair topology of the railway network; *default:* **false** |
| **--railway.topology.repair.connect-straight** {{DT_BOOL}} | Allow bidiretional rail use wherever rails with opposite directions meet at a straight angle; *default:* **false** |
| **--railway.topology.repair.stop-turn** {{DT_BOOL}} | Add turn-around connections at all loaded stops.; *default:* **false** |
| **--railway.topology.all-bidi** {{DT_BOOL}} | Make all rails usable in both direction; *default:* **false** |
| **--railway.topology.all-bidi.input-file** {{DT_FILE}} | Make all rails edge ids from FILE usable in both direction |
| **--railway.topology.direction-priority** {{DT_BOOL}} | Set edge priority values based on estimated main direction; *default:* **false** |
| **--railway.access-distance** {{DT_FLOAT}} | The search radius for finding suitable road accesses for rail stops; *default:* **150** |
| **--railway.max-accesses** {{DT_INT}} | The maximum roud accesses registered per rail stops; *default:* **5** |
| **--railway.access-factor** {{DT_FLOAT}} | The walking length of the access is computed as air-line distance multiplied by FLOAT; *default:* **1.5** |

### Formats

| Option | Description |
|--------|-------------|
| **--discard-params** {{DT_STR[]}} | Remove the list of keys from all params |
| **--osm.skip-duplicates-check** {{DT_BOOL}} | Skips the check for duplicate nodes and edges; *default:* **false** |
| **--osm.elevation** {{DT_BOOL}} | Imports elevation data; *default:* **false** |
| **--osm.layer-elevation** {{DT_FLOAT}} | Reconstruct (relative) elevation based on layer data. Each layer is raised by FLOAT m; *default:* **0** |
| **--osm.layer-elevation.max-grade** {{DT_FLOAT}} | Maximum grade threshold in % at 50km/h when reconstrucing elevation based on layer data. The value is scaled according to road speed.; *default:* **10** |
| **--osm.oneway-spread-right** {{DT_BOOL}} | Whether one-way roads should be spread to the side instead of centered; *default:* **false** |
| **--osm.stop-output.length** {{DT_FLOAT}} | The default length of a public transport stop in FLOAT m; *default:* **25** |
| **--osm.stop-output.length.bus** {{DT_FLOAT}} | The default length of a bus stop in FLOAT m; *default:* **15** |
| **--osm.stop-output.length.tram** {{DT_FLOAT}} | The default length of a tram stop in FLOAT m; *default:* **25** |
| **--osm.stop-output.length.train** {{DT_FLOAT}} | The default length of a train stop in FLOAT m; *default:* **200** |
| **--osm.all-attributes** {{DT_BOOL}} | Whether additional attributes shall be imported; *default:* **false** |
| **--matsim.keep-length** {{DT_BOOL}} | The edge lengths given in the MATSIM-file will be kept; *default:* **false** |
| **--matsim.lanes-from-capacity** {{DT_BOOL}} | The lane number will be computed from the capacity; *default:* **false** |
| **--shapefile.street-id** {{DT_STR}} | Read edge ids from column STR |
| **--shapefile.from-id** {{DT_STR}} | Read from-node ids from column STR |
| **--shapefile.to-id** {{DT_STR}} | Read to-node ids from column STR |
| **--shapefile.type-id** {{DT_STR}} | Read type ids from column STR |
| **--shapefile.laneNumber** {{DT_STR}} | Read lane number from column STR |
| **--shapefile.speed** {{DT_STR}} | Read speed from column STR |
| **--shapefile.name** {{DT_STR}} | Read (non-unique) name from column STR |
| **--shapefile.node-join-dist** {{DT_FLOAT}} | Distance threshold for determining whether distinct shapes are connected (used when from-id and to-id are not available); *default:* **0** |
| **--shapefile.add-params** {{DT_STR[]}} | Add the list of field names as edge params |
| **--shapefile.use-defaults-on-failure** {{DT_BOOL}} | Uses edge type defaults on problems; *default:* **false** |
| **--shapefile.all-bidirectional** {{DT_BOOL}} | Insert edges in both directions; *default:* **false** |
| **--shapefile.guess-projection** {{DT_BOOL}} | Guess the proper projection; *default:* **false** |
| **--shapefile.traditional-axis-mapping** {{DT_BOOL}} | Use traditional axis order (lon, lat); *default:* **false** |
| **--dlr-navteq.tolerant-permissions** {{DT_BOOL}} | Allow more vehicle classes by default; *default:* **false** |
| **--vissim.join-distance** {{DT_FLOAT}} | Structure join offset; *default:* **5** |
| **--vissim.default-speed** {{DT_FLOAT}} | Use FLOAT as default speed; *default:* **50** |
| **--vissim.speed-norm** {{DT_FLOAT}} | Factor for edge velocity; *default:* **1** |
| **--vissim.report-unset-speeds** {{DT_BOOL}} | Writes lanes without an explicit speed set; *default:* **false** |
| **--visum.language-file** {{DT_FILE}} | Load language mappings from FILE |
| **--visum.use-type-priority** {{DT_BOOL}} | Uses priorities from types; *default:* **false** |
| **--visum.use-type-laneno** {{DT_BOOL}} | Uses lane numbers from types; *default:* **false** |
| **--visum.use-type-speed** {{DT_BOOL}} | Uses speeds from types; *default:* **false** |
| **--visum.connector-speeds** {{DT_FLOAT}} | Sets connector speed; *default:* **100** |
| **--visum.connectors-lane-number** {{DT_INT}} | Sets connector lane number; *default:* **3** |
| **--visum.no-connectors** {{DT_BOOL}} | Excludes connectors; *default:* **true** |
| **--visum.recompute-lane-number** {{DT_BOOL}} | Computes the number of lanes from the edges' capacities; *default:* **false** |
| **--visum.verbose-warnings** {{DT_BOOL}} | Prints all warnings, some of which are due to VISUM misbehaviour; *default:* **false** |
| **--visum.lanes-from-capacity.norm** {{DT_FLOAT}} | The factor for flow to no. lanes conversion; *default:* **1800** |
| **--opendrive.import-all-lanes** {{DT_BOOL}} | Imports all lane types; *default:* **false** |
| **--opendrive.ignore-widths** {{DT_BOOL}} | Whether lane widths shall be ignored.; *default:* **false** |
| **--opendrive.curve-resolution** {{DT_FLOAT}} | The geometry resolution in m when importing curved geometries as line segments.; *default:* **2** |
| **--opendrive.advance-stopline** {{DT_FLOAT}} | Allow stop lines to be built beyond the start of the junction if the geometries allow so; *default:* **0** |
| **--opendrive.min-width** {{DT_FLOAT}} | The minimum lane width for determining start or end of variable-width lanes; *default:* **1.8** |
| **--opendrive.internal-shapes** {{DT_BOOL}} | Import internal lane shapes; *default:* **false** |

### Report

| Option | Description |
|--------|-------------|
| **-v** {{DT_BOOL}}<br> **--verbose** {{DT_BOOL}} | Switches to verbose output; *default:* **false** |
| **--print-options** {{DT_BOOL}} | Prints option values before processing; *default:* **false** |
| **-?** {{DT_BOOL}}<br> **--help** {{DT_BOOL}} | Prints this screen or selected topics; *default:* **false** |
| **-V** {{DT_BOOL}}<br> **--version** {{DT_BOOL}} | Prints the current version; *default:* **false** |
| **-X** {{DT_STR}}<br> **--xml-validation** {{DT_STR}} | Set schema validation scheme of XML inputs ("never", "auto" or "always"); *default:* **auto** |
| **--xml-validation.net** {{DT_STR}} | Set schema validation scheme of SUMO network inputs ("never", "auto" or "always"); *default:* **never** |
| **-W** {{DT_BOOL}}<br> **--no-warnings** {{DT_BOOL}} | Disables output of warnings; *default:* **false** |
| **--aggregate-warnings** {{DT_INT}} | Aggregate warnings of the same type whenever more than INT occur; *default:* **-1** |
| **-l** {{DT_FILE}}<br> **--log** {{DT_FILE}} | Writes all messages to FILE (implies verbose) |
| **--message-log** {{DT_FILE}} | Writes all non-error messages to FILE (implies verbose) |
| **--error-log** {{DT_FILE}} | Writes all warnings and errors to FILE |
| **--ignore-errors** {{DT_BOOL}} | Continue on broken input; *default:* **false** |
| **--ignore-errors.connections** {{DT_BOOL}} | Continue on invalid connections; *default:* **false** |
| **--show-errors.connections-first-try** {{DT_BOOL}} | Show errors in connections at parsing; *default:* **false** |
| **--ignore-errors.edge-type** {{DT_BOOL}} | Continue on unknown edge types; *default:* **false** |

### Random Number

| Option | Description |
|--------|-------------|
| **--random** {{DT_BOOL}} | Initialises the random number generator with the current system time; *default:* **false** |
| **--seed** {{DT_INT}} | Initialises the random number generator with the given value; *default:* **23423** |

# Lefthand Networks

By default, netconvert assumes networks to follow right-hand traffic
rules. When importing importing/building a network for a jurisdiction
wiht left-hand traffic, the option **--lefthand** must be set.

Note, that this also influences geometries if the edges which make up a
two-directional road are defined using the road-center-line as geometry.
In right-hand networks the lane shapes will be placed to the right of
the center line whereas in left-hand networks they will be placed to the
left. This means that for some edges, the same input geometries can be
used to build right-hand as well as left-hand networks. However, if the
edge geometries are defined using the center-line of the directional
edge (attribute \{\{XML|1=spreadType="center"/\>) then the geometries are
only useable for a specific type of jurisdiction (networks with the
wrong setting for **--lefthand** will look strange).

# Warnings during Import

Several types of warnings and errors with different levels of severity
may be issued during network building.

| Message                          | Explanation              | Recommended Action                               |
| -------------------------------- | ------------------------ | ------------------------------------------------ |
| Error: Could not set connection ...                                                                       | A connection definition could not be applied because the lanes do not exist or do not share a common junction                                    | Check your input files                                                                                                                                                                               |
| Warning: Removed a road without junctions ...                                                             | Isolated road is removed from the network                                                                                                        |                                                                                                                                                                                                      |
| Warning: Not joining junctions ... because the cluster is too complex                                     | The heuristic for option **--junctions.join** cannot handle a complex junction                                                                                       | Inspect the mentioned nodes and provide a [join-description](Networks/PlainXML.md#joining_nodes) manually.                                                |
| Warning: Found angle of ... degrees                                                                       | An edge has an unusually sharp turn somewhere in the middle                                                                                      | Inspect the mentioned edge for correctness. Warning may be suppressed using option **--geometry.max-angle**.                                                                                                                 |
| Warning: Warning: Found sharp turn with radius ...                                                        | An edge has an unusually sharp turn where entering or leaving an intersection                                                                    | Inspect the mentioned edge for correctness. Problems may be corrected automatically by setting option **--geometry.min-radius.fix**. Warning may be suppressed by setting option **--geometry.min-radius**.                                                |
| Warning: Intersecting left turns at junction ...                                                          | The junction is very small so that left-turning vehicles from opposite directions obstruct each other                                            | Inspect the mentioned junction shape for correctness. Warning is not critical because the right-of-way model will be adapted to the conflict.                                                        |
| Warning: Lane '...' is not connected from any incoming edge at junction '...'.                            | The connection layout at the junction is suspicious                                                                                              | Inspect the mentioned junction's connections for correctness.                                                                                                                                        |
| Warning: Lane '...' is not connected to outgoing edges at junction '...'.                                 | The connection layout at the junction is suspicious                                                                                              | Inspect the mentioned junction's connections for correctness.                                                                                                                                        |
| Warning: Edge / Connection '...' has a grade of ...%.                                                     | There is a sharp jump in elevation in the edge or connection geometry                                                                            | [Inspect the z values for correctness](Networks/Elevation.md#visualizing_elevation_data)                                                                                                     |
| Warning: Speed of ... connection '...' reduced by ... due to turning radius of ... (length=... angle=...) | There is an unusually sharp turn relative to the road speed                                                                                      | Check edge speed limit and junction geometry for correctness. Warning may be suppressed by setting higher threshold via option **--junctions.limit-turn-speed.warn.turn {{DT_FLOAT}}, --junctions.limit-turn-speed.warn.straight {{DT_FLOAT}}**.                                                                     |
| Warning: For node '...': could not compute shape.                                                         | The shape of the junction area could not be computed                                                                                             | Inspect the mentioned node and maybe [provide a custom node shape](Networks/PlainXML.md#node_descriptions).                                               |
| Warning: Junction shape for '...' has distance ... to its given position.                                 | The shape and location of the junction area is suspicious.                                                                                       | Inspect the mentioned node and maybe [provide a custom node shape](Networks/PlainXML.md#node_descriptions). Warning may be suppressed by setting option **--geometry.junction-mismatch-threshold**. |
| Warning: The traffic light '...' does not control any links; it will not be build.                        | The traffic light at the given node sits at a junction without connections or the node was removed altogether.                                   | Inspect the mentioned node if it still exists. Can usually be ignored.                                                                                                                               |
| Warning: Splitting vector close to end ...                                                                | A geometry computation somewhere deep in the code is suspicious. Often related to computation of internal lanes at intersections with bad shape. | Ignore for lack of sufficient information (or try to narrow down the problem by building smaller sections of the network).                                                                           |
| Warning: Splitting vector close to end ...                                                                | A geometry computation somewhere deep in the code is suspicious. Often related to computation of internal lanes at intersections with bad shape. | Ignore for lack of sufficient information (or try to narrow down the problem by building smaller sections of the network).                                                                           |
