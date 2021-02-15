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

# @file    rilsa1.py
# @author  Daniel Krajzewicz
# @date    2014-09-01


from __future__ import absolute_import


from . import fileNeedsRebuild, Scenario
import os
import subprocess
import sumolib.net.generator.demand as demandGenerator
import sumolib


flowsRiLSA1 = [
    ["nmp", [
        ["ms", 359, 9],
        ["me", 59, 9],
        ["mw", 64, 12]
    ]],

    ["wmp", [
        ["me", 508, 10],
        ["mn", 80, 14],
        ["ms", 130, 2]
    ]],

    ["emp", [
        ["mw", 571, 10],
        ["mn", 57, 9],
        ["ms", 47, 3]
    ]],

    ["smp", [
        ["mn", 354, 2],
        ["me", 49, 2],
        ["mw", 92, 2]
    ]]

]


class Scenario_RiLSA1(Scenario):
    NAME = "RiLSA1"
    THIS_DIR = os.path.join(os.path.abspath(os.path.dirname(__file__)), NAME)
    TLS_FILE = os.path.join(THIS_DIR, "tls.add.xml")
    NET_FILE = os.path.join(THIS_DIR, "rilsa1.net.xml")

    def __init__(self, withDefaultDemand=True):
        Scenario.__init__(self, self.NAME)
        self.netName = self.fullPath(self.NET_FILE)
        self.demandName = self.fullPath("routes.rou.xml")
        # network
        if fileNeedsRebuild(self.netName, "netconvert"):
            netconvert = sumolib.checkBinary("netconvert")
            subprocess.call([netconvert, "-c", os.path.join(self.THIS_DIR, "build.netc.cfg")])
        # build the demand model (streams)
        if withDefaultDemand:
            self.demand = demandGenerator.Demand()
            for f in flowsRiLSA1:
                for rel in f[1]:
                    prob = rel[2] / 100.
                    iprob = 1. - prob
                    self.demand.addStream(demandGenerator.Stream(
                        f[0] + "__" + rel[0], 0, 3600, rel[1], f[0], rel[0], {"hdv": prob, "passenger": iprob}))
            if fileNeedsRebuild(self.demandName, "duarouter"):
                self.demand.build(0, 3600, self.netName, self.demandName)
