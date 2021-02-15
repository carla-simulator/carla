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

# @file    filterDistricts.py
# @author  Jakob Erdmann
# @date    2017-10-05

"""
Filters a TAZ file for a specific vehicle class
"""
from __future__ import absolute_import
from __future__ import print_function
import os
import sys
from optparse import OptionParser
SUMO_HOME = os.environ.get('SUMO_HOME',
                           os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
sys.path.append(os.path.join(SUMO_HOME, 'tools'))
import sumolib  # noqa


def getOptions():
    optParser = OptionParser()
    optParser.add_option(
        "-v", "--verbose", action="store_true", default=False,
        help="tell me what you are doing")
    optParser.add_option("-n", "--net-file", dest="netfile", help="the network to read lane and edge permissions")
    optParser.add_option("-t", "--taz-file", dest="tazfile", help="the district file to be filtered")
    optParser.add_option("-o", "--output", default="taz_filtered.add.xml",
                         help="write filtered districts to FILE (default: %default)", metavar="FILE")
    optParser.add_option("--vclass", help="filter taz edges that allow the given vehicle class")
    (options, args) = optParser.parse_args()
    if not options.netfile or not options.tazfile or not options.vclass:
        optParser.print_help()
        optParser.exit(
            "Error! net-file, taz-file and vclass are mandatory")
    return options


if __name__ == "__main__":
    options = getOptions()
    if options.verbose:
        print("Reading net")
    net = sumolib.net.readNet(options.netfile)
    with open(options.output, 'w') as outf:
        outf.write("<tazs>\n")
        for taz in sumolib.output.parse(options.tazfile, "taz"):
            if taz.edges is not None:
                taz.edges = " ".join(
                    [e for e in taz.edges.split() if
                     net.hasEdge(e) and net.getEdge(e).allows(options.vclass)])
            deleteSources = []
            if taz.tazSink is not None:
                taz.tazSink = [s for s in taz.tazSink if net.hasEdge(s.id) and net.getEdge(s.id).allows(options.vclass)]
            if taz.tazSource is not None:
                taz.tazSource = [s for s in taz.tazSource if net.hasEdge(
                    s.id) and net.getEdge(s.id).allows(options.vclass)]
            outf.write(taz.toXML(initialIndent=" " * 4))
        outf.write("</tazs>\n")
