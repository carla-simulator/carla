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

# @file    walkFactor.py
# @author  Jakob Erdmann
# @date    2017-12-04

"""compute person.walkfactor from walks in tripinfo-output"""

from __future__ import absolute_import
from __future__ import print_function
import os
import sys
sys.path.append(os.path.join(os.path.dirname(sys.argv[0]), '..'))
from sumolib.output import parse  # noqa
from sumolib.miscutils import Statistics  # noqa


def main(tripinfos, lengthThreshold=0.1):
    lengthThreshold = float(lengthThreshold)
    stats = Statistics('walkfactor')
    statsZeroDuration = Statistics('length of zero-duration walks')
    statsShort = Statistics('duration of short walks (length <= %s)' % lengthThreshold)
    numUnfinished = 0
    for personinfo in parse(tripinfos, 'personinfo'):
        if personinfo.hasChild('walk'):
            for i, walk in enumerate(personinfo.walk):
                if walk.arrival[0] == '-':
                    numUnfinished += 1
                    continue
                walkID = "%s.%s" % (personinfo.id, i)
                duration = float(walk.duration)
                routeLength = float(walk.routeLength)
                if duration > 0:
                    if routeLength <= lengthThreshold:
                        statsShort.add(duration, walkID)
                    else:
                        avgSpeed = routeLength / duration
                        walkFactor = avgSpeed / float(walk.maxSpeed)
                        stats.add(walkFactor, walkID)
                else:
                    statsZeroDuration.add(routeLength, walkID)

    print(stats)
    if statsZeroDuration.count() > 0:
        print(statsZeroDuration)
    if statsShort.count() > 0:
        print(statsShort)
    if numUnfinished > 0:
        print("unfinished walks: %s" % numUnfinished)


if __name__ == "__main__":
    main(*sys.argv[1:])
