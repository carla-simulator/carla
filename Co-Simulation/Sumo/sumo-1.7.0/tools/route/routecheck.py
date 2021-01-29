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

# @file    routecheck.py
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @author  Yun-Pang Floetteroed
# @date    2007-03-09

"""
This script does simple checks for the routes on a given network.
Warnings will be issued if there is an unknown edge in the route,
if the route is disconnected,
or if the route definition does not use the edges attribute.
If one specifies -f or --fix all route files will be fixed
(if possible). At the moment this means adding an intermediate edge
if just one link is missing in a disconnected route, or adding an edges
attribute if it is missing.
All changes are documented within the output file which has the same name
as the input file with an additional .fixed suffix.
If the route file uses the old format (<route>edge1 edge2</route>)
this is changed without adding comments. The same is true for camelCase
changes of attributes.
"""
from __future__ import absolute_import
from __future__ import print_function
import os
import sys
try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO
from xml.sax import saxutils, make_parser, handler
from optparse import OptionParser
from collections import defaultdict

camelCase = {"departlane": "departLane",
             "departpos": "departPos",
             "departspeed": "departSpeed",
             "arrivallane": "arrivalLane",
             "arrivalpos": "arrivalPos",
             "arrivalspeed": "arrivalSpeed",
             "maxspeed": "maxSpeed",
             "bus_stop": "busStop",
             "vclass": "vClass",
             "fromtaz": "fromTaz",
             "totaz": "toTaz",
             "no": "number",
             "vtype": "vType",
             "vtypeDistribution": "vTypeDistribution",
             "tripdef": "trip"}

deletedKeys = defaultdict(list)
deletedKeys["route"] = ["edges", "multi_ref"]


class NetReader(handler.ContentHandler):

    def __init__(self):
        self._nb = {}

    def startElement(self, name, attrs):
        if name == 'edge' and ('function' not in attrs or attrs['function'] != 'internal'):
            self._nb[attrs['id']] = set()
        elif name == 'connection':
            if attrs['from'] in self._nb and attrs['to'] in self._nb:
                self._nb[attrs['from']].add(attrs['to'])

    def hasEdge(self, edge):
        return edge in self._nb

    def isNeighbor(self, orig, dest):
        return dest in self._nb[orig]

    def getIntermediateEdge(self, orig, dest):
        for inter in self._nb[orig]:
            if dest in self._nb[inter]:
                return inter
        return ''


class RouteReader(handler.ContentHandler):

    def __init__(self, net, outFileName):
        self._vID = ''
        self._routeID = ''
        self._routeString = ''
        self._addedString = ''
        self._net = net
        if outFileName:
            self._out = open(outFileName, 'w')
        else:
            self._out = None
        self._fileOut = None
        self._isRouteValid = True
        self._changed = False

    def startDocument(self):
        if self._out:
            print('<?xml version="1.0"?>', file=self._out)

    def endDocument(self):
        if self._out:
            self._out.close()
            if not self._changed:
                os.remove(self._out.name)

    def condOutputRedirect(self):
        if self._out and not self._fileOut:
            self._fileOut = self._out
            self._out = StringIO()

    def endOutputRedirect(self):
        if self._fileOut:
            if not self._isRouteValid:
                self._changed = True
                self._fileOut.write("<!-- ")
            self._fileOut.write(self._out.getvalue())
            if not self._isRouteValid:
                self._fileOut.write(" -->")
            if self._addedString != '':
                self._fileOut.write(
                    "<!-- added edges: " + self._addedString + "-->")
                self._addedString = ''
            self._out.close()
            self._out = self._fileOut
            self._fileOut = None

    def startElement(self, name, attrs):
        if name == 'vehicle' and 'route' not in attrs:
            self.condOutputRedirect()
            self._vID = attrs['id']
        if name == 'route':
            self.condOutputRedirect()
            if 'id' in attrs:
                self._routeID = attrs['id']
            else:
                self._routeID = "for vehicle " + self._vID
            self._routeString = ''
            if 'edges' in attrs:
                self._routeString = attrs['edges']
            else:
                self._changed = True
                print("Warning: No edges attribute in route " + self._routeID)
        elif self._routeID:
            print(
                "Warning: This script does not handle nested '%s' elements properly." % name)
        if self._out:
            if name in camelCase:
                name = camelCase[name]
                self._changed = True
            self._out.write('<' + name)
            if options.fix_length and 'length' in attrs and 'minGap' not in attrs:
                length = float(attrs["length"])
                minGap = 2.5
                if 'guiOffset' in attrs:
                    minGap = float(attrs["guiOffset"])
                attrs = dict(attrs)
                attrs["length"] = str(length - minGap)
                attrs["minGap"] = str(minGap)
                self._changed = True
            for key, value in sorted(attrs.items()):
                if key in camelCase:
                    key = camelCase[key]
                    self._changed = True
                if key not in deletedKeys[name]:
                    self._out.write(' %s="%s"' % (key, saxutils.escape(value)))
            if name != 'route':
                if name in ["ride", "stop", "walk"]:
                    self._out.write('/')
                self._out.write('>')

    def endElement(self, name):
        if name in ["ride", "stop", "walk"]:
            return
        if name == 'route':
            self._isRouteValid = self.testRoute()
            if self._out:
                self._out.write(' edges="%s"/>' % self._routeString)
            self._routeID = ''
            self._routeString = ''
            if self._vID == '':
                self.endOutputRedirect()
        elif name == 'vehicle' and self._vID != '':
            self._vID = ''
            if self._out:
                self._out.write('</vehicle>')
            self.endOutputRedirect()
        elif self._out:
            self._out.write('</%s>' % camelCase.get(name, name))

    def characters(self, content):
        if self._routeID != '':
            self._routeString += content
        elif self._out:
            self._out.write(saxutils.escape(content))

    def testRoute(self):
        if self._routeID != '':
            returnValue = True
            edgeList = self._routeString.split()
            if len(edgeList) == 0:
                print("Warning: Route %s is empty" % self._routeID)
                return False
            if net is None:
                return True
            doConnectivityTest = True
            cleanedEdgeList = []
            for v in edgeList:
                if self._net.hasEdge(v):
                    cleanedEdgeList.append(v)
                else:
                    print(
                        "Warning: Unknown edge " + v + " in route " + self._routeID)
                    returnValue = False
            while doConnectivityTest:
                doConnectivityTest = False
                for i, v in enumerate(cleanedEdgeList):
                    if i < len(cleanedEdgeList) - 1 and not self._net.isNeighbor(v, cleanedEdgeList[i + 1]):
                        print("Warning: Route " + self._routeID +
                              " disconnected between " + v + " and " + cleanedEdgeList[i + 1])
                        interEdge = self._net.getIntermediateEdge(
                            v, cleanedEdgeList[i + 1])
                        if interEdge != '':
                            cleanedEdgeList.insert(i + 1, interEdge)
                            self._changed = True
                            self._addedString += interEdge + " "
                            self._routeString = ' '.join(cleanedEdgeList)
                            doConnectivityTest = True
                            break
                        returnValue = False
            return returnValue
        return False

    def ignorableWhitespace(self, content):
        if self._out:
            self._out.write(content)

    def processingInstruction(self, target, data):
        if self._out:
            self._out.write('<?%s %s?>' % (target, data))


optParser = OptionParser(
    usage=os.path.basename(__file__) + " [options] <routes>*")
optParser.add_option("-v", "--verbose", action="store_true",
                     default=False, help="tell me what you are doing")
optParser.add_option("-f", "--fix", action="store_true",
                     default=False, help="fix errors into '.fixed' file")
optParser.add_option("-l", "--fix-length", action="store_true",
                     default=False, help="fix vehicle type's length and guiOffset attributes")
optParser.add_option("-i", "--inplace", action="store_true",
                     default=False, help="replace original files")
optParser.add_option("-n", "--net", help="network to check connectivity")
(options, args) = optParser.parse_args()
if len(args) == 0:
    optParser.print_help()
    sys.exit()
parser = make_parser()
net = None
if options.net:
    net = NetReader()
    parser.setContentHandler(net)
    parser.parse(options.net)
parser.setContentHandler(RouteReader(net, ''))

if options.fix_length:
    deletedKeys["vType"] += ['guiOffset']

for f in args:
    ffix = f + '.fixed'
    if options.fix:
        if options.verbose:
            print("fixing " + f)
        parser.setContentHandler(RouteReader(net, ffix))
    else:
        if options.verbose:
            print("checking " + f)
    parser.parse(f)
    if options.fix and os.path.exists(ffix) and options.inplace:
        os.rename(ffix, f)
