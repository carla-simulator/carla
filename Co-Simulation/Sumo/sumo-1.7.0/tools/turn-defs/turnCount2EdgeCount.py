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

# @file    turnCount2EdgeCount.py
# @author  Jakob Erdmann
# @date    2020-02-09

"""
Converts turn-count data into edgeData
"""
from __future__ import absolute_import
from __future__ import print_function

import os
import sys
from argparse import ArgumentParser
from collections import defaultdict

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
import sumolib  # noqa


def get_options(args=None):
    parser = ArgumentParser(description="Sample routes to match counts")
    parser.add_argument("-t", "--turn-file", dest="turnFile",
                        help="Input turn-count file")
    parser.add_argument("-o", "--output-file", dest="out",
                        help="Output edgeData file")
    parser.add_argument("--edgedata-attribute", dest="edgeDataAttr", default="entered",
                        help="Write edgeData counts with the given attribute")
    parser.add_argument("--turn-attribute", dest="turnAttr", default="count",
                        help="Read turning counts from the given attribute")

    options = parser.parse_args(args=args)
    if options.turnFile is None or options.out is None:
        parser.print_help()
        sys.exit()
    return options


def parseEdgeCounts(turnfile, attr):
    for interval in sumolib.xml.parse(turnfile, 'interval'):
        counts = defaultdict(lambda: 0)
        for edgeRel in interval.edgeRelation:
            count = float(getattr(edgeRel, attr))
            if count == int(count):
                count = int(count)
            counts[edgeRel.attr_from] += count
        yield interval.id, interval.begin, interval.end, counts


def main(options):
    with open(options.out, 'w') as outf:
        sumolib.writeXMLHeader(outf, "$Id$", "meandata")  # noqa
        for interval_id, interval_begin, interval_end, counts in parseEdgeCounts(options.turnFile, options.turnAttr):
            outf.write('    <interval id="%s" begin="%s" end="%s">\n' % (
                interval_id, interval_begin, interval_end))
            for edge in sorted(counts.keys()):
                outf.write(' ' * 8 + '<edge id="%s" %s="%s"/>\n' % (edge, options.edgeDataAttr, counts[edge]))
            outf.write('    </interval>\n')
        outf.write('</meandata>\n')


if __name__ == "__main__":
    main(get_options())
