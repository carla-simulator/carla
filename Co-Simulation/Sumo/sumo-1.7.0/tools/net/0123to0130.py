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

# @file    0123to0130.py
# @author  Daniel Krajzewicz
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @date    2007

from __future__ import absolute_import
from __future__ import print_function
import os
import sys
import glob
import xml
from xml.sax import parse, handler
from optparse import OptionParser
from collections import defaultdict

# attributes sorting lists
a = {}
a['net'] = ('version', )
a['edge'] = ('id', 'from', 'to', 'name', 'priority',
             'type', 'function', 'spread_type', 'shape')
a['lane'] = ('id', 'index', 'vclasses', 'allow', 'disallow',
             'maxspeed', 'maxSpeed', 'length', 'endOffset', 'width', 'shape')
a['junction'] = ('id', 'type', 'x', 'y', 'incLanes', 'intLanes', 'shape')
a['logicitem'] = ('response', 'foes', 'cont')
a['succlane'] = ('via', 'tl', 'linkno', 'dir', 'state')
a['connection'] = (
    'from', 'to', 'fromLane', 'toLane', 'via', 'tl', 'linkIdx', 'dir', 'state')
a['row-logic'] = a['ROWLogic'] = ('id', 'requestSize')
a['tl-logic'] = a['tlLogic'] = ('id', 'type', 'programID', 'offset')
a['location'] = ('netOffset', 'convBoundary', 'origBoundary', 'projParameter')
a['phase'] = ('duration', 'state', 'min_dur', 'max_dur')
a['district'] = ('id', 'shape', 'edges')
a['dsink'] = ('id', 'weight')
a['dsource'] = ('id', 'weight')
a['taz'] = ('id', 'shape', 'edges')
a['tazSink'] = ('id', 'weight')
a['tazSource'] = ('id', 'weight')
a['roundabout'] = ('nodes', 'dummy')
a['request'] = ('index', 'response', 'foes', 'cont')
a['succ'] = ('edge', 'lane', 'junction')

# attributes which are optional
b = defaultdict(dict)
b['edge']['type'] = ''
b['edge']['function'] = 'normal'
b['succlane']['tl'] = ''
b['succlane']['linkno'] = ''
b['succlane']['linkIndex'] = ''
b['junction']['shape'] = ''

# elements which are single (not using opening/closing tag)
SINGLE = ('roundabout', 'logicitem', 'phase', 'succlane', 'dsource', 'dsink', 'location',
          'lane', 'timed_event', 'connection', 'request')

# remove these
REMOVED = ('lanes', 'logic', 'succ', 'row-logic', 'ROWLogic', 'logicitem')

# renamed elements
RENAMED_TAGS = {'tl-logic': 'tlLogic',
                'row-logic': 'ROWLogic',
                'district': 'taz',
                'dsource': 'tazSource',
                'dsink': 'tazSink',
                'succlane': 'connection'}

renamedAttrs = {'min_dur': 'minDur',
                'max_dur': 'maxDur',
                'spread_type': 'spreadType',
                'maxspeed': 'speed',
                'maxSpeed': 'speed',
                'linkIdx': 'linkIndex',
                'linkno': 'linkIndex'}

renamedValues = {'state': {'t': 'o'}, 'type': {'DEAD_END': 'dead_end'}}


def getBegin(file):
    fd = open(file)
    content = ""
    for line in fd:
        if '<net' in line or '<districts' in line or '<add' in line:
            fd.close()
            return content
        if '<?xml version' in line:
            continue
        content = content + line
    fd.close()
    return ""


class NetConverter(handler.ContentHandler):

    def __init__(self, outFileName, begin, has_no_destination):
        self._out = open(outFileName, "w")
        self._out.write('<?xml version="1.0" encoding="iso-8859-1"?>\n')
        self._out.write(begin)
        self._tree = []
        self._content = ""
        self._am_parsing_rowlogic = None  # parsing old element
        self._logicitems = defaultdict(list)  # maps junction ids to items
        self._laneCount = 0
        self._has_no_destination = has_no_destination

    def isSingle(self, name):
        if name in SINGLE:
            return True
        if name == "junction" and self._single_junction:
            return True
        return False

    def remove(self, name):
        if name == "succ" and self._has_no_destination:
            return False
        if name in REMOVED:
            return True
        else:
            return False

    def rename(self, name):
        if name == "succlane" and self._has_no_destination:
            return False
        if name in RENAMED_TAGS:
            return True
        else:
            return False

    def checkWrite(self, what):
        self._out.write(what.encode('iso-8859-1'))

    def indent(self):
        self._out.write(" " * (4 * len(self._tree)))

    def endDocument(self):
        self.checkWrite("\n")
        self._out.close()

    def startElement(self, name, attrs):
        # special preparations
        if name == "succ":
            self._succ_from = attrs["edge"]
            self._succ_fromIdx = attrs["lane"].split('_')[-1]
        if name == "row-logic" or name == "ROWLogic":
            self._am_parsing_rowlogic = attrs["id"]
        if name == "logicitem":
            self._logicitems[self._am_parsing_rowlogic].append(attrs)
        if name == "edge":
            self._laneCount = 0
        if name == "junction":
            self._single_junction = (attrs["id"][0] == ":" or
                                     attrs["type"] == 'DEAD_END' or
                                     attrs["type"] == 'dead_end')
        # skip removed
        if self.remove(name):
            return
        # compress empty lines
        if self._content.find("\n\n") >= 0:
            self.checkWrite("\n")
        self._content = ""
        # open tag
        self.indent()
        if self.rename(name):
            self.checkWrite("<" + RENAMED_TAGS[name])
        else:
            self.checkWrite("<" + name)
        # special attribute handling
        if name == "succlane":
            if self._has_no_destination:
                # keep original
                self.checkWrite(' lane="%s"' % attrs["lane"])
            else:
                # upgrade
                self.checkWrite(' from="%s"' % self._succ_from)
                sepIndex = attrs['lane'].rindex('_')
                toEdge = attrs['lane'][:sepIndex]
                toIdx = attrs['lane'][sepIndex + 1:]
                self.checkWrite(' to="%s"' % toEdge)
                self.checkWrite(' fromLane="%s"' % self._succ_fromIdx)
                self.checkWrite(' toLane="%s"' % toIdx)
        # write attributes
        if name == "net":
            self.checkWrite(
                ' version="0.13" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ' +
                'xsi:noNamespaceSchemaLocation="http://sumo.dlr.de/xsd/net_file.xsd"')
        else:
            if name in a:
                for key in a[name]:
                    val = None
                    if key in renamedAttrs and renamedAttrs[key] in attrs:
                        key = renamedAttrs[key]
                    if key in attrs:
                        val = attrs[key]
                        if key in renamedValues:
                            val = renamedValues[key].get(val, val)
                        if name == "succlane" and (key == "linkno" or key == "linkIndex") and attrs["tl"] == '':
                            val = ''
                        if name not in b or key not in b[name] or val != b[name][key]:
                            self.checkWrite(
                                ' ' + renamedAttrs.get(key, key) + '="%s"' % val)
                    elif name == "lane" and key == "index":
                        self.checkWrite(' %s="%s"' % (key, self._laneCount))
                        self._laneCount += 1
                    elif name == "connection" and key == "fromLane":
                        self.checkWrite(
                            ' fromLane="%s" toLane="%s"' % tuple(attrs["lane"].split(":")))
        # close tag
        if self.isSingle(name):
            self.checkWrite("/>\n")
        else:
            self.checkWrite(">\n")
        self._tree.append(name)
        # transfer items from removed element ROWLogic
        if name == "junction":
            for logicitem_attrs in self._logicitems[attrs["id"]]:
                self.indent()
                self.checkWrite("<request")
                self.checkWrite(' index="%s"' % logicitem_attrs["request"])
                for key in a["logicitem"]:
                    if key in logicitem_attrs:  # cont is optional
                        self.checkWrite(' ' + key + '="%s"' %
                                        logicitem_attrs[key])
                self.checkWrite("/>\n")

    def endElement(self, name):
        # special preparations
        if name == "row-logic" or name == "ROWLogic":
            self._am_parsing_rowlogic = None
        # skip removed
        if self.remove(name):
            return
        # optionaly write closing tag
        self._tree.pop()
        if name == "net":
            self.checkWrite("\n")
        if not self.isSingle(name):
            self.indent()
            if self.rename(name):
                self.checkWrite("</" + RENAMED_TAGS[name] + ">")
            else:
                self.checkWrite("</" + name + ">")
            if name != "net":
                self.checkWrite("\n")

    def characters(self, content):
        self._content += content

    def processingInstruction(self, target, data):
        self.checkWrite('<?%s %s?>' % (target, data))


def changeFile(fname):
    if options.verbose:
        print("Patching " + fname + " ...")
    if (("_deprecated_" in fname and "net.netconvert" not in fname) or
            (os.path.join('tools', 'net', '0') in fname)):
        print(
            "Skipping file (looks like intentionally deprecated input): " + fname)
        return
    has_no_destination = False
    if "SUMO_NO_DESTINATION" in open(fname).read():
        print(
            "Partial conversion (cannot convert SUMO_NO_DESTINATION): " + fname)
        has_no_destination = True
    net = NetConverter(fname + ".chg", getBegin(fname), has_no_destination)
    try:
        parse(fname, net)
    except xml.sax._exceptions.SAXParseException:
        print("Could not parse '%s' maybe it contains non-ascii chars?" %
              fname)
    if options.inplace:
        os.remove(fname)
        os.rename(fname + ".chg", fname)


def changeEdgeFile(fname):
    if options.verbose:
        print("Patching " + fname + " ...")
    out = open(fname + ".chg", 'w')
    for line in open(fname):
        if "<lane" in line:
            line = line.replace(" id", " index")
        if "<edge" in line:
            line = line.replace(" fromnode", " from").replace(" tonode", " to")
            line = line.replace(" spread_type", " spreadType").replace(
                " maxspeed", " maxSpeed")
            line = line.replace(" nolanes", " numLanes")
        out.write(line)
    out.close()
    if options.inplace:
        os.remove(fname)
        os.rename(fname + ".chg", fname)


def changeConnectionFile(fname):
    if options.verbose:
        print("Patching " + fname + " ...")
    out = open(fname + ".chg", 'w')
    for line in open(fname):
        if "<connection" in line and "lane" in line:
            line = line.replace(" lane", " fromLane")
        for i in range(10):
            line = line.replace(':%s"' % i, '" toLane="%s"' % i)
        out.write(line)
    out.close()
    if options.inplace:
        os.remove(fname)
        os.rename(fname + ".chg", fname)


def walkDir(srcRoot):
    for root, dirs, files in os.walk(srcRoot):
        for name in files:
            if name.endswith(".net.xml") or name in ["net.netconvert", "net.netgen",
                                                     "tls.scenario", "net.scenario"]:
                changeFile(os.path.join(root, name))
            if options.edges and name.endswith(".edg.xml"):
                changeEdgeFile(os.path.join(root, name))
            elif options.connections and name.endswith(".con.xml"):
                changeConnectionFile(os.path.join(root, name))
        for ignoreDir in ['.svn', 'foreign']:
            if ignoreDir in dirs:
                dirs.remove(ignoreDir)


optParser = OptionParser()
optParser.add_option("-v", "--verbose", action="store_true",
                     default=False, help="tell me what you are doing")
optParser.add_option("-i", "--inplace", action="store_true",
                     default=False, help="replace original files")
optParser.add_option("-e", "--edges", action="store_true",
                     default=False, help="include edge XML files")
optParser.add_option("-c", "--connections", action="store_true",
                     default=False, help="include connection XML files")
(options, args) = optParser.parse_args()

if len(args) == 0:
    print("Usage: " + sys.argv[0] + " <net>+")
    sys.exit()
for arg in args:
    for fname in glob.glob(arg):
        if os.path.isdir(fname):
            walkDir(fname)
        else:
            if options.edges and fname.endswith(".edg.xml"):
                changeEdgeFile(fname)
            elif options.connections and fname.endswith(".con.xml"):
                changeConnectionFile(fname)
            else:
                changeFile(fname)
