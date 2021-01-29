#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    corridor.py
# @author  Daniel Krajzewicz
# @date    2013-10-10

from __future__ import absolute_import
import os
import sys
sys.path.append(
    os.path.join(os.path.dirname(__file__), '..', '..', '..', '..', 'tools'))
sys.path.append(os.path.join(os.environ.get("SUMO_HOME", os.path.join(
    os.path.dirname(__file__), '..', '..', '..', '..')), 'tools'))

from . import network  # noqa


def corridor(numIntersections=10, defaultNode=None, defaultEdge=None, centralReservation=0, hdistance=500):
    net = network.Net(defaultNode, defaultEdge)
    net.addNode(network.Node("0/1", 0, 500, "priority"))
    for i in range(0, numIntersections):
        net.addNode(
            network.Node(str(i + 1) + "/0", (i + 1) * hdistance, 0, "priority"))
        net.addNode(
            network.Node(str(i + 1) + "/1", (i + 1) * hdistance, 500, "traffic_light"))
        net.addNode(
            network.Node(str(i + 1) + "/2", (i + 1) * hdistance, 1000, "priority"))
        net.connectNodes(
            str(i) + "/1", str(i + 1) + "/1", True, centralReservation)
        net.connectNodes(
            str(i + 1) + "/0", str(i + 1) + "/1", True, centralReservation)
        net.connectNodes(
            str(i + 1) + "/1", str(i + 1) + "/2", True, centralReservation)
    net.addNode(network.Node(str(numIntersections + 1) + "/1",
                             (numIntersections + 1) * 500, 500, "priority"))
    net.connectNodes(str(numIntersections) + "/1",
                     str(numIntersections + 1) + "/1", True, centralReservation)
    return net
#  d = demand.Demand()
#  d.addStream(demand.Stream("1/0_to_1/2", 10, "1/0 1/2"))
#  d.build(3600)


if __name__ == "__main__":
    net = corridor()
    net.build()
