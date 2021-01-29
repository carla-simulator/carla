---
title: Tools/Shapes
permalink: /Tools/Shapes/
---

# pois2inductionLoops.py

Converts a PoIs within a given PoI-file which are located on lanes into
induction loop detectors. Each PoI is replicated to cover all lanes of
the road. The detectors are named "<POINAME\>__l<LANE_INDEX\>".

```
pois2inductionLoops.py <NET> <POIS> <OUTPUT>
```

- <NET\>: The net to use for retrieving lane numbers
- <POIS\>: The file containing the PoIs
- <OUTPUT\>: The file to write the detectors into

The tool uses the <SUMO\>/tools/lib/sumopoi.py library.

# poi_alongRoads.py

Spatial distribute of POIs along given edges on a given network.

So far POIs are situated in the middle on all edges without regard to
the type of the edge (street, junction). Edges may be given in arbitrary
order, connected edges are found automatically. Therefore: crossing
chains of connected edges are not allowed -\> this needs two different
runs of this script. Output is written in file 'pois.add.xml'

```
poi_alongRoads.py <NET> <EDGE_ID>[,<EDGE_ID>]* <DISTANCE>
```

- <NET\>: The net to use for retrieving the geometry
- <EDGE_ID\>\[,<EDGE_ID\>\]\*: The edges to go along
- <DISTANCE\>: Distance between PoIs

Edges are separated with comma and without spaces in between. The
distance between POIs may be any positive real number PoIs are stored
with type="default", color="1,0,0", and layer="0".

The tool uses the <SUMO\>/tools/lib/sumonet.py library.

# poi_atTLS.py

Spatial distribute of POIs along given edges on a given network.

Generates a PoI-file containing a PoI for each tls controlled
intersection from the given net.

```
poi_atTLS.py <NET> [nojoin]
```

- <NET\>: The net to read traffic light (tls) positions from
- \[nojoin\]: If "nojoin" is given, PoIs will be built on all nodes
  covered by traffic lights, otherwise, if a traffic light spans over
  multiple intersections, only one PoI will be built for this traffic
  lights, at the center of all intersections controlled by this
  traffic light

PoIs are stored with type="default", color="1,0,0", and layer="0".

The tool uses the <SUMO\>/tools/lib/sumonet.py library.

# circlePolygon.py

Generate circular polygons with custom radius, and number of vertices.
See **--help** for additional options.

```
<SUMO\>/tools/shape/ciclePolygon.py x,y,radius,points [x2,y2,radius2,points2] ....
```