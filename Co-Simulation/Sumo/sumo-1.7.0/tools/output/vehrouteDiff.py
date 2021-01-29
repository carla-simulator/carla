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

# @file    vehrouteDiff.py
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @date    2012-11-20

from __future__ import absolute_import
from __future__ import print_function
import os
import sys
from collections import defaultdict
sys.path.append(os.path.join(os.path.dirname(sys.argv[0]), '..'))
from sumolib.output import parse  # noqa
from sumolib.miscutils import uMax, Statistics, parseTime  # noqa


def update_earliest(earliest_diffs, diff, timestamp, tag):
    if timestamp < earliest_diffs[diff][0]:
        earliest_diffs[diff] = (timestamp, tag)


def write_diff(orig, new, out, earliest_out=None):
    attr_conversions = {"depart": parseTime, "arrival": parseTime}
    earliest_diffs = defaultdict(lambda: (uMax, None))  # diff -> (time, veh)
    vehicles_orig = dict([(v.id, v) for v in parse(orig, 'vehicle',
                                                   attr_conversions=attr_conversions)])
    origDurations = Statistics('original durations')
    durations = Statistics('new durations')
    durationDiffs = Statistics('duration differences')
    with open(out, 'w') as f:
        f.write("<routeDiff>\n")
        for v in parse(new, 'vehicle', attr_conversions=attr_conversions):
            if v.id in vehicles_orig:
                vOrig = vehicles_orig[v.id]
                departDiff = v.depart - vOrig.depart
                arrivalDiff = v.arrival - vOrig.arrival
                if v.route[0].exitTimes is None:
                    sys.exit("Error: Need route input with 'exitTimes'\n")
                exitTimes = map(parseTime, v.route[0].exitTimes.split())
                origExitTimes = map(parseTime, vOrig.route[0].exitTimes.split())
                exitTimesDiff = [
                    e - eOrig for e, eOrig in zip(exitTimes, origExitTimes)]

                durations.add(v.arrival - v.depart, v.id)
                origDurations.add(vOrig.arrival - vOrig.depart, v.id)
                durationDiffs.add(arrivalDiff - departDiff, v.id)

                update_earliest(
                    earliest_diffs, departDiff, vOrig.depart, v.id + ' (depart)')
                for diff, timestamp in zip(exitTimesDiff, origExitTimes):
                    update_earliest(earliest_diffs, diff, timestamp, v.id)

                f.write('''    <vehicle id="%s" departDiff="%s" arrivalDiff="%s" exitTimesDiff="%s"/>\n''' % (
                    v.id, departDiff, arrivalDiff, ' '.join(map(str, exitTimesDiff))))
                del vehicles_orig[v.id]
            else:
                f.write('    <vehicle id="%s" comment="new"/>\n' % v.id)
        for id in vehicles_orig.keys():
            f.write('    <vehicle id="%s" comment="missing"/>\n' % id)
        f.write("</routeDiff>\n")

    if earliest_out is not None:
        with open(earliest_out, 'w') as f:
            for diff in reversed(sorted(earliest_diffs.keys())):
                f.write("%s, %s\n" % (diff, earliest_diffs[diff]))

    print(origDurations)
    print(durations)
    print(durationDiffs)


if __name__ == "__main__":
    write_diff(*sys.argv[1:])
