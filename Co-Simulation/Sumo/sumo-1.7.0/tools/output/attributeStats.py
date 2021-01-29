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

# @file    attributeStats.py
# @author  Jakob Erdmann
# @date    2019-04-11

"""
compute statistics for a specific xml attribute (e.g. timeLoss in tripinfo-output)
"""
from __future__ import absolute_import
from __future__ import print_function

import os
import sys
from collections import defaultdict
from optparse import OptionParser

if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(os.path.join(tools))
    import sumolib
    from sumolib.xml import parse, parse_fast  # noqa
    from sumolib.miscutils import Statistics  # noqa
    from sumolib.statistics import setPrecision
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")


def get_options():
    USAGE = """Usage %prog [options] <data.xml>"""
    optParser = OptionParser(usage=USAGE)
    optParser.add_option("-v", "--verbose", action="store_true",
                         default=False, help="Give more output")
    optParser.add_option("-e", "--element", type="string",
                         default="tripinfo", help="element to analyze")
    optParser.add_option("-a", "--attribute", type="string",
                         default="timeLoss", help="attribute to analyze")
    optParser.add_option("-i", "--id-attribute", type="string", dest="idAttr",
                         default="id", help="attribute to identify data elements")
    optParser.add_option("-b", "--binwidth", type="float",
                         default=50, help="binning width of result histogram")
    optParser.add_option("--hist-output", type="string",
                         default=None, help="output file for histogram (gnuplot compatible)")
    optParser.add_option("-o", "--full-output", type="string",
                         default=None, help="output file for full data dump")
    optParser.add_option("-q", "--fast", action="store_true",
                         default=False, help="use fast parser (does not track missing data)")
    optParser.add_option("-p", "--precision", type="int",
                         default=2, help="Set output precision")
    options, args = optParser.parse_args()

    if len(args) != 1:
        sys.exit(USAGE)

    options.datafile = args[0]
    return options


def main():
    options = get_options()

    vals = defaultdict(list)
    stats = Statistics("%s %ss" % (options.element, options.attribute),
                       histogram=options.binwidth > 0, scale=options.binwidth)
    missingAttr = set()
    invalidType = set()

    if options.fast:
        def elements():
            for element in parse_fast(options.datafile, options.element, [options.idAttr, options.attribute]):
                yield getattr(element, options.idAttr), getattr(element, options.attribute)
    else:
        def elements():
            for element in parse(options.datafile, options.element, heterogeneous=True):
                elementID = None
                if element.hasAttribute(options.idAttr):
                    elementID = element.getAttribute(options.idAttr)
                stringVal = None
                if element.hasAttribute(options.attribute):
                    stringVal = element.getAttribute(options.attribute)
                yield elementID, stringVal

    for elementID, stringVal in elements():
        if stringVal is not None:
            try:
                val = sumolib.miscutils.parseTime(stringVal)
                vals[elementID].append(val)
                stats.add(val, elementID)
            except Exception:
                invalidType.add(stringVal)
        else:
            missingAttr.add(elementID)

    print(stats.toString(options.precision))
    if missingAttr:
        print("%s elements did not provide attribute '%s' Example ids: %s" %
              (len(missingAttr), options.attribute, sorted(missingAttr)[:10]))
    if invalidType:
        print(("%s distinct values of attribute '%s' could not be interpreted " +
               "as numerical value or time. Example values: %s") %
              (len(invalidType), options.attribute, sorted(invalidType)[:10]))

    if options.hist_output is not None:
        with open(options.hist_output, 'w') as f:
            for bin, count in stats.histogram():
                f.write("%s %s\n" % (bin, count))

    if options.full_output is not None:
        with open(options.full_output, 'w') as f:
            for id, data in vals.items():
                for x in data:
                    f.write(setPrecision("%.2f %s\n", options.precision) % (x, id))


if __name__ == "__main__":
    main()
