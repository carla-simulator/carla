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

# @file    edgesInDistricts.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @date    2007-07-26

"""
Parsing a number of networks and taz (district) files with shapes
this script writes a taz file with all the edges which are inside
the relevant taz.
"""
from __future__ import print_function
from __future__ import absolute_import
import os
import sys
import argparse
import collections
from xml.sax import parse
sys.path.append(os.path.join(os.environ["SUMO_HOME"], 'tools'))
import sumolib  # noqa


# written into the net. All members are "private".
class DistrictEdgeComputer:

    def __init__(self, net):
        self._net = net
        self._districtEdges = collections.defaultdict(list)
        self._edgeDistricts = collections.defaultdict(list)
        self._invalidatedEdges = set()

    def computeWithin(self, polygons, options):
        districtBoxes = {}
        for district in polygons:
            districtBoxes[district.id] = district.getBoundingBox()
        for idx, edge in enumerate(self._net.getEdges()):
            shape = edge.getShape()
            if (edge.getSpeed() < options.maxspeed and edge.getSpeed() > options.minspeed and
                    (options.internal or edge.getFunction() != "internal")):
                if options.vclass is None or edge.allows(options.vclass):
                    if options.assign_from:
                        xmin, ymin = shape[0]
                        xmax, ymax = shape[0]
                    else:
                        xmin, ymin, xmax, ymax = edge.getBoundingBox()
                    for district in polygons:
                        dxmin, dymin, dxmax, dymax = districtBoxes[district.id]
                        if dxmin <= xmax and dymin <= ymax and dxmax >= xmin and dymax >= ymin:
                            if options.assign_from:
                                if sumolib.geomhelper.isWithin(shape[0], district.shape):
                                    self._districtEdges[district].append(edge)
                                    self._edgeDistricts[edge].append(district)
                                    break
                            else:
                                for pos in shape:
                                    if sumolib.geomhelper.isWithin(pos, district.shape):
                                        self._districtEdges[
                                            district].append(edge)
                                        self._edgeDistricts[
                                            edge].append(district)
                                        break
            if options.verbose and idx % 100 == 0:
                sys.stdout.write("%s/%s\r" % (idx, len(self._net.getEdges())))
        if options.complete:
            for edge in self._edgeDistricts:
                if len(self._edgeDistricts[edge]) > 1:
                    self._invalidatedEdges.add(edge)

    def getEdgeDistrictMap(self):
        result = {}
        for edge, districts in self._edgeDistricts.items():
            if len(districts) == 1:
                result[edge] = districts[0]
        return result

    def writeResults(self, options):
        fd = open(options.output, "w")
        sumolib.xml.writeHeader(fd, "$Id$", "tazs", "taz_file.xsd")  # noqa
        lastId = None
        lastEdges = set()
        key = (lambda i: i[0].attributes[options.merge_param]) if options.merge_param else None
        for idx, (district, edges) in enumerate(sorted(self._districtEdges.items(), key=key)):
            filtered = [edge for edge in edges if edge not in self._invalidatedEdges and edge.getLength() >
                        options.min_length]
            if len(filtered) == 0:
                print("District '" + district.id + "' has no edges!")
            else:
                if options.weighted:
                    if options.shapeinfo:
                        color = ' color="%s"' % district.color if district.color is not None else ''
                        fd.write('    <taz id="%s" shape="%s"%s>\n' %
                                 (district.id, district.getShapeString(), color))
                    else:
                        fd.write('    <taz id="%s">\n' % district.id)
                    for edge in filtered:
                        weight = edge.getSpeed() * edge.getLength()
                        fd.write(
                            '        <tazSource id="%s" weight="%.2f"/>\n' % (edge.getID(), weight))
                        fd.write(
                            '        <tazSink id="%s" weight="%.2f"/>\n' % (edge.getID(), weight))
                    fd.write("    </taz>\n")
                else:
                    if options.shapeinfo:
                        fd.write('    <taz id="%s" shape="%s" edges="%s"/>\n' %
                                 (district.id, district.getShapeString(), " ".join([e.getID() for e in filtered])))
                    else:
                        currentId = district.id
                        if options.merge_param is not None:
                            currentId = district.attributes[options.merge_param]
                        if options.merge_separator is not None and options.merge_separator in currentId:
                            currentId = currentId[:currentId.index(options.merge_separator)]
                        if lastId is not None:
                            if lastId == currentId:
                                lastEdges.update(filtered)
                            else:
                                fd.write('    <taz id="%s" edges="%s"/>\n' %
                                         (lastId, " ".join(sorted([e.getID() for e in lastEdges]))))
                                lastId = None
                        if lastId is None:
                            lastId = currentId
                            lastEdges = set(filtered)
            if options.verbose and idx % 100 == 0:
                sys.stdout.write("%s/%s\r" % (idx, len(self._districtEdges)))
        if lastId is not None:
            fd.write('    <taz id="%s" edges="%s"/>\n' % (lastId, " ".join(sorted([e.getID() for e in lastEdges]))))
        fd.write("</tazs>\n")
        fd.close()

    def getTotalLength(self, edgeID):
        edge = self._net.getEdge(edgeID)
        return edge.getLength() * edge.getLaneNumber()


def fillOptions(argParser):
    argParser.add_argument("-v", "--verbose", action="store_true",
                           default=False, help="tell me what you are doing")
    argParser.add_argument("--complete", action="store_true",
                           default=False, help="assign edges only if they are not in more than one district")
    argParser.add_argument("-n", "--net-file",
                           help="read SUMO network from FILE (mandatory)", metavar="FILE")
    argParser.add_argument("-t", "--taz-files",
                           help="read districts from FILEs", metavar="FILE")
    argParser.add_argument("-o", "--output", default="districts.taz.xml",
                           help="write results to FILE", metavar="FILE")
    argParser.add_argument("-x", "--max-speed", type=float, dest="maxspeed",
                           default=1000.0, help="use lanes where speed is not greater than this (m/s)")
    argParser.add_argument("-m", "--min-speed", type=float, dest="minspeed",
                           default=0., help="use lanes where speed is greater than this (m/s)")
    argParser.add_argument("-w", "--weighted", action="store_true",
                           default=False, help="Weights sources/sinks by lane number and length")
    argParser.add_argument("-f", "--assign-from", action="store_true",
                           default=False, help="Assign the edge always to the district where the \"from\" node " +
                           "is located")
    argParser.add_argument("-i", "--internal", action="store_true",
                           default=False, help="Include internal edges in output")
    argParser.add_argument("-l", "--vclass", help="Include only edges allowing VCLASS")
    argParser.add_argument("-s", "--shapeinfo", action="store_true",
                           default=False, help="write also the shape info in the file")
    argParser.add_argument("--merge-separator",
                           help="merge edge lists of taz starting with the same string up to the given separator")
    argParser.add_argument("--merge-param",
                           help="merge edge lists of taz/polygons having the same value for the given parameter")
    argParser.add_argument("--min-length", type=float,
                           default=0., help="use edges where length is greater than this (m)")


def parse_args(args=None):
    argParser = sumolib.options.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    fillOptions(argParser)
    return argParser.parse_args(args), argParser


if __name__ == "__main__":
    options, argParser = parse_args()
    if not options.net_file:
        argParser.print_help()
        argParser.exit("Error! Providing a network is mandatory")

    if options.verbose:
        print("Reading net '" + options.net_file + "'")
    nets = options.net_file.split(",")
    if len(nets) > 1:
        print("Warning! Multiple networks specified. Parsing the first one for edges and tazs, the others for " +
              "taz only.")
    reader = DistrictEdgeComputer(sumolib.net.readNet(nets[0]))
    tazFiles = nets + options.taz_files.split(",")
    polyReader = sumolib.shapes.polygon.PolygonReader(True)
    for tf in tazFiles:
        parse(tf, polyReader)
    if options.verbose:
        print("Calculating")
    reader.computeWithin(polyReader.getPolygons(), options)
    if options.verbose:
        print("Writing results")
    reader.writeResults(options)
