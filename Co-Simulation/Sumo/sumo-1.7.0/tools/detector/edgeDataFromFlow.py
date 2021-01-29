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

# @file    edgeDataFromFlow.py
# @author  Jakob Erdmann
# @date    2020-02-27

"""
This script converts a flow file in csv-format to XML
(generalized meandata format : http://sumo.dlr.de/xsd/meandata_file.xsd)
"""
from __future__ import absolute_import
from __future__ import print_function
import sys
import os

from collections import defaultdict

import detector

SUMO_HOME = os.environ.get('SUMO_HOME',
                           os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
sys.path.append(os.path.join(SUMO_HOME, 'tools'))
import sumolib  # noqa
from sumolib.xml import parse  # noqa
DEBUG = False


def get_options(args=None):
    parser = sumolib.options.ArgumentParser(description="Convert detector flow file to edgeData format")
    parser.add_argument("-d", "--detector-file", dest="detfile",
                        help="read detectors from FILE (mandatory)", metavar="FILE")
    parser.add_argument("-f", "--detector-flow-file", dest="flowfile",
                        help="read detector flows to compare to from FILE (mandatory)", metavar="FILE")
    parser.add_argument("-o", "--output-file", dest="output",
                        help="output edgeData FILE (mandatory)", metavar="FILE")
    parser.add_argument("-q", "--flow-columns", dest="flowcols", default="qPKW,qLKW",
                        help="which columns contains flows", metavar="STRING")
    parser.add_argument("-b", "--begin", default=0,
                        help="custom begin time (minutes or H:M:S)")
    parser.add_argument("-e", "--end", default=1440,
                        help="custom end time (minutes or H:M:S)")
    parser.add_argument("-i", "--interval", default=1440,
                        help="custom aggregation interval (minutes or H:M:S)")
    parser.add_argument("-v", "--verbose", action="store_true", dest="verbose",
                        default=False, help="tell me what you are doing")
    options = parser.parse_args(args=args)
    if not options.detfile or not options.flowfile or not options.output:
        parser.print_help()
        sys.exit()

    return options


class LaneMap:
    def get(self, key, default):
        return key[0:-2]


def main(options):
    readers = {}
    flowcols = options.flowcols.split(',')
    tMin = None
    tMax = None
    for flowcol in flowcols:
        detReader = detector.DetectorReader(options.detfile, LaneMap())
        tMin, tMax = detReader.findTimes(options.flowfile, tMin, tMax)
        hasData = detReader.readFlows(options.flowfile, flow=flowcol, time="Time", timeVal=0, timeMax=1440)
        if options.verbose:
            print("flowColumn: %s hasData: %s" % (flowcol, hasData))
        readers[flowcol] = detReader

    if options.verbose:
        print("found data from minute %s to %s" % (int(tMin), int(tMax)))

    beginM = int(sumolib.miscutils.parseTime(options.begin, 60) / 60)
    intervalM = int(sumolib.miscutils.parseTime(options.interval, 60) / 60)
    endM = min(int(sumolib.miscutils.parseTime(options.end, 60) / 60), tMax)

    with open(options.output, "w") as outf:
        sumolib.writeXMLHeader(outf, "$Id$")  # noqa
        outf.write('<data>\n')
        while beginM <= endM:
            iEndM = beginM + intervalM
            edges = defaultdict(dict)  # edge : {attr:val}
            maxGroups = defaultdict(lambda: 0)  # edge : nGroups

            for flowcol in flowcols:
                detReader = detector.DetectorReader(options.detfile, LaneMap())
                detReader.readFlows(options.flowfile, flow=flowcol, time="Time", timeVal=beginM, timeMax=iEndM)
                for edge, detData in detReader._edge2DetData.items():
                    maxFlow = 0
                    nGroups = 0
                    for group in detData:
                        if group.isValid:
                            maxFlow = max(maxFlow, group.totalFlow)
                            nGroups += 1
                    # if options.verbose:
                    #    print("flowColumn: %s edge: %s flow: %s groups: %s" % (
                    #        flowcol, edge, maxFlow, nGroups))
                    edges[edge][flowcol] = maxFlow
                    maxGroups[edge] = max(maxGroups[edge], nGroups)

            outf.write('    <interval id="flowdata" begin="%s" end="%s">\n' % (beginM * 60, iEndM * 60))
            for edge in sorted(edges.keys()):
                attrs = ' '.join(['%s="%s"' % (k, v) for k, v in edges[edge].items()])
                outf.write('        <edge id="%s" %s groups="%s"/>\n' % (edge, attrs, nGroups))
            outf.write('    </interval>\n')
            beginM += intervalM
        outf.write('</data>\n')


if __name__ == "__main__":
    main(get_options())
