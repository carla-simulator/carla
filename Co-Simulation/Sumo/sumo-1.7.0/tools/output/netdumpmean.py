#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2008-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    netdumpmean.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2007-10-25

"""
This script reads two network dumps,
 computes the mean values
 and writes the results into the output file
"""
from __future__ import absolute_import
from __future__ import print_function
from optparse import OptionParser
from xml.sax import make_parser, handler
from datetime import datetime


class WeightsReader(handler.ContentHandler):

    """Reads the dump file"""

    def __init__(self):
        self._id = ''
        self._edgeValues = {}
        self._intervalBegins = []
        self._intervalEnds = []

    def startElement(self, name, attrs):
        if name == 'interval':
            self._beginTime = int(attrs['begin'])
            self._intervalBegins.append(self._beginTime)
            self._intervalEnds.append(int(attrs['end']))
            self._edgeValues[self._beginTime] = {}
        if name == 'edge':
            self._id = attrs['id']
            self._edgeValues[self._beginTime][self._id] = {}
            for attr in attrs.getQNames():
                if attr != "id":
                    self._edgeValues[self._beginTime][
                        self._id][attr] = float(attrs[attr])

    def sub(self, weights, exclude):
        for t in self._edgeValues:
            for e in self._edgeValues[t]:
                for a in self._edgeValues[t][e]:
                    if a not in exclude:
                        self._edgeValues[t][e][a] = (
                            self._edgeValues[t][e][a] + weights._edgeValues[t][e][a]) / 2.

    def write(self, options):
        fd = open(options.output, "w")
        fd.write("<?xml version=\"1.0\"?>\n\n")
        fd.write("<!-- generated on %s by netdumpdiv.py \n" % datetime.now())
        fd.write("   -1 %s\n" % options.dump1)
        fd.write("   -2 %s\n" % options.dump2)
        fd.write("   -o %s\n" % options.output)
        fd.write("-->\n\n")
        fd.write("<netstats>\n")
        for i in range(0, len(self._intervalBegins)):
            fd.write("   <interval begin=\"%s\" end=\"%s\">\n" %
                     (self._intervalBegins[i], self._intervalEnds[i]))
            t = self._intervalBegins[i]
            for e in self._edgeValues[t]:
                fd.write("      <edge id=\"%s\"" % e)
                for a in self._edgeValues[t][e]:
                    fd.write(" %s=\"%s\"" % (a, self._edgeValues[t][e][a]))
                fd.write("/>\n")
            fd.write("   </interval>\n")
        fd.write("</netstats>\n")


# initialise
optParser = OptionParser()
optParser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                     default=False, help="tell me what you are doing")
# i/o
optParser.add_option("-1", "--dump1", dest="dump1",
                     help="First dump (mandatory)", metavar="FILE")
optParser.add_option("-2", "--dump2", dest="dump2",
                     help="Second  dump (mandatory)", metavar="FILE")
optParser.add_option("-o", "--output", dest="output",
                     help="Name for the output", metavar="FILE")
optParser.add_option("-e", "--exclude", dest="exclude",
                     help="Exclude these values from being changed (stay as in 1)", metavar="FILE")
# parse options
(options, args) = optParser.parse_args()


parser = make_parser()
# read dump1
if options.verbose:
    print("Reading dump1...")
weights1 = WeightsReader()
parser.setContentHandler(weights1)
parser.parse(options.dump1)
# read dump2
if options.verbose:
    print("Reading dump2...")
weights2 = WeightsReader()
parser.setContentHandler(weights2)
parser.parse(options.dump2)
# process
if options.verbose:
    print("Computing mean...")
exclude = []
if options.exclude:
    exclude = options.exclude.split(",")
weights1.mean(weights2, exclude)
# save
if options.verbose:
    print("Writing...")
weights1.write(options)
