#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2008-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    accelerations.py
# @author  Jakob Erdmann
# @date    2012-11-20

from __future__ import absolute_import
from __future__ import print_function
import os
import sys
sys.path.append(os.path.join(os.path.dirname(sys.argv[0]), '..'))
from sumolib.output import parse_fast  # noqa
from sumolib.miscutils import Statistics  # noqa


def accelStats(netstate):
    lastSpeed = {}
    stats = Statistics(
        "Accelerations", histogram=True, printMin=True, scale=0.2)
    for vehicle in parse_fast(netstate, 'vehicle', ['id', 'speed']):
        speed = float(vehicle.speed)
        prevSpeed = lastSpeed.get(vehicle.id, speed)
        stats.add(speed - prevSpeed, (vehicle.id, vehicle.speed))
        lastSpeed[vehicle.id] = speed
    print(stats)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit("call %s <netstate-dump>" % sys.argv[0])
    accelStats(*sys.argv[1:])
