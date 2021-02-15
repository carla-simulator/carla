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

# @file    basic_rilsanet.py
# @author  Daniel Krajzewicz
# @date    2014-09-01

from __future__ import absolute_import
from __future__ import print_function


from . import fileNeedsRebuild, Scenario
import os
import shutil
import sumolib.net.generator.grid as netGenerator
import sumolib.net.generator.demand as demandGenerator
from sumolib.net.generator.network import Edge, Lane


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


class Scenario_BasicRiLSANet(Scenario):
    NAME = "BasicRiLSANet"
    THIS_DIR = os.path.join(os.path.abspath(os.path.dirname(__file__)), NAME)
    TLS_FILE = "tls.add.xml"
    NET_FILE = "network.net.xml"

    def __init__(self, name, params, withDefaultDemand=True):
        Scenario.__init__(self, self.NAME)
        self.params = params
        self.demandName = self.fullPath("routes.rou.xml")
        self.netName = self.fullPath(self.NET_FILE)
        # network
        if fileNeedsRebuild(self.fullPath(self.NET_FILE), "netconvert"):
            print("Network in '%s' needs to be rebuild" % self.netName)
            # , Lane(dirs="l", disallowed="pedestrian")]
            lanes = [Lane(dirs="s", allowed="pedestrian"), Lane(
                dirs="rs", disallowed="pedestrian")]
            defaultEdge = Edge(numLanes=2, maxSpeed=13.89, lanes=lanes)
            defaultEdge.addSplit(100, 0, 1)
            defaultEdge.lanes[-1].dirs = "l"
            netGen = netGenerator.grid(3, 3, None, defaultEdge)
            for n in netGen._nodes:
                nid = n.split("/")
                nid[0] = int(nid[0])
                nid[1] = int(nid[1])
                if nid[0] > 0 and nid[0] < 4 and nid[1] > 0 and nid[1] < 4:
                    netGen._nodes[n].addCrossing(
                        "%s/%s_to_%s.-100" % (nid[0] - 1, nid[1], n), "%s_to_%s/%s" % (n, nid[0] - 1, nid[1]))
                    netGen._nodes[n].addCrossing(
                        "%s/%s_to_%s.-100" % (nid[0] + 1, nid[1], n), "%s_to_%s/%s" % (n, nid[0] + 1, nid[1]))
                    netGen._nodes[n].addCrossing(
                        "%s/%s_to_%s.-100" % (nid[0], nid[1] - 1, n), "%s_to_%s/%s" % (n, nid[0], nid[1] - 1))
                    netGen._nodes[n].addCrossing(
                        "%s/%s_to_%s.-100" % (nid[0], nid[1] + 1, n), "%s_to_%s/%s" % (n, nid[0], nid[1] + 1))
            # not nice, the network name should be given/returned
            netGen.build(self.netName)
            fdow = open(self.fullPath(self.TLS_FILE), "w")
            fdow.write('<additional>\n\n')
            for y in range(1, 4):
                for x in range(1, 4):
                    eedge = "%s/%s_to_%s/%s.-100" % (x - 1, y, x, y)
                    wedge = "%s/%s_to_%s/%s.-100" % (x + 1, y, x, y)
                    nedge = "%s/%s_to_%s/%s.-100" % (x, y + 1, x, y)
                    sedge = "%s/%s_to_%s/%s.-100" % (x, y - 1, x, y)
                    fdow.write(
                        '   <tlLogic id="%s/%s" type="static" programID="adapted" offset="0">\n' % (x, y))
                    fdow.write(
                        ('      <phase duration="31" state="rrrrrGGgrrrrrGGgGrGr" minDur="10" maxDur="50" ' +
                         'type="target;decisional" targetLanes="%s_1 %s_2 %s_1 %s_2"/>\n') % (
                            eedge, eedge, wedge, wedge))
                    fdow.write(
                        '      <phase duration="4"  state="rrrrryygrrrrryygrrrr" type="transient"/>\n')
                    fdow.write(
                        ('      <phase duration="6"  state="rrrrrrrGrrrrrrrGrrrr" minDur="2" maxDur="20" ' +
                         'type="decisional" targetLanes="%s_2 %s_2"/>\n') % (eedge, wedge))
                    fdow.write(
                        '      <phase duration="4"  state="rrrrrrryrrrrrrryrrrr" type="transient"/>\n')
                    fdow.write(
                        '      <phase duration="4"  state="rrrrrrrrrrrrrrrrrrrr" type="transient;commit"/>\n')
                    fdow.write(
                        ('      <phase duration="31" state="rGGgrrrrrGGgrrrrrGrG" minDur="10" maxDur="50" ' +
                         'type="target;decisional" targetLanes="%s_1 %s_2 %s_1 %s_2"/>\n') % (
                            sedge, sedge, nedge, nedge))
                    fdow.write(
                        '      <phase duration="4"  state="ryygrrrrryygrrrrrrrr" type="transient"/>\n')
                    fdow.write(
                        ('      <phase duration="6"  state="rrrGrrrrrrrGrrrrrrrr" minDur="2" maxDur="20" ' +
                         'type="decisional" targetLanes="%s_2 %s_2"/>\n') % (sedge, nedge))
                    fdow.write(
                        '      <phase duration="4"  state="ryryrrrrryryrrrrrrrr" type="transient"/>\n')
                    fdow.write(
                        '      <phase duration="4"  state="rrrrrrrrrrrrrrrrrrrr" type="transient;commit"/>\n')
                    fdow.write('   </tlLogic>\n\n')
            fdow.write('</additional>\n')
        # demand
        if withDefaultDemand:
            self.demand = demandGenerator.Demand()
            for f in flowsRiLSA1:
                for oe, rel in enumerate(f[1]):
                    flow = int(rel[1] * .75)
                    prob = rel[2] / 100.
                    iprob = 1. - prob
                    pkwEprob = iprob * self.params["equipment-rate"]
                    pkwNprob = iprob - pkwEprob
                    lkwEprob = prob * self.params["equipment-rate"]
                    lkwNprob = prob - lkwEprob
                    for ie in range(1, 4):  # over input
                        via = []
                        if f[0] == "nmp":
                            iedge = "%s/4_to_%s/3" % (ie, ie)
                            for ve in range(4, 0, -1):
                                via.append("%s/%s_to_%s/%s" %
                                           (ie, ve, ie, ve - 1))
                        if f[0] == "smp":
                            iedge = "%s/0_to_%s/1" % (ie, ie)
                            for ve in range(0, 4, 1):
                                via.append("%s/%s_to_%s/%s" %
                                           (ie, ve, ie, ve + 1))
                        if f[0] == "wmp":
                            iedge = "0/%s_to_1/%s" % (ie, ie)
                            for ve in range(0, 4, 1):
                                via.append("%s/%s_to_%s/%s" %
                                           (ve, ie, ve + 1, ie))
                        if f[0] == "emp":
                            iedge = "4/%s_to_3/%s" % (ie, ie)
                            for ve in range(4, 0, -1):
                                via.append("%s/%s_to_%s/%s" %
                                           (ve, ie, ve - 1, ie))
                        if oe == 0:
                            if rel[0] == "mn":
                                oedge = "%s/3_to_%s/4.-100" % (ie, ie)
                            if rel[0] == "ms":
                                oedge = "%s/1_to_%s/0.-100" % (ie, ie)
                            if rel[0] == "mw":
                                oedge = "1/%s_to_0/%s.-100" % (ie, ie)
                            if rel[0] == "me":
                                oedge = "3/%s_to_4/%s.-100" % (ie, ie)
                            self.demand.addStream(demandGenerator.Stream(iedge + "__" + oedge, 0, 3600, flow, iedge,
                                                                         oedge,
                                                                         {"passenger": pkwEprob,
                                                                          "COLOMBO_undetectable_passenger": pkwNprob,
                                                                          "hdv": lkwEprob,
                                                                          "COLOMBO_undetectable_hdv": lkwNprob}))
                            continue
                        for oee in range(1, 4):
                            if rel[0] == "mn":
                                oedge = "%s/3_to_%s/4.-100" % (oee, oee)
                            if rel[0] == "ms":
                                oedge = "%s/1_to_%s/0.-100" % (oee, oee)
                            if rel[0] == "mw":
                                oedge = "1/%s_to_0/%s.-100" % (oee, oee)
                            if rel[0] == "me":
                                oedge = "3/%s_to_4/%s.-100" % (oee, oee)
                            print("%s %s " % (oee, via))
                            self.demand.addStream(demandGenerator.Stream(iedge + "__" + oedge, 0, 3600, int(flow / 3.),
                                                                         iedge, oedge,
                                                                         {"passenger": pkwEprob,
                                                                          "COLOMBO_undetectable_passenger": pkwNprob,
                                                                          "hdv": lkwEprob,
                                                                          "COLOMBO_undetectable_hdv": lkwNprob}))
            if fileNeedsRebuild(self.demandName, "duarouter"):
                self.demand.build(0, 86400, self.netName, self.demandName)

        shutil.copy(self.NET_FILE, self.sandboxPath(self.NET_FILE))
        shutil.copy(self.TLS_FILE, self.sandboxPath(self.TLS_FILE))
