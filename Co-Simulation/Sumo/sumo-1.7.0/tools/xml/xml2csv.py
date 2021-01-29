#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    xml2csv.py
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @author  Laura Bieker
# @date    2013-12-08


from __future__ import print_function
from __future__ import absolute_import
import os
import sys
import socket
import gzip
import io
import collections
from optparse import OptionParser
import xml.sax
try:
    import lxml.etree
    import lxml.sax
    haveLxml = True
except ImportError:
    haveLxml = False

import xsd


class NestingHandler(xml.sax.handler.ContentHandler):

    """A handler which knows the current nesting of tags"""

    def __init__(self):
        self.tagstack = []

    def startElement(self, name, attrs):
        self.tagstack.append(name)

    def endElement(self, name):
        self.tagstack.pop()

    def depth(self):
        # do not count the root element
        return len(self.tagstack) - 1


class AttrFinder(NestingHandler):

    def __init__(self, xsdFile, source, split):
        NestingHandler.__init__(self)
        self.tagDepths = {}  # tag -> depth of appearance
        self.tagAttrs = collections.defaultdict(
            collections.OrderedDict)  # tag -> set of attrs
        self.renamedAttrs = {}  # (name, attr) -> renamedAttr
        self.attrs = {}
        self.depthTags = {}  # child of root: depth of appearance -> tag list
        self.rootDepth = 1 if split else 0
        if xsdFile:
            self.xsdStruc = xsd.XsdStructure(xsdFile)
            if split:
                for ele in self.xsdStruc.root.children:
                    self.attrs[ele.name] = []
                    self.depthTags[ele.name] = [[]]
                    self.recursiveAttrFind(ele, ele, 1)
            else:
                self.attrs[self.xsdStruc.root.name] = []
                self.depthTags[self.xsdStruc.root.name] = []
                self.recursiveAttrFind(
                    self.xsdStruc.root, self.xsdStruc.root, 0)
        else:
            self.xsdStruc = None
            xml.sax.parse(source, self)

    def addElement(self, root, name, depth):
        # print("adding", root, name, depth)
        if len(self.depthTags[root]) == depth:
            self.tagDepths[name] = depth
            self.depthTags[root].append([name])
            return True
        if name not in self.tagDepths:
            self.depthTags[root][depth].append(name)
            return True
        if name not in self.depthTags[root][depth]:
            print("Ignoring tag %s at depth %s" %
                  (name, depth), file=sys.stderr)
        return False

    def recursiveAttrFind(self, root, currEle, depth):
        if not self.addElement(root.name, currEle.name, depth):
            return
        for a in currEle.attributes:
            if ":" not in a.name:  # no namespace support yet
                self.tagAttrs[currEle.name][a.name] = a
                anew = "%s_%s" % (currEle.name, a.name)
                self.renamedAttrs[(currEle.name, a.name)] = anew
                attrList = self.attrs[root.name]
                if anew in attrList:
                    del attrList[attrList.index(anew)]
                attrList.append(anew)
        for ele in currEle.children:
            # print("attr", root.name, ele.name, depth)
            self.recursiveAttrFind(root, ele, depth + 1)

    def startElement(self, name, attrs):
        NestingHandler.startElement(self, name, attrs)
        if self.depth() >= self.rootDepth:
            root = self.tagstack[self.rootDepth]
            if self.depth() == self.rootDepth and root not in self.attrs:
                self.attrs[root] = []
                self.depthTags[root] = [[]] * self.rootDepth
            if not self.addElement(root, name, self.depth()):
                return
            # collect attributes
            for a in sorted(list(attrs.keys())):
                if a not in self.tagAttrs[name] and ":" not in a:
                    self.tagAttrs[name][a] = xsd.XmlAttribute(a)
                    if not (name, a) in self.renamedAttrs:
                        anew = "%s_%s" % (name, a)
                        self.renamedAttrs[(name, a)] = anew
                        self.attrs[root].append(anew)


class CSVWriter(NestingHandler):

    def __init__(self, attrFinder, options):
        NestingHandler.__init__(self)
        self.attrFinder = attrFinder
        self.options = options
        self.currentValues = collections.defaultdict(lambda: "")
        self.haveUnsavedValues = False
        self.outfiles = {}
        self.rootDepth = 1 if options.split else 0
        for root in sorted(attrFinder.depthTags):
            if not options.output:
                if isinstance(options.source, str):
                    options.output = os.path.splitext(options.source)[0]
                else:
                    options.output = options.source.name
            if len(attrFinder.depthTags) == 1:
                if not options.output.isdigit() and not options.output.endswith(".csv"):
                    options.output += ".csv"
                self.outfiles[root] = getOutStream(options.output)
            else:
                outfilename = options.output + "%s.csv" % root
                self.outfiles[root] = getOutStream(outfilename)
            self.outfiles[root].write(
                options.separator.join(map(self.quote, attrFinder.attrs[root])) + u"\n")

    def quote(self, s):
        return "%s%s%s" % (self.options.quotechar, s, self.options.quotechar)

# the following two are needed for the lxml saxify to work
    def startElementNS(self, name, qname, attrs):
        self.startElement(qname, attrs)

    def endElementNS(self, name, qname):
        self.endElement(qname)

    def startElement(self, name, attrs):
        NestingHandler.startElement(self, name, attrs)
        if self.depth() >= self.rootDepth:
            root = self.tagstack[self.rootDepth]
            # print("start", name, root, self.depth(), self.attrFinder.depthTags[root][self.depth()])
            if name in self.attrFinder.depthTags[root][self.depth()]:
                for a, v in attrs.items():
                    if isinstance(a, tuple):
                        a = a[1]
                    # print(a, dict(self.attrFinder.tagAttrs[name]))
                    if a in self.attrFinder.tagAttrs[name]:
                        if self.attrFinder.xsdStruc:
                            enum = self.attrFinder.xsdStruc.getEnumeration(
                                self.attrFinder.tagAttrs[name][a].type)
                            if enum:
                                v = enum.index(v)
                        a2 = self.attrFinder.renamedAttrs.get((name, a), a)
                        self.currentValues[a2] = v
                        self.haveUnsavedValues = True

    def endElement(self, name):
        if self.depth() >= self.rootDepth:
            root = self.tagstack[self.rootDepth]
            # print("end", name, root, self.depth(), self.attrFinder.depthTags[root][self.depth()],
            # self.haveUnsavedValues)
            if name in self.attrFinder.depthTags[root][self.depth()]:
                if self.haveUnsavedValues:
                    self.outfiles[root].write(self.options.separator.join(
                        [self.quote(self.currentValues[a]) for a in self.attrFinder.attrs[root]]) + u"\n")
                    self.haveUnsavedValues = False
                for a in self.attrFinder.tagAttrs[name]:
                    a2 = self.attrFinder.renamedAttrs.get((name, a), a)
                    del self.currentValues[a2]
        NestingHandler.endElement(self, name)


def getSocketStream(port, mode='rb'):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind(("localhost", port))
    s.listen(1)
    conn, _ = s.accept()
    return conn.makefile(mode)


def getOutStream(output):
    if output.isdigit():
        return getSocketStream(int(output), 'wb')
    if output.endswith(".gz"):
        return gzip.open(output, 'wb', encoding="utf8")
    return io.open(output, 'w', encoding="utf8")


def get_options(arglist=None):
    optParser = OptionParser(
        usage=os.path.basename(sys.argv[0]) + " [<options>] <input_file_or_port>")
    optParser.add_option("-s", "--separator", default=";",
                         help="separating character for fields")
    optParser.add_option("-q", "--quotechar", default='',
                         help="quoting character for fields")
    optParser.add_option("-x", "--xsd", help="xsd schema to use")
    optParser.add_option("-a", "--validation", action="store_true",
                         default=False, help="enable schema validation")
    optParser.add_option("-p", "--split", action="store_true",
                         default=False, help="split in different files for the first hierarchy level")
    optParser.add_option("-o", "--output", help="base name for output")
    options, args = optParser.parse_args(arglist)
    if len(args) != 1:
        optParser.print_help()
        sys.exit()
    if options.validation and not haveLxml:
        print("lxml not available, skipping validation", file=sys.stderr)
        options.validation = False
    if args[0].isdigit():
        if not options.xsd:
            print("a schema is mandatory for stream parsing", file=sys.stderr)
            sys.exit()
        options.source = getSocketStream(int(args[0]))
    elif args[0].endswith(".gz"):
        options.source = gzip.open(args[0])
    else:
        options.source = args[0]
    if options.output and options.output.isdigit() and options.split:
        print("it is not possible to use splitting together with stream output", file=sys.stderr)
        sys.exit()
    return options


def main(args=None):
    options = get_options(args)
    # get attributes
    attrFinder = AttrFinder(options.xsd, options.source, options.split)
    # write csv
    handler = CSVWriter(attrFinder, options)
    if options.validation:
        schema = lxml.etree.XMLSchema(file=options.xsd)
        parser = lxml.etree.XMLParser(schema=schema)
        tree = lxml.etree.parse(options.source, parser)
        lxml.sax.saxify(tree, handler)
    else:
        if not options.xsd and hasattr(options.source, "name") and options.source.name.endswith(".gz"):
            # we need to reopen the file because the AttrFinder already read and closed it
            options.source = gzip.open(options.source.name)
        xml.sax.parse(options.source, handler)


if __name__ == "__main__":
    main()
