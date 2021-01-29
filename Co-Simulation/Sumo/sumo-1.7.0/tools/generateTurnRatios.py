#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    generateTurnRatios.py
# @author  Yun-Pang Floetteroed
# @date    2019-04-25

"""
- calculate the turn ratios or turn movements at each node
  with a given route file

- The output can be directly used as input in jtrrouter,
  where the time interval will be set for one day
"""
from __future__ import absolute_import
from __future__ import print_function
from __future__ import division

import os
import sys
import optparse

if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(tools)
    import sumolib
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")


def get_options(args=None):
    optParser = optparse.OptionParser()
    optParser.add_option("-o", "--output-file", dest="outfile",
                         default="turnRatios.add.xml", help="define the output filename")
    optParser.add_option("-r", "--route-files", dest="routefiles",
                         help="define the route file seperated by comma(mandatory)")
    optParser.add_option("-p", "--probabilities", dest="prob", action="store_true", default=False,
                         help=" calculate the turning probabilities instead of traffic volumes")
    optParser.add_option("--id", default="generated",
                         help="define the interval id")
    optParser.add_option("-v", "--verbose", dest="verbose", action="store_true",
                         default=False, help="tell me what you are doing")
    (options, args) = optParser.parse_args(args=args)

    if not options.routefiles:
        optParser.print_help()
        sys.exit()

    return options


def getFlows(routeFiles, verbose):
    # get flows for each edge pair
    minDepart = 1e20
    maxDepart = 0
    for file in routeFiles.split(','):
        edgePairFlowsMap = {}
        if verbose:
            print("route file:%s" % file)
        for veh in sumolib.output.parse(file, 'vehicle'):
            edgesList = veh.route[0].edges.split()
            minDepart = min(minDepart, float(veh.depart))
            maxDepart = max(maxDepart, float(veh.depart))
            for i, e in enumerate(edgesList):
                if i < len(edgesList)-1:
                    next = edgesList[i+1]
                    if e not in edgePairFlowsMap:
                        edgePairFlowsMap[e] = {}
                    if next not in edgePairFlowsMap[e]:
                        edgePairFlowsMap[e][next] = 0

                    edgePairFlowsMap[e][next] += 1
    return edgePairFlowsMap, minDepart, maxDepart


def main(options):
    # get traffic flows for each edge pair
    edgePairFlowsMap, minDepart, maxDepart = getFlows(options.routefiles, options.verbose)

    with open(options.outfile, 'w') as outf:
        sumolib.writeXMLHeader(outf, "$Id$", "edgeRelations", "edgerelations_file.xsd")  # noqa
        outf.write('    <interval id="%s" begin="%s" end="%s">\n' % (options.id, minDepart, maxDepart))
        for from_edge in sorted(edgePairFlowsMap.keys()):
            if options.prob:
                s = sum(edgePairFlowsMap[from_edge].values())
                for to_edge, count in sorted(edgePairFlowsMap[from_edge].items()):
                    outf.write(' ' * 8 + '<edgeRelation from="%s" to="%s" probability="%.2f"/>\n' %
                               (from_edge, to_edge, count / s))
            else:
                for to_edge, count in sorted(edgePairFlowsMap[from_edge].items()):
                    outf.write(' ' * 8 + '<edgeRelation from="%s" to="%s" count="%s"/>\n' %
                               (from_edge, to_edge, count))
        outf.write('    </interval>\n')
        outf.write('</edgeRelations>\n')
    outf.close()


if __name__ == "__main__":
    options = get_options(sys.argv)
    main(options)
