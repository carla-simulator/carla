# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    basic_crossl.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2014-09-01

from __future__ import absolute_import
from __future__ import print_function

from . import fileNeedsRebuild, Scenario
import os
import sumolib.net.generator.cross as netGenerator
import sumolib.net.generator.demand as demandGenerator
from sumolib.net.generator.network import Edge, Lane


class Scenario_BasicCrossL(Scenario):
    NAME = "BasicCrossL"
    THIS_DIR = os.path.join(os.path.abspath(os.path.dirname(__file__)), NAME)
    TLS_FILE = os.path.join(THIS_DIR, "tls.add.xml")
    NET_FILE = os.path.join(THIS_DIR, "net.net.xml")

    def __init__(self, withDefaultDemand=True):
        Scenario.__init__(self, self.NAME)
        self.netName = self.fullPath(self.NET_FILE)
        self.demandName = self.fullPath("routes.rou.xml")
        # network
        if fileNeedsRebuild(self.netName, "netconvert"):
            print("Network in '%s' needs to be rebuild" % self.netName)
            defaultEdge = Edge(numLanes=1, maxSpeed=13.89)
            defaultEdge.addSplit(100, 1)
            defaultEdge.lanes = [Lane(dirs="rs"), Lane(dirs="l")]
            netGen = netGenerator.cross(None, defaultEdge)
            # not nice, the network name should be given/returned
            netGen.build(self.netName)
        # demand
        if withDefaultDemand:
            print("Demand in '%s' needs to be rebuild" % self.demandName)
            self.demand = demandGenerator.Demand()
            # why isn't it possible to get a network and return all possible
            # routes or whatever - to ease the process
            self.demand.addStream(demandGenerator.Stream(
                None, 0, 3600, 1000, "2/1_to_1/1", "1/1_to_0/1", {.2: "hdv", .8: "passenger"}))
            if fileNeedsRebuild(self.demandName, "duarouter"):
                self.demand.build(0, 3600, self.netName, self.demandName)
