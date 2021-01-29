#!/usr/bin/env python
# -*- coding: utf-8 -*-
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

# @file    analyze_pedestrian_jam.py
# @author  Jakob Erdmann
# @date    2020-01-06

from __future__ import absolute_import
from __future__ import print_function
import sys
import re
from collections import defaultdict
import optparse


def get_options(args=None):
    usage = "usage: %prog [options] logfile"
    optParser = optparse.OptionParser(usage=usage)
    optParser.add_option("-a", "--aggregation", type="int", default=3600, help="define the time aggregation in seconds")
    optParser.add_option("-p", "--gnuplot-output", dest="plotfile", help="define the gnuplot output file")
    optParser.add_option("-e", "--edgedata-output", dest="edgedata", help="define the edgedata output file")

    (options, args) = optParser.parse_args(args=args)

    if len(args) != 1:
        print(usage)
        sys.exit(1)
    options.logfile = args[0]

    if options.plotfile is None:
        options.plotfile = options.logfile + '.plot'

    if options.edgedata is None:
        options.edgedata = options.logfile + "data.xml"
    return options


def parse_log(logfile, aggregate=3600):
    print("Parsing %s" % logfile)
    reEdge = re.compile("edge '([^']*)'")
    reTime = re.compile(r"time.(\d*)\.")
    reHRTime = re.compile(r"time.(\d):(\d\d):(\d\d):(\d*).")
    # counts per edge
    jamCounts = defaultdict(lambda: 0)
    # counts per step
    jamStepCounts = defaultdict(lambda: 0)
    for index, line in enumerate(open(logfile)):
        try:
            if "is jammed on edge" in line:
                match = reEdge.search(line)
                edge = match.group(1)
                timeMatch = reTime.search(line)
                if timeMatch:
                    time = int(timeMatch.group(1))
                else:
                    timeMatch = reHRTime.search(line)
                    time = (24 * 3600 * int(timeMatch.group(1))
                            + 3600 * int(timeMatch.group(2))
                            + 60 * int(timeMatch.group(3))
                            + int(timeMatch.group(4)))
                jamCounts[edge] += 1
                jamStepCounts[time / aggregate] += 1
        except Exception:
            print(sys.exc_info())
            sys.exit("error when parsing line '%s'" % line)
        if index % 1000 == 0:
            sys.stdout.write(".")
            sys.stdout.flush()
    print()
    print("read %s lines" % index)

    return jamCounts, jamStepCounts


def print_counts(countDict, label, num=10):
    counts = sorted(countDict.items(), key=lambda k: -k[1])
    print(label, "worst %s edges: " % num, counts[:num])
    print(label, 'total:', sum(countDict.values()))


def main(options):
    jamCounts, jamStepCounts = parse_log(options.logfile, aggregate=options.aggregation)
    print_counts(jamCounts, 'waiting')
    # generate plot
    if len(jamStepCounts) > 0:
        min_step = min(jamStepCounts.keys())
        max_step = max(jamStepCounts.keys())
        with open(options.plotfile, 'w') as f:
            if options.aggregation >= 3600:
                xfactor = options.aggregation / 3600.0
                xlabel = "%s hours" % xfactor
            else:
                xfactor = options.aggregation / 60.0
                xlabel = "%s minute" % xfactor
            if xfactor != 1:
                xlabel += "s"
            f.write(("# plot '%s' using 1:2 with lines title 'jammed per %s'\n") % (options.plotfile, xlabel))
            for step in range(min_step, max_step + 1):
                print(' '.join(
                    map(str, [xfactor * step, jamStepCounts[step]])), file=f)
    with open(options.edgedata, 'w') as f:
        print('<meandata>\n    <interval begin="0" end="100:00:00:00">', file=f)
        for item in jamCounts.items():
            print('        <edge id="%s" jammed="%s"/>' % item, file=f)
        print("    </interval>\n</meandata>", file=f)


if __name__ == "__main__":
    main(get_options())
