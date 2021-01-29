#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2009-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    generateITetrisIntersectionMetrics.py
# @author  Daniel Krajzewicz
# @author  Lena Kalleske
# @author  Michael Behrisch
# @date    2007-10-25

from __future__ import absolute_import
from __future__ import print_function
from optparse import OptionParser
import os
import sys
from numpy import mean, log
from xml.sax import parse, handler
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import sumolib.net  # noqa


def getBasicStats(net, lanesInfo, T):
    tlsInfo = {}
    for tl in net._tlss:
        tlID = tl._id
        mQueueLen = []
#       mWaitTime = []
        nbStops = []
        tWaitTime = []
        seenLanes = set()
        for conn in tl._connections:
            lane = conn[0]
            if lane in seenLanes:
                continue
            seenLanes.add(lane)
            mQueueLenInfo = sum(lanesInfo[lane.getID()]['mQueueLen'])
            mQueueLen.append(mQueueLenInfo)
#           mWaitTimeInfo = mean(lanesInfo[lane.getID()]['mWaitTime'])
#           mWaitTime.append(mWaitTimeInfo)
            nbStopsInfo = sum(lanesInfo[lane.getID()]['nbStops'])
            nbStops.append(nbStopsInfo)
            tWaitTimeInfo = sum(lanesInfo[lane.getID()]['tWaitTime'])
            tWaitTime.append(tWaitTimeInfo)
        tlsInfo[tlID] = {}
        tlsInfo[tlID]['mQueueLen'] = mean(mQueueLen) / T
        tlsInfo[tlID]['mWaitTime'] = mean(tWaitTime) / T
        tlsInfo[tlID]['nbStops'] = sum(nbStops)
        tlsInfo[tlID]['tWaitTime'] = sum(tWaitTime)
    return tlsInfo


def mergeInfos(tlsInfoAll, tlsInfoOne, metric):
    for tl in tlsInfoOne.keys():
        tlsInfoAll[tl][metric] = tlsInfoOne[tl]


def getStatisticsOutput(tlsInfo, outputfile):
    opfile = open(outputfile, 'w')
    for tl in tlsInfo.keys():
        opfile.write('Traffic Light %s\n' % tl)
        opfile.write('=================\n')
        opfile.write(
            'mean queue length in front of the junction: %s\n' % tlsInfo[tl]['mQueueLen'])
        opfile.write(
            'mean waiting time in front of the junction: %s\n' % tlsInfo[tl]['mWaitTime'])
        if 'noise' in tlsInfo[tl]:
            opfile.write('mean noise emission: %s\n' % tlsInfo[tl]['noise'])
        if 'CO' in tlsInfo[tl]:
            opfile.write('mean CO emission: %s\n' % tlsInfo[tl]['CO'])
            opfile.write('mean CO2 emission: %s\n' % tlsInfo[tl]['CO2'])
            opfile.write('mean HC emission: %s\n' % tlsInfo[tl]['HC'])
            opfile.write('mean PMx emission: %s\n' % tlsInfo[tl]['PMx'])
            opfile.write('mean NOx emission: %s\n' % tlsInfo[tl]['NOx'])
            opfile.write('mean fuel consumption: %s\n' % tlsInfo[tl]['fuel'])
        opfile.write('number of stops: %s\n' % tlsInfo[tl]['nbStops'])
        opfile.write('total waiting time at junction: %s\n\n' %
                     tlsInfo[tl]['tWaitTime'])


def tlsIDToNodeID(net):
    tlsID2NodeID = {}
    for tls in net._tlss:
        tlsID = tls._id
        tlsID2NodeID[tlsID] = []
        seenNodes = set()
        for conn in tls._connections:
            lane = conn[0]
            edge = lane._edge
            node = edge._to
            nodeID = node._id
            if nodeID not in seenNodes:
                tlsID2NodeID[tlsID].append(nodeID)
                seenNodes.add(nodeID)
    return tlsID2NodeID


class E2OutputReader(handler.ContentHandler):

    def __init__(self):
        self._lanes = {}
        self._maxT = 0

    def startElement(self, name, attrs):
        if name == 'interval':
            detID = attrs['id']
            laneID = detID[6:len(detID)]
            if laneID not in self._lanes:
                self._lanes[laneID] = {}
                self._lanes[laneID]['mQueueLen'] = []
#               self._lanes[laneID]['mWaitTime'] = []
                self._lanes[laneID]['nbStops'] = []
                self._lanes[laneID]['tWaitTime'] = []
            if float(attrs['end']) < 100000000:
                self._lanes[laneID]['mQueueLen'].append(
                    float(attrs['jamLengthInMetersSum']))
#               self._lanes[laneID]['mWaitTime'].append(float(attrs['meanHaltingDuration']))
                self._lanes[laneID]['nbStops'].append(
                    float(attrs['startedHalts']))
                self._lanes[laneID]['tWaitTime'].append(
                    float(attrs['haltingDurationSum']))
                self._maxT = max(float(attrs['end']), self._maxT)


class HarmonoiseReader(handler.ContentHandler):

    def __init__(self, net, tlsID2NodeID):
        self._nodeIntervalNoise = {}
        self._maxT = 0
        self._net = net
        self._tlsNoise = {}
        self._tlsID2NodeID = tlsID2NodeID

    def startElement(self, name, attrs):
        if name == 'interval':
            self._maxT = max(float(attrs['end']), self._maxT)
        if name == 'edge':
            edgeID = attrs['id']
            noiseStr = attrs['noise']
            if len(noiseStr) < 10:
                noise = float(noiseStr)
            else:
                noise = 0
            if edgeID[0] == ':':
                nodeID = edgeID[1:edgeID.find('_')]
                if nodeID not in self._nodeIntervalNoise:
                    self._nodeIntervalNoise[nodeID] = []
                self._nodeIntervalNoise[nodeID].append(noise)
            else:
                fromNodeID = net.getEdge(edgeID)._from._id
                if fromNodeID not in self._nodeIntervalNoise:
                    self._nodeIntervalNoise[fromNodeID] = []
                self._nodeIntervalNoise[fromNodeID].append(noise)
                toNodeID = net.getEdge(edgeID)._to._id
                if toNodeID not in self._nodeIntervalNoise:
                    self._nodeIntervalNoise[toNodeID] = []
                self._nodeIntervalNoise[toNodeID].append(noise)

    def endElement(self, name):
        if name == 'interval':
            self.sumIntervalNoise()
        if name == 'netstats':
            self.sumNoise()

    def sumIntervalNoise(self):
        for tls in net._tlss:
            sum = 0
            tlsID = tls._id
            if tlsID not in self._tlsNoise:
                self._tlsNoise[tlsID] = []
            for nodeID in self._tlsID2NodeID[tlsID]:
                for noise in self._nodeIntervalNoise[nodeID]:
                    sum = sum + pow(10, noise / 10)
            self._tlsNoise[tlsID].append(10 * log(sum) / log(10))

    def sumNoise(self):
        for tls in net._tlss:
            tlsID = tls._id
            self._tlsNoise[tlsID] = sum(self._tlsNoise[tlsID]) / self._maxT


class HBEFAReader(handler.ContentHandler):

    def __init__(self, net, tlsID2NodeID):
        self._maxT = 0
        self._net = net
        self._nodeIntervalCO = {}
        self._nodeIntervalCO2 = {}
        self._nodeIntervalHC = {}
        self._nodeIntervalPMx = {}
        self._nodeIntervalNOx = {}
        self._nodeIntervalfuel = {}
        self._tlsCO = {}
        self._tlsCO2 = {}
        self._tlsHC = {}
        self._tlsPMx = {}
        self._tlsNOx = {}
        self._tlsfuel = {}
        self._tlsID2NodeID = tlsID2NodeID

    def startElement(self, name, attrs):
        if name == 'interval':
            self._maxT = max(float(attrs['end']), self._maxT)
        if name == 'edge':
            edgeID = attrs['id']
            CO = float(attrs['CO_perVeh'])
            CO2 = float(attrs['CO2_perVeh'])
            HC = float(attrs['HC_perVeh'])
            PMx = float(attrs['PMx_perVeh'])
            NOx = float(attrs['NOx_perVeh'])
            fuel = float(attrs['fuel_perVeh'])
            if edgeID[0] == ':':
                nodeIDs = edgeID[1:edgeID.find('_')]
            else:
                fromNodeID = net.getEdge(edgeID)._from._id
                toNodeID = net.getEdge(edgeID)._to._id
                nodeIDs = [fromNodeID, toNodeID]
            for nodeID in nodeIDs:
                if nodeID not in self._nodeIntervalCO:
                    self._nodeIntervalCO[nodeID] = []
                    self._nodeIntervalCO2[nodeID] = []
                    self._nodeIntervalHC[nodeID] = []
                    self._nodeIntervalPMx[nodeID] = []
                    self._nodeIntervalNOx[nodeID] = []
                    self._nodeIntervalfuel[nodeID] = []
                self._nodeIntervalCO[nodeID].append(CO)
                self._nodeIntervalCO2[nodeID].append(CO2)
                self._nodeIntervalHC[nodeID].append(HC)
                self._nodeIntervalPMx[nodeID].append(PMx)
                self._nodeIntervalNOx[nodeID].append(NOx)
                self._nodeIntervalfuel[nodeID].append(fuel)

    def endElement(self, name):
        if name == 'interval':
            self.sumInterval()
        if name == 'netstats':
            self.sum()

    def sumInterval(self):
        for tls in net._tlss:
            tlsID = tls._id
            if tlsID not in self._tlsCO:
                self._tlsCO[tlsID] = []
                self._tlsCO2[tlsID] = []
                self._tlsHC[tlsID] = []
                self._tlsPMx[tlsID] = []
                self._tlsNOx[tlsID] = []
                self._tlsfuel[tlsID] = []
            sum = 0
            for nodeID in self._tlsID2NodeID[tlsID]:
                for v in self._nodeIntervalCO[nodeID]:
                    sum = sum + v
            self._tlsCO[tlsID].append(sum)
            sum = 0
            for nodeID in self._tlsID2NodeID[tlsID]:
                for v in self._nodeIntervalCO2[nodeID]:
                    sum = sum + v
            self._tlsCO2[tlsID].append(sum)
            sum = 0
            for nodeID in self._tlsID2NodeID[tlsID]:
                for v in self._nodeIntervalHC[nodeID]:
                    sum = sum + v
            self._tlsHC[tlsID].append(sum)
            sum = 0
            for nodeID in self._tlsID2NodeID[tlsID]:
                for v in self._nodeIntervalPMx[nodeID]:
                    sum = sum + v
            self._tlsPMx[tlsID].append(sum)
            sum = 0
            for nodeID in self._tlsID2NodeID[tlsID]:
                for v in self._nodeIntervalNOx[nodeID]:
                    sum = sum + v
            self._tlsNOx[tlsID].append(sum)
            sum = 0
            for nodeID in self._tlsID2NodeID[tlsID]:
                for v in self._nodeIntervalfuel[nodeID]:
                    sum = sum + v
            self._tlsfuel[tlsID].append(sum)

    def sum(self):
        for tls in net._tlss:
            tlsID = tls._id
            self._tlsCO[tlsID] = sum(self._tlsCO[tlsID]) / self._maxT
            self._tlsCO2[tlsID] = sum(self._tlsCO2[tlsID]) / self._maxT
            self._tlsHC[tlsID] = sum(self._tlsHC[tlsID]) / self._maxT
            self._tlsPMx[tlsID] = sum(self._tlsPMx[tlsID]) / self._maxT
            self._tlsNOx[tlsID] = sum(self._tlsNOx[tlsID]) / self._maxT
            self._tlsfuel[tlsID] = sum(self._tlsfuel[tlsID]) / self._maxT


# initialise
optParser = OptionParser()
optParser.add_option("-n", "--netfile", dest="netfile",
                     help="name of the netfile (f.e. 'inputs\\pasubio\\a_costa.net.xml')", metavar="<FILE>",
                     type="string")
optParser.add_option("-p", "--path", dest="path",
                     help="name of folder to work with (f.e. 'inputs\\a_costa\\')", metavar="<FOLDER>",
                     type="string", default="./")
optParser.add_option("-o", "--harmonoiseFile", dest="harmonoiseFile",
                     help="name of the harmonoise file", metavar="<FOLDER>", type="string")
optParser.add_option("-e", "--HBEFAFile", dest="hbefaFile",
                     help="name of the HBEFA file", metavar="<FOLDER>", type="string")


optParser.set_usage(
    '\n-n inputs\\pasubio\\pasubio.net.xml -p inputs\\pasubio\\')
# parse options
(options, args) = optParser.parse_args()
if not options.netfile:
    print("Missing arguments")
    optParser.print_help()
    exit()

netfile = options.netfile
e2OutputFile = os.path.join(options.path, 'e2_output.xml')

net = sumolib.net.readNet(netfile)

e2Output = E2OutputReader()
parse(e2OutputFile, e2Output)

tlsID2NodeID = tlsIDToNodeID(net)
tlsInfo = getBasicStats(net, e2Output._lanes, e2Output._maxT)

if options.harmonoiseFile:
    harmonoiseOutput = HarmonoiseReader(net, tlsID2NodeID)
    parse(options.harmonoiseFile, harmonoiseOutput)
    mergeInfos(tlsInfo, harmonoiseOutput._tlsNoise, 'noise')

if options.hbefaFile:
    hbefaOutput = HBEFAReader(net, tlsID2NodeID)
    parse(options.hbefaFile, hbefaOutput)
    mergeInfos(tlsInfo, hbefaOutput._tlsCO, 'CO')
    mergeInfos(tlsInfo, hbefaOutput._tlsCO2, 'CO2')
    mergeInfos(tlsInfo, hbefaOutput._tlsHC, 'HC')
    mergeInfos(tlsInfo, hbefaOutput._tlsPMx, 'PMx')
    mergeInfos(tlsInfo, hbefaOutput._tlsNOx, 'NOx')
    mergeInfos(tlsInfo, hbefaOutput._tlsfuel, 'fuel')

getStatisticsOutput(
    tlsInfo, os.path.join(options.path, "intersection_metrics_summary.txt"))

print('The calculation is done!')
