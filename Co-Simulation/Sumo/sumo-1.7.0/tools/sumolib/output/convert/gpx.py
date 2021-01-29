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

# @file    gpx.py
# @author  Jakob Erdmann
# @author  Laura Bieker
# @date    2014-02-13

"""
This module includes functions for converting SUMO's fcd-output into
GPX format (http://en.wikipedia.org/wiki/GPS_eXchange_Format)
"""
from __future__ import absolute_import
from collections import defaultdict


def fcd2gpx(inpFCD, outSTRM, ignored):
    tracks = defaultdict(list)
    for timestep in inpFCD:
        for v in timestep.vehicle:
            tracks[v.id].append((timestep.time, v.x, v.y))

    outSTRM.write('<?xml version="1.0" encoding="UTF-8"?>\n')
    outSTRM.write('<gpx version="1.0">\n')
    for vehicle in sorted(tracks):
        outSTRM.write("  <trk><name>%s</name><trkseg>\n" % vehicle)
        for timestamp, lon, lat in tracks[vehicle]:
            outSTRM.write('    <trkpt lon="%s" lat="%s"><time>%s</time></trkpt>\n' % (
                lon, lat, timestamp))
        outSTRM.write("  </trkseg></trk>\n")
    outSTRM.write('</gpx>\n')
