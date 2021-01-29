#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2007-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    detector.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2007-06-28

from __future__ import absolute_import
from __future__ import print_function
import os
import sys
from collections import defaultdict
from xml.sax import make_parser, handler

MAX_POS_DEVIATION = 10


class LaneMap:
    def get(self, key, default):
        return key[0:-2]


def relError(actual, expected):
    if expected == 0:
        if actual == 0:
            return 0
        else:
            return 1
    else:
        return (actual - expected) / expected


def parseFlowFile(flowFile, detCol="Detector", timeCol="Time", flowCol="qPKW", speedCol="vPKW", begin=None, end=None):
    detIdx = -1
    flowIdx = -1
    speedIdx = -1
    timeIdx = -1
    with open(flowFile) as f:
        for l in f:
            if ';' not in l:
                continue
            flowDef = [e.strip() for e in l.split(';')]
            if detIdx == -1 and detCol in flowDef:
                # init columns
                detIdx = flowDef.index(detCol)
                if flowCol in flowDef:
                    flowIdx = flowDef.index(flowCol)
                if speedCol in flowDef:
                    speedIdx = flowDef.index(speedCol)
                if timeCol in flowDef:
                    timeIdx = flowDef.index(timeCol)
            elif flowIdx != -1:
                # columns are initialized
                if timeIdx == -1 or begin is None:
                    curTime = None
                    timeIsValid = True
                else:
                    curTime = float(flowDef[timeIdx])
                    timeIsValid = (end is None and curTime == begin) or (
                        curTime >= begin and curTime < end)
                if timeIsValid:
                    speed = float(flowDef[speedIdx]) if speedIdx != -1 else None
                    yield (flowDef[detIdx], curTime, float(flowDef[flowIdx]), speed)


class DetectorGroupData:

    def __init__(self, pos, isValid, id=None, detType=None):
        self.ids = []
        self.pos = pos
        self.isValid = isValid
        self.totalFlow = 0
        self.avgSpeed = 0
        self.entryCount = 0
        self.type = detType
        if id is not None:
            self.ids.append(id)
        self.begin = 0
        self.lastTime = None
        self.interval = None
        self.timeline = []

    def addDetFlow(self, flow, speed):
        oldFlow = self.totalFlow
        self.totalFlow += flow
        if flow > 0 and speed is not None:
            self.avgSpeed = (
                self.avgSpeed * oldFlow + speed * flow) / self.totalFlow
        self.entryCount += 1

    def addDetFlowTime(self, time, flow, speed):
        if self.interval is None:
            raise RuntimeError("DetectorGroupData interval not initialized")
        time -= self.begin
        index = int(time / self.interval)
        if index > len(self.timeline):
            if len(self.timeline) == 0:
                # init
                self.timeline = [[None, None] for i in range(index)]
                self.timeline.append([0, 0])
            else:
                sys.stderr.write(("Gap in data for group=%s. Or data interval is higher than aggregation interval " +
                                  "(i=%s, time=%s, begin=%s, lastTime=%s)\n") % (
                    self.ids, self.interval, time, self.begin, len(self.timeline) * self.interval))
                while len(self.timeline) < index:
                    self.timeline.append([None, None])
                self.timeline.append([0, 0])
        if index == len(self.timeline):
            # new entry
            if time % self.interval != 0 and time > self.interval:
                sys.stderr.write(("Aggregation interval is not a multiple of data interval for group=%s (i=%s " +
                                  "time=%s begin=%s)\n") % (
                    self.ids, self.interval, time, self.begin))
            self.timeline.append([0, 0])
        oldFlow, oldSpeed = self.timeline[index]
        newFlow = oldFlow + flow
        if flow > 0 and speed is not None:
            newSpeed = (oldSpeed * oldFlow + speed * flow) / newFlow
        else:
            newSpeed = oldSpeed
        self.timeline[index] = [newFlow, newSpeed]
        self.entryCount += 1

    def clearFlow(self, begin, interval):
        self.totalFlow = 0
        self.avgSpeed = 0
        self.entryCount = 0
        self.begin = begin
        self.lastTime = None
        self.interval = interval
        self.timeline = []

    def getName(self, longName, firstName, sep='|'):
        if firstName:
            return self.ids[0]
        name = os.path.commonprefix(self.ids)
        if name == "" or longName:
            name = sep.join(sorted(self.ids))
        return name


class DetectorReader(handler.ContentHandler):

    def __init__(self, detFile=None, laneMap={}):
        self._edge2DetData = defaultdict(list)
        self._det2edge = {}
        self._currentGroup = None
        self._currentEdge = None
        self._laneMap = laneMap
        if detFile:
            parser = make_parser()
            parser.setContentHandler(self)
            parser.parse(detFile)

    def addDetector(self, id, pos, edge, detType):
        if id in self._det2edge:
            print("Warning! Detector %s already known." % id, file=sys.stderr)
            return
        if edge is None:
            raise RuntimeError("Detector '%s' has no edge" % id)
        if self._currentGroup:
            self._currentGroup.ids.append(id)
        else:
            haveGroup = False
            for data in self._edge2DetData[edge]:
                if abs(data.pos - pos) <= MAX_POS_DEVIATION:
                    data.ids.append(id)
                    haveGroup = True
                    break
            if not haveGroup:
                self._edge2DetData[edge].append(
                    DetectorGroupData(pos, True, id, detType))
        self._det2edge[id] = edge

    def getEdgeDetGroups(self, edge):
        return self._edge2DetData[edge]

    def startElement(self, name, attrs):
        if name == 'detectorDefinition' or name == 'e1Detector' or name == 'inductionLoop':
            detType = attrs['type'] if 'type' in attrs else None
            self.addDetector(attrs['id'], float(attrs['pos']),
                             self._laneMap.get(attrs['lane'], self._currentEdge), detType)
        elif name == 'group':
            self._currentGroup = DetectorGroupData(float(attrs['pos']),
                                                   attrs.get('valid', "1") == "1")
            edge = attrs['orig_edge']
            self._currentEdge = edge
            self._edge2DetData[edge].append(self._currentGroup)

    def endElement(self, name):
        if name == 'group':
            self._currentGroup = None

    def getGroup(self, det):
        if det in self._det2edge:
            edge = self._det2edge[det]
            for group in self._edge2DetData[edge]:
                if det in group.ids:
                    return group
        return None

    def addFlow(self, det, flow, speed=0.0):
        group = self.getGroup(det)
        if group is not None:
            group.addDetFlow(flow, speed)

    def clearFlows(self, begin=0, interval=None):
        for groupList in self._edge2DetData.values():
            for group in groupList:
                group.clearFlow(begin, interval)

    def readFlows(self, flowFile, det="Detector", flow="qPKW", speed=None, time=None, timeVal=None, timeMax=None):
        values = parseFlowFile(
            flowFile,
            detCol=det, timeCol=time, flowCol=flow,
            speedCol=speed, begin=timeVal, end=timeMax)
        hadFlow = False
        for det, time, flow, speed in values:
            hadFlow = True
            self.addFlow(det, flow, speed)
        return hadFlow

    def readFlowsTimeline(self, flowFile, interval, **args):
        values = parseFlowFile(flowFile, **args)
        hadFlow = False
        for det, time, flow, speed in values:
            hadFlow = True
            group = self.getGroup(det)
            if group is not None:
                group.addDetFlowTime(time, flow, speed)
        return hadFlow

    def findTimes(self, flowFile, tMin, tMax, det="Detector", time="Time"):
        timeIdx = 1
        with open(flowFile) as f:
            for l in f:
                if ';' not in l:
                    continue
                flowDef = [e.strip() for e in l.split(';')]
                if det in flowDef:
                    if time in flowDef:
                        timeIdx = flowDef.index(time)
                elif len(flowDef) > timeIdx:
                    curTime = float(flowDef[timeIdx])
                    if tMin is None or tMin > curTime:
                        tMin = curTime
                    if tMax is None or tMax < curTime:
                        tMax = curTime
        return tMin, tMax

    def getGroups(self):
        for edge, detData in self._edge2DetData.items():
            for group in detData:
                if group.isValid:
                    yield edge, group
