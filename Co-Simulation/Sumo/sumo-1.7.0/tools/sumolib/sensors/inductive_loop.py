# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    inductive_loop.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @date    2010-02-18

"""
Library for reading and storing Inductive Loop detector representations and
 measurements.
"""
from __future__ import absolute_import

from xml.sax import handler, parse


class InductiveLoop:

    def __init__(self, id, lane, pos, frequency=60, file="NUL", friendlyPos=True):
        self.id = id
        self.lane = lane
        self.pos = pos
        self.frequency = frequency
        self.file = file
        self.friendlyPos = friendlyPos

    def toXML(self):
        return '<e1Detector id="%s" lane="%s" pos="%s" freq="%s" file="%s" friendlyPos="%s"/>' % (
            self.id, self.lane, self.pos, self.frequency, self.file, self.friendlyPos)


class InductiveLoopReader(handler.ContentHandler):

    def __init__(self):
        self._id2il = {}
        self._ils = []
        self._lastIL = None
        self.attributes = {}

    def startElement(self, name, attrs):
        if name == 'e1Detector':
            poi = InductiveLoop(attrs['id'], attrs['lane'], float(
                attrs['pos']), float(attrs['freq']), attrs['file'])
            self._id2il[poi.id] = poi
            self._ils.append(poi)
            self._lastIL = poi
        if name == 'param' and self._lastIL is not None:
            self._lastIL.attributes[attrs['key']] = attrs['value']

    def endElement(self, name):
        if name == 'e1Detector':
            self._lastIL = None


def read(filename):
    ils = InductiveLoopReader()
    parse(filename, ils)
    return ils._ils
