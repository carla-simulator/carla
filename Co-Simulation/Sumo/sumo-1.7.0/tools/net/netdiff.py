#!/usr/bin/env python
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

# @file    netdiff.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @date    2011-10-04

"""
Reads two networks (source, dest) and tries to produce the minimal plain-xml input
which can be loaded with netconvert alongside source to create dest
"""
from __future__ import absolute_import
from __future__ import print_function

import sys
import os
import codecs
try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO
from xml.dom import pulldom
from xml.dom import Node
from optparse import OptionParser
from subprocess import call
from collections import defaultdict

sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
import sumolib  # noqa
from OrderedMultiSet import OrderedMultiSet  # noqa

INDENT = 4

# file types to compare
TYPE_NODES = '.nod.xml'
TYPE_EDGES = '.edg.xml'
TYPE_CONNECTIONS = '.con.xml'
TYPE_TLLOGICS = '.tll.xml'
PLAIN_TYPES = [
    TYPE_NODES,
    TYPE_EDGES,
    TYPE_CONNECTIONS,
    TYPE_TLLOGICS
]

# traffic lights have some peculiarities
# CAVEAT1 - ids are not unique (only in combination with programID)
# CAVEAT2 - the order of their children (phases) is important.
#     this makes partial diffs unfeasible. The easiest solution is to forgo diffs and always
#     export the whole new traffic light
# CAVEAT3 - deletes need not be written because they are also signaled by a changed node type
#     (and they complicate the handling of deleted tl-connections)
# CAVEAT4 - deleted connections must be written with their tlID and tlIndex, otherwise
#     parsing in netconvert becomes tedious
# CAVEAT5 - phases must maintain their order
# CAVEAT6 - identical phases may occur multiple times, thus OrderedMultiSet
# CAVEAT7 - changing edge type triggers 'type override'
#     (all attributes defined for the edge type are applied. This must be avoided)
# CAVEAT8 - TAG_TLL must always be written before TAG_CONNECTION
# CAVEAT9 - when TAG_NEIGH is removed, <neigh lane=""/> must written into the diff to indicate removal
# CAVEAT10 - when a connection element is written without 'to' it describes an edge without connections.
#     This must be omitted from 'deleted elements'

TAG_TLL = 'tlLogic'
TAG_CONNECTION = 'connection'
TAG_CROSSING = 'crossing'
TAG_ROUNDABOUT = 'roundabout'
TAG_LANE = 'lane'
TAG_NEIGH = 'neigh'
TAG_EDGE = 'edge'
TAG_PARAM = 'param'
TAG_LOCATION = 'location'

# see CAVEAT1
IDATTRS = defaultdict(lambda: ('id',))
IDATTRS[TAG_TLL] = ('id', 'programID')
IDATTRS[TAG_CONNECTION] = ('from', 'to', 'fromLane', 'toLane')
IDATTRS[TAG_CROSSING] = ('node', 'edges')
IDATTRS[TAG_ROUNDABOUT] = ('edges',)
IDATTRS['interval'] = ('begin', 'end')
IDATTRS[TAG_LANE] = ('index',)
IDATTRS[TAG_NEIGH] = ('lane',)
IDATTRS[TAG_PARAM] = ('key',)

DELETE_ELEMENT = 'delete'  # the xml element for signifying deletes

# provide an order for the attribute names
ATTRIBUTE_NAMES = {
    # '.nod.xml' : ()
    # '.edg.xml' : ()
    # '.con.xml' : ()
}

# default values for the given attribute (needed when attributes appear in
# source but do not appear in dest)
DEFAULT_VALUES = defaultdict(lambda: None)
DEFAULT_VALUES['offset'] = "0"
DEFAULT_VALUES['spreadType'] = "right"
DEFAULT_VALUES['customShape'] = "false"
DEFAULT_VALUES['keepClear'] = "true"
DEFAULT_VALUES['contPos'] = "-1"
DEFAULT_VALUES['visibility'] = "-1"
DEFAULT_VALUES['z'] = "0"
DEFAULT_VALUES['radius'] = "-1"
RESET = 0

IGNORE_TAGS = set([TAG_LOCATION])


# stores attributes for later comparison
class AttributeStore:
    patchImport = False

    def __init__(self, type, copy_tags, level=1):
        # xml type being parsed
        self.type = type
        # tag names to copy even if unchanged
        self.copy_tags = copy_tags
        # indent level
        self.level = level
        # dict of names-tuples
        self.attrnames = {}
        # sets of (tag, id) preserve order to avoid dangling references during
        # loading
        self.ids_deleted = OrderedMultiSet()
        self.ids_created = OrderedMultiSet()
        self.ids_copied = OrderedMultiSet()
        # dict from (tag, id) to (names, values, children)
        self.id_attrs = {}
        # dict from tag to (names, values)-sets, need to preserve order
        # (CAVEAT5)
        self.idless_deleted = defaultdict(OrderedMultiSet)
        self.idless_created = defaultdict(OrderedMultiSet)
        self.idless_copied = defaultdict(OrderedMultiSet)

    def __str__(self):
        return ("AttributeStore(level=%s, attrnames=%s, id_attrs:%s)" % (
            self.level, self.attrnames,
            ''.join(["\n%s%s: n=%s, v=%s, c=%s" % ('  ' * self.level, k, n, v, c)
                     for k, (n, v, c) in self.id_attrs.items()])))

    # getAttribute returns "" if not present
    def getValue(self, node, name):
        if node.hasAttribute(name):
            return node.getAttribute(name)
        else:
            return None

    def getNames(self, xmlnode):
        idattrs = IDATTRS[xmlnode.localName]
        a = xmlnode.attributes
        all = [a.item(i).localName for i in range(a.length)]
        instance = tuple([n for n in all if n not in idattrs])
        if instance not in self.attrnames:
            self.attrnames[instance] = instance
        # only store a single instance of this tuple to conserve memory
        return self.attrnames[instance]

    def getAttrs(self, xmlnode):
        names = self.getNames(xmlnode)
        values = tuple([self.getValue(xmlnode, a) for a in names])
        children = None
        if any([c.nodeType == Node.ELEMENT_NODE for c in xmlnode.childNodes]):
            children = AttributeStore(
                self.type, self.copy_tags, self.level + 1)
        tag = xmlnode.localName
        id = tuple([xmlnode.getAttribute(a)
                    for a in IDATTRS[tag] if xmlnode.hasAttribute(a)])
        return tag, id, children, (names, values, children)

    def store(self, xmlnode):
        tag, id, children, attrs = self.getAttrs(xmlnode)
        tagid = (tag, id)
        if id != ():
            self.ids_deleted.add(tagid)
            self.ids_copied.add(tagid)
            self.id_attrs[tagid] = attrs
            if children:
                for child in xmlnode.childNodes:
                    if child.nodeType == Node.ELEMENT_NODE:
                        children.store(child)
        else:
            self.no_children_supported(children, tag)
            self.idless_deleted[tag].add(attrs)

    def compare(self, xmlnode):
        tag, id, children, attrs = self.getAttrs(xmlnode)
        oldChildren = None
        tagid = (tag, id)
        if id != ():
            if AttributeStore.patchImport:
                if self.hasChangedConnection(tagid, attrs):
                    # export all connections from the same edge
                    fromEdge = id[0]
                    markChanged = []
                    for tagid2 in self.ids_deleted:
                        fromEdge2 = tagid2[1][0]
                        if fromEdge == fromEdge2:
                            markChanged.append(tagid2)
                    for tagid2 in markChanged:
                        self.ids_deleted.remove(tagid2)
                return
            if tagid in self.ids_deleted:
                oldChildren = self.id_attrs[tagid][2]
                self.ids_deleted.remove(tagid)
                self.id_attrs[tagid] = self.compareAttrs(
                    self.id_attrs[tagid], attrs, tag)
            else:
                self.ids_created.add(tagid)
                self.id_attrs[tagid] = attrs

            children = self.id_attrs[tagid][2]
            if children:
                for child in xmlnode.childNodes:
                    if child.nodeType == Node.ELEMENT_NODE:
                        children.compare(child)
                if tag == TAG_TLL or tag in self.copy_tags:  # see CAVEAT2
                    child_strings = StringIO()
                    children.writeDeleted(child_strings)
                    children.writeCreated(child_strings)
                    children.writeChanged(child_strings)

                    if len(child_strings.getvalue()) > 0 or tag in self.copy_tags:
                        # there are some changes. Go back and store everything
                        children = AttributeStore(
                            self.type, self.copy_tags, self.level + 1)
                        for child in xmlnode.childNodes:
                            if child.nodeType == Node.ELEMENT_NODE:
                                children.compare(child)
                        self.id_attrs[tagid] = self.id_attrs[
                            tagid][0:2] + (children,)

            elif tag == TAG_EDGE and oldChildren:
                # see CAVEAT9
                children = oldChildren
                for k, (n, v, c) in oldChildren.id_attrs.items():
                    if c:
                        deletedNeigh = False
                        for k2, (n2, v2, c2) in c.id_attrs.items():
                            if k2[0] == TAG_NEIGH:
                                deletedNeigh = True
                        if deletedNeigh:
                            # print("k2=%s n2=%s v2=%s c2=%s" % (k2, n2, v2, c2))
                            delkey = (TAG_NEIGH, ("",))
                            children.id_attrs[k][2].id_attrs = {delkey: ([], [], None)}
                            children.id_attrs[k][2].ids_created.add(delkey)
                            children.ids_deleted.discard(k)
                        else:
                            del children.id_attrs[k]
                self.id_attrs[tagid] = self.id_attrs[tagid][0:2] + (children,)

        else:
            self.no_children_supported(children, tag)
            if attrs in self.idless_deleted[tag]:
                self.idless_deleted[tag].remove(attrs)
                if tag in self.copy_tags:
                    self.idless_copied[tag].add(attrs)
            elif tag in IGNORE_TAGS:
                self.idless_deleted[tag].clear()
            else:
                self.idless_created[tag].add(attrs)

    def no_children_supported(self, children, tag):
        if children:
            print(
                "WARNING: Handling of children only supported for elements with id. Ignored for element '%s'" % tag)

    def compareAttrs(self, sourceAttrs, destAttrs, tag):
        snames, svalues, schildren = sourceAttrs
        dnames, dvalues, dchildren = destAttrs
        # for traffic lights, always use dchildren
        if schildren and dchildren:
            # trigger compare
            dchildren = schildren
        if snames == dnames:
            values = tuple([self.diff(tag, n, s, d)
                            for n, s, d in zip(snames, svalues, dvalues)])
            return snames, values, dchildren
        else:
            sdict = defaultdict(lambda: None, zip(snames, svalues))
            ddict = defaultdict(lambda: None, zip(dnames, dvalues))
            names = tuple(set(snames + dnames))
            values = tuple([self.diff(tag, n, sdict[n], ddict[n]) for n in names])
            return names, values, dchildren

    def diff(self, tag, name, sourceValue, destValue):
        if (sourceValue == destValue or
                # CAVEAT7
                (tag == TAG_EDGE and name == "type")):
            return None
        elif destValue is None:
            return DEFAULT_VALUES[name]
        else:
            return destValue

    def hasChangedConnection(self, tagid, attrs):
        tag, id = tagid
        if tag != TAG_CONNECTION:
            return False
        if tagid in self.ids_deleted:
            names, values, children = self.compareAttrs(self.id_attrs[tagid], attrs, tag)
            for v in values:
                if v is not None:
                    return True
            return False
        else:
            return True

    def writeDeleted(self, file):
        # data loss if two elements with different tags
        # have the same id
        for tag, id in self.ids_deleted:
            comment_start, comment_end = ("", "")
            additional = ""
            delete_element = DELETE_ELEMENT

            if self.type == TYPE_TLLOGICS and tag == TAG_CONNECTION:
                # see CAVEAT4
                names, values, children = self.id_attrs[(tag, id)]
                additional = " " + self.attr_string(names, values)

            if tag == TAG_TLL:  # see CAVEAT3
                comment_start, comment_end = (
                    "<!-- implicit via changed node type: ", " -->")

            if tag == TAG_CROSSING:
                delete_element = tag
                additional = ' discard="true"'

            if tag == TAG_ROUNDABOUT:
                delete_element = tag
                additional = ' discard="true"'
                comment_start, comment_end = (
                    "<!-- deletion of roundabouts not yet supported. see #2225 ", " -->")

            if tag == TAG_NEIGH:
                delete_element = tag
                additional = ' lane=""'

            if self.type == TYPE_CONNECTIONS and tag == TAG_CONNECTION and len(id) == 1:
                # see CAVEAT10
                comment_start, comment_end = (
                    "<!-- disconnected edge implicitly loses connections when deleted: ", " -->")

            self.write(file, '%s<%s %s%s/>%s\n' % (
                comment_start,
                delete_element, self.id_string(tag, id), additional,
                comment_end))
        # data loss if two elements with different tags
        # have the same list of attributes and values
        for value_set in self.idless_deleted.values():
            self.write_idless(file, value_set, DELETE_ELEMENT)

    def writeCreated(self, file, whiteList=None, blackList=None):
        self.write_tagids(file, self.filterTags(self.ids_created, whiteList, blackList), True)
        for tag, value_set in self.idless_created.items():
            if ((whiteList is not None and tag not in whiteList)
                    or (blackList is not None and tag in blackList)):
                continue
            self.write_idless(file, value_set, tag)

    def getTagidsChanged(self):
        return self.ids_copied - (self.ids_deleted | self.ids_created)

    def writeChanged(self, file, whiteList=None, blackList=None):
        tagids_changed = self.getTagidsChanged()
        self.write_tagids(file, self.filterTags(tagids_changed, whiteList, blackList), False)

    def writeCopies(self, file, copy_tags):
        tagids_unchanged = self.ids_copied - \
            (self.ids_deleted | self.ids_created)
        self.write_tagids(file, tagids_unchanged, False)
        for tag, value_set in self.idless_copied.items():
            self.write_idless(file, value_set, tag)

    def write_idless(self, file, attr_set, tag):
        for names, values, children in attr_set:
            self.write(file, '<%s %s/>\n' %
                       (tag, self.attr_string(names, values)))

    def write_tagids(self, file, tagids, create):
        for tagid in tagids:
            tag, id = tagid
            names, values, children = self.id_attrs[tagid]
            attrs = self.attr_string(names, values)
            child_strings = StringIO()
            if children:
                # writeDeleted is not supported
                children.writeCreated(child_strings)
                children.writeChanged(child_strings)

            if len(attrs) > 0 or len(child_strings.getvalue()) > 0 or create or tag in self.copy_tags:
                close_tag = "/>\n"
                if len(child_strings.getvalue()) > 0:
                    close_tag = ">\n%s" % child_strings.getvalue()
                self.write(file, '<%s %s %s%s' % (
                    tag,
                    self.id_string(tag, id),
                    attrs,
                    close_tag))
                if len(child_strings.getvalue()) > 0:
                    self.write(file, "</%s>\n" % tag)

    def write(self, file, item):
        file.write(" " * INDENT * self.level)
        file.write(item)

    def attr_string(self, names, values):
        return ' '.join(['%s="%s"' % (n, v) for n, v in sorted(zip(names, values)) if v is not None])

    def id_string(self, tag, id):
        idattrs = IDATTRS[tag]
        return ' '.join(['%s="%s"' % (n, v) for n, v in sorted(zip(idattrs, id))])

    def filterTags(self, tagids, whiteList, blackList):
        if whiteList is not None:
            return [tagid for tagid in tagids if tagid[0] in whiteList]
        elif blackList is not None:
            return [tagid for tagid in tagids if tagid[0] not in blackList]
        else:
            return tagids

    def reorderTLL(self):
        for tag, id in self.ids_created:
            if tag == TAG_CONNECTION:
                for tag2, id2 in self.getTagidsChanged():
                    if tag2 == TAG_TLL:
                        return True
                return False
        return False


def parse_args():
    USAGE = "Usage: " + sys.argv[0] + " <source> <dest> <output-prefix>"
    optParser = OptionParser()
    optParser.add_option("-v", "--verbose", action="store_true",
                         default=False, help="Give more output")
    optParser.add_option("-p", "--use-prefix", action="store_true",
                         default=False, help="interpret source and dest as plain-xml prefix instead of network names")
    optParser.add_option("-d", "--direct", action="store_true",
                         default=False, help="compare source and dest files directly")
    optParser.add_option("-i", "--patch-on-import", action="store_true",
                         dest="patchImport",
                         default=False, help="generate patch that can be applied during initial network import" +
                         " (exports additional connection elements)")
    optParser.add_option(
        "-c", "--copy", help="comma-separated list of element names to copy (if they are unchanged)")
    optParser.add_option("--path", dest="path", help="Path to binaries")
    options, args = optParser.parse_args()
    if len(args) != 3:
        sys.exit(USAGE)
    if options.use_prefix and options.direct:
        optParser.error(
            "Options --use-prefix and --direct are mutually exclusive")
    options.source, options.dest, options.outprefix = args
    return options


def create_plain(netfile, netconvert):
    prefix = netfile[:-8]
    call([netconvert,
          "--sumo-net-file", netfile,
          "--plain-output-prefix", prefix,
          "--roundabouts.guess", "false"])
    return prefix


# creates diff of a flat xml structure
# (only children of the root element and their attrs are compared)
def xmldiff(source, dest, diff, type, copy_tags, patchImport):
    attributeStore = AttributeStore(type, copy_tags)
    root_open = None
    have_source = os.path.isfile(source)
    have_dest = os.path.isfile(dest)
    if have_source:
        root_open, root_close = handle_children(source, attributeStore.store)
    if have_dest:
        if patchImport:
            # run diff twice to determine edges with changed connections
            AttributeStore.patchImport = True
            root_open, root_close = handle_children(dest, attributeStore.compare)
            AttributeStore.patchImport = False
            root_open, root_close = handle_children(dest, attributeStore.compare)
        else:
            root_open, root_close = handle_children(dest, attributeStore.compare)

    if not have_source and not have_dest:
        print("Skipping %s due to lack of input files" % diff)
    else:
        if not have_source:
            print(
                "Source file %s is missing. Assuming all elements are created" % source)
        elif not have_dest:
            print(
                "Dest file %s is missing. Assuming all elements are deleted" % dest)

        with codecs.open(diff, 'w', 'utf-8') as diff_file:
            diff_file.write('<?xml version="1.0" encoding="UTF-8"?>\n')
            diff_file.write(root_open)
            if copy_tags:
                attributeStore.write(diff_file, "<!-- Copied Elements -->\n")
                attributeStore.writeCopies(diff_file, copy_tags)
            attributeStore.write(diff_file, "<!-- Deleted Elements -->\n")
            attributeStore.writeDeleted(diff_file)

            if attributeStore.reorderTLL():
                # CAVEAT8
                attributeStore.write(diff_file, "<!-- Created Elements -->\n")
                attributeStore.writeCreated(diff_file, whiteList=[TAG_TLL])
                attributeStore.write(diff_file, "<!-- Changed Elements -->\n")
                attributeStore.writeChanged(diff_file, whiteList=[TAG_TLL])
                attributeStore.write(diff_file, "<!-- Created Elements -->\n")
                attributeStore.writeCreated(diff_file, blackList=[TAG_TLL])
                attributeStore.write(diff_file, "<!-- Changed Elements -->\n")
                attributeStore.writeChanged(diff_file, blackList=[TAG_TLL])
            else:
                attributeStore.write(diff_file, "<!-- Created Elements -->\n")
                attributeStore.writeCreated(diff_file)
                attributeStore.write(diff_file, "<!-- Changed Elements -->\n")
                attributeStore.writeChanged(diff_file)
            diff_file.write(root_close)


# calls function handle_parsenode for all children of the root element
# returns opening and closing tag of the root element
def handle_children(xmlfile, handle_parsenode):
    root_open = None
    root_close = None
    level = 0
    xml_doc = pulldom.parse(xmlfile)
    for event, parsenode in xml_doc:
        if event == pulldom.START_ELEMENT:
            # print level, parsenode.getAttribute(ID_ATTR)
            if level == 0:
                root_open = parsenode.toprettyxml(indent="")
                # since we did not expand root_open contains the closing slash
                root_open = root_open[:-3] + ">\n"
                # change the schema for edge diffs
                root_open = root_open.replace(
                    "edges_file.xsd", "edgediff_file.xsd")
                root_close = "</%s>\n" % parsenode.localName
            if level == 1:
                # consumes END_ELEMENT, no level increase
                xml_doc.expandNode(parsenode)
                handle_parsenode(parsenode)
            else:
                level += 1
        elif event == pulldom.END_ELEMENT:
            level -= 1
    return root_open, root_close


# run
def main():
    options = parse_args()
    copy_tags = options.copy.split(',') if options.copy else []
    if options.direct:
        type = '.xml'
        xmldiff(options.source,
                options.dest,
                options.outprefix + type,
                type,
                copy_tags,
                options.patchImport)
    else:
        if not options.use_prefix:
            netconvert = sumolib.checkBinary("netconvert", options.path)
            options.source = create_plain(options.source, netconvert)
            options.dest = create_plain(options.dest, netconvert)
        for type in PLAIN_TYPES:
            xmldiff(options.source + type,
                    options.dest + type,
                    options.outprefix + type,
                    type,
                    copy_tags,
                    options.patchImport)


if __name__ == "__main__":
    main()
