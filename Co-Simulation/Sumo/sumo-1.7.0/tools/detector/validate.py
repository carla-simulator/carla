#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    validate.py
# @author  Michael Behrisch
# @date    2013-06-12

"""
This script validates detector data resulting from dfrouter
validation detectors against the original data fed into dfrouter
"""
from __future__ import print_function
from __future__ import absolute_import
import sys
import os
import collections
from optparse import OptionParser
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt  # noqa
sys.path.append(os.path.join(os.environ["SUMO_HOME"], 'tools'))
import sumolib  # noqa

optParser = OptionParser(usage="usage: %prog [options] <input_flows.csv>")
optParser.add_option(
    "-d", "--detectorfile", help="read detector list from file")
optParser.add_option(
    "-v", "--validation", help="read validation data from file")
optParser.add_option("-i", "--interval", default=15,
                     help="aggregation interval in minutes (default: %default)")
optParser.add_option("-l", "--legacy", action="store_true", default=False,
                     help="legacy style, input file is whitespace separated, detector_definition")
(options, args) = optParser.parse_args()

sources = set()
sinks = set()
dets = {}
sims = {}
detDef = "detector_definition" if options.legacy else "detectorDefinition"
if options.detectorfile:
    for det in sumolib.output.parse_fast(options.detectorfile, detDef, ["id"]):
        dets[det.id] = []
        sims[det.id] = []
counts = {}
c = collections.defaultdict(int)
v = collections.defaultdict(float)
totals = collections.defaultdict(int)
countIn = 0
countOut = 0
start = 0
end = options.interval
with open(args[0]) as f:
    skipFirst = True
    for line in f:
        if skipFirst:
            skipFirst = False
            continue
        if options.legacy:
            item = line.split()
        else:
            item = line.split(";")
        detId = item[0]
        time = int(item[1])
        if time >= end:
            counts[start] = countIn - countOut
            start = end
            end += options.interval
            for det, vals in dets.iteritems():
                if c[det] > 0:
                    vals.append((time, c[det], v[det] / c[det]))
            c.clear()
            v.clear()
        if options.legacy:
            total = int(item[3])
            totalSpeed = float(item[2]) if total > 0 else 0.
        else:
            total = int(item[2]) + int(item[3])
            totalSpeed = int(item[2]) * float(item[4]) + \
                int(item[3]) * float(item[5])
        c[detId] += total
        v[detId] += totalSpeed
        totals[detId] += total
        if detId in sources:
            countIn += total
        if detId in sinks:
            countOut += total
print("detIn: %s detOut: %s" % (countIn, countOut))
totalSim = collections.defaultdict(int)
if options.validation:
    c.clear()
    v.clear()
    countIn = 0
    countOut = 0
    start = 0
    end = options.interval
#   <interval begin="0.00" end="60.00" id="validation_MQ11O_DS_FS1_ERU"
#             nVehContrib="1" flow="60.00" occupancy="1.35" speed="6.19" length="5.00" nVehEntered="1"/>
    for interval in sumolib.output.parse_fast(options.validation, "interval", ["begin", "id", "speed", "nVehEntered"]):
        detId = interval.id[11:]
        time = int(float(interval.begin) / 60)
        if time >= end:
            start = end
            end += options.interval
            for det, vals in sims.iteritems():
                if c[det] > 0:
                    vals.append((time, c[det], v[det] / c[det]))
            c.clear()
            v.clear()
        c[detId] += int(interval.nVehEntered)
        totalSim[detId] += int(interval.nVehEntered)
        v[detId] += 3.6 * int(interval.nVehEntered) * float(interval.speed)
        if detId in sources:
            countIn += int(interval.nVehEntered)
        if detId in sinks:
            countOut += int(interval.nVehEntered)
    print("simIn: %s simOut: %s" % (countIn, countOut))

for det, vals in dets.iteritems():
    print("Plotting", det, 'totaldet', totals[det], 'totalSim', totalSim[det])
    plt.bar(*(zip(*vals)[:2]))  # select first and second entry (time and flow)
    if det in sims:
        plt.plot(*(zip(*sims[det])[:2]))
    plt.suptitle('%s flow, totalDet: %s, totalSim: %s' %
                 (det, totals[det], totalSim[det]))
    plt.xlabel('time')
    plt.ylabel('flow')
    plt.ylim(0, 600)
    plt.legend(["simulation", "measured value"])
    plt.savefig('%s_flow.png' % det)
    plt.close()
    # select first and third entry (time and speed)
    plt.bar(*(zip(*vals)[::2]))
    if det in sims:
        plt.plot(*(zip(*sims[det])[::2]))
    plt.suptitle('%s_speed' % det)
    plt.xlabel('time')
    plt.ylabel('speed')
    plt.ylim(0, 200)
    plt.legend(["simulation", "measured value"])
    plt.savefig('%s_speed.png' % det)
    plt.close()
plt.bar(counts.keys(), counts.values())
plt.show()
