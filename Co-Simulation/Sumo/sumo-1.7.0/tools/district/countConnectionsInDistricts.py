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

# @file    countConnectionsInDistricts.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2007-07-26

from __future__ import absolute_import
from __future__ import print_function
from xml.sax import make_parser, handler
from optparse import OptionParser


# written into the net. All members are "private".
class NetDistrictConnectionCountingHandler(handler.ContentHandler):

    def __init__(self):
        self._currentID = ""
        self._districtSinkNo = {}
        self._districtSourceNo = {}

    def startElement(self, name, attrs):
        if name == 'taz':
            self._currentID = attrs['id']
        elif name == 'tazSink':
            if self._currentID in self._districtSinkNo:
                self._districtSinkNo[self._currentID] = self._districtSinkNo[
                    self._currentID] + 1
            else:
                self._districtSinkNo[self._currentID] = 1
        elif name == 'tazSource':
            if self._currentID in self._districtSinkNo:
                self._districtSourceNo[self._currentID] = self._districtSourceNo[
                    self._currentID] + 1
            else:
                self._districtSourceNo[self._currentID] = 1

    def writeResults(self, output):
        fd = open(output, "w")
        for district in self._districtSourceNo:
            fd.write(district + ";" + str(self._districtSourceNo[district]) + ";" + str(
                self._districtSinkNo[district]) + "\n")
        fd.close()


optParser = OptionParser()
optParser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                     default=False, help="tell me what you are doing")
optParser.add_option("-n", "--net-file", dest="netfile",
                     help="read SUMO network(s) from FILE(s) (mandatory)", metavar="FILE")
optParser.add_option("-o", "--output", dest="output",
                     help="read SUMO network(s) from FILE(s) (mandatory)", metavar="FILE")
(options, args) = optParser.parse_args()

parser = make_parser()
reader = NetDistrictConnectionCountingHandler()
parser.setContentHandler(reader)
if options.verbose:
    print("Reading net '" + options.netfile + "'")
parser.parse(options.netfile)
reader.writeResults(options.output)
