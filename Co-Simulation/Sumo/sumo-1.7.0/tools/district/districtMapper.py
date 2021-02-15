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

# @file    districtMapper.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @date    2007-07-26

"""
Maps the geometry of the districts of two networks by calculating
translation and scale parameters from junctions which have been
identified by the user as reference points.
"""
from __future__ import absolute_import
from __future__ import print_function
from xml.sax import make_parser, handler
from optparse import OptionParser


def parseShape(shape):
    poses = shape.split()
    ret = []
    for pos in poses:
        xy = pos.split(",")
        ret.append((float(xy[0]), float(xy[1])))
    return ret


# All members are "private".
class JunctionPositionsReader(handler.ContentHandler):

    def __init__(self):
        self._xPos = {}
        self._yPos = {}

    def startElement(self, name, attrs):
        if name == 'junction':
            self._xPos[attrs['id']] = float(attrs['x'])
            self._yPos[attrs['id']] = float(attrs['y'])

    def getJunctionXPoses(self, junctions1):
        ret = []
        for junction in junctions1:
            ret.append(self._xPos[junction])
        return ret

    def getJunctionYPoses(self, junctions1):
        ret = []
        for junction in junctions1:
            ret.append(self._yPos[junction])
        return ret


# All members are "private".
class DistrictMapper(handler.ContentHandler):

    def __init__(self):
        self._haveDistrict = False
        self._parsingDistrictShape = False
        self._districtShapes = {}
        self._currentID = ""
        self._shape = ""

    def startElement(self, name, attrs):
        if name == 'taz':
            self._haveDistrict = True
            self._currentID = attrs['id']
            if 'shape' in attrs:
                self._shape = attrs['shape']
        elif name == 'shape' and self._haveDistrict:
            self._parsingDistrictShape = True

    def characters(self, content):
        if self._parsingDistrictShape:
            self._shape += content

    def endElement(self, name):
        if name == 'taz':
            self._haveDistrict = False
            if self._shape != '':
                self._districtShapes[self._currentID] = parseShape(self._shape)
                self._shape = ""
        elif name == 'shape' and self._haveDistrict:
            self._parsingDistrictShape = False

    def convertShapes(self, xoff1, xoff2, xscale, yoff1, yoff2, yscale):
        for district in self._districtShapes:
            shape = self._districtShapes[district]
            nshape = []
            for i in range(0, len(shape)):
                nx = ((shape[i][0] - xoff1) * xscale + xoff2)
                ny = ((shape[i][1] - yoff1) * yscale + yoff2)
                nshape.append((nx, ny))
            self._districtShapes[district] = nshape

    def writeResults(self, output, color, polyoutput):
        fd = open(output, "w")
        fd.write("<tazs>\n")
        for district in self._districtShapes:
            shape = self._districtShapes[district]
            shapeStr = " ".join(["%s,%s" % s for s in shape])
            fd.write('   <taz id="%s" shape="%s"/>\n' % (district, shapeStr))
        fd.write("</tazs>\n")
        fd.close()
        if polyoutput:
            fd = open(polyoutput, "w")
            fd.write("<shapes>\n")
            for district in self._districtShapes:
                shape = self._districtShapes[district]
                shapeStr = " ".join(["%s,%s" % s for s in shape])
                fd.write('   <poly id="%s" color="%s" shape="%s"/>\n' %
                         (district, color, shapeStr))
            fd.write("</shapes>\n")
            fd.close()


if __name__ == "__main__":
    optParser = OptionParser()
    optParser.add_option("-v", "--verbose", action="store_true",
                         default=False, help="tell me what you are doing")
    optParser.add_option("-1", "--net-file1", dest="netfile1",
                         help="read first SUMO network from FILE (mandatory)", metavar="FILE")
    optParser.add_option("-2", "--net-file2", dest="netfile2",
                         help="read second SUMO network from FILE (mandatory)", metavar="FILE")
    optParser.add_option("-o", "--output", default="districts.add.xml",
                         help="write resulting districts to FILE (default: %default)", metavar="FILE")
    optParser.add_option("-p", "--polyoutput",
                         help="write districts as polygons to FILE", metavar="FILE")
    optParser.add_option("-a", "--junctions1",
                         help="list of junction ids to use from first network (mandatory)")
    optParser.add_option("-b", "--junctions2",
                         help="list of junction ids to use from second network (mandatory)")
    optParser.add_option("-c", "--color", default="1,0,0",
                         help="Assign this color to districts (default: %default)")
    (options, args) = optParser.parse_args()
    if not options.netfile1 or not options.netfile2 or not options.junctions1 or not options.junctions2:
        optParser.print_help()
        optParser.exit(
            "Error! Providing two networks and junction lists is mandatory")
    parser = make_parser()
    if options.verbose:
        print("Reading net#1")
    reader1 = JunctionPositionsReader()
    parser.setContentHandler(reader1)
    parser.parse(options.netfile1)
    if options.verbose:
        print("Reading net#2")
    reader2 = JunctionPositionsReader()
    parser.setContentHandler(reader2)
    parser.parse(options.netfile2)

    junctions1 = options.junctions1.split(",")
    junctions2 = options.junctions2.split(",")
    xposes1 = reader1.getJunctionXPoses(junctions1)
    yposes1 = reader1.getJunctionYPoses(junctions1)
    xposes2 = reader2.getJunctionXPoses(junctions2)
    yposes2 = reader2.getJunctionYPoses(junctions2)

    xmin1 = min(xposes1)
    xmax1 = max(xposes1)
    ymin1 = min(yposes1)
    ymax1 = max(yposes1)
    xmin2 = min(xposes2)
    xmax2 = max(xposes2)
    ymin2 = min(yposes2)
    ymax2 = max(yposes2)

    width1 = xmax1 - xmin1
    height1 = ymax1 - ymin1
    width2 = xmax2 - xmin2
    height2 = ymax2 - ymin2

    reader = DistrictMapper()
    parser.setContentHandler(reader)
    parser.parse(options.netfile1)
    reader.convertShapes(
        xmin1, xmin2, width1 / width2, ymin1, ymin2, height1 / height2)
    reader.writeResults(options.output, options.color, options.polyoutput)
