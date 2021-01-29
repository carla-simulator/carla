#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2009-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    0103to0110.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2007

from __future__ import absolute_import
from __future__ import print_function
import sys
from xml.sax import make_parser, handler

# attributes sorting lists
a = {}
a['edge'] = ('id', 'from', 'to', 'priority', 'type', 'function', 'inner')
a['lane'] = ('id', 'depart', 'vclasses', 'allow',
             'disallow', 'maxspeed', 'length', 'shape')
a['junction'] = ('id', 'type', 'x', 'y', 'incLanes', 'intLanes', 'shape')
a['logicitem'] = ('request', 'response', 'foes', 'cont')
a['succ'] = ('edge', 'lane', 'junction')
a['succlane'] = (
    'lane', 'via', 'tl', 'linkno', 'yield', 'dir', 'state', 'int_end')
a['row-logic'] = ('id', 'requestSize', 'laneNumber')
a['tl-logic'] = ('id', 'type', 'programID', 'offset')
a['location'] = ('netOffset', 'convBoundary', 'origBoundary', 'projParameter')

# elements which are single (not using opening/closing tag)
c = ('logicitem', 'phase', 'succlane', 'dsource', 'dsink',
     'junction', 'roundabout', 'location', 'lane', 'timed_event')

# delay output till this point
d = {}
d['junction'] = None
d['row-logic'] = 'logic'
d['tl-logic'] = 'phase'

# subelements to use as attributes
i = {}
i['junction'] = (
    ('inclanes', 'incLanes'), ('intlanes', 'intLanes'), ('shape', 'shape'))
i['row-logic'] = (('key', 'id'),
                  ('requestsize', 'requestSize'), ('lanenumber', 'laneNumber'))
i['tl-logic'] = (('key', 'id'), ('subkey', 'programID'), ('phaseno', 'phaseNo'),
                 ('offset', 'offset'), ('logicno', 'dismiss'), ('inclanes', 'inclanes'))

# join these
j = ('net-offset', 'conv-boundary', 'orig-boundary', 'orig-proj')


def getBegin(file):
    fd = open(file)
    content = ""
    for line in fd:
        if line.find("<net>") >= 0:
            fd.close()
            return content
        content = content + line
    fd.close()
    return ""


def patchPhase(attrs):
    state = ""
    for i in range(0, len(attrs['phase'])):
        c = 'g'
        if attrs['phase'][i] == '0':
            if attrs['yellow'][i] == '1':
                c = 'y'
            else:
                c = 'r'
        if attrs['brake'][i] == '0':
            if c == 'y':
                c = 'Y'
            if c == 'g':
                c = 'G'
        state = c + state
    return state


class NetConverter(handler.ContentHandler):

    def __init__(self, outFileName, begin):
        self._out = open(outFileName, "w")
        self._out.write(begin)
        self._collect = False
        self._tree = []
        self._attributes = {}
        self._shapePatch = False
        self._skipping = False
        self._hadShape = False

    def beginCollect(self):
        self._collect = True

    def endCollect(self):
        self._collect = False
        self.checkWrite(self._buffer)
        self._buffer = ""

    def checkWrite(self, what, isCharacters=False):
        cp = None
        if len(self._tree) > 2:
            if self._tree[-2] in i:
                for p in i[self._tree[-2]]:
                    if self._tree[-1] == p[0]:
                        cp = p
                if cp and isCharacters:
                    if cp[1] in self._attributes:
                        self._attributes[
                            cp[1]] = self._attributes[cp[1]] + what
                    else:
                        self._attributes[cp[1]] = what
        if len(self._tree) > 1:
            if self._tree[-1] in d:
                cp = 1
        if not cp:
            self._out.write(what)
            if not self._skipping:
                self._attributes = {}

    def flushStored(self, name):
        if len(self._attributes) == 0:
            return
        self._out.write("<" + name)
        for key in a[name]:
            if key in self._attributes:
                self._out.write(' ' + key + '="' + self._attributes[key] + '"')
            else:
                self._out.write(' ' + key + '=""')
        self._attributes = {}
        if name in c:
            self._out.write("/>")
        else:
            self._out.write(">")

    def endDocument(self):
        self.checkWrite("\n")
        self._out.close()

    def startElement(self, name, attrs):
        if len(self._tree) > 0 and self._tree[-1] in d and d[self._tree[-1]] == name:
            self.flushStored(self._tree[-1])
            self._out.write("\n" + self._lastChars)
        self._tree.append(name)
        if name in j:
            self._skipping = True
            return
        self.checkWrite("<" + name)
        if name in d:
            self._attributes = {}
            for key in attrs.getNames():
                self._attributes[key] = attrs[key]
        elif name == "phase" and 'phase' in attrs:
            # patch phase definition
            state = patchPhase(attrs)
            for key in attrs.getNames():
                if key == 'phase':
                    self.checkWrite(' state="' + state + '"')
                elif key != 'yellow' and key != 'brake':
                    self.checkWrite(' ' + key + '="' + attrs[key] + '"')
        elif name == 'lane' and 'vclasses' in attrs:
            for key in a['lane']:
                if key == 'vclasses':
                    allowed = []
                    disallowed = []
                    for clazz in attrs['vclasses'].split(";"):
                        if clazz:
                            if clazz[0] == '-':
                                disallowed.append(clazz[1:])
                            else:
                                allowed.append(clazz)
                    if allowed:
                        self.checkWrite(' allow="%s"' % (" ".join(allowed)))
                    if disallowed:
                        self.checkWrite(' disallow="%s"' %
                                        (" ".join(disallowed)))
                elif key in attrs:
                    self.checkWrite(' ' + key + '="' + attrs[key] + '"')
        else:
            if name not in a:
                for key in attrs.getNames():
                    self.checkWrite(' ' + key + '="' + attrs[key] + '"')
            else:
                for key in a[name]:
                    if key in attrs:
                        self.checkWrite(' ' + key + '="' + attrs[key] + '"')
        if (name != "lane" and name != "poly") or "shape" in attrs:
            if name in c:
                self.checkWrite("/")
            self.checkWrite(">")
        else:
            self.checkWrite(" shape=\"")
            self._shapePatch = True

    def endElement(self, name):
        if name in d:
            if not d[name]:
                self.flushStored(name)
            else:
                self._out.write("\n" + self._lastChars + "</" + name + ">")
        elif name == "orig-proj":
            self._out.write(
                "<location netOffset=\"" + self._attributes["net-offset"])
            self._out.write(
                "\" convBoundary=\"" + self._attributes["conv-boundary"])
            self._out.write(
                "\" origBoundary=\"" + self._attributes["orig-boundary"])
            self._out.write(
                "\" projParameter=\"" + self._attributes["orig-proj"])
            self._out.write("\"/>")
            self._attributes = {}
            self._skipping = False
        elif name in j:
            name = ""
        elif self._shapePatch:
            self.checkWrite("\"/>")
            self._shapePatch = False
        elif name not in c:
            self.checkWrite("</" + name)
            self.checkWrite(">")
        self._tree.pop()

    def characters(self, content):
        if self._skipping:
            if content.strip() != "":
                e = self._tree[-1]
                if e in self._attributes:
                    self._attributes[e] = self._attributes[e] + content
                else:
                    self._attributes[e] = content
            return
        self._lastChars = content
        self.checkWrite(content, True)

    def ignorableWhitespace(self, content):
        self.checkWrite(content)

    def skippedEntity(self, content):
        self.checkWrite(content)

    def processingInstruction(self, target, data):
        self.checkWrite('<?%s %s?>' % (target, data))


if len(sys.argv) < 2:
    print("Usage: " + sys.argv[0] + " <net>")
    sys.exit()
beg = getBegin(sys.argv[1])
parser = make_parser()
net = NetConverter(sys.argv[1] + ".chg", beg)
parser.setContentHandler(net)
parser.parse(sys.argv[1])
