#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    xml2protobuf.py
# @author  Michael Behrisch
# @author  Laura Bieker
# @author  Jakob Erdmann
# @date    2014-01-22


from __future__ import print_function
from __future__ import absolute_import
import os
import sys
import subprocess
import importlib
import struct
from optparse import OptionParser
import xml.sax
try:
    import lxml.etree
    import lxml.sax
    haveLxml = True
except ImportError:
    haveLxml = False

import xml2csv


def capitalFirst(s):
    return s[0].upper() + s[1:]


class ProtoWriter(xml.sax.handler.ContentHandler):

    def __init__(self, module, attrFinder, output):
        self.module = module
        self.attrFinder = attrFinder
        self.out = xml2csv.getOutStream(output)
        self.msgStack = []
        self.emptyRootMsg = None

    def convert(self, attr, value):
        typ = getProtobufType(attr.type)
        if typ == "float" or typ == "double":
            return float(value)
        if typ == "int32" or typ == "uint32":
            return int(value)
        return value

    def startElementNS(self, name, qname, attrs):
        self.startElement(qname, attrs)

    def endElementNS(self, name, qname):
        self.endElement(qname)

    def startElement(self, name, attrs):
        if len(self.msgStack) == 0:
            self.emptyRootMsg = vars(self.module)[capitalFirst(name)]()
            obj = vars(self.module)[capitalFirst(name)]()
        else:
            obj = getattr(self.msgStack[-1], name).add()
        for a, v in attrs.items():
            if a in self.attrFinder.tagAttrs[name]:
                enum = self.attrFinder.xsdStruc.getEnumeration(
                    self.attrFinder.tagAttrs[name][a].type)
                if enum:
                    v = enum.index(v)
                setattr(
                    obj, a, self.convert(self.attrFinder.tagAttrs[name][a], v))
        if len(self.msgStack) == 0:
            self.emptyRootMsg.CopyFrom(obj)
        self.msgStack.append(obj)

    def endElement(self, name):
        if len(self.msgStack) == 2:
            self.out.write(struct.pack('>L', self.msgStack[0].ByteSize()))
            self.out.write(self.msgStack[0].SerializeToString())
            self.msgStack[0].Clear()
            self.msgStack[0].CopyFrom(self.emptyRootMsg)
        self.msgStack = self.msgStack[:-1]

    def endDocument(self):
        self.out.write(struct.pack('>L', 0))
        self.out.close()


def get_options():
    optParser = OptionParser(
        usage=os.path.basename(sys.argv[0]) + " [<options>] <input_file_or_port>")
    optParser.add_option("-p", "--protodir", default=".",
                         help="where to put and read .proto files")
    optParser.add_option("-x", "--xsd", help="xsd schema to use (mandatory)")
    optParser.add_option("-a", "--validation", action="store_true",
                         default=False, help="enable schema validation")
    optParser.add_option("-o", "--output", help="output file name")
    options, args = optParser.parse_args()
    if len(args) != 1:
        optParser.print_help()
        sys.exit()
    if not options.xsd:
        print("a schema is mandatory", file=sys.stderr)
        sys.exit()
    if options.validation and not haveLxml:
        print("lxml not available, skipping validation", file=sys.stderr)
        options.validation = False
    if args[0].isdigit():
        options.source = xml2csv.getSocketStream(int(args[0]))
    else:
        options.source = args[0]
    if not options.output:
        options.output = os.path.splitext(args[0])[0] + ".protomsg"
    return options


def getProtobufType(typ):
    if ":" in typ:
        typ = typ.split(":")[-1]
    ltyp = typ.lower()
    if ltyp == "decimal" or "double" in ltyp:
        return "double"
    if "float" in ltyp:
        return "float"
    if "unsigned" in ltyp:
        return "uint32"
    if "int" in ltyp or "short" in ltyp or "byte" in ltyp:
        return "int32"
    if ltyp in ["double", "string"]:
        return ltyp
    return capitalFirst(typ)


def writeField(protof, use, typ, name, tagNumber):
    if use == "":
        use = "optional"
    protof.write("  %s %s %s = %s;\n" %
                 (use, getProtobufType(typ), name, tagNumber))


def generateProto(tagAttrs, depthTags, enums, protodir, base):
    with open(os.path.join(protodir, "%s.proto" % base), 'w') as protof:
        protof.write('syntax = "proto2";\npackage %s;\n' % base)
        for name, enum in enums.iteritems():
            protof.write("\nenum %s {\n" % capitalFirst(name))
            for idx, entry in enumerate(enum):
                if entry[0].isdigit():
                    entry = "_" + entry
                protof.write("  %s = %s;\n" % (entry, idx))
            protof.write("}\n")
        for tagList in depthTags.itervalues():
            next = 1
            for tags in tagList:
                for tag in tags:
                    protof.write("\nmessage %s {\n" % capitalFirst(tag))
                    count = 1
                    for a in tagAttrs[tag].itervalues():
                        writeField(protof, a.use, a.type, a.name, count)
                        count += 1
                    if next < len(tagList):
                        for n in tagList[next]:
                            writeField(protof, "repeated", n, n, count)
                            count += 1
                    next += 1
                    protof.write("}\n")
    subprocess.call(
        ["protoc", "%s.proto" % base, "--python_out=%s" % protodir])
    sys.path.append(protodir)
    return importlib.import_module("%s_pb2" % base)


def main():
    options = get_options()
    # get attributes
    attrFinder = xml2csv.AttrFinder(options.xsd, options.source, False)
    base = os.path.basename(options.xsd).split('.')[0]
    # generate proto format description
    module = generateProto(attrFinder.tagAttrs, attrFinder.depthTags,
                           attrFinder.xsdStruc._namedEnumerations, options.protodir, base)
    # write proto message
    handler = ProtoWriter(module, attrFinder, options.output)
    if options.validation:
        schema = lxml.etree.XMLSchema(file=options.xsd)
        parser = lxml.etree.XMLParser(schema=schema)
        tree = lxml.etree.parse(options.source, parser)
        lxml.sax.saxify(tree, handler)
    else:
        xml.sax.parse(options.source, handler)


if __name__ == "__main__":
    main()
