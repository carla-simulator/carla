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

# @file    net2kml.py
# @author  Jakob Erdmann
# @date    2020-02-21

"""
This script converts a sumo network to KML
"""
from __future__ import absolute_import
from __future__ import print_function
import os
import sys

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
import sumolib  # noqa


def parse_args():
    USAGE = "Usage: " + sys.argv[0] + " -n <net> <options>"
    argParser = sumolib.options.ArgumentParser(usage=USAGE)
    argParser.add_argument("-n", "--net-file", dest="netFile", help="The .net.xml file to convert")
    argParser.add_argument("-o", "--output-file", dest="outFile", help="The KML output file name")
    argParser.add_argument("-l", "--lanes", action="store_true", default=False,
                           help="Export lane geometries instead of edge geometries")
    argParser.add_argument("-i", "--internal", action="store_true", default=False,
                           help="Export internal geometries")
    argParser.add_argument("--color", default="0f0000ff", help="Color for normal edges")
    argParser.add_argument("--internal-color", dest="iColor", default="5f0000ff", help="Color for internal edges")

    options = argParser.parse_args()
    if not options.netFile:
        print("Missing arguments")
        argParser.print_help()
        exit()
    return options


def getGeometries(options, net):
    for edge in net.getEdges():
        if options.lanes:
            for lane in edge.getLanes():
                yield lane.getID(), lane.getShape(), lane.getWidth()
        else:
            yield edge.getID(), edge.getShape(), sum([l.getWidth() for l in edge.getLanes()])


if __name__ == "__main__":
    options = parse_args()
    net = sumolib.net.readNet(options.netFile, withInternal=options.internal)
    geomType = 'lane' if options.lanes else 'edge'

    with open(options.outFile, 'w') as outf:
        outf.write('<?xml version="1.0" encoding="UTF-8"?>\n')
        outf.write('<kml xmlns="http://www.opengis.net/kml/2.2">\n')
        outf.write('<Document>\n')
        for id, geometry, width in getGeometries(options, net):
            color = options.iColor if id[0] == ":" else options.color
            outf.write("\t<name>%s</name>\n" % options.netFile)
            outf.write("\t<open>1</open>\n")
            outf.write("\t<description>network geometries</description>\n")
            outf.write("\t<Style id=\"trace-%s\">\n" % id)
            outf.write("\t\t<LineStyle>\n")
            outf.write("\t\t<color>%s</color>\n" % color)
            # outf.write("\t\t<colorMode>random</colorMode> \n")
            outf.write("\t\t<width>%s</width>\n" % width)
            outf.write("\t\t<gx:labelVisibility>1</gx:labelVisibility>\n")
            outf.write("\t\t</LineStyle>\n")
            outf.write("\t</Style>\n")
            outf.write("\t<Placemark>\n")
            outf.write("\t\t<name>%s %s</name>\n" % (geomType, id))
            outf.write("\t\t<styleUrl>#trace-%s</styleUrl>\n" % id)
            outf.write("\t\t<LineString>\n")
            outf.write("\t\t<extrude>1</extrude>\n")
            outf.write("\t\t<tessellate>1</tessellate>\n")
            outf.write("\t\t\t<coordinates>\n")
            for x, y in geometry:
                lon, lat = net.convertXY2LonLat(x, y)
                outf.write('\t\t\t\t%.10f,%.10f\n' % (lon, lat))
            outf.write("\t\t\t</coordinates>\n")
            outf.write("\t\t</LineString>\n")
            outf.write("\t</Placemark>\n")
        outf.write('</Document>\n')
        outf.write('</kml>\n')
