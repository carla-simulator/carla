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

# @file    tls_check.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2009-08-01

"""
Verifies the traffic lights in the given network.
Currently verified:
- phase length matches controlled link number
"""
from __future__ import absolute_import
from __future__ import print_function

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import sumolib.net  # noqa


if len(sys.argv) < 2:
    print("Call: tls_check.py <NET>", file=sys.stderr)
    sys.exit()

net1 = sumolib.net.readNet(sys.argv[1], withPrograms=True)

for tlsID in net1._id2tls:
    print("Checking tls '%s'" % tlsID)
    tls = net1._id2tls[tlsID]
    noConnections = tls._maxConnectionNo + 1
    for prog in tls._programs:
        print("   Checking program '%s'" % prog)
        prog = tls._programs[prog]
        for i, phase in enumerate(prog._phases):
            if len(phase.state) != noConnections:
                print("      Error: phase %s describes %s signals instead of %s." % (
                    i, len(phase.state), noConnections))
