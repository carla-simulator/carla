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

# @file    protobuf2xml.py
# @author  Michael Behrisch
# @author  Laura Bieker
# @author  Jakob Erdmann
# @date    2014-01-23


from __future__ import print_function
from __future__ import absolute_import
import os
import sys
import struct
import contextlib

from optparse import OptionParser
import google.protobuf.descriptor
import xml2csv
import xml2protobuf


def get_options():
    optParser = OptionParser(
        usage=os.path.basename(sys.argv[0]) + " [<options>] <input_file_or_port>")
    optParser.add_option("-p", "--protodir", default=".",
                         help="where to put and read .proto files")
    optParser.add_option("-x", "--xsd", help="xsd schema to use")
    optParser.add_option("-o", "--output", help="name for generic output file")
    options, args = optParser.parse_args()
    if len(args) != 1:
        optParser.print_help()
        sys.exit()
    if not options.xsd:
        print("a schema is mandatory", file=sys.stderr)
        sys.exit()
    options.source = args[0]
    if not options.output:
        options.output = os.path.splitext(options.source)[0] + ".xml"
    return options


def read_n(inputf, n):
    """ Read exactly n bytes from the input.
        Raise RuntimeError if the stream ended before
        n bytes were read.
    """
    buf = ''
    while n > 0:
        data = inputf.read(n)
        if data == '':
            raise RuntimeError('unexpected connection close')
        buf += data
        n -= len(data)
    return buf


def msg2xml(desc, cont, out, depth=1):
    out.write(u">\n%s<%s" % (depth * '    ', desc.name))
    haveChildren = False
#    print(depth, cont)
    for attr, value in cont.ListFields():
        if attr.type == google.protobuf.descriptor.FieldDescriptor.TYPE_MESSAGE:
            if attr.label == google.protobuf.descriptor.FieldDescriptor.LABEL_REPEATED:
                haveChildren = True
                for item in value:
                    msg2xml(attr, item, out, depth + 1)
        else:
            if attr.type == google.protobuf.descriptor.FieldDescriptor.TYPE_ENUM:
                value = attr.enum_type.values_by_number[value].name
                if value[0] == "_" and value[1].isdigit():
                    value = value[1:]
            out.write(u' %s="%s"' % (attr.name, value))
    if haveChildren:
        out.write(u">\n%s</%s" % (depth * '    ', desc.name))
    else:
        out.write(u"/")


def writeXml(root, module, options):
    with contextlib.closing(xml2csv.getOutStream(options.output)) as outputf:
        outputf.write(u'<?xml version="1.0" encoding="UTF-8"?>\n\n<%s' % root)
        if options.source.isdigit():
            inp = xml2csv.getSocketStream(int(options.source))
        else:
            inp = open(options.source, 'rb')
        with contextlib.closing(inp) as inputf:
            first = True
            while True:
                length = struct.unpack('>L', read_n(inputf, 4))[0]
                if length == 0:
                    break
                obj = vars(module)[root.capitalize()]()
                obj.ParseFromString(read_n(inputf, length))
                for attr, value in obj.ListFields():
                    if attr.type == google.protobuf.descriptor.FieldDescriptor.TYPE_MESSAGE:
                        if attr.label == google.protobuf.descriptor.FieldDescriptor.LABEL_REPEATED:
                            for item in value:
                                msg2xml(attr, item, outputf)
                    elif first:
                        outputf.write(u' %s="%s"' % (attr.name, value))
                first = False
        outputf.write(u">\n</%s>\n" % root)


def main():
    options = get_options()
    # get attributes
    attrFinder = xml2csv.AttrFinder(options.xsd, options.source, False)
    base = os.path.basename(options.xsd).split('.')[0]
    # generate proto format description
    module = xml2protobuf.generateProto(attrFinder.tagAttrs, attrFinder.depthTags,
                                        attrFinder.xsdStruc._namedEnumerations, options.protodir, base)
    writeXml(attrFinder.xsdStruc.root.name, module, options)


if __name__ == "__main__":
    main()
