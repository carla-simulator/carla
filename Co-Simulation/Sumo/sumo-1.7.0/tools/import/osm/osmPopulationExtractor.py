#!/usr/bin/env python
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

# @file    osmPopulationExtractor.py
# @author  Yun-Pang Floetteroed
# @author  Melanie Knocke
# @author  Michael Behrisch
# @date    2013-02-08

"""
This script is to
- extract the popoulation data from a given Open Street Map (OSM).
- match the population data from OSM and BSA (with csv format)
The redundant information is removed and saved in the output file *_redundantOSMData.txt.
If there are data entries without names, they will be saved in *_nameNone.txt.
"""
from __future__ import absolute_import
from __future__ import print_function

import os
import sys
from xml.sax import make_parser, handler
from optparse import OptionParser


class Net():

    def __init__(self):
        self._relations = []
        self._nodes = []
        self._nodeMap = {}
        self._relationMap = {}
        self._uidNodeMap = {}
        self._uidRelationMap = {}

    def addNode(self, id, lat, lon, population):
        if id not in self._nodeMap:
            node = Node(id, lat, lon, population)
            self._nodes.append(node)
            self._nodeMap[id] = node
        return self._nodeMap[id]

    def addRelation(self, id, uid, population):
        if id not in self._relationMap:
            relation = Relation(id, uid, population)
            self._relations.append(relation)
            self._relationMap[id] = relation

        return self._relationMap[id]


class Node:

    """
    This class is to store node information.
    """

    def __init__(self, id, lat, lon, population):
        self.id = id
        self.lat = lat
        self.lon = lon
        self.attribute = "node"
        self.uid = None
        self.place = None
        self.name = None
        self.population = population

    def __repr__(self):
        return "<%s|%s_%s_%s>" % (self.id, self.name, self.uid, self.population)


class Relation:

    """
    This class is to store relation information.
    """

    def __init__(self, id, uid, population):
        self.id = id
        self.uid = uid
        self.attribute = "relation"
        self.name = None
        self.type = None
        self.population = population

    def __repr__(self):
        return "%s|%s_%s" % (self.id, self.name, self.uid, self.population)


class PopulationReader(handler.ContentHandler):

    """The class is for parsing the OSM XML file.
       The data parsed is written into the net.
    """

    def __init__(self, net, foutredundant, encoding):
        self._net = net
        self._nodeId = None
        self._nodeObj = None
        self._nodeLat = None
        self._nodeLon = None
        self._nodeuid = None
        self._place = None
        self._relationId = None
        self._relationObj = None
        self._relationuid = None
        self._type = None
        self._name = None
        self._population = None
        self._nodeNamesList = []
        self._fout = foutredundant
        self._encoding = encoding

    def startElement(self, name, attrs):
        if name == 'node':
            self._nodeId = attrs['id']
            self._nodeLat = attrs['lat']
            self._nodeLon = attrs['lon']
            if 'uid' in attrs:
                self._nodeuid = attrs['uid']
        if self._nodeId and name == 'tag':
            if attrs['k'] == 'name':
                self._name = attrs['v']
            if not self._name and attrs['k'] == 'name:de':
                self._name = attrs['v']
            if not self._name and attrs['k'] == 'openGeoDB:name':
                self._name = attrs['v']
            if attrs['k'] == 'place':
                self._place = attrs['v']
            if not self._population and attrs['k'] == 'openGeoDB:population':
                self._population = attrs['v']
            if not self._population and attrs['k'] == 'population':
                self._population = attrs['v']
        if name == 'relation':
            self._relationId = attrs['id']
            self._uid = attrs['uid']
            if 'uid' in attrs:
                self._relationuid = attrs['uid']
        if self._relationId and name == 'tag':
            if attrs['k'] == 'name':
                self._name = attrs['v']
            if attrs['k'] == 'type':
                self._type = attrs['v']
            if not self._population and attrs['k'] == 'openGeoDB:population':
                self._population = attrs['v']
            if not self._population and attrs['k'] == 'population':
                self._population = attrs['v']

    def endElement(self, name):
        if name == 'node' and self._population:
            newInput = True
            for n in self._net._nodes:
                # diffLat = abs(float(self._nodeLat) - float(n.lat))
                # diffLon = abs(float(self._nodeLon) - float(n.lon))
                # and diffLat < 0.003 and diffLon < 0.003 and
                # int(self._population) == int(n.population):
                if self._name and self._name == n.name and self._population == n.population:
                    newInput = False
                    self._fout.write(('node\t%s\t%s\t%s\t%s\t%s\n' % (
                        self._name, self._nodeId, self._nodeLat, self._nodeLon,
                        self._population)).encode(self._encoding))
                    break
            if newInput:
                self._nodeObj = self._net.addNode(
                    self._nodeId, self._nodeLat, self._nodeLon, self._population)
                if self._nodeuid:
                    self._nodeObj.uid = self._nodeuid
                    if self._nodeuid not in self._net._uidNodeMap:
                        self._net._uidNodeMap[self._nodeuid] = []
                    self._net._uidNodeMap[self._nodeuid].append(self._nodeObj)
                if self._name:
                    self._nodeObj.name = self._name
                if self._place:
                    self._nodeObj.place = self._place
                self._nodeId = None
                self._nodeObj = None
                self._nodeLat = None
                self._nodeLon = None
                self._nodeuid = None
                self._place = None
                self._name = None
                self._population = None

        if name == 'relation' and self._population:
            newInput = True
            for r in self._net._relations:
                if self._name and self._name == r.name and self._population == r.population:
                    newInput = False
                    self._fout.write(('relation\t%s\t%s\t%s\t%s\n' % (
                        self._name, self._relationId, self._relationuid, self._population)).encode(options.encoding))
                    break
            if newInput:
                self._relationObj = self._net.addRelation(
                    self._relationId, self._relationuid, self._population)
                self._relationObj.population = self._population
                if self._relationuid not in self._net._uidRelationMap:
                    self._net._uidRelationMap[self._relationuid] = []
                self._net._uidRelationMap[
                    self._relationuid].append(self._relationObj)

                if self._name:
                    self._relationObj.name = self._name
                if self._type:
                    self._relationObj.place = self._type
                self._relationId = None
                self._relationObj = None
                self._relationuid = None
                self._type = None
                self._name = None
                self._population = None


def main():
    parser = make_parser()
    osmFile = options.osmfile
    print('osmFile:', osmFile)
    if options.bsafile:
        bsaFile = options.bsafile
        print('bsaFile:', bsaFile)
    if options.outputfile:
        prefix = options.outputfile
    else:
        prefix = osmFile.split('.')[0]
    redundantDataFile = '%s_redundantOSMData.txt' % prefix
    foutredundant = open(redundantDataFile, 'w')
    net = Net()
    parser.setContentHandler(
        PopulationReader(net, foutredundant, options.encoding))
    parser.parse(osmFile)
    foutredundant.close()
    print('finish with data parsing')
    if options.generateoutputs:
        print('write the population to the output file')
        outputfile = '%s_populations.txt' % prefix
        fout = open(outputfile, 'w')
        fout.write("attribute\tid\tname\tuid\tpopulation\tlat\tlon\n")
        for n in net._nodes:
            fout.write(("%s\t%s\t%s\t%s\t%s\t%s\t%s\n" % (
                n.attribute, n.id, n.name, n.uid, n.population, n.lat, n.lon)).encode(options.encoding))
        fout.close()

        if os.path.exists(outputfile):
            fout = open(outputfile, 'a')
        else:
            print("there is no file named %s", outputfile)
            print("A new file will be open.")
            fout = open(outputfile, 'w')

        for r in net._relations:
            fout.write(("%s\t%s\t%s\t%s\t%s\tNone\tNone\n" % (
                r.attribute, r.id, r.name, r.uid, r.population)).encode(options.encoding))
        fout.close()

        fout = open('%s_nodesWithSameUid.txt' % prefix, 'w')
        fout.write('nodeUid\tnodeId\tname\n')
        for r in net._uidNodeMap:
            fout.write('%s' % r)
            for n in net._uidNodeMap[r]:
                fout.write(
                    ('\t%s\t%s' % (n.id, n.name)).encode(options.encoding))
            fout.write('\n')
        fout.close()

        fout = open('%s_uidRelations.txt' % prefix, 'w')
        fout.write('relationUid\trelationId\tname\n')
        for r in net._uidRelationMap:
            fout.write('%s' % r)
            for n in net._uidRelationMap[r]:
                fout.write(
                    ('\t%s\t%s' % (n.id, n.name)).encode(options.encoding))
            fout.write('\n')
        fout.close()

    if options.bsafile:
        print('compare the data with the data from BSA')
        bsaTotalCount = 0
        matchedCount = 0

        fout = open("%s_matchedAreas.txt" % prefix, 'w')
        fout.write(
            "#bsaName\tbsaArea\tbsaPop\tbsaLat\tbsaLon\tosmName\tosmAtt\tosmPop\tosmLat\tosmLon\n")
        noneList = []
        for line in open(options.bsafile):
            if '#' not in line:
                line = line.split('\n')[0]
                line = line.split(';')
                name = line[0].decode("latin1")
                area = float(line[1])
                pop = int(line[2])
                lon = line[3]
                lat = line[4]
                bsaTotalCount += 1

                for n in net._nodes:
                    if n.name is None and n not in noneList:
                        noneList.append(n)
                    # and n.name not in areasList:
                    elif n.name is not None and name == n.name:
                        matchedCount += 1
                        fout.write(("%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n" % (
                            name, area, pop, lat, lon, n.name, n.attribute, n.population,
                            n.lat, n.lon)).encode(options.encoding))

                for r in net._relations:
                    if r.name is None and r not in noneList:
                        noneList.append(r)
                    # and r.name not in areasList:
                    elif r.name is not None and name == r.name:
                        matchedCount += 1
                        fout.write(("%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\tNone\tNone\n" % (
                            name, area, pop, lat, lon, r.name, r.attribute, r.population)).encode(options.encoding))
        fout.close()
        if len(noneList) > 0:
            foutnone = open("%s_nameNone.txt" % prefix, 'w')
            foutnone.write("nodeId\tnodeName\tPopulation\tLat\tLon\n")
            for n in noneList:
                foutnone.write(("%s\t%s\t%s\t%s\t%s\n" % (
                    n.id, n.name, n.population, n.lat, n.lon)).encode(options.encoding))
            foutnone.close()
        # Duplicated data does not exist.
        osmTotalCount = len(net._nodes) + len(net._relations)
        print('matched count in OSM and BSA data:', matchedCount)
        print('Number of entries in the BSA data:', bsaTotalCount)
        print('Number of entries in the OSM data:', osmTotalCount)


optParser = OptionParser()
optParser.add_option("-s", "--osm-file", dest="osmfile",
                     help="read OSM file from FILE (mandatory)", metavar="FILE")
optParser.add_option("-b", "--bsa-file", dest="bsafile",
                     help="read population (in csv form) provided by German federal statistic authority " +
                          "(Bundesstatistikamt) from FILE", metavar="FILE")
optParser.add_option("-o", "--output-file", dest="outputfile",
                     help="define the prefix name of the output file")
optParser.add_option(
    "-e", "--encoding", help="output file encoding (default: %default)", default="utf8")
optParser.add_option("-g", "--generate-outputs", dest="generateoutputs", action="store_true",
                     default=False, help="generate output files")
(options, args) = optParser.parse_args()

if not options.osmfile:
    optParser.print_help()
    sys.exit()
main()
