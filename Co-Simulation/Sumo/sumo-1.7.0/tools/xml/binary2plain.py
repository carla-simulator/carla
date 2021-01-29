#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    binary2plain.py
# @author  Michael Behrisch
# @date    2012-03-11

from __future__ import absolute_import
from __future__ import print_function
import sys
import struct

BYTE = 0
INTEGER = 1
FLOAT = 2
STRING = 3
LIST = 4
XML_TAG_START = 5
XML_TAG_END = 6
XML_ATTRIBUTE = 7
EDGE = 8
LANE = 9
POSITION_2D = 10
POSITION_3D = 11
BOUNDARY = 12
COLOR = 13
NODE_TYPE = 14
EDGE_FUNCTION = 15
ROUTE = 16
SCALED2INT = 17
SCALED2INT_POSITION_2D = 18
SCALED2INT_POSITION_3D = 19


def read(content, format):
    return struct.unpack(format, content.read(struct.calcsize(format)))


def readByte(content):
    return read(content, "B")[0]


def readInt(content, withType=False):
    if withType:
        valType = readByte(content)
        assert(valType == INTEGER)
    return read(content, "i")[0]


def readDouble(content):
    return read(content, "d")[0]


def readString(content):
    length = readInt(content)
    return read(content, "%ss" % length)[0]


def readStringList(content):
    n = readInt(content)
    list = []
    for _ in range(n):
        read(content, "B")  # type
        list.append(readString(content))
    return list


def readIntListList(content):
    n = readInt(content)
    list = []
    for _ in range(n):
        read(content, "B")  # type
        n1 = readInt(content)
        list.append([readInt(content, True) for __ in range(n1)])
    return list


def readRoute(content):
    n = readInt(content)
    list = []
    first = readInt(content)
    if first < 0:
        bits = -first
        numFields = 8 * 4 / bits
        mask = (1 << bits) - 1
        edge = readInt(content)
        list.append(edges[edge])
        n -= 1
        field = numFields
        while n > 0:
            if field == numFields:
                data = readInt(content)
                field = 0
            followIndex = (data >> ((numFields - field - 1) * bits)) & mask
            edge = followers[edge][followIndex]
            list.append(edges[edge])
            field += 1
            n -= 1
    else:
        list.append(edges[first])
        n -= 1
        while n > 0:
            list.append(edges[readInt(content)])
            n -= 1
    return list


def typedValueStr(content):
    valType = readByte(content)
    if valType == BYTE:
        return str(readByte(content))
    elif valType == INTEGER:
        return str(readInt(content))
    elif valType == FLOAT:
        return '%.2f' % readDouble(content)
    elif valType == STRING:
        return readString(content)
    elif valType == LIST:
        return " ".join([typedValueStr(content) for _ in range(readInt(content))])
    elif valType == EDGE:
        return edges[readInt(content)]
    elif valType == LANE:
        return '%s_%s' % (edges[readInt(content)], readByte(content))
    elif valType == POSITION_2D:
        return '%.2f,%.2f' % (readDouble(content), readDouble(content))
    elif valType == POSITION_3D:
        return '%.2f,%.2f,%.2f' % (readDouble(content), readDouble(content), readDouble(content))
    elif valType == BOUNDARY:
        return '%.2f,%.2f,%.2f,%.2f' % (readDouble(content), readDouble(content),
                                        readDouble(content), readDouble(content))
    elif valType == COLOR:
        val = read(content, "BBBB")
        return '%.2f,%.2f,%.2f' % (val[0] / 255., val[1] / 255., val[2] / 255.)
    elif valType == NODE_TYPE:
        return nodeTypes[readByte(content)]
    elif valType == EDGE_FUNCTION:
        return edgeTypes[readByte(content)]
    elif valType == ROUTE:
        return " ".join(readRoute(content))
    elif valType == SCALED2INT:
        return '%.2f' % (readInt(content) / 100.)
    elif valType == SCALED2INT_POSITION_2D:
        return '%.2f,%.2f' % (readInt(content) / 100., readInt(content) / 100.)
    elif valType == SCALED2INT_POSITION_3D:
        return '%.2f,%.2f,%.2f' % (readInt(content) / 100., readInt(content) / 100., readInt(content) / 100.)


out = sys.stdout
content = open(sys.argv[1], 'rb')
_, version, _ = read(content, "BBB")  # type, sbx version, type
readString(content)  # sumo version
read(content, "B")  # type
elements = readStringList(content)
read(content, "B")  # type
attributes = readStringList(content)
read(content, "B")  # type
nodeTypes = readStringList(content)
read(content, "B")  # type
edgeTypes = readStringList(content)
read(content, "B")  # type
edges = readStringList(content)
read(content, "B")  # type
followers = readIntListList(content)
stack = []
startOpen = False
while True:
    typ = readByte(content)
    if typ == XML_TAG_START:
        if startOpen:
            out.write(">\n")
        out.write("    " * len(stack))
        tag = readByte(content)
        if version > 1:
            tag += 256 * readByte(content)
        stack.append(tag)
        out.write("<" + elements[tag])
        startOpen = True
    elif typ == XML_TAG_END:
        if startOpen:
            out.write("/>\n")
            stack.pop()
            startOpen = False
        else:
            out.write("    " * (len(stack) - 1))
            out.write("</%s>\n" % elements[stack.pop()])
        if version == 1:
            readByte(content)
        if len(stack) == 0:
            break
    elif typ == XML_ATTRIBUTE:
        attr = readByte(content)
        if version > 1:
            attr += 256 * readByte(content)
        out.write(' %s="%s"' %
                  (attributes[attr], typedValueStr(content)))
    else:
        print("Unknown type %s" % typ, file=sys.stderr)
