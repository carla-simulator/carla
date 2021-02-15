#!/usr/bin/env python
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

# @file    stopOrder.py
# @author  Jakob Erdmann
# @date    2020-08-25

"""
Compare ordering of vehicle departure at stops based on a route file with until
times (ground truth) and stop-output
"""

from __future__ import absolute_import
from __future__ import print_function

import os
import sys
from collections import defaultdict

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
import sumolib  # noqa
from sumolib.miscutils import parseTime  # noqa
from sumolib.xml import parse  # noqa


def get_options(args=None):
    parser = sumolib.options.ArgumentParser(description="Sample routes to match counts")
    parser.add_argument("-r", "--route-file", dest="routeFile",
                        help="Input route file")
    parser.add_argument("-s", "--stop-file", dest="stopFile",
                        help="Input stop-output file")
    parser.add_argument("-v", "--verbose", action="store_true",
                        default=False, help="tell me what you are doing")

    options = parser.parse_args(args=args)
    if options.routeFile is None or options.stopFile is None:
        parser.print_help()
        sys.exit()

    return options


def main(options):

    # stop (stoppingPlaceID or (lane, pos)) -> [(depart1, veh1), (depart2, veh2), ...]
    expected_departs = defaultdict(list)
    actual_departs = defaultdict(dict)
    ignored_stops = 0
    parsed_stops = 0

    for vehicle in parse(options.routeFile, ['vehicle', 'trip'], heterogeneous=True):
        if vehicle.stop is not None:
            for stop in vehicle.stop:
                if stop.hasAttribute("until"):
                    if stop.hasAttribute("busStop"):
                        stopCode = stop.busStop
                    else:
                        stopCode = (stop.lane, stop.endPos)
                    expected_departs[stopCode].append((parseTime(stop.until), vehicle.id))
                    parsed_stops += 1
                else:
                    ignored_stops += 1

    print("Parsed %s expected stops at %s locations" % (parsed_stops, len(expected_departs)))
    if ignored_stops > 0:
        sys.stderr.write("Ignored %s stops without 'until' attribute\n" % ignored_stops)

    output_stops = 0
    for stop in parse(options.stopFile, "stopinfo", heterogeneous=True):
        if stop.hasAttribute("busStop"):
            stopCode = stop.busStop
        else:
            stopCode = (stop.lane, stop.endPos)
        ended = parseTime(stop.ended)
        until = ended - parseTime(stop.delay)
        actual_departs[stopCode][(until, stop.id)] = ended
        output_stops += 1
    print("Parsed %s actual stops at %s locations" % (output_stops, len(actual_departs)))

    missing = defaultdict(list)
    for stopCode, vehicles in expected_departs.items():
        if stopCode in actual_departs:
            actual_vehicles = actual_departs[stopCode]
            comparable_expected = []
            comparable_actual = []
            for vehCode in vehicles:
                if vehCode in actual_vehicles:
                    comparable_expected.append(vehCode)
                    comparable_actual.append((actual_vehicles[vehCode], vehCode))  # (ended, (until, vehID))
                else:
                    missing[stopCode].append(vehCode)
            comparable_expected.sort()
            comparable_actual.sort()
            num_unexpected = len(actual_vehicles) - len(comparable_actual)
            if num_unexpected > 0:
                print("Found %s unexpected stops at %s" % (num_unexpected, stopCode))

            # after sorting, discard the 'ended' attribute and only keep vehCode
            comparable_actual2 = [v[1] for v in comparable_actual]

            # find and remove duplicate
            tmp = []
            for vehCode in comparable_expected:
                if len(tmp) != 0:
                    if vehCode != tmp[-1]:
                        tmp.append(vehCode)
                    else:
                        if options.verbose:
                            print("Found duplicate stop at %s for vehicle %s" % (stopCode, vehCode))
                        comparable_actual2.remove(vehCode)
                else:
                    tmp.append(vehCode)

            comparable_expected = tmp

            if options.verbose:
                actual = [(v[0], v[1][1]) for v in comparable_actual]
                print("stop:", stopCode)
                print("   expected:", comparable_expected)
                print("     actual:", actual)

            for i, vehCode in enumerate(comparable_expected):
                j = comparable_actual2.index(vehCode)
                if i < j:
                    print("At %s vehicle %s comes after %s (i=%s, j=%s)" % (
                        stopCode, vehCode,
                        ','.join(map(str, comparable_actual2[i:j])),
                        i, j
                    ))
                elif j < i:
                    print("At %s vehicle %s comes before %s (i=%s, j=%s)" % (
                        stopCode, vehCode,
                        ','.join(map(str, comparable_actual2[j:i])),
                        i, j
                    ))
                if i != j:
                    # swap to avoid duplicate out-of-order warnings
                    tmp = comparable_actual2[i]
                    comparable_actual2[i] = comparable_actual2[j]
                    comparable_actual2[j] = tmp
        else:
            missing[stopCode] = vehicles

    print("Simulation missed %s stops at %s locations" % (sum(map(len, missing.values())), len(missing)))


if __name__ == "__main__":
    main(get_options())
