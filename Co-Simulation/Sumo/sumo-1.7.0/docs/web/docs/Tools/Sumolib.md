---
title: Tools/Sumolib
permalink: /Tools/Sumolib/
---

**sumolib** is a set of python modules for working with sumo networks,
simulation output and other simulation artifacts. For a detailed list of
available functions see the [pydoc generated
documentation](http://sumo.dlr.de/pydoc/sumolib.html). You can
[browse the code here](https://github.com/eclipse/sumo/tree/master/tools/sumolib).

# importing **sumolib** in a script

To use the library, the {{SUMO}}/tools directory must be on the python load
path. This is typically done with a stanza like this:

```
import os, sys
if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(tools)
else:   
    sys.exit("please declare environment variable 'SUMO_HOME'")
```

# usage examples

## import a network and retrieve nodes and edges

```
# import the library
import sumolib
# parse the net
net = sumolib.net.readNet('myNet.net.xml')

# retrieve the coordinate of a node based on its ID
print net.getNode('myNodeID').getCoord()

# retrieve the successor node ID of an edge
nextNodeID = net.getEdge('myEdgeID').getToNode().getID()
```

## compute the average edge speed in a *plain xml* edge file

```
speedSum = 0.0
edgeCount = 0
for edge in sumolib.output.parse('myNet.edg.xml', ['edge']):
    speedSum += float(edge.speed)
    edgeCount += 1
avgSpeed = speedSum / edgeCount
```

## compute the median speed using the [Statistics](http://sumo.dlr.de/pydoc/sumolib.miscutils.html#Statistics) module

```
edgeStats = sumolib.miscutils.Statistics("edge speeds")
for edge in sumolib.output.parse('myNet.edg.xml', ['edge']):
    edgeStats.add(float(edge.speed))
avgSpeed = edgeStats.median()
```

!!! note
    Attribute *speed* is optional in user-generated *.edg.xml* files but will always be included if that file was written by [netconvert](../netconvert.md) or [netedit](../netedit.md).

## locate nearby edges based on the geo-coordinate
This requires the module [pyproj](https://code.google.com/p/pyproj/) to be installed.
For larger networks [rtree](https://pypi.org/project/Rtree/) is also strongly recommended.

```
net = sumolib.net.readNet('myNet.net.xml')
radius = 0.1
x, y = net.convertLonLat2XY(lon, lat)
edges = net.getNeighboringEdges(x, y, radius)
# pick the closest edge
if len(edges) > 0:
    distancesAndEdges = sorted([(dist, edge) for edge, dist in edges])
    dist, closestEdge = distancesAndEdges[0]
```

## parse all edges in a route file

```
for route in sumolib.output.parse_fast("myRoutes.rou.xml", 'route', ['edges']):
    edge_ids = route.edges.split()
    # do something with the vector of edge ids
```

## parse all edges in a edge data (meanData) file

```
for interval in sumolib.output.parse("edgedata.xml", "interval"):
    for edge in interval.edge:    
        # do something with the edge attributes i.e. edge.entered
```

## coordinate transformations

```
net = sumolib.net.readNet('myNet.net.xml')

# network coordinates (lower left network corner is at x=0, y=0)
x, y = net.convertLonLat2XY(lon, lat)
lon, lat = net.convertXY2LonLat(x, y)

# raw UTM coordinates
x, y = net.convertLonLat2XY(lon, lat, True)
lon, lat = net.convertXY2LonLat(x, y, True)

# lane/offset coordinates
# from lane position to network coordinates
x,y = sumolib.geomhelper.positionAtShapeOffset(net.getLane(laneID).getShape(), lanePos)
# from network coordinates to lane position
lane = net.getNeighboringLanes(x, y, radius) (see "locate nearby edges based on the geo-coordinate" above)
lanePos, dist = sumolib.geomhelper.polygonOffsetAndDistanceToPoint((x,y), lane.getShape())
```

see also
[TraCI/Interfacing_TraCI_from_Python\#coordinate_transformations](../TraCI/Interfacing_TraCI_from_Python.md#coordinate_transformations)

# Further Examples

The *runner.py* files in the test subfolders of [{{SUMO}}/tests/tools/sumolib]({{Source}}tests/tools/sumolib) provide additional
examples for sumolib use.
