# -*- coding: utf-8 -*-
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2011-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    xml.py
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @date    2011-06-23

from __future__ import print_function
from __future__ import absolute_import
import os
import sys
import re
import gzip
import io
import datetime
try:
    import xml.etree.cElementTree as ET
except ImportError as e:
    print("recovering from ImportError '%s'" % e)
    import xml.etree.ElementTree as ET
from collections import namedtuple, OrderedDict
from keyword import iskeyword
from functools import reduce
import xml.sax.saxutils

from . import version

DEFAULT_ATTR_CONVERSIONS = {
    # shape-like
    'shape': lambda coords: map(lambda xy: map(float, xy.split(',')), coords.split()),
    # float
    'speed': float,
    'length': float,
    'width': float,
    'angle': float,
    'endOffset': float,
    'radius': float,
    'contPos': float,
    'visibility': float,
    'startPos': float,
    'endPos': float,
    'position': float,
    'x': float,
    'y': float,
    'lon': float,
    'lat': float,
    'freq': float,
    # int
    'priority': int,
    'numLanes': int,
    'index': int,
    'linkIndex': int,
    'linkIndex2': int,
    'fromLane': int,
    'toLane': int,
}


def _prefix_keyword(name, warn=False):
    result = name
    # create a legal identifier (xml allows '-', ':' and '.' ...)
    result = ''.join([c for c in name if c.isalnum() or c == '_'])
    if result != name:
        if result == '':
            result == 'attr_'
        if warn:
            print("Warning: Renaming attribute '%s' to '%s' because it contains illegal characters" % (
                name, result), file=sys.stderr)
    if name == "name":
        result = 'attr_name'
        if warn:
            print("Warning: Renaming attribute '%s' to '%s' because it conflicts with a reserved field" % (
                name, result), file=sys.stderr)

    if iskeyword(name):
        result = 'attr_' + name
        if warn:
            print("Warning: Renaming attribute '%s' to '%s' because it conflicts with a python keyword" % (
                name, result), file=sys.stderr)
    return result


def compound_object(element_name, attrnames, warn=False):
    """return a class which delegates bracket access to an internal dict.
       Missing attributes are delegated to the child dict for convenience.
       @note: Care must be taken when child nodes and attributes have the same names"""
    class CompoundObject():
        _original_fields = sorted(attrnames)
        _fields = [_prefix_keyword(a, warn) for a in _original_fields]

        def __init__(self, values, child_dict=None, text=None, child_list=None):
            for name, val in zip(self._fields, values):
                self.__dict__[name] = val
            self._child_dict = child_dict if child_dict else {}
            self.name = element_name
            self._text = text
            self._child_list = child_list if child_list else []

        def getAttributes(self):
            return [(k, getattr(self, k)) for k in self._fields]

        def hasAttribute(self, name):
            return name in self._fields

        def getAttribute(self, name):
            if self.hasAttribute(name):
                return self.__dict__[name]
            raise AttributeError

        def getAttributeSecure(self, name, default=None):
            if self.hasAttribute(name):
                return self.__dict__[name]
            return default

        def setAttribute(self, name, value):
            if name not in self._original_fields:
                self._original_fields.append(name)
                self._fields.append(_prefix_keyword(name, warn))
            self.__dict__[_prefix_keyword(name, warn)] = value

        def hasChild(self, name):
            return name in self._child_dict

        def getChild(self, name):
            return self._child_dict[name]

        def addChild(self, name, attrs=None):
            if attrs is None:
                attrs = {}
            clazz = compound_object(name, attrs.keys())
            child = clazz([attrs.get(a) for a in sorted(attrs.keys())])
            self._child_dict.setdefault(name, []).append(child)
            self._child_list.append(child)
            return child

        def removeChild(self, child):
            self._child_dict[child.name].remove(child)
            self._child_list.remove(child)

        def setChildList(self, childs):
            for c in self._child_list:
                self._child_dict[c.name].remove(c)
            for c in childs:
                self._child_dict.setdefault(c.name, []).append(c)
            self._child_list = childs

        def getChildList(self):
            return self._child_list

        def getText(self):
            return self._text

        def setText(self, text):
            self._text = text

        def __getattr__(self, name):
            if name[:2] != "__":
                return self._child_dict.get(name, None)
            raise AttributeError

        def __setattr__(self, name, value):
            if name != "_child_dict" and name in self._child_dict:
                # this could be optimized by using the child_list only if there are different children
                for c in self._child_dict[name]:
                    self._child_list.remove(c)
                self._child_dict[name] = value
                for c in value:
                    self._child_list.append(c)
            else:
                self.__dict__[name] = value

        def __delattr__(self, name):
            if name in self._child_dict:
                for c in self._child_dict[name]:
                    self._child_list.remove(c)
                del self._child_dict[name]
            else:
                if name in self.__dict__:
                    del self.__dict__[name]
                self._original_fields.remove(name)
                self._fields.remove(_prefix_keyword(name, False))

        def __getitem__(self, name):
            return self._child_dict[name]

        def __str__(self):
            nodeText = '' if self._text is None else ",text=%s" % self._text
            return "<%s,child_dict=%s%s>" % (self.getAttributes(), dict(self._child_dict), nodeText)

        def toXML(self, initialIndent="", indent="    "):
            fields = ['%s="%s"' % (self._original_fields[i], getattr(self, k))
                      for i, k in enumerate(self._fields) if getattr(self, k) is not None and
                      # see #3454
                      '{' not in self._original_fields[i]]
            if not self._child_dict and self._text is None:
                return initialIndent + "<%s %s/>\n" % (self.name, " ".join(fields))
            else:
                s = initialIndent + "<%s %s>\n" % (self.name, " ".join(fields))
                for c in self._child_list:
                    s += c.toXML(initialIndent + indent)
                if self._text is not None:
                    s += self._text.strip()
                return s + "%s</%s>\n" % (initialIndent, self.name)

        def __repr__(self):
            return str(self)

    return CompoundObject


def parse(xmlfile, element_names, element_attrs={}, attr_conversions={},
          heterogeneous=False, warn=False):
    """
    Parses the given element_names from xmlfile and yield compound objects for
    their xml subtrees (no extra objects are returned if element_names appear in
    the subtree) The compound objects provide all element attributes of
    the root of the subtree as attributes unless attr_names are supplied. In this
    case attr_names maps element names to a list of attributes which are
    supplied. If attr_conversions is not empty it must map attribute names to
    callables which will be called upon the attribute value before storing under
    the attribute name.
    The compound objects gives dictionary style access to list of compound
    objects o for any children with the given element name
    o['child_element_name'] = [osub0, osub1, ...]
    As a shorthand, attribute style access to the list of child elements is
    provided unless an attribute with the same name as the child elements
    exists (i.e. o.child_element_name = [osub0, osub1, ...])
    @Note: All elements with the same name must have the same type regardless of
    the subtree in which they occur (heterogeneous cases may be handled by
    setting heterogeneous=True (with reduced parsing speed)
    @Note: Attribute names may be modified to avoid name clashes
    with python keywords. (set warn=True to receive renaming warnings)
    @Note: The element_names may be either a single string or a list of strings.
    @Example: parse('plain.edg.xml', ['edge'])
    """
    if isinstance(element_names, str):
        element_names = [element_names]
    elementTypes = {}
    for _, parsenode in ET.iterparse(_open(xmlfile, None)):
        if parsenode.tag in element_names:
            yield _get_compound_object(parsenode, elementTypes,
                                       parsenode.tag, element_attrs,
                                       attr_conversions, heterogeneous, warn)
            parsenode.clear()


def _IDENTITY(x):
    return x


def _get_compound_object(node, elementTypes, element_name, element_attrs, attr_conversions, heterogeneous, warn):
    if element_name not in elementTypes or heterogeneous:
        # initialized the compound_object type from the first encountered #
        # element
        attrnames = element_attrs.get(element_name, node.keys())
        if len(attrnames) != len(set(attrnames)):
            raise Exception(
                "non-unique attributes %s for element '%s'" % (attrnames, element_name))
        elementTypes[element_name] = compound_object(
            element_name, attrnames, warn)
    # prepare children
    child_dict = {}
    child_list = []
    if len(node) > 0:
        for c in node:
            child = _get_compound_object(c, elementTypes, c.tag, element_attrs, attr_conversions, heterogeneous, warn)
            child_dict.setdefault(c.tag, []).append(child)
            child_list.append(child)
    attrnames = elementTypes[element_name]._original_fields
    return elementTypes[element_name](
        [attr_conversions.get(a, _IDENTITY)(node.get(a)) for a in attrnames],
        child_dict, node.text, child_list)


def create_document(root_element_name, attrs=None, schema=None):
    if attrs is None:
        attrs = {}
    if schema is None:
        attrs["xmlns:xsi"] = "http://www.w3.org/2001/XMLSchema-instance"
        attrs["xsi:noNamespaceSchemaLocation"] = "http://sumo.dlr.de/xsd/" + root_element_name + "_file.xsd"
    clazz = compound_object(root_element_name, sorted(attrs.keys()))
    return clazz([attrs.get(a) for a in sorted(attrs.keys())], OrderedDict())


def sum(elements, attrname):
    # for the given elements (as returned by method parse) compute the sum for attrname
    # attrname must be the name of a numerical attribute
    return reduce(lambda x, y: x + y, [float(getattr(e, attrname)) for e in elements])


def average(elements, attrname):
    # for the given elements (as returned by method parse) compute the average for attrname
    # attrname must be the name of a numerical attribute
    if elements:
        return sum(elements, attrname) / len(elements)
    else:
        raise Exception("average of 0 elements is not defined")


def _createRecordAndPattern(element_name, attrnames, warn, optional):
    if isinstance(attrnames, str):
        attrnames = [attrnames]
    prefixedAttrnames = [_prefix_keyword(a, warn) for a in attrnames]
    if optional:
        pattern = ''.join(['<%s' % element_name] +
                          ['(\\s+%s="(?P<%s>[^"]*?)")?' % a for a in zip(attrnames, prefixedAttrnames)])
    else:
        pattern = '.*'.join(['<%s' % element_name] +
                            ['%s="([^"]*)"' % attr for attr in attrnames])
    Record = namedtuple(_prefix_keyword(element_name, warn), prefixedAttrnames)
    reprog = re.compile(pattern)
    return Record, reprog


def _open(xmlfile, encoding="utf8"):
    if isinstance(xmlfile, str):
        if xmlfile.endswith(".gz"):
            return gzip.open(xmlfile, "rt")
        if encoding is not None:
            return io.open(xmlfile, encoding=encoding)
    return xmlfile


def parse_fast(xmlfile, element_name, attrnames, warn=False, optional=False, encoding="utf8"):
    """
    Parses the given attrnames from all elements with element_name
    @Note: The element must be on its own line and the attributes must appear in
    the given order.
    @Example: parse_fast('plain.edg.xml', 'edge', ['id', 'speed'])
    """
    Record, reprog = _createRecordAndPattern(element_name, attrnames, warn, optional)
    for line in _open(xmlfile, encoding):
        m = reprog.search(line)
        if m:
            if optional:
                yield Record(**m.groupdict())
            else:
                yield Record(*m.groups())


def parse_fast_nested(xmlfile, element_name, attrnames, element_name2, attrnames2,
                      warn=False, optional=False, encoding="utf8"):
    """
    Parses the given attrnames from all elements with element_name
    And attrnames2 from element_name2 where element_name2 is a child element of element_name
    @Note: The element must be on its own line and the attributes must appear in
    the given order.
    @Example: parse_fast_nested('fcd.xml', 'timestep', ['time'], 'vehicle', ['id', 'speed', 'lane']):
    """
    Record, reprog = _createRecordAndPattern(element_name, attrnames, warn, optional)
    Record2, reprog2 = _createRecordAndPattern(element_name2, attrnames2, warn, optional)
    record = None
    for line in _open(xmlfile, encoding):
        m2 = reprog2.search(line)
        if m2:
            if optional:
                yield record, Record2(**m2.groupdict())
            else:
                yield record, Record2(*m2.groups())
        else:
            m = reprog.search(line)
            if m:
                if optional:
                    record = Record(**m.groupdict())
                else:
                    record = Record(*m.groups())


def writeHeader(outf, script=None, root=None, schemaPath=None):
    script = os.path.basename(sys.argv[0]) + " " + version.gitDescribe()
    outf.write(u"""<?xml version="1.0" encoding="UTF-8"?>
<!-- generated on %s by %s
  options: %s
-->
""" % (datetime.datetime.now(), script,
       (' '.join(sys.argv[1:]).replace('--', '<doubleminus>'))))
    if root is not None:
        if schemaPath is None:
            schemaPath = root + "_file.xsd"
        outf.write((u'<%s xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ' +
                    u'xsi:noNamespaceSchemaLocation="http://sumo.dlr.de/xsd/%s">\n') % (root, schemaPath))


def quoteattr(val):
    # saxutils sometimes uses single quotes around the attribute
    # we can prevent this by adding an artificial single quote to the value and removing it again
    return '"' + xml.sax.saxutils.quoteattr("'" + val)[2:]
