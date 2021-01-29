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

# @file    tripinfoDiff.py
# @author  Jakob Erdmann
# @date    2016-15-04

"""
Compare differences between tripinfo files that contain the same vehicles
"""

from __future__ import absolute_import
from __future__ import print_function
import os
import sys
import argparse
from collections import OrderedDict
sys.path.append(os.path.join(os.environ["SUMO_HOME"], 'tools'))
from sumolib.output import parse  # noqa
from sumolib.miscutils import Statistics, parseTime  # noqa


def get_options(args=None):
    argParser = argparse.ArgumentParser()
    argParser.add_argument("orig", help="the first tripinfo file")
    argParser.add_argument("new", help="the second tripinfo file")
    argParser.add_argument("output", help="the output file")
    argParser.add_argument("--filter-ids", dest="filterIDs",
                           help="only use trip ids with the given substring")
    argParser.add_argument("--persons", action="store_true",
                           default=False, help="compute personinfo differences")
    argParser.add_argument("--histogram-scale", type=float, dest="histScale",
                           help="compute data histogram with the FLOAT granularity")
    options = argParser.parse_args(args=args)
    options.useHist = options.histScale is not None
    return options


def write_diff(options):
    attrs = ["depart", "arrival", "timeLoss", "duration", "routeLength"]
    # parseTime works just fine for floats
    attr_conversions = dict([(a, parseTime) for a in attrs])
    vehicles_orig = OrderedDict([(v.id, v) for v in parse(options.orig, 'tripinfo',
                                                          attr_conversions=attr_conversions)])
    descr = ""
    if options.filterIDs:
        descr = " (%s)" % options.filterIDs
    origDurations = Statistics('original durations%s' % descr,
                               histogram=options.useHist, scale=options.histScale)
    durations = Statistics('new durations%s' % descr,
                           histogram=options.useHist, scale=options.histScale)
    durationDiffs = Statistics('duration differences%s new-old' % descr,
                               histogram=options.useHist, scale=options.histScale)
    numNew = 0
    numMissing = 0
    with open(options.output, 'w') as f:
        f.write("<tripDiffs>\n")
        for v in parse(options.new, 'tripinfo', attr_conversions=attr_conversions):
            if options.filterIDs and options.filterIDs not in v.id:
                del vehicles_orig[v.id]
                continue
            if v.id in vehicles_orig:
                vOrig = vehicles_orig[v.id]
                diffs = [v.getAttribute(a) - vOrig.getAttribute(a) for a in attrs]
                durations.add(v.duration, v.id)
                origDurations.add(vOrig.duration, v.id)
                durationDiffs.add(v.duration - vOrig.duration, v.id)
                diffAttrs = ''.join([' %sDiff="%s"' % (a, x) for a, x in zip(attrs, diffs)])
                f.write('    <vehicle id="%s"%s/>\n' % (v.id, diffAttrs))
                del vehicles_orig[v.id]
            else:
                f.write('    <vehicle id="%s" comment="new"/>\n' % v.id)
                numNew += 1
        for id in vehicles_orig.keys():
            f.write('    <vehicle id="%s" comment="missing"/>\n' % id)
            numMissing += 1
        f.write("</tripDiffs>\n")

    if numMissing > 0:
        print("missing: %s" % numMissing)
    if numNew > 0:
        print("new: %s" % numNew)
    print(origDurations)
    print(durations)
    print(durationDiffs)


def write_persondiff(options):
    attrs = ["depart", "arrival", "timeLoss", "duration", "routeLength", "waitingTime"]
    attr_conversions = dict([(a, parseTime) for a in attrs])
    persons_orig = OrderedDict([(p.id, p) for p in parse(options.orig, 'personinfo',
                                                         attr_conversions=attr_conversions)])
    origDurations = Statistics('original durations')
    durations = Statistics('new durations')
    durationDiffs = Statistics('duration differences')
    statAttrs = ["duration", "walkTimeLoss", "rideWait", "walks", "accesses", "rides", "stops"]
    with open(options.output, 'w') as f:
        f.write("<tripDiffs>\n")
        for p in parse(options.new, 'personinfo', attr_conversions=attr_conversions):
            if p.id in persons_orig:
                pOrig = persons_orig[p.id]
                stats = plan_stats(p)
                statsOrig = plan_stats(pOrig)
                diffs = [a - b for a, b in zip(stats, statsOrig)]
                durations.add(stats[0], p.id)
                origDurations.add(statsOrig[0], p.id)
                durationDiffs.add(stats[0] - statsOrig[0], p.id)
                diffAttrs = ''.join([' %sDiff="%s"' % (a, x) for a, x in zip(statAttrs, diffs)])
                f.write('    <personinfo id="%s"%s/>\n' % (p.id, diffAttrs))
                del persons_orig[p.id]
            else:
                f.write('    <personinfo id="%s" comment="new"/>\n' % p.id)
        for id in persons_orig.keys():
            f.write('    <personinfo id="%s" comment="missing"/>\n' % id)
        f.write("</tripDiffs>\n")

    print(origDurations)
    print(durations)
    print(durationDiffs)


def plan_stats(pInfo):
    duration = 0
    timeLoss = 0
    rideWait = 0
    walks = 0
    accesses = 0
    rides = 0
    stops = 0
    if pInfo.walk:
        walks = len(pInfo.walk)
        for walk in pInfo.walk:
            timeLoss += walk.timeLoss
            duration += walk.duration
    if pInfo.access:
        accesses = len(pInfo.access)
        for access in pInfo.access:
            duration += access.duration
    if pInfo.ride:
        rides = len(pInfo.ride)
        for ride in pInfo.ride:
            duration += ride.duration
            rideWait += ride.waitingTime
    if pInfo.stop:
        stops = len(pInfo.stop)
        for stop in pInfo.stop:
            duration += stop.duration
    return (duration, timeLoss, rideWait, walks, accesses, rides, stops)


if __name__ == "__main__":
    options = get_options()
    if options.persons:
        write_persondiff(options)
    else:
        write_diff(options)
