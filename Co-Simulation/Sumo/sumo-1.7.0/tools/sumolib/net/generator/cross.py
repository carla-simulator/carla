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

# @file    cross.py
# @author  Daniel Krajzewicz
# @date    2013-10-10

from __future__ import absolute_import
from . import network  # noqa


def cross(defaultNode=None, defaultEdge=None, centralReservation=0):
    net = network.Net(defaultNode, defaultEdge)
    net.addNode(network.Node("1/0", 500, 0, "priority"))
    net.addNode(network.Node("0/1", 0, 500, "priority"))
    net.addNode(network.Node("1/1", 500, 500, "traffic_light"))
    net.addNode(network.Node("2/1", 1000, 500, "priority"))
    net.addNode(network.Node("1/2", 500, 1000, "priority"))
    net.connectNodes("1/0", "1/1", True, centralReservation)
    net.connectNodes("0/1", "1/1", True, centralReservation)
    net.connectNodes("2/1", "1/1", True, centralReservation)
    net.connectNodes("1/2", "1/1", True, centralReservation)
    return net


if __name__ == "__main__":
    net = cross()
    net.build()
