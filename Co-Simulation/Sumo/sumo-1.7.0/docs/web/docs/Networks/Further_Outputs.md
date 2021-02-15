---
title: Networks/Further Outputs
permalink: /Networks/Further_Outputs/
---

[netconvert](../netconvert.md),
[netgenerate](../netgenerate.md), and [netedit](../netedit.md)
allow to generate additional output files besides writing a SUMO network
file. They will be presented in the following.

# Writing/Exporting Networks

If no other output option is given, [netconvert](../netconvert.md)
and [netgenerate](../netgenerate.md) will write the result of
network import/generation as a SUMO network file into "net.net.xml".
Otherwise the specified output will be generated.

Currently, the applications allow to write networks in the following
formats:

- SUMO road networks
- plain XML definitions, as described in [Networks/Building Networks
  from own
  XML-descriptions](../Networks/PlainXML.md)
- MATsim networks

## SUMO Road Networks

This is the default output format, see above. The name of the file to
write the network into can be given using the option **--output-file** {{DT_FILE}}. **--sumo-output** {{DT_FILE}} and **--output** {{DT_FILE}} are
synonyms.

## Plain XML Output

Parsed node and edge definitions may be saved into a XML-files which
have the same formats as the ones used for importing XML-networks (as
described in [Node
Descriptions](../Networks/PlainXML.md#node_descriptions)
and [Edge
Descriptions](../Networks/PlainXML.md#node_descriptions)).
This shall ease processing of networks read from other formats than XML.
The option **--plain-output** {{DT_FILE}} forces [netconvert](../netconvert.md) and
[netgenerate](../netgenerate.md) to generate a file named
"{{DT_FILE}}.nod.xml" which contains the previously imported nodes, a file named
"{{DT_FILE}}.edg.xml" which contains the previously imported edges, and a file
named "{{DT_FILE}}.con.xml" which contains the previously imported connections. The
edge file will contain the list of previously read edges and each edge
will have the information about the edge's id, the allowed velocity, the
number of lanes, and the from/to - nodes stored. Geometry information is
stored only if the imported edge has a shape, meaning that it is not
only a straight connection between the from/to-nodes. The lane spread
type and the basic edge type are only saved if differing from defaults
("right" and "normal", respectively). Additionally, if one of the lanes
prohibits/allows vehicle classes, this information is saved, too (see
also "Defining allowed Vehicle Types").

## MATsim Road Networks

To write the imported/generated network as a MATsim file, use the option
**--matsim-output** {{DT_FILE}}. The extension for MATsim networks is usually ".xml".

Please note that the capacity is computed by multiplying an edge's lane
number with the capacity norm:

```
MAXIMUM_FLOW = LANE_NUMBER * CAPACITY_NORM
```

The value of CAPACITY_NORM is controlled via the option **--capacity-norm** {{DT_FLOAT}} (default: 1800).

## OpenDRIVE Road Networks

To write the imported/generated network as a
[OpenDRIVE](../Networks/Import/OpenDRIVE.md) file (version 1.4),
use the option **--opendrive-output** {{DT_FILE}}. The extension for OpenDRIVE networks is usually
".xodr".

Some notes:

- The feature is currently under implementation
- `road` - the normal ones
  - the road `type` is always set to
    "`town`" for the complete street
  - the lane `type` is set to either
    *biking, sidewalk, tram, none* or *driving* according to the
    edge permissions.
  - `link`
    - The road is always connected to the nodes it is outgoing
      (`predecessor`) / incoming
      (`successor`) from/to
  - `planView`
    - the geometry is given as lines and paramPoly3
  - no road widenings are modeled - if the number of lanes changes,
    the road changes
  - `elevationProfile` is written if the
    network contains 3D geometries
  - `lateralProfile` does not contain
    relevant information
  - the roads are always unidirectional, this means only the center
    lane and the right lanes are given
  - `objects` and
    `signals` do not contain relevant
    information

Recommended options

- **--junctions.scurve-stretch 1.0**. This elongates junction shapes to allow for smooth transitions
  (values around *1.0* can be used to reduced or increase stretching)

- **--output.original-names**. This records the edge IDs from the corresponding *.net.xml* within `<userData sumoID="..."/>` elements as children of the `<road>`

### Embedding Road Objects

To include road objects in the generated *xodr*-output, the following
conditions must be met:

- a polygon file is loaded with `<poly>` elements that encode a rectangular
  shape (4 points) by setting the option **--polygon-files** {{DT_FILE}}
- edges include the [generic
  parameter](../Simulation/GenericParameters.md) `<param key="roadObjects" value="POLY_D1 POLY_ID2 ... POLY_IDK"/>`

Such edges will receive the polygon objects with the indicated IDs as
road objects

## (Q)GIS / GeoJSON
Conversion of .net.xml file with [python tool net2geojson](../Tools/Net.md#net2geojsonpy)

## KML
Conversion of .net.xml file with [python tool net2kml](../Tools/Net.md#net2kmlpy)

# Further Outputs

## Public Transport Stops

The option **--ptstop-output** {{DT_FILE}} causes an {{AdditionalFile}} to be written that contains `<busStop/>` elements for the
imported network. These can be loaded directly into
[sumo](../sumo.md) or further modified with
[netedit](../netedit.md).

## Public Transport Lines

The option **--ptline-output** {{DT_FILE}} causes a data file to be written that contains information
on public transport lines. These can be
[processed further](../Tutorials/PT_from_OpenStreetMap.md#finding_feasible_stop-to-stop_travel_times_and_creating_pt_schedules)
to generate public transport schedules for simulating public transport
and intermodal traffic. This is done automatically when using the
[osmWebWizard tool](../Tutorials/OSMWebWizard.md)

The ptline data format is described below:

```
<ptLines>
    <ptLine id="0" name="M2: Alexanderplatz to Heinersdorf" line="M2"
      type="tram" period="1200" completeness="0.11">
        <busStop id="-1615531689" name="S+U Alexanderplatz/Dircksenstraße"/>
        <busStop id="30732068" name="Memhardstraße"/>
    </ptLine>
  ...
</ptLines>
```

The above describes a public transport line which serves two stops with
a **period** of 1200 seconds. The line name as well as the stop names
are optional and only serve to enhance human reader comprehension. The
*tram* **type** is one of the recognized public transport types from OSM
(other allowed types are
*train,subway,light_rail,monorail,trolleybus,aerialway,ferry*). The
**line** attribute will be used in the simulation when distinguishing
public transport lines. The optional **completeness** attribute serves to
inform the human reader that the stops only descibe 11% of the complete
line. Incomplete lines are typical when importing only a part of the
complete public transport network.

## Information on Joined Junctions

The option **--junctions.join-output** {{DT_FILE}} causes a file to be written that specifies the junctions
which were joined (usualy due to option **--junctions.join**). The resulting output file is
suitable for loading with the **--node-files** option.

## Street Signs

The option **--street-sign-output** {{DT_FILE}} causes a file with
[POIs](../Simulation/Shapes.md#poi_point_of_interest_definitions)
to be written. These POIs encode the type of street signs that are
encountered on each edge and can be loaded as {{AdditionalFile}} in
[sumo-gui](../sumo-gui.md). Currently used sign types are:

- priority
- yield
- stop
- allway_stop
- right_before_left

## Parking Areas

Currently, importing road-side [parking
areas](../Simulation/ParkingArea.md) from OSM is supported by
setting the option **----parking-output** {{DT_FILE}}

## OpenDRIVE road objects

When loading an OpenDRIVE file, [embedded road objects can be imported
as well.](../Networks/Import/OpenDRIVE.md#road_objects)

## Railway Topology

The option **--railway.topology.output** causes a file for analyzing the topology of railway networks
to be written. This is useful when analyzing network problems in regard
to bi-directional track usage and to evaluate the effect of option **--railway.topology.repair**.

## Additional Information within the output file

The option **--output.street-names** {{DT_BOOL}} ensures that street names from suitable input networks such
as [OSM](../Networks/Import/OpenStreetMap.md) or
[OpenDrive](../Networks/Import/OpenDRIVE.md) are included in the
generated *.net.xml* file.

When reading or writing OpenDrive networks, the option **--output.original-names** {{DT_BOOL}} [writtes
additional data for mapping between sumo-ids and OpenDrive-ids into the
generated
networks](../Networks/Import/OpenDRIVE.md#referencing_original_ids).
