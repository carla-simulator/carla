__all__ = ['TestEntity', 'test_3WayTCrossing2', 'testSimpleRoad']

                                    #Comes from 02_parsing.ipynb, cell
class TestEntity:
    def __init__(self, idn, tags, lon=0, lat=0, nodes=0):
        self.id = idn
        self.tags = tags
        self.lon = lon
        self.lat = lat
        self.nodes = nodes

                                    #Comes from 02_parsing.ipynb, cell
from .osmParsing import rNode, OSMWay, OSMPreWay, OSMWayEndcap, JunctionRoad
from .utils import convertTopoMap, setHeights, convertLongitudeLatitude
from .xodrWriting import startBasicXODRFile, fillNormalRoads, fillJunctionRoads
from .utils import giveHeading, distance,schnittpunkt,getXYPositionFromLineLength, getPositiveHeading,getDeltaHdg
from .arcCurves import giveHeading,getArcEndposition,distance,schnittpunkt,getArcCurvatureAndLength,getXYPositionFromLineLength,getArcCurvatureAndLength2Point,endTurn2LaneStreet
from .osmParsing import parseAll,rNode, OSMWay, OSMWayEndcap, createOSMJunctionRoadLine, createOSMWayNodeList2XODRRoadLine

def _test_nodes(nodes, ways):
    rNode.reset()
    OSMPreWay.reset()
    OSMWay.reset()
    JunctionRoad.reset()
    OSMWayEndcap.reset()
    global topoParameter
    setHeights(0.0, 100.0)
    osmPfad = 'map.osm'
    topographieKartenPfad = 'topomap.png'
    topoParameter = convertTopoMap(topographieKartenPfad, osmPfad)
    minlat = 999999.0
    maxlat = -999999.0
    minlon = 999999.0
    maxlon = -999999.0
    for entity in nodes:
                        if minlat > entity.lat:
                                minlat = entity.lat
                        if maxlat < entity.lat:
                                maxlat = entity.lat
                        if minlon > entity.lon:
                                minlon = entity.lon
                        if maxlon < entity.lon:
                                maxlon = entity.lon
    xmin,ymin = convertLongitudeLatitude(minlon,minlat)
    xmax,ymax = convertLongitudeLatitude(maxlon,maxlat)
    topoParameter =  [xmin, xmax, ymin, ymax]
    for entity in nodes:
        rNode(entity, substractMin=topoParameter)
    for entity in ways:
        OSMPreWay(entity)
    for preWay in OSMPreWay.allWays.values():
        preWay._evaluate()
    for node in rNode.allrNodes.values():
        for way in node.incomingWays:
            node.createConnections(way)
        for way in node.outgoingWays:
            node.createConnections(way)
        node.evaluateJunction()
    for node in rNode.allrNodes.values(): #createJunctionRoads
        node.evaluateJunction2()
    for way in OSMWay.allWays.values():
        way.roadElements, way.elevationElements = createOSMWayNodeList2XODRRoadLine(way)
    startBasicXODRFile()
    fillNormalRoads()
    fillJunctionRoads()
    print("New Outputfile written. Look into it to see if it is as expected.")

                                    #Comes from 02_parsing.ipynb, cell
def test_3WayTCrossing2():
    nodes = [TestEntity(1,None,lon=1, lat=1, nodes=0),
            TestEntity(2,None,lon=1, lat=1.001, nodes=0),
            TestEntity(3,None,lon=1.001, lat=1.001, nodes=0),
            TestEntity(4,None,lon=0.999, lat=1.001, nodes=0)]
    ways = [TestEntity(1,{"highway":"residential","lanes":"3", "lanes:forward":"2", "lanes:backward":"1","oneway":"no"}, nodes=[1,2]),
            TestEntity(2,{"highway":"residential","lanes":"2", "lanes:forward":"1", "lanes:backward":"1","oneway":"no"}, nodes=[2,3]),
            TestEntity(3,{"highway":"residential","lanes":"3", "lanes:forward":"2", "lanes:backward":"1","oneway":"no"}, nodes=[2,4])]
    _test_nodes(nodes, ways)

                                    #Comes from 02_parsing.ipynb, cell
def testSimpleRoad():
        nodes = [TestEntity(1,None,lon=0, lat=0, nodes=0),
            TestEntity(2,None,lon=0, lat=10.000, nodes=0),
            TestEntity(3,None,lon=10.000, lat=20.000, nodes=0),
            TestEntity(4,None,lon=20.000, lat=10.000, nodes=0),
            TestEntity(5,None,lon=20.000, lat=0.000, nodes=0)
            ]
        ways = [TestEntity(1,{"highway":"residential","lanes":"5", "lanes:forward":"3", "lanes:backward":"2","oneway":"no"}, nodes=[1,2,3,4,5])]
        _test_nodes(nodes, ways)
