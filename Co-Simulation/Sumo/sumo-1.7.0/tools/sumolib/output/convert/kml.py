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

# @file    kml.py
# @author  Jakob Erdmann
# @author  Laura Bieker
# @author  Ronald Nippold
# @date    2019-08-20

"""
This module includes functions for converting SUMO's FCD output into Google's
KML format (https://developers.google.com/kml/documentation/kmlreference)
"""
from __future__ import absolute_import
from collections import defaultdict


def fcd2kml(inpFCD, outSTRM, ignored):
    tracks = defaultdict(list)
    for timestep in inpFCD:
        for v in timestep.vehicle:
            tracks[v.id].append((timestep.time, v.x, v.y))

    outSTRM.write('<?xml version="1.0" encoding="UTF-8"?>\n')
    outSTRM.write('<kml xmlns="http://www.opengis.net/kml/2.2">\n')
    outSTRM.write('<Document>\n')
    for vehicle in sorted(tracks):
        outSTRM.write("\t<name>SUMO FCD</name>\n")
        outSTRM.write("\t<open>1</open>\n")
        outSTRM.write("\t<description>traces of SUMO FCD output</description>\n")
        outSTRM.write("\t<Style id=\"trace-%s\">\n" % vehicle)
        outSTRM.write("\t\t<LineStyle>\n")
        outSTRM.write("\t\t<color>7f0000ff</color>\n")
        outSTRM.write("\t\t<colorMode>random</colorMode> \n")
        outSTRM.write("\t\t<width>6</width>\n")
        outSTRM.write("\t\t<gx:labelVisibility>1</gx:labelVisibility>\n")
        outSTRM.write("\t\t</LineStyle>\n")
        outSTRM.write("\t</Style>\n")
        outSTRM.write("\t<Placemark>\n")
        outSTRM.write("\t\t<name>FCD %s</name>\n" % vehicle)
        outSTRM.write("\t\t<styleUrl>#trace-%s</styleUrl>\n" % vehicle)
        outSTRM.write("\t\t<LineString>\n")
        outSTRM.write("\t\t<extrude>1</extrude>\n")
        outSTRM.write("\t\t<tessellate>1</tessellate>\n")
        outSTRM.write("\t\t\t<coordinates>\n")
        for timestamp, lon, lat in tracks[vehicle]:
            outSTRM.write('\t\t\t\t%s,%s\n' % (lon, lat))
        outSTRM.write("\t\t\t</coordinates>\n")
        outSTRM.write("\t\t</LineString>\n")
        outSTRM.write("\t</Placemark>\n")
    outSTRM.write('</Document>\n')
    outSTRM.write('</kml>\n')
