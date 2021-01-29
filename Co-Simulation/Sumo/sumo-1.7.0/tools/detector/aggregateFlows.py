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

# @file    aggregateFlows.py
# @author  Michael Behrisch
# @date    2007-06-28

"""
This script aggregates flows as they come from the Datenverteiler
into files suitable for the dfrouter.
It takes as an optional argument a file listing the detectors of
interest (one per line, additional whitespace separated entries
do not hurt, for instance a bab_map.txt will do)
"""
from __future__ import absolute_import
from __future__ import print_function
import os
import sys
import zipfile
from optparse import OptionParser


class Entry:

    def __init__(self):
        self.carFlow = 0
        self.truckFlow = 0
        self.carSpeed = 0.0
        self.truckSpeed = 0.0

    def add(self, qCar, qTruck, vCar, vTruck):
        if qCar > 0:
            self.carSpeed = (
                self.carFlow * self.carSpeed + qCar * vCar) / (self.carFlow + qCar)
            self.carFlow += qCar
        if qTruck > 0:
            self.truckSpeed = (
                self.truckFlow * self.truckSpeed + qTruck * vTruck) / (self.truckFlow + qTruck)
            self.truckFlow += qTruck

    def __repr__(self):
        return str(self.carFlow) + ";" + str(self.truckFlow) + ";" + str(self.carSpeed) + ";" + str(self.truckSpeed)


def readLines(lines):
    for l in lines:
        flowDef = l.split()
        if not cityDets or flowDef[0] in cityDets:
            if not flowDef[0] in totalFlow:
                totalFlow[flowDef[0]] = Entry()
            totalFlow[flowDef[0]].add(
                int(flowDef[1]), int(flowDef[2]), float(flowDef[3]), float(flowDef[4]))


optParser = OptionParser(usage="usage: %prog [options] [flow.txt|flows.zip]+")
optParser.add_option("-d", "--det-file", dest="detfile",
                     help="read detectors of interest from FILE", metavar="FILE")
(options, args) = optParser.parse_args()
if len(args) == 0:
    optParser.print_help()
    sys.exit()
if options.detfile:
    cityDets = set()
    for line in open(options.detfile):
        cityDets.add(line.split()[0])
else:
    cityDets = None
totalFlow = {}
for f in args:
    if os.access(f, os.R_OK):
        if f.endswith(".zip"):
            zipf = zipfile.ZipFile(f)
            for fileName in zipf.namelist():
                readLines(zipf.read(fileName).splitlines())
            zipf.close()
        else:
            readLines(open(f))
    else:
        print("Cannot read", f, file=sys.stderr)
print("Detector;Time;qPKW;qLKW;vPKW;vLKW")
for det, flow in totalFlow.iteritems():
    print(det + ";0;" + str(flow))
