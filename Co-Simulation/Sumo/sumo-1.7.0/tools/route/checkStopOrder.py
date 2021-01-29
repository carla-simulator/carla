#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    checkStopOrder.py
# @author  Jakob Erdmann
# @date    2020-03-19

"""
Check order of arriving and leaving vehicles at stopping places.
Report if vehicles change their order at a stop (a vehicle arrives later but leaves earlier)
"""

from __future__ import absolute_import
from __future__ import print_function

import os
import sys
from collections import defaultdict

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
    import sumolib  # noqa
    from sumolib.miscutils import parseTime, humanReadableTime
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")


def get_options(args=None):
    parser = sumolib.options.ArgumentParser(description="Sample routes to match counts")
    parser.add_argument("-r", "--route-files", dest="routeFiles",
                        help="Input route file")
    parser.add_argument("-H", "--human-readable-time", dest="hrTime", action="store_true", default=False,
                        help="Write time values as hour:minute:second or day:hour:minute:second rathern than seconds")
    parser.add_argument("-p", "--ignore-parking", dest="ignoreParking", action="store_true", default=False,
                        help="Do not report conflicts with parking vehicles")

    options = parser.parse_args(args=args)
    options.routeFiles = options.routeFiles.split(',')

    return options


def main(options):

    tf = humanReadableTime if options.hrTime else lambda x: x

    stopTimes = defaultdict(list)
    for routefile in options.routeFiles:
        for vehicle in sumolib.xml.parse(routefile, ['vehicle', 'trip'], heterogeneous=True):
            if vehicle.stop is None:
                continue
            for stop in vehicle.stop:
                if stop.parking in ["true", "True", "1"] and options.ignoreParking:
                    continue
                until = parseTime(stop.until)
                arrival = parseTime(stop.arrival) if stop.arrival else until - parseTime(stop.duration)
                stopTimes[stop.busStop].append((arrival, until, vehicle.id))

    for stop, times in stopTimes.items():
        times.sort()
        for i, (a, u, v) in enumerate(times):
            for a2, u2, v2 in times[i + 1:]:
                if u2 < u:
                    print("Vehicle %s (%s, %s) overtakes %s (%s, %s) at stop %s" % (
                        v2, tf(a2), tf(u2), v, tf(a), tf(u), stop))


if __name__ == "__main__":
    main(get_options())
