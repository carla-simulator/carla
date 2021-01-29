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

# @file    plotFlows.py
# @author  Jakob Erdmann
# @date    2017-12-01

from __future__ import absolute_import
from __future__ import print_function
import math
import sys
import os

from optparse import OptionParser
import matplotlib.pyplot as plt

import detector

SUMO_HOME = os.environ.get('SUMO_HOME',
                           os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
sys.path.append(os.path.join(SUMO_HOME, 'tools'))
import sumolib  # noqa


def get_options(args=None):
    optParser = OptionParser()
    optParser.add_option("-d", "--detector-file", dest="detfile",
                         help="read detectors from FILE (mandatory)", metavar="FILE")
    optParser.add_option("-c", "--flow-column", dest="flowcol", default="qPKW",
                         help="which column contains flows", metavar="STRING")
    optParser.add_option("-z", "--respect-zero", action="store_true", dest="respectzero",
                         default=False, help="respect detectors without data (or with permanent zero) with zero flow")
    optParser.add_option("-i", "--interval", type="int", default=60, help="aggregation interval in minutes")
    optParser.add_option("--long-names", action="store_true", dest="longnames",
                         default=False, help="do not use abbreviated names for detector groups")
    optParser.add_option("--edge-names", action="store_true", dest="edgenames",
                         default=False, help="include detector group edge name in output")
    optParser.add_option("-b", "--begin", type="float", default=0, help="begin time in minutes")
    optParser.add_option("-e", "--end", type="float", default=None, help="end time in minutes")
    optParser.add_option("-o", "--csv-output", dest="csv_output", help="write plot data with prefix", metavar="FILE")
    optParser.add_option("--extension", help="extension for saving plots", default="png")
    optParser.add_option("-s", "--show", action="store_true", default=False, help="show plot directly")
    optParser.add_option("-g", "--group-by", dest="groupby", help="group detectors (all, none, type) ", default="all")
    optParser.add_option("-t", "--type-filter", dest="typefilter", help="only show selected types")
    optParser.add_option("-r", "--reference-flow", dest="reference",
                         help="reference flow file that should not be grouped", metavar="FILE")
    optParser.add_option("--id-filter", dest="idfilter", help="filter detector ids")
    optParser.add_option("--single-plot", action="store_true", dest="singleplot",
                         default=False, help="put averything in a single plot")
    # optParser.add_option("--boxplot", action="store_true", dest="boxplot", default=False, help="boxplot")
    optParser.add_option("-m", "--max-files", type="int", dest="maxfiles", help="limit number of input files")
    optParser.add_option("-n", "--no-legend", dest="nolegend", action="store_true",
                         default=False, help="dont draw legend")
    optParser.add_option("-v", "--verbose", action="store_true", default=False, help="tell me what you are doing")
    options, args = optParser.parse_args(args=args)
    options.flowfiles = args
    if options.maxfiles is not None:
        options.flowfiles = options.flowfiles[:options.maxfiles]
    print(options.flowfiles)
    if not options.detfile or not options.flowfiles:
        optParser.print_help()
        sys.exit()
    return options


def write_csv(keys, data, fname):
    with open(fname, 'w') as f:
        f.write("#key;value\n")
        for key, v in zip(keys, data):
            f.write("%s;%s\n" % (key, v))


def initDataList(begin, end, interval):
    return [None] * int(math.ceil((end - begin) / interval))


def addToDataList(data, i, val):
    if val is not None:
        if data[i] is None:
            data[i] = val
        else:
            data[i] += val


def plot(options, allData, prefix="", linestyle="-"):
    if not options.singleplot:
        plt.figure()

    labelsuffix = ""
    if prefix != "":
        labelsuffix = "_%s" % prefix
    plt.ylabel("Avg. Simulation Flow %s" % prefix)

    plt.xlabel("Minutes")
    x = range(int(options.begin), int(options.end), options.interval)
    # if options.boxplot:
    #    for f, data in zip(options.flowfiles, allData):
    #        label = f[-12:-4] + labelsuffix
    #        #plt.plot(x, data, label=label, linestyle=linestyle)
    #        label = label.replace(";","_")
    #        if options.csv_output is not None:
    #            write_csv(x, data, options.csv_output + label + ".csv")
    #    for f, data in zip(options.flowfiles, allData[:1]):
    #        label = f[-12:-4] + labelsuffix
    #        plt.plot(x, data, label=label, linestyle=linestyle)
    #        label = label.replace(";","_")
    #    plt.boxplot(x, allData[1:])
    # else:
    for f, data in zip(options.flowfiles, allData):
        label = f[-12:-4] + labelsuffix
        plt.plot(x, data, label=label, linestyle=linestyle)
        label = label.replace(";", "_")
        if options.csv_output is not None:
            lastdir = os.path.basename(os.path.dirname(f))
            write_csv(x, data, "%s_%s.%s.csv" % (options.csv_output, label, lastdir))
    if not options.nolegend:
        plt.legend(loc='best')
    if not options.singleplot:
        plt.savefig(label + "." + options.extension)


def main(options):
    detReaders = [detector.DetectorReader(options.detfile, detector.LaneMap()) for f in options.flowfiles]
    for f in options.flowfiles:
        options.begin, options.end = detReaders[0].findTimes(f, options.begin, options.end)
    if options.verbose:
        print("begin=%s, end=%s" % (options.begin, options.end))
    for detReader, f in zip(detReaders, options.flowfiles):
        if options.verbose:
            print("reading %s" % f)
        detReader.clearFlows(options.begin, options.interval)  # initialize
        detReader.readFlowsTimeline(f, options.interval, begin=options.begin, end=options.end, flowCol=options.flowcol)
    # aggregated detectors
    if options.singleplot:
        plt.figure(figsize=(14, 9), dpi=100)
    if options.groupby == "all":
        allData = []  # one list for each file
        for detReader, f in zip(detReaders, options.flowfiles):
            data = initDataList(options.begin, options.end, options.interval)
            for edge, group in detReader.getGroups():
                if options.idfilter is not None and options.idfilter not in group.ids[0]:
                    continue
                assert(len(group.timeline) <= len(data))
                for i, (flow, speed) in enumerate(group.timeline):
                    addToDataList(data, i, flow)
            allData.append(data)
        plot(options, allData)

    elif options.groupby == "type":
        for detType, linestyle in [("source", "-"), ("sink", ":"), ("between", "--")]:
            allData = []  # one list for each file
            for detReader, f in zip(detReaders, options.flowfiles):
                data = initDataList(options.begin, options.end, options.interval)
                for edge, group in detReader.getGroups():
                    if options.idfilter is not None and options.idfilter not in group.ids[0]:
                        continue
                    assert(len(group.timeline) <= len(data))
                    if group.type == detType:
                        for i, (flow, speed) in enumerate(group.timeline):
                            addToDataList(data, i, flow)
                allData.append(data)
            plot(options, allData, detType, linestyle)

    elif options.groupby == "none":
        for det, groupName in [(g.ids[0], g.getName(options.longnames)) for e, g in detReaders[0].getGroups()]:
            allData = []  # one list for each file
            for detReader, f in zip(detReaders, options.flowfiles):
                data = initDataList(options.begin, options.end, options.interval)
                group = detReader.getGroup(det)
                if options.typefilter is not None and group.type != options.typefilter:
                    continue
                if options.idfilter is not None and options.idfilter not in group.ids[0]:
                    continue
                assert(len(group.timeline) <= len(data))
                for i, (flow, speed) in enumerate(group.timeline):
                    addToDataList(data, i, flow)
                allData.append(data)
            plot(options, allData, "%s_%s" % (groupName, group.type))

    else:
        raise RuntimeError("group-by '%s' not supported" % options.groupby)

    if options.singleplot:
        plt.savefig("singlePlot." + options.extension)
    if options.show:
        plt.show()


if __name__ == "__main__":
    main(get_options())
