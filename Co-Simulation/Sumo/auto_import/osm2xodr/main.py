from math import floor, pi
import numpy as np
from OSMParser.testing import TestEntity, _test_nodes, testSimpleRoad, test_3WayTCrossing2
from OSMParser.osmParsing import parseAll,rNode, OSMWay,JunctionRoad, OSMWayEndcap, createOSMJunctionRoadLine, createOSMWayNodeList2XODRRoadLine
from OSMParser.xodrWriting import startBasicXODRFile,fillNormalRoads,fillJunctionRoads

osmPfad = 'map.osm'
topographieKartenPfad = 'topomap.png'
xodrPfad = 'output.xodr'

parseAll(osmPfad, bildpfad=topographieKartenPfad, minimumHeight = 163.0, maximumHeight= 192.0, curveRadius=12)

startBasicXODRFile(xodrPfad)
fillNormalRoads(xodrPfad)
fillJunctionRoads(xodrPfad)

