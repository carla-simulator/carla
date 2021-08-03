__all__ = ['rNode', 'OSMPreWay', 'OSMWayEndcap', 'OSMWay', 'parseAll', 'createOSMWayNodeList2XODRRoadLine',
           'createEndCap', 'JunctionRoad', 'createOSMJunctionRoadLine']

#Cell
import uuid
from .utils import createVirtualLastPointForJunctionRoads, convertLongitudeLatitude, giveHeading, convertTopoMap, giveMaxMinLongLat, getDeltaHdg, giveHeight, distance,getXYPositionFromLineLength, setHeights
from .arcCurves import getArcCurvatureAndLength2Point, getArcCurvatureAndLength, endTurn2LaneStreet
import numpy as np
from osmread import parse_file, Way, Node
import copy
import pathlib


#Cell
class rNode:
    allrNodes = {}
    elementID = 0

    @staticmethod
    def giveNextElementID():
        rNode.elementID += 1
        return str(rNode.elementID)

    @staticmethod
    def reset():
        rNode.allrNodes = {}
        rNode.elementID = 0

    def __init__(self,entity, register = True, debug=False, substractMin=None):
        if debug:
            self.id = str(uuid.uuid1())
            if register: rNode.allrNodes[self.id] = self
            return
        self.id = str(entity.id)
        if register: rNode.allrNodes[self.id] = self
        self.tags = entity.tags
        try:  self.x,self.y = convertLongitudeLatitude(entity.lon, entity.lat)
        except: self.x,self.y = entity.x, entity.y
        if substractMin is not None:
            self.x -= substractMin[0]
            self.y -= substractMin[2]
        try: self.height = giveHeight(self.x, self.y, minRemoved=True)
        except: self.height = 0.0

        self.Junction = ""
        self.JunctionRoads = []
        self.wayList = []
        self._PreWayIdList = []

        self.incomingWays = []   # in der regel 1
        self.incomingrNodes = []
        self.incomingLanes = []
        self.incomingLanesOpposite = []
        self.incomingTurnTags = []

        self.outgoingLanes = []
        self.outgoingLanesOpposite = []
        self.outgoingLanesOppositeTurnTags = []
        self.outgoingrNodes = []
        self.outgoingWays = []

        self.Connections = {}


    def _givePossibleTurnIdxs(self, Way):
        '''Gives the Indexes of the registered Ways with >0 outgoing Lanes'''
        turnIdxToIncoming = []
        turnIdxToOutgoing = []
        if Way in self.incomingWays or Way in self.outgoingWays:
            for incIdx in range(len(self.incomingWays)):
                if Way != self.incomingWays[incIdx]:     # no U-Turn
                    if self.incomingLanesOpposite[incIdx] > 0:  # no Turning in One-Way Streets
                        turnIdxToIncoming.append(incIdx)
            for outIdx in range(len(self.outgoingWays)):
                if Way != self.outgoingWays[outIdx]:     # no U-Turn
                    if self.outgoingLanes[outIdx] > 0:  # no Turning in One-Way Streets
                        turnIdxToOutgoing.append(outIdx)
        return turnIdxToIncoming, turnIdxToOutgoing

    def giveTurnPossibilities(self, incomingWay):
        '''Gives the Angles, Lanes, Ways, rNodes and Directions of all valid Turns from a Way as an incoming Way'''
        turnsInc = []
        turnsOut = []
        selfHeading = 0
        if incomingWay in self.incomingWays: # no turn possibilities for a not really incoming way (way with no incoming lanes)
            if self.incomingLanes[self.incomingWays.index(incomingWay)] == 0:
                return{'Angles':[],'rNodes': [],'Lanes': [],'Ways': [],'WayDirection': []}
        elif incomingWay in self.outgoingWays:
            if self.outgoingLanesOpposite[self.outgoingWays.index(incomingWay)] == 0:
                return{'Angles':[],'rNodes': [],'Lanes': [],'Ways': [],'WayDirection': []}

        incIdx, outIdx = self._givePossibleTurnIdxs(incomingWay)
        if incomingWay in self.incomingWays:
            selfincrNode = self.incomingrNodes[self.incomingWays.index(incomingWay)]
        elif incomingWay in self.outgoingWays:
            selfincrNode = self.outgoingrNodes[self.outgoingWays.index(incomingWay)]
        selfHeading = giveHeading(selfincrNode.x, selfincrNode.y, self.x, self.y) # heading von "incoming" Node to self
        for i in range(len(incIdx)):
            nodeHeading = giveHeading(self.x, self.y, self.incomingrNodes[incIdx[i]].x, self.incomingrNodes[incIdx[i]].y)
            turn = getDeltaHdg(selfHeading,nodeHeading)
            turnsInc.append(turn)
        for i in range(len(outIdx)):
            nodeHeading = giveHeading(self.x, self.y, self.outgoingrNodes[outIdx[i]].x, self.outgoingrNodes[outIdx[i]].y)
            turn = getDeltaHdg(selfHeading,nodeHeading)
            turnsOut.append(turn)
        return {'Angles':turnsInc+turnsOut,
                'rNodes': [self.incomingrNodes[i] for i in incIdx]+[self.outgoingrNodes[i] for i in outIdx],
                'Lanes': [self.incomingLanesOpposite[i] for i in incIdx]+[self.outgoingLanes[i] for i in outIdx],
                'Ways': [self.incomingWays[i] for i in incIdx]+[self.outgoingWays[i] for i in outIdx],
                'WayDirection': [False]*len(turnsInc)+[True]*len(turnsOut)}

    def createConnections(self, Way):
        '''Creates Laneconnections ([Lane, successorLane]) of the way for all successors and stores them in self.Connections[Way][Successorway].
        The Laneconnections are already adjusted for Waydirection'''
        # check if way is incoming or outgoing and get the incoming lanes as well as the index
        positiveIncLanes = True
        lanenumbers = 0
        wayIdx = -1
        if Way in self.incomingWays:
            positiveIncLanes = False
            wayIdx = self.incomingWays.index(Way)
            lanenumbers = self.incomingLanes[wayIdx]
        else:
            wayIdx = self.outgoingWays.index(Way)
            lanenumbers = self.outgoingLanesOpposite[wayIdx]
        turnPossibilities = self.giveTurnPossibilities(Way)
        sortangles = copy.copy(turnPossibilities['Angles'])
        sortidx = sorted(range(len(sortangles)), key=lambda k: sortangles[k])
        sortangles.sort()
        if lanenumbers == 0 or sum(turnPossibilities['Lanes']) == 0:
            return
        wayConnections = {}
        #sort all turnPossibilities according to angles
        for key in turnPossibilities.keys():
            tmp = []
            for i in range(len(turnPossibilities[key])):
                tmp.append(turnPossibilities[key][sortidx[i]])
            turnPossibilities[key] = tmp

        for idx in range(len(turnPossibilities['Angles'])):
            lanesum = 0
            for i in range(idx):     # get the lanenumber of the incoming lane
                lanesum += turnPossibilities['Lanes'][i]
                if lanesum > lanenumbers:
                    lanesum = lanenumbers-1      #lanes, die bisher verbraucht wurden

            # ist noch platz für die outgoinglanes?
            if lanesum+turnPossibilities['Lanes'][idx] > lanenumbers:
                lanesum = max(0,lanesum-turnPossibilities['Lanes'][idx])
            laneConnections = []
            for i in range(turnPossibilities['Lanes'][idx]):   #lanes, die hier outgoing sind
                if lanesum+i+1 > lanenumbers: # more lanes to turn into in one Possibility than incoming lanes
                    if len(sortangles) == 1: # merging and splitting Lanes - all Lanes should be accessible
                        lanesum -= 1
                    else:
                        break   # turning into a main street - only use outer lane
                # create Connection
                if positiveIncLanes:  # Way is in OutgoingWays
                    laneConnections.append([lanesum+i+1, -i-1 if turnPossibilities['WayDirection'][idx] else i+1])
                else:
                    laneConnections.append([-lanesum-i-1, -i-1 if turnPossibilities['WayDirection'][idx] else i+1])
            # extra merging lanes
            if turnPossibilities['Lanes'][idx] < lanenumbers and len(self.wayList)==2:
                for i in range(lanenumbers-turnPossibilities['Lanes'][idx]):
                    # create Connection
                    if positiveIncLanes:  # Way is in OutgoingWays
                        laneConnections.append([turnPossibilities['Lanes'][idx]+i+1, -turnPossibilities['Lanes'][idx] if turnPossibilities['WayDirection'][idx] else turnPossibilities['Lanes'][idx]])
                    else:
                        laneConnections.append([-turnPossibilities['Lanes'][idx]-i-1, -turnPossibilities['Lanes'][idx] if turnPossibilities['WayDirection'][idx] else turnPossibilities['Lanes'][idx]])

            wayConnections[(turnPossibilities['Ways'][idx]).id] = laneConnections
        if positiveIncLanes: # Way is in OutgoingWays
            try: self.Connections[Way.id]["Opposite"] =  wayConnections
            except:
                self.Connections[Way.id] = {}
                self.Connections[Way.id]["Opposite"] =  wayConnections
        else:
            try: self.Connections[Way.id]["Direction"] =  wayConnections
            except:
                self.Connections[Way.id] = {}
                self.Connections[Way.id]["Direction"] =  wayConnections

    def evaluateJunction(self):
        if len(self.wayList)>1:
            self.Junction = rNode.giveNextElementID()
            jrxs = []
            jrys = []
            jx,jy = [self.x, self.y]
            nodes = self.incomingrNodes + self.outgoingrNodes
            for node in nodes:
                if len(node.wayList) > 1: #junction to junction -> do not go beyond half of the way
                    jrxs.append((node.x+self.x)/2.0)
                    jrys.append((node.y+self.y)/2.0)
                else:
                    jrxs.append(node.x)
                    jrys.append(node.y)
            maxlanes = max(self.incomingLanes+self.outgoingLanes)
            radius = 4.0 * maxlanes
            self.lastPoints = createVirtualLastPointForJunctionRoads(jx,jy,jrxs,jrys,radius = radius)
            for way in self.wayList:
                if self.id == way.OSMNodes[0]:
                    way.startJunction = self.Junction
                else:
                    way.endJunction = self.Junction

    def evaluateJunction2(self):
        #get all way connections in direction relevant from wayX to wayY
        laneconnections = {}
        for wayX in self.Connections.keys():
            wayXisIncoming = True if OSMWay.allWays[wayX].OSMNodes[-1] == self.id else False
            for direc in self.Connections[wayX].keys():
                if wayXisIncoming and direc == "Opposite": continue
                if not wayXisIncoming and direc == "Direction": continue
                for wayY in self.Connections[wayX][direc].keys():
                    CName = wayX+"_"+wayY# if wayX < wayY else wayY+"_"+wayX
                    if CName in laneconnections:
                        dic = laneconnections[CName]
                    else:
                        laneconnections[CName] = {}
                        dic = laneconnections[CName]
                    for connection in self.Connections[wayX][direc][wayY]:
                        startlane = connection[0]# if wayX < wayY else connection[1]
                        endlane = connection[1]# if wayX < wayY else connection[0]
                        if startlane in dic:
                            dic[startlane].append(endlane)
                        else:
                            dic[startlane] = [endlane]
        #create a road for every connection
        for key in laneconnections.keys():
            keys = key.split("_")
            predecessorway = OSMWay.allWays[keys[0]]
            successorway = OSMWay.allWays[keys[1]]
            self.JunctionRoads += JunctionRoad.createJunctionRoadsForConnection(predecessorway,successorway,self)

    def getRelevantLastPoint(self, way):
        ways = self.incomingWays + self.outgoingWays
        wayidx = ways.index(way)
        return self.lastPoints[wayidx]

    @staticmethod
    def _connectionID(Way,Way2):
        if str(Way) < str(Way2):
            return str(Way)+'#'+str(Way2)
        else:
            return str(Way2)+'#'+str(Way)


#Cell
class OSMPreWay:
    allWays = {}

    @staticmethod
    def reset():
        OSMPreWay.allWays = {}

    def __init__(self,entity, register = True, debug=False):
        if debug:
            self.id = str(uuid.uuid1())
            if register:
                OSMPreWay.allWays[self.id] = self
            return
        self.id = str(entity.id)
        if register:
            OSMPreWay.allWays[self.id] = self
        self.tags = entity.tags
        self.rNodes = []

        for node in entity.nodes:
            if str(node) not in rNode.allrNodes:
                continue
            self.rNodes.append(rNode.allrNodes[str(node)].id)
        if len(self.rNodes) > 1:
            for node in entity.nodes:
                   rNode.allrNodes[str(node)]._PreWayIdList.append(str(self.id))

    def _evaluate(self):
        startIdx = 0
        endIdx = -1
        if len(self.rNodes) < 2:
            return
        lastIdx = -1
        for rNodeId in self.rNodes:
            node = rNode.allrNodes[rNodeId]
            idx = self.rNodes.index(rNodeId)
            if idx < lastIdx: #straße geht im Kreis - es wurde die vorherige node gefunden
                if startIdx == lastIdx: # letzte straße war ein neubeginn
                    idx = lastIdx+1  # es hat eh eine neue straße angefangen
                else:
                    #create fake mergeRoad
                    idx = lastIdx
                    endIdx = idx+1
                    OSMWay(self.id, self.tags, self.rNodes[startIdx:endIdx], self.rNodes[startIdx], self.rNodes[endIdx-1])
                    startIdx = idx
                    idx = lastIdx+1
            if len(node._PreWayIdList) > 1:    # dies wird eine Junction - Weg muss gesplittet werden
                if idx == startIdx or idx == endIdx-1:
                    continue
                else:
                    endIdx = idx+1
                    OSMWay(self.id, self.tags, self.rNodes[startIdx:endIdx], self.rNodes[startIdx], self.rNodes[endIdx-1])
                    startIdx = idx
            lastIdx = idx
        if endIdx < len(self.rNodes):
            endIdx = len(self.rNodes)
            OSMWay(self.id, self.tags, self.rNodes[startIdx:endIdx], self.rNodes[startIdx], self.rNodes[endIdx-1])


#Cell
class OSMWayEndcap:
    allCaps = {}
    def reset():
        OSMWayEndcap.allCaps = {}
    def __init__(self,way, roadLineElements, roadElevationElements, isStartPoint = True):
        self.id = way.id + "_start" if isStartPoint else way.id + "_end"
        self.Junction = rNode.giveNextElementID()
        if isStartPoint:
            way.startJunction = self.Junction
        else:
            way.endJunction = self.Junction
        OSMWayEndcap.allCaps[self.id] = self
        self.roadLineElements = roadLineElements
        self.roadElevationElements = roadElevationElements
        self.isStartPoint = isStartPoint
        self.lanesEnd = way.laneNumberDirection if self.isStartPoint else way.laneNumberOpposite
        self.lanesStart = way.laneNumberOpposite if self.isStartPoint else way.laneNumberDirection
        self.way = way
        self.JunctionRoads = []
        startLane = 0
        endLane = 0
        #all startlanes have to go somewhere
        for i in range(self.lanesStart):
            startLane += 1
            endLane +=1
            if endLane > self.lanesEnd:
                endLane = self.lanesEnd
            self.JunctionRoads.append(JunctionRoad(way,way,startLane if isStartPoint else -startLane,-endLane if isStartPoint else endLane,
                         self,"start" if isStartPoint else "end","start" if isStartPoint else "end",
                         self.roadLineElements,self.roadElevationElements))
        #there may be some unused endlanes....
        if self.lanesEnd - self.lanesStart > 0:
            for i in range(self.lanesEnd - self.lanesStart):
                endLane += 1
                self.JunctionRoads.append(JunctionRoad(way,way,startLane if isStartPoint else -startLane,-endLane if isStartPoint else endLane,
                         self,"start" if isStartPoint else "end","start" if isStartPoint else "end",
                         self.roadLineElements,self.roadElevationElements))


#Cell
class OSMWay:
    allWays = {}
    idNr = 0

    @staticmethod
    def reset():
        OSMWay.allWays = {}
        OSMWay.idNr = 0

    @staticmethod
    def giveID():
        OSMWay.idNr += 1
        return OSMWay.idNr

    def __init__(self,OSMid, tags, OSMNodes, StartrNode, EndrNode, register = True, debug=False):

        self.id = str(OSMWay.giveID())
        self.xodrID = rNode.giveNextElementID()
        if debug:
            if register:
                OSMWay.allWays[self.id] = self
            return
        self.OSMId = OSMid
        if register:
            OSMWay.allWays[self.id] = self
        self.tags = tags
        self.OSMNodes = OSMNodes

        self.laneNumberDirection = -1
        self.laneNumberOpposite = -1

        self.K1rNode = EndrNode   # end
        self.K2rNode = StartrNode   # start
        self.startJunction = ""
        self.endJunction = ""
        self.K1Links = []
        self.K2Links = []
        self.lastPoints = []
        self.K1_turnLanesDirection = []
        self.K1_ConnectionsTurnLanesDirection = []
        self.K1_incomingLanesFromK1 = []
        self.K2_turnLanesOpposite = []
        self.K2_ConnectionsTurnLanesOpposite = []
        self.K2_incomingLanesFromK2 = []
        self.roadElements = []
        self.elevationElements = []
        self.checkLanes()
        previousrNode = None
        if len(self.OSMNodes) > 1:
            for nodeid in self.OSMNodes:
                node = rNode.allrNodes[nodeid]
                node.wayList.append(self)
                if previousrNode is not None:
                    previousrNode.outgoingrNodes.append(node)
                    previousrNode.outgoingWays.append(self)
                    previousrNode.outgoingLanes.append(self.laneNumberDirection)
                    previousrNode.outgoingLanesOpposite.append(self.laneNumberOpposite)
                    previousrNode.outgoingLanesOppositeTurnTags.append(self.K2_turnLanesOpposite)
                    node.incomingrNodes.append(previousrNode)
                    node.incomingWays.append(self)
                    node.incomingLanes.append(self.laneNumberDirection)
                    node.incomingLanesOpposite.append(self.laneNumberOpposite)
                    node.incomingTurnTags.append(self.K1_turnLanesDirection)
                    if previousrNode:
                        if len(previousrNode.wayList) > 1:
                            assert len(previousrNode.wayList) == len(previousrNode.incomingWays)+len(previousrNode.outgoingWays)
                    if len(node.wayList) > 1:
                        assert len(node.wayList) == len(node.incomingWays)+len(node.outgoingWays)
                previousrNode = node

        self.prepareConnections()

    def prepareConnections(self):
        if len(self.K1_turnLanesDirection) < self.laneNumberDirection:
            self.K1_turnLanesDirection = [""]*self.laneNumberDirection
        if len(self.K2_turnLanesOpposite) < self.laneNumberOpposite:
            self.K2_turnLanesOpposite = [""]*self.laneNumberOpposite

        for i in range(self.laneNumberDirection):
            #self.K1_turnLanesDirection.append([])
            self.K2_incomingLanesFromK2.append([])
            self.K1_ConnectionsTurnLanesDirection.append([])
        for i in range(self.laneNumberOpposite):
            self.K1_incomingLanesFromK1.append([])
            #self.K2_turnLanesOpposite.append([])
            self.K2_ConnectionsTurnLanesOpposite.append([])

    def checkLanes(self):
        '''
        checks how many Lanes this street should have
        '''
        #laneNumberDirection und laneNumberOpposite sind die groben Uebersichten.
        laneNumberDirection = -1
        laneNumberOpposite = -1
        self.K1_turnLanesDirection = []
        self.K2_turnLanesOpposite = []
        lanes = -1
        oneWay = False
        try:
            if 'yes' in self.tags["oneway"]:
                oneWay = True
                #print("oneway found")
        except:  pass

        try:
            lanes = int(self.tags["lanes"])
            #print("lanes found")
        except: pass
        try:
            laneNumberDirection = int(self.tags["lanes:forward"])
            #print("lanes:forward found")
        except: pass
        try:
            laneNumberOpposite = int(self.tags["lanes:backward"])
            #print("lanes:backward found")
        except: pass
        try: self.K1_turnLanesDirection = self.tags["turn:lanes:forward"].replace("slight_left","slight_l").replace("slight_right","slight_r").replace("merge_to_right","merge_r").replace("merge_to_left", "merge_l").split("|")
        except:
            try: self.K1_turnLanesDirection = self.tags["turn:lanes"].replace("slight_left","slight_l").replace("slight_right","slight_r").replace("merge_to_right","merge_r").replace("merge_to_left", "merge_l").split("|")
            except: pass
        try:self.K2_turnLanesOpposite = self.tags["turn:lanes:backward"].replace("slight_left","slight_l").replace("slight_right","slight_r").replace("merge_to_right","merge_r").replace("merge_to_left", "merge_l").split("|")
        except: pass
        if lanes > 0 and laneNumberDirection + laneNumberOpposite == lanes:  #best case
            #print("all clear")
            self.laneNumberDirection = laneNumberDirection
            self.laneNumberOpposite = laneNumberOpposite
        if lanes > 0 and oneWay:
            laneNumberOpposite = 0
            laneNumberDirection = lanes
            #print("all clear")
            self.laneNumberDirection = laneNumberDirection
            self.laneNumberOpposite = laneNumberOpposite
            return
        if laneNumberDirection > 0 and oneWay:
            #print("all clear")
            lanes = laneNumberDirection
            self.laneNumberDirection = laneNumberDirection
            self.laneNumberOpposite = laneNumberOpposite
            return
        if laneNumberDirection > 0 and laneNumberOpposite>0:
            #print("all clear")
            lanes = laneNumberDirection + laneNumberOpposite
            self.laneNumberDirection = laneNumberDirection
            self.laneNumberOpposite = laneNumberOpposite
            return
        if (len(self.K1_turnLanesDirection) > 0 or len(self.K2_turnLanesOpposite) > 0) and lanes == -1:
            lanes = len(self.K1_turnLanesDirection) + len(self.K2_turnLanesOpposite)
            self.laneNumberDirection = len(self.K1_turnLanesDirection)
            self.laneNumberOpposite = len(self.K2_turnLanesOpposite)
            return
        if lanes > 0 and laneNumberDirection >= 0:
            laneNumberOpposite = lanes - laneNumberDirection
        if lanes > 0 and laneNumberOpposite >= 0:
            laneNumberDirection = lanes - laneNumberOpposite
            self.laneNumberDirection = laneNumberDirection
            self.laneNumberOpposite = laneNumberOpposite
            return
        if lanes == -1:
            lanes = 1 if oneWay else 2
        laneNumberDirection = lanes if oneWay else 1
        laneNumberOpposite = 0 if oneWay else 1
        if len(self.K1_turnLanesDirection) > 0:
            laneNumberDirection = len(self.K1_turnLanesDirection)
            laneNumberOpposite = lanes-laneNumberDirection
        if len(self.K2_turnLanesOpposite) > 0:
            laneNumberOpposite = len(self.K2_turnLanesOpposite)
            laneNumberDirection = lanes-laneNumberOpposite
        self.laneNumberDirection = laneNumberDirection
        self.laneNumberOpposite = laneNumberOpposite


#Cell
def parseAll(pfad, bildpfad = None, minimumHeight = 0.0, maximumHeight = 100.0, curveRadius=8):
    global topoParameter
    setHeights(minimumHeight, maximumHeight)
    if bildpfad is not None:
        topoParameter = convertTopoMap(bildpfad, pfad)
    else:
        topoParameter = convertTopoMap(None, pfad)
    #create rNodedict with counter
    for entity in parse_file(pfad):
        if isinstance(entity, Node):
            #if minLongitude <entity.lon< maxLongitude and minLatitude <entity.lat< maxLatitude:   # approximate longitude and latitude of Wuppertal
                 rNode(entity, substractMin=topoParameter)
    #create streetrNodedict and count rNodeuse
    for entity in parse_file(pfad):
        if isinstance(entity, Way):
            for word in ["highway"]:#, "lanes", "oneway", "cycleway", "foot", "sidewalk",  "footway"]:
                if word in entity.tags and not "stairs" in entity.tags["highway"] and not "steps" in entity.tags["highway"] and not  "pedestrian" in entity.tags["highway"] and not "elevator" in entity.tags["highway"] and not "footway" in entity.tags["highway"] and not "bridleway" in entity.tags["highway"] and not "cycleway" in entity.tags["highway"] and not "path" in entity.tags["highway"]:
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
    #for node in rNode.allrNodes.values():
    #    node.createOpenDriveElements(r=curveRadius)
    #    node.createOpenDriveLanesAndInternalRoadConnections()
    #for node in rNode.allrNodes.values():
    #    node.connectOpenDriveLanes()

#Cell
def createOSMWayNodeList2XODRRoadLine(way, maxerror=2.0):
    Points = []
    hdgs = []
    RoadElements = [] #xstart,ystart, length, heading, curvature
    ElevationElements = []

    #prepare raw points
    #first element:
    firstNode = rNode.allrNodes[way.OSMNodes[0]]
    if len(firstNode.wayList) == 1:  #firstnode ist sackgasse
        #createEndCap
        createEndCap(way,[firstNode.x,firstNode.y],giveHeading(rNode.allrNodes[way.OSMNodes[1]].x,rNode.allrNodes[way.OSMNodes[1]].y,firstNode.x,firstNode.y),isStartPoint = True)
        #get the full node involved
        Points.append([firstNode.x,firstNode.y, giveHeight(firstNode.x,firstNode.y,minRemoved = True)])
    else: #firstnode is junction
        #get the relevant lastPoint as NodePoint
        x,y = firstNode.getRelevantLastPoint(way)
        Points.append([x,y,giveHeight(x,y,minRemoved = True)])

    #middle element:
    for nodeId in way.OSMNodes[1:-1]:
        node = rNode.allrNodes[nodeId]
        hdgs.append(giveHeading(Points[-1][0],Points[-1][1],node.x,node.y))
        Points.append([node.x,node.y, giveHeight(node.x,node.y,minRemoved = True)])


    #last element:
    lastNode = rNode.allrNodes[way.OSMNodes[-1]]
    if len(lastNode.wayList) == 1:  #firstnode ist sackgasse
        #createEndCap()
        createEndCap(way,[lastNode.x,lastNode.y],giveHeading(Points[-1][0],Points[-1][1],lastNode.x,lastNode.y),isStartPoint = False)
        #get the full node involved
        hdgs.append(giveHeading(Points[-1][0],Points[-1][1],lastNode.x,lastNode.y))
        Points.append([lastNode.x,lastNode.y, giveHeight(lastNode.x,lastNode.y,minRemoved = True)])

    else: #lastnode is junction
        #get the relevant lastPoint as NodePoint
        x,y = lastNode.getRelevantLastPoint(way)
        Points.append([x,y,giveHeight(x,y,minRemoved = True)])
        hdgs.append(giveHeading(x,y,lastNode.x,lastNode.y))

    if len(Points) == 2:
        #junction to junction -> Points can be the same!
        length = distance(Points[0][0], Points[0][1],Points[1][0],Points[1][1])
        x1,y1,z1 = Points[0]
        x2,y2,z2 = Points[1]
        RoadElements.append({"xstart":x1,"ystart":y1, "length":length, "heading":giveHeading(firstNode.x,firstNode.y,lastNode.x,lastNode.y), "curvature":0.0})
        ElevationElements.append({"xstart":x1,"ystart":y1,"zstart":z1,"steigung":(z2-z1)/(length+0.00000001),"length":length})
    else: #mehr als 1 Punkt auf dem Weg
        for i in range(len(Points)-2):
            x1,y1,z1 = Points[i]
            x2,y2,z2 = Points[i+1]
            x3,y3,z3 = Points[i+2] #hdgs sind automatisch korrekt bei 3 point curves
            #for all but the first and last Point: get the Point halfway between x1/x2 and x2/x3
            if i == 0:
                pass
            else:
                x1 = (x1+x2)/2.0
                y1 = (y1+y2)/2.0
                z1 = giveHeight(x1,y1,minRemoved=True)
            if i == len(Points)-3:
                pass
            else:
                x3 = (x3+x2)/2.0
                y3 = (y3+y2)/2.0
                z3 = giveHeight(x3,y3,minRemoved=True)
            #calculate the parameter
            xarc,yarc,xendline,yendline,curvature,length = getArcCurvatureAndLength(x1,y1,x3,y3,x2,y2, maxerror = maxerror, minradius = 0.5, iterations = 10)

            if distance(x1,y1,xarc,yarc) > 0.1:
                RoadElements.append({"xstart":x1,"ystart":y1, "length":distance(x1,y1,xarc,yarc), "heading":hdgs[i], "curvature":0.0})
                ElevationElements.append({"xstart":x1,"ystart":y1,"zstart":z1,
                                          "steigung":(giveHeight(xarc,yarc,minRemoved=True)-z1)/distance(x1,y1,xarc,yarc),"length":distance(x1,y1,xarc,yarc)})
            RoadElements.append({"xstart":xarc,"ystart":yarc, "length":length, "heading":hdgs[i], "curvature":curvature})
            ElevationElements.append({"xstart":xarc,"ystart":yarc,"zstart":giveHeight(xarc,yarc,minRemoved=True),
                                      "steigung":(giveHeight(xendline,yendline,minRemoved=True)-giveHeight(xarc,yarc,minRemoved=True))/length,"length":length})
            if distance(xendline,yendline,x3,y3) > 0.1:
                RoadElements.append({"xstart":xendline,"ystart":yendline, "length":distance(xendline,yendline,x3,y3), "heading":giveHeading(xendline,yendline,x3,y3), "curvature":0.0})
                ElevationElements.append({"xstart":xendline,"ystart":yendline,"zstart":giveHeight(xendline,yendline,minRemoved=True),
                                          "steigung":(z3-giveHeight(xendline,yendline,minRemoved=True))/distance(xendline,yendline,x3,y3),"length":distance(xendline,yendline,x3,y3)})
    #unite same curvatures
    '''condensedSomething = True
    while(condensedSomething):
        CondensedRoadElements = RoadElements if len(RoadElements)==1 else []
        CondensedElevationElements = ElevationElements if len(ElevationElements)==1 else []
        condensedSomething = False
        i = 0
        while i < len(RoadElements)-1:

            if abs(RoadElements[i]["curvature"] - RoadElements[i+1]["curvature"])*(RoadElements[i]["length"]+RoadElements[i+1]["length"])< 0.1 and abs(RoadElements[i]["curvature"] - RoadElements[i+1]["curvature"]) < 0.001:
                condensedSomething = True
                CondensedRoadElements.append({"xstart":RoadElements[i]["xstart"],"ystart":RoadElements[i]["ystart"],
                                              "length":RoadElements[i]["length"]+RoadElements[i+1]["length"], "heading":RoadElements[i]["heading"], "curvature":RoadElements[i]["curvature"]})
                i += 1
                #print("Condensed something")
            else:
                CondensedRoadElements.append({"xstart":RoadElements[i]["xstart"],"ystart":RoadElements[i]["ystart"],
                                              "length":RoadElements[i]["length"], "heading":RoadElements[i]["heading"], "curvature":RoadElements[i]["curvature"]})
                if i == len(RoadElements)-2:
                    CondensedRoadElements.append({"xstart":RoadElements[i+1]["xstart"],"ystart":RoadElements[i+1]["ystart"],
                                              "length":RoadElements[i+1]["length"], "heading":RoadElements[i+1]["heading"], "curvature":RoadElements[i+1]["curvature"]})
            i += 1
        i = 0
        while i < len(ElevationElements)-1:
            if abs(ElevationElements[i]["steigung"]-ElevationElements[i+1]["steigung"])*(ElevationElements[i]["length"]+ElevationElements[i+1]["length"])<0.1 and abs(ElevationElements[i]["steigung"]-ElevationElements[i+1]["steigung"]) < 0.01:
                condensedSomething = True
                CondensedElevationElements.append({"xstart":ElevationElements[i]["xstart"],"ystart":ElevationElements[i]["ystart"],"zstart":ElevationElements[i]["zstart"],
                                          "steigung":ElevationElements[i]["steigung"],"length":ElevationElements[i]["length"]+ElevationElements[i+1]["length"]})
                i += 1
                #print("Condensed something")
            else:
                CondensedElevationElements.append({"xstart":ElevationElements[i]["xstart"],"ystart":ElevationElements[i]["ystart"],"zstart":ElevationElements[i]["zstart"],
                                          "steigung":ElevationElements[i]["steigung"],"length":ElevationElements[i]["length"]})
                if i == len(ElevationElements)-2:
                    CondensedElevationElements.append({"xstart":ElevationElements[i+1]["xstart"],"ystart":ElevationElements[i+1]["ystart"],"zstart":ElevationElements[i+1]["zstart"],
                                          "steigung":ElevationElements[i+1]["steigung"],"length":ElevationElements[i+1]["length"]})
            i += 1
        ElevationElements = CondensedElevationElements
        RoadElements = CondensedRoadElements'''
    return RoadElements,ElevationElements

#Cell
def createEndCap(way,point,hdg,isStartPoint = True):
    lineDic = endTurn2LaneStreet(point[0],point[1], hdg)
    roadLineElements = []
    roadElevationElements = []
    #beginningArc
    roadLineElements.append({"xstart":point[0],"ystart":point[1], "length":lineDic["BeginningArcCurvatureLength"][1], "heading":hdg, "curvature":lineDic["BeginningArcCurvatureLength"][0]})
    roadElevationElements.append({"xstart":point[0],"ystart":point[1],"zstart":giveHeight(point[0],point[1],minRemoved=True),
                                      "steigung":(giveHeight(lineDic['ArcStartCoordinatesXY'][0],lineDic['ArcStartCoordinatesXY'][1],minRemoved=True)-giveHeight(point[0],point[1],minRemoved=True))/lineDic["BeginningArcCurvatureLength"][1],"length":lineDic["BeginningArcCurvatureLength"][1]})
    #arc
    roadLineElements.append({"xstart":lineDic['ArcStartCoordinatesXY'][0],"ystart":lineDic['ArcStartCoordinatesXY'][1], "length":lineDic['ArcCurvatureLength'][1], "heading":lineDic['BeginningArcEndXYHdg'][2], "curvature":lineDic['ArcCurvatureLength'][0]})
    roadElevationElements.append({"xstart":lineDic['ArcStartCoordinatesXY'][0],"ystart":lineDic['ArcStartCoordinatesXY'][1],"zstart":giveHeight(lineDic['ArcStartCoordinatesXY'][0],lineDic['ArcStartCoordinatesXY'][1],minRemoved=True),
                                      "steigung":(giveHeight(lineDic['EndlineStartCoordinatesXY'][0],lineDic['EndlineStartCoordinatesXY'][1],minRemoved=True)-giveHeight(lineDic['ArcStartCoordinatesXY'][0],lineDic['ArcStartCoordinatesXY'][1],minRemoved=True))/lineDic["ArcCurvatureLength"][1],"length":lineDic["ArcCurvatureLength"][1]})

    #endarc
    roadLineElements.append({"xstart":lineDic['EndArcBeginningXYHdgCurvatureLength'][0],"ystart":lineDic['EndArcBeginningXYHdgCurvatureLength'][1], "length":lineDic['EndArcBeginningXYHdgCurvatureLength'][4], "heading":lineDic['EndArcBeginningXYHdgCurvatureLength'][2], "curvature":lineDic['EndArcBeginningXYHdgCurvatureLength'][3]})
    roadElevationElements.append({"xstart":lineDic['EndArcBeginningXYHdgCurvatureLength'][0],"ystart":lineDic['EndArcBeginningXYHdgCurvatureLength'][1],"zstart":giveHeight(lineDic['EndArcBeginningXYHdgCurvatureLength'][0],lineDic['EndArcBeginningXYHdgCurvatureLength'][1],minRemoved=True),
                                      "steigung":(giveHeight(point[0],point[1],minRemoved=True)-giveHeight(lineDic['EndArcBeginningXYHdgCurvatureLength'][0],lineDic['EndArcBeginningXYHdgCurvatureLength'][1],minRemoved=True))/lineDic["EndArcBeginningXYHdgCurvatureLength"][4],"length":lineDic["EndArcBeginningXYHdgCurvatureLength"][4]})
    OSMWayEndcap(way, roadLineElements, roadElevationElements, isStartPoint = isStartPoint)

#Cell
class JunctionRoad:
    junctionNodes = {}
    @staticmethod
    def reset():
        JunctionRoad.junctionNodes = {}
    @staticmethod
    def giveJunctionDict(junctionNode):
        if junctionNode.Junction in JunctionRoad.junctionNodes:
            return JunctionRoad.junctionNodes[junctionNode.Junction]
        else:
            JunctionRoad.junctionNodes[junctionNode.Junction] = {}
            return JunctionRoad.junctionNodes[junctionNode.Junction]
    @staticmethod
    def createJunctionRoadsForConnection(predecessorway,successorway,junctionNode, maxerror=2.0):
        contactPointPredecessor = "start" if predecessorway.OSMNodes[0] == junctionNode.id else "end"
        contactPointSuccessor = "start" if successorway.OSMNodes[0] == junctionNode.id else "end"
        roadElements,elevationElements = createOSMJunctionRoadLine(predecessorway,successorway,junctionNode, maxerror=maxerror)
        predecessor2successorLaneConnections = []
        try:
            predecessorconnections = junctionNode.Connections[predecessorway.id]
            try:
                if successorway.id in predecessorconnections['Direction']:
                    for connection in predecessorconnections['Direction'][successorway.id]:
                        predecessor2successorLaneConnections.append(connection)
            except: pass
            try:
                if successorway.id in predecessorconnections['Opposite']:
                    for connection in predecessorconnections['Opposite'][successorway.id]:
                        predecessor2successorLaneConnections.append(connection)
            except: pass
        except: pass
        try:
            successorconnections = junctionNode.Connections[successorway.id]
            try:
                if predecessorway.id in successorconnections['Direction']:
                    for connection in successorconnections['Direction'][predecessorway.id]:
                        self.predecessor2successorLaneConnections.append([connection[1],connectionn[0]])
            except: pass
            try:
                if predecessorway.id in successorconnections['Opposite']:
                    for connection in successorconnections['Opposite'][predecessorway.id]:
                        predecessor2successorLaneConnections.append([connection[1],connectionn[0]])
            except: pass
        except: pass
        roads = []
        for connection in predecessor2successorLaneConnections:
            roads.append(JunctionRoad(predecessorway,successorway,connection[0],connection[1],junctionNode,contactPointPredecessor,contactPointSuccessor,roadElements,elevationElements))
        return roads

    def __init__(self,predecessorway,successorway,startlane,endlane,junctionNode,contactPointPredecessor,contactPointSuccessor,roadElements,elevationElements):
        self.id = str(predecessorway.id)+"_to_"+str(successorway.id)+"_lane_"+str(startlane)+"_to_"+str(endlane)
        self.xodrID = str(rNode.giveNextElementID())
        junctionDict = JunctionRoad.giveJunctionDict(junctionNode)
        if (str(predecessorway.id)+"_to_"+str(successorway.id)) in junctionDict:
            waydic = junctionDict[str(predecessorway.id)+"_to_"+str(successorway.id)]
        else:
            waydic = {}
            junctionDict[str(predecessorway.id)+"_to_"+str(successorway.id)] = waydic
        waydic[str(startlane)+"_to_"+str(endlane)] = self
        self.predecessorlane = startlane
        self.successorlane = endlane
        self.junctionNode = junctionNode
        self.contactPointPredecessor = contactPointPredecessor
        self.contactPointSuccessor = contactPointSuccessor
        self.roadElements = roadElements
        self.elevationElements = elevationElements
        self.laneWidth = 4.0
        length = 0.0
        for element in self.roadElements:
            length += element["length"]
        predecessorIsBackward = True if self.contactPointPredecessor == "start" else False
        successorIsBackward = True if self.contactPointSuccessor == "end" else False
        self.laneOffsetA = (abs(self.predecessorlane)-1.0)* np.sign(self.predecessorlane) * self.laneWidth
        laneOffsetEnd = (abs(self.successorlane)-1.0)* np.sign(self.successorlane) * self.laneWidth
        self.laneOffsetB = -(self.laneOffsetA-laneOffsetEnd)/length


#Cell
def createOSMJunctionRoadLine(way1,way2,junctionNode, maxerror=2.0):

    x1,y1 = junctionNode.getRelevantLastPoint(way1)
    x2 = junctionNode.x
    y2 = junctionNode.y
    x3,y3 = junctionNode.getRelevantLastPoint(way2)
    #calculate the parameter
    xarc,yarc,xendline,yendline,curvature,length = getArcCurvatureAndLength(x1,y1,x3,y3,x2,y2, maxerror = 999999.9, minradius = 0.5, iterations = 10)
    RoadElements = [] #xstart,ystart, length, heading, curvature
    ElevationElements = []
    z1 = giveHeight(x1,y1,minRemoved = True)
    z2 = giveHeight(x2,y2,minRemoved = True)
    z3 = giveHeight(x3,y3,minRemoved = True)
    if distance(x1,y1,xarc,yarc) > 0.1:
                RoadElements.append({"xstart":x1,"ystart":y1, "length":distance(x1,y1,xarc,yarc), "heading":giveHeading(x1,y1,x2,y2), "curvature":0.0})
                ElevationElements.append({"xstart":x1,"ystart":y1,"zstart":z1,
                                          "steigung":(giveHeight(xarc,yarc,minRemoved=True)-z1)/distance(x1,y1,xarc,yarc),"length":distance(x1,y1,xarc,yarc)})
    RoadElements.append({"xstart":xarc,"ystart":yarc, "length":length, "heading":giveHeading(x1,y1,x2,y2), "curvature":curvature})
    ElevationElements.append({"xstart":xarc,"ystart":yarc,"zstart":giveHeight(xarc,yarc,minRemoved=True),
                                      "steigung":(giveHeight(xendline,yendline,minRemoved=True)-giveHeight(xarc,yarc,minRemoved=True))/length,"length":length})
    if distance(xendline,yendline,x3,y3) > 0.1:
                RoadElements.append({"xstart":xendline,"ystart":yendline, "length":distance(xendline,yendline,x3,y3), "heading":giveHeading(xendline,yendline,x3,y3), "curvature":0.0})
                ElevationElements.append({"xstart":xendline,"ystart":yendline,"zstart":giveHeight(xendline,yendline,minRemoved=True),
                                          "steigung":(z3-giveHeight(xendline,yendline,minRemoved=True))/distance(xendline,yendline,x3,y3),"length":distance(xendline,yendline,x3,y3)})
    return RoadElements,ElevationElements