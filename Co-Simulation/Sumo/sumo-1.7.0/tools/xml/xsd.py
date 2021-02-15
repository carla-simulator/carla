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

# @file    xsd.py
# @author  Marek Heinrich
# @author  Michael Behrisch
# @date    2014-01-20

from __future__ import absolute_import

import os
from xml.dom import minidom


class XmlAttribute:

    def __init__(self, entity):
        if hasattr(entity, "getAttribute"):
            self.name = entity.getAttribute('name')
            self.use = entity.getAttribute('use')
            self.type = entity.getAttribute('type')
        else:
            self.name = entity
            self.use = None
            self.type = None

    def __repr__(self):
        return self.name


class XmlElement:

    def __init__(self, entity):
        self.name = entity.getAttribute('name')
        self.ref = entity.getAttribute('ref')
        self.type = entity.getAttribute('type')
        self.attributes = []
        self.children = []
        self.resolved = False

    def __repr__(self):
        childList = [c.name for c in self.children]
        return "name '%s' ref '%s' type '%s' attrs %s %s" % (self.name, self.ref, self.type, self.attributes, childList)


class XsdStructure():

    def __init__(self, xsdFile):
        xmlDoc = minidom.parse(open(xsdFile))
        self.root = None
        self._namedElements = {}
        self._namedTypes = {}
        self._namedEnumerations = {}
        for btEntity in xmlDoc.getElementsByTagName('xsd:include'):
            path = btEntity.getAttribute('schemaLocation')
            fullPath = os.path.join(os.path.dirname(xsdFile), path)
            subStruc = XsdStructure(fullPath)
            self._namedElements.update(subStruc._namedElements)
            self._namedTypes.update(subStruc._namedTypes)
        for btEntity in xmlDoc.getElementsByTagName('xsd:element'):
            if btEntity.nodeType == 1 and btEntity.hasAttribute('name'):
                el = self.getElementStructure(btEntity, True)
                self._namedElements[el.name] = el
                if self.root is None:
                    self.root = el
        for btEntity in xmlDoc.getElementsByTagName('xsd:complexType'):
            if btEntity.nodeType == 1 and btEntity.hasAttribute('name'):
                el = self.getElementStructure(btEntity)
                self._namedTypes[el.name] = el
        for btEntity in xmlDoc.getElementsByTagName('xsd:simpleType'):
            if btEntity.nodeType == 1 and btEntity.hasAttribute('name'):
                enum = [e.getAttribute(
                    'value') for e in btEntity.getElementsByTagName('xsd:enumeration')]
                if enum:
                    self._namedEnumerations[
                        btEntity.getAttribute('name')] = enum
        self.resolveRefs()
#        pp = pprint.PrettyPrinter(indent=4)
#        pp.pprint(self._namedElements)
#        pp.pprint(self._namedTypes)
#        pp.pprint(self._namedEnumerations)

    def getEnumeration(self, name):
        return self._namedEnumerations.get(name, None)

    def getEnumerationByAttr(self, ele, attr):
        if ele in self._namedElements:
            for a in self._namedElements[ele].attributes:
                if a.name == attr:
                    return self._namedEnumerations.get(a.type, None)
        return None

    def getElementStructure(self, entity, checkNestedType=False):
        eleObj = XmlElement(entity)
        if checkNestedType:
            nestedTypes = entity.getElementsByTagName('xsd:complexType')
            if nestedTypes:
                entity = nestedTypes[0]  # skip xsd:complex-tag
        for ext in entity.getElementsByTagName('xsd:extension'):
            if ext.parentNode.parentNode == entity:
                eleObj.type = ext.getAttribute('base')
                entity = ext
                break
        for aa in entity.childNodes:
            if aa.nodeName == 'xsd:attribute':
                eleObj.attributes.append(XmlAttribute(aa))
            elif aa.nodeName == 'xsd:sequence' or aa.nodeName == 'xsd:choice':
                for aae in aa.getElementsByTagName('xsd:element'):
                    eleObj.children.append(XmlElement(aae))
        return eleObj

    def resolveRefs(self):
        for ele in self._namedTypes.values():
            if ele.type and ele.type in self._namedTypes and not ele.resolved:
                t = self._namedTypes[ele.type]
                ele.attributes += t.attributes
                ele.children += t.children
                ele.resolved = True
        for ele in self._namedElements.values():
            if ele.type and ele.type in self._namedTypes and not ele.resolved:
                t = self._namedTypes[ele.type]
                ele.attributes += t.attributes
                ele.children += t.children
                ele.resolved = True
        for ele in self._namedElements.values():
            newChildren = []
            for child in ele.children:
                if child.ref:
                    newChildren.append(self._namedElements[child.ref])
                else:
                    newChildren.append(self._namedElements[child.name])
            ele.children = newChildren
