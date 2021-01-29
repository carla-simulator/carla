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

# @file    routeUsage.py
# @author  Jakob Erdmann
# @date    2017-03-30

from __future__ import absolute_import
from __future__ import print_function
import os
import sys
from collections import defaultdict
from optparse import OptionParser

if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(os.path.join(tools))
    import sumolib  # noqa
    from sumolib.output import parse  # noqa
    from sumolib.miscutils import Statistics  # noqa
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")


def get_options():
    USAGE = """Usage %prog <emitters.xml> [<routes.xml>]"""
    optParser = OptionParser(usage=USAGE)
    optParser.add_option("-v", "--verbose", action="store_true",
                         default=False, help="Give more output")
    optParser.add_option("--threshold", type="int", default=0,
                         help="Output routes that are used less than the threshold value")
    optParser.add_option("--unused-output",
                         help="Output route ids that are used less than the threshold value to file")
    optParser.add_option("-r", "--flow-restrictions", dest="restrictionfile",
                         help="Output route ids that are used more often than the threshold value given in file")
    options, args = optParser.parse_args()

    if len(args) not in (1, 2):
        sys.exit(USAGE)
    options.emitters = args[0]
    options.routes = args[1] if len(args) == 2 else None
    return options


def main():
    options = get_options()

    routes = defaultdict(list)
    if options.routes is not None:
        for route in parse(options.routes, 'route'):
            routes[route.edges].append(route.id)

    restrictions = {}
    if options.restrictionfile is not None:
        for line in open(options.restrictionfile):
            count, edges = line.strip().split(None, 1)
            for rID in routes[edges]:
                restrictions[rID] = int(count)

    routeUsage = defaultdict(int)
    for flow in parse(options.emitters, 'flow'):
        num = int(flow.number)
        if flow.route is None:
            dist = flow.routeDistribution[0]
            probs = map(float, dist.probabilities.split())
            probs = [p / sum(probs) for p in probs]
            for rID, p in zip(dist.routes.split(), probs):
                routeUsage[rID] += p * num
        else:
            routeUsage[flow.route] += num

    usage = Statistics("routeUsage")
    restrictUsage = Statistics("restrictedRouteUsage")
    for rID, count in routeUsage.items():
        usage.add(count, rID)
        if rID in restrictions:
            restrictUsage.add(count, rID)
    print(usage)
    print(restrictUsage, "total:", sum(restrictUsage.values))

    if options.unused_output is not None:
        with open(options.unused_output, 'w') as outf:
            for rID, count in routeUsage.items():
                if count <= options.threshold:
                    outf.write("%s\n" % rID)
                if rID in restrictions and count > restrictions[rID]:
                    outf.write("%s %s %s\n" % (rID, count, restrictions[rID]))


if __name__ == "__main__":
    main()
