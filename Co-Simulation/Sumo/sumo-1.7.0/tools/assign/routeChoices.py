# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2007-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    routeChoices.py
# @author  Evamarie Wiessner
# @author  Yun-Pang Floetteroed
# @author  Michael Behrisch
# @date    2007-02-27

"""
This script is to calculate the route choice probabilities based on different methods.
- Gawron
- step-size (TBD)
- ......
"""
from __future__ import absolute_import
from __future__ import print_function

import os
import random
import math
from xml.sax import handler
from xml.sax import parse


class Vehicle:

    def __init__(self, label, depart, departlane='first', departpos='base', departspeed=0):
        self.label = label
        self.CO_abs = 0.
        self.CO2_abs = 0.
        self.HC_abs = 0.
        self.PMx_abs = 0.
        self.NOx_abs = 0.
        self.fuel_abs = 0.
        self.routesList = []
        # self.speed = 0.
        self.depart = float(depart)
        self.departlane = departlane
        self.departpos = departpos
        self.departspeed = departspeed
        self.selectedRoute = None


class Edge:

    def __init__(self, label):
        self.label = label
        self.length = 0.
        self.freespeed = 0.
        self.CO_abs = 0.
        self.CO2_abs = 0.
        self.HC_abs = 0.
        self.PMx_abs = 0.
        self.NOx_abs = 0.
        self.fuel_abs = 0.
        self.traveltime = 0.
        self.CO_perVeh = 0.
        self.CO2_perVeh = 0.
        self.HC_perVeh = 0.
        self.PMx_perVeh = 0.
        self.NOx_perVeh = 0.
        self.fuel_perVeh = 0.
        # only one veh on the edge
        self.fuel_perVeh_default = 0.
        self.CO_perVeh_default = 0.
        self.CO2_perVeh_default = 0.
        self.HC_perVeh_default = 0.
        self.PMx_perVeh_default = 0.
        self.NOx_perVeh_default = 0.
        self.fuel_perVeh_default = 0.
        self.freetraveltime = 0.


pathNum = 0


class Route:

    def __init__(self, edges):
        global pathNum
        self.label = "%s" % pathNum
        pathNum += 1
        self.edges = edges
        # self.ex_probability = None
        self.probability = 0.
        self.selected = False
        self.ex_cost = 0.
        self.act_cost = 0.


class netReader(handler.ContentHandler):

    def __init__(self, edgesList, edgesMap):
        self._edgesList = edgesList
        self._edgesMap = edgesMap
        self._edgeObj = None

    def startElement(self, name, attrs):
        if name == 'edge' and 'function' not in attrs:
            if attrs['id'] not in self._edgesMap:
                self._edgeObj = Edge(attrs['id'])
                self._edgesList.append(self._edgeObj)
                self._edgesMap[attrs['id']] = self._edgeObj
        if self._edgeObj and name == 'lane':
            self._edgeObj.length = float(attrs['length'])
            self._edgeObj.freespeed = float(attrs['speed'])
            self._edgeObj.freetraveltime = self._edgeObj.length / \
                self._edgeObj.freespeed

    def endElement(self, name):
        if name == 'edge':
            self._edgeObj = None


class addweightsReader(handler.ContentHandler):

    def __init__(self, edgesList, edgesMap):
        self._edgesList = edgesList
        self._edgesMap = edgesMap
        self._edgObj = None

    def startElement(self, name, attrs):
        if name == 'edge':
            if attrs['id'] in self._edgesMap:
                self._edgeObj = self._edgesMap[attrs['id']]
            if 'traveltime' in attrs:
                self._edgeObj.freetraveltime = float(attrs['traveltime'])
            if 'CO_perVeh' in attrs:
                self._edgeObj.CO_perVeh_default = float(attrs['CO_perVeh'])
            if 'CO2_perVeh' in attrs:
                self._edgeObj.CO2_perVeh_default = float(attrs['CO2_perVeh'])
            if 'HC_perVeh' in attrs:
                self._edgeObj.HC_perVeh_default = float(attrs['HC_perVeh'])
            if 'PMx_perVeh' in attrs:
                self._edgeObj.PMx_perVeh_default = float(attrs['PMx_perVeh'])
            if 'NOx_perVeh' in attrs:
                self._edgeObj.NOx_perVeh_default = float(attrs['NOx_perVeh'])
            if 'fuel_perVeh' in attrs:
                self._edgeObj.fuel_perVeh_default = float(attrs['fuel_perVeh'])
            if 'fuel_abs' in attrs:
                self._edgeObj.fuel_abs_default = float(attrs['fuel_abs'])
            if 'NOx_abs' in attrs:
                self._edgeObj.NOx_abs_default = float(attrs['NOx_abs'])
            if 'PMx_abs' in attrs:
                self._edgeObj.PMx_abs_default = float(attrs['PMx_abs'])
            if 'HC_abs' in attrs:
                self._edgeObj.HC_abs_default = float(attrs['HC_abs'])
            if 'CO2_abs' in attrs:
                self._edgeObj.CO2_abs_default = float(attrs['CO2_abs'])
            if 'CO_abs' in attrs:
                self._edgeObj.CO_abs_default = float(attrs['CO_abs'])


class routeReader(handler.ContentHandler):

    def __init__(self, vehList, vehMap):
        self._vehList = vehList
        self._vehMap = vehMap
        self._vehObj = None
        self._routObj = None

    def startElement(self, name, attrs):
        if name == 'vehicle':
            if ('departPos' in attrs):
                self._vehObj = Vehicle(attrs['id'], attrs['depart'], attrs[
                                       'departLane'], attrs['departPos'], attrs['departSpeed'])
            else:
                self._vehObj = Vehicle(attrs['id'], attrs['depart'])
            self._vehMap[attrs['id']] = self._vehObj
            self._vehList.append(self._vehObj)

        if self._vehObj and name == 'route':
            edgesList = attrs['edges'].split(' ')
            self._routObj = Route(" ".join(edgesList))
            self._vehObj.routesList.append(self._routObj)

    def endElement(self, name):
        if name == 'vehicle':
            self._vehObj = None
            self._routObj = None


class vehrouteReader(handler.ContentHandler):

    def __init__(self, vehList, vehMap, edgesMap, fout, foutrout, ecoMeasure, alpha, beta):
        self._vehList = vehList
        self._vehMap = vehMap
        self._edgesMap = edgesMap
        self._fout = fout
        self._foutrout = foutrout
        self._ecoMeasure = ecoMeasure
        self._newroutesList = []
        self._alpha = alpha
        self._beta = beta
        self._vehObj = None
        self._routObj = None
        self._selected = None
        self._currentSelected = None
        self._count = 0
        self._existed = False

    def startElement(self, name, attrs):
        if name == 'vehicle':
            self._vehObj = self._vehMap[attrs['id']]

        if self._vehObj and name == 'routeDistribution':
            self._currentSelected = attrs['last']

        if self._vehObj and name == 'route':
            if self._count == int(self._currentSelected):
                self._vehObj.selectedRouteEdges = attrs['edges']
            self._count += 1
            for r in self._vehObj.routesList:
                if r.edges == attrs['edges']:
                    self._existed = True
                    self._routObj = r
                    break
            if not self._existed:
                self._routObj = Route(attrs['edges'])
                self._vehObj.routesList.append(self._routObj)

            if 'probability' in attrs:
                self._routObj.probability = float(attrs['probability'])
                if self._routObj.probability == 0.0:
                    # check with Micha if there is a better way to avoid the
                    # prob. = 0.
                    self._routObj.probability = 1.02208127529e-16
            if 'cost' in attrs:
                self._routObj.ex_cost = float(attrs['cost'])
            for e in self._routObj.edges.split(' '):
                eObj = self._edgesMap[e]
                if self._ecoMeasure != 'fuel' and eObj.traveltime == 0.:
                    self._routObj.act_cost += eObj.freetraveltime
                elif self._ecoMeasure != 'fuel' and eObj.traveltime > 0.:
                    self._routObj.act_cost += eObj.traveltime
                elif self._ecoMeasure == 'fuel' and eObj.fuel_perVeh == 0.:
                    self._routObj.act_cost += eObj.fuel_perVeh_default
                elif self._ecoMeasure == 'fuel' and eObj.fuel_perVeh > 0.:
                    self._routObj.act_cost += eObj.fuel_perVeh
            if self._routObj.ex_cost == 0.:
                self._routObj.ex_cost = self._routObj.act_cost

    def endElement(self, name):
        if name == 'vehicle':
            # if len(self._vehObj.routesList) == 1:
            #    self._vehObj.routesList[0].probability = 1.
            # for the routes which are from the sumo's rou.alt.xml file
            for r in self._vehObj.routesList:
                if r.act_cost == 0.:
                    for e in r.edges.split(' '):
                        eObj = self._edgesMap[e]
                        if self._ecoMeasure != 'fuel' and eObj.traveltime == 0.:
                            r.act_cost += eObj.freetraveltime
                        elif self._ecoMeasure != 'fuel' and eObj.traveltime > 0.:
                            r.act_cost += eObj.traveltime
                        elif self._ecoMeasure == 'fuel' and eObj.fuel_perVeh == 0.:
                            r.act_cost += eObj.fuel_perVeh_default
                        elif self._ecoMeasure == 'fuel' and eObj.fuel_perVeh > 0.:
                            r.act_cost += eObj.fuel_perVeh
                if r.ex_cost == 0.:
                    r.ex_cost = r.act_cost
                # calcuate the probabilites for the new routes
                if not r.probability:
                    r.probability = 1. / float(len(self._vehObj.routesList))
                    print('new probability for route', r.label,
                          'for veh', self._vehObj.label)
                    self._newroutesList.append(r)

            # adjust the probabilites of the existing routes due to the new
            # routes
            if len(self._newroutesList) > 0:
                addProb = 0.
                origProbSum = 0.
                for r in self._vehObj.routesList:
                    if r in self._newroutesList:
                        addProb += r.probability
                    else:
                        origProbSum += r.probability
                for r in self._vehObj.routesList:
                    if r not in self._newroutesList:
                        r.probability = r.probability / \
                            origProbSum * (1. - addProb)

            # update the costs of routes not used by the driver
            for r in self._vehObj.routesList:
                if r.edges != self._vehObj.selectedRouteEdges:
                    r.act_cost = self._beta * r.act_cost + \
                        (1. - self._beta) * r.ex_cost

            # calcuate the route choice probabilities based on Gawron
            # todo: add "one used route to all routes"
            for r1 in self._vehObj.routesList:
                for r2 in self._vehObj.routesList:
                    if r1.label != r2.label:
                        gawron(r1, r2, self._alpha)

            # decide which route will be selected
            randProb = random.random()
            if len(self._vehObj.routesList) == 1:
                self._vehObj.routesList[0].probability = 1.
                self._selected = 0
            else:
                cumulatedProbs = 0.
                for i, r in enumerate(self._vehObj.routesList):
                    cumulatedProbs += r.probability
                    if cumulatedProbs >= randProb:
                        self._selected = i
                        break

            # generate the *.rou.xml
            self._foutrout.write('    <vehicle id="%s" depart="%.2f" departLane="%s" departPos="%s" departSpeed="%s">\n'
                                 % (self._vehObj.label, self._vehObj.depart, self._vehObj.departlane,
                                    self._vehObj.departpos, self._vehObj.departspeed))
            self._foutrout.write(
                '        <route edges="%s"/>\n' % self._vehObj.routesList[self._selected].edges)
            self._foutrout.write('    </vehicle> \n')

            # generate the *.rou.alt.xml
            self._fout.write('    <vehicle id="%s" depart="%.2f" departLane="%s" departPos="%s" departSpeed="%s">\n'
                             % (self._vehObj.label, self._vehObj.depart, self._vehObj.departlane,
                                self._vehObj.departpos, self._vehObj.departspeed))
            self._fout.write(
                '        <routeDistribution last="%s">\n' % self._selected)

            for route in self._vehObj.routesList:
                self._fout.write('            <route cost="%.4f" probability="%s" edges="%s"/>\n' % (
                    route.act_cost, route.probability, route.edges))
            self._fout.write('        </routeDistribution>\n')
            self._fout.write('    </vehicle> \n')

            self._newroutesList = []
            self._vehObj = None
            self._selected = None
            self._currentSelected = None
            self._count = 0
        if name == 'route':
            self._routObj = None
        if (name == 'route-alternatives' or name == 'routes'):
            self._fout.write('</route-alternatives>\n')
            self._fout.close()
            self._foutrout.write('</routes>\n')
            self._foutrout.close()


class dumpsReader(handler.ContentHandler):

    def __init__(self, edgesList, edgesMap):
        self._edgesList = edgesList
        self._edgeObj = None
        self._edgesMap = edgesMap

    def startElement(self, name, attrs):
        if name == 'edge':
            if attrs['id'] not in self._edgesMap:
                self._edgeObj = Edge(attrs['id'])
                self._edgesList.append(self._edgeObj)
                self._edgesMap[attrs['id']] = self._edgeObj
            else:
                self._edgeObj = self._edgesMap[attrs['id']]

            if 'traveltime' in attrs:
                self._edgeObj.traveltime = float(attrs['traveltime'])
            if 'CO_perVeh' in attrs:
                self._edgeObj.CO_perVeh = float(attrs['CO_perVeh'])
            if 'CO2_perVeh' in attrs:
                self._edgeObj.CO2_perVeh = float(attrs['CO2_perVeh'])
            if 'HC_perVeh' in attrs:
                self._edgeObj.HC_perVeh = float(attrs['HC_perVeh'])
            if 'PMx_perVeh' in attrs:
                self._edgeObj.PMx_perVeh = float(attrs['PMx_perVeh'])
            if 'NOx_perVeh' in attrs:
                self._edgeObj.NOx_perVeh = float(attrs['NOx_perVeh'])
            if 'fuel_perVeh' in attrs:
                self._edgeObj.fuel_perVeh = float(attrs['fuel_perVeh'])
            if 'fuel_abs' in attrs:
                self._edgeObj.fuel_abs = float(attrs['fuel_abs'])
            if 'NOx_abs' in attrs:
                self._edgeObj.NOx_abs = float(attrs['NOx_abs'])
            if 'PMx_abs' in attrs:
                self._edgeObj.PMx_abs = float(attrs['PMx_abs'])
            if 'HC_abs' in attrs:
                self._edgeObj.HC_abs = float(attrs['HC_abs'])
            if 'CO2_abs' in attrs:
                self._edgeObj.CO2_abs = float(attrs['CO2_abs'])
            if 'CO_abs' in attrs:
                self._edgeObj.CO_abs = float(attrs['CO_abs'])

    def endElement(self, name):
        if name == 'edge':
            self._edgeObj = None


def resetEdges(edgesMap):
    for eid in edgesMap:
        e = edgesMap[eid]
        e.traveltime = 0.
        e.CO_abs = 0.
        e.CO2_abs = 0.
        e.HC_abs = 0.
        e.PMx_abs = 0.
        e.NOx_abs = 0.
        e.fuel_abs = 0.
        e.CO_perVeh = 0.
        e.CO2_perVeh = 0.
        e.HC_perVeh = 0.
        e.PMx_perVeh = 0.
        e.NOx_perVeh = 0.
        e.fuel_perVeh = 0.


# check with Micha
def getRouteChoices(edgesMap, dumpfile, routeAltfile, netfile, addWeightsfile, alpha, beta, step, ecoMeasure=None):
    random.seed(42)   # check with micha
    edgesList = []
    vehList = []
    vehMap = {}
    print('run getRouteChoices')
    print('ecoMeasure:', ecoMeasure)
    outputPath = os.path.abspath(routeAltfile)
    outputPath = os.path.dirname(outputPath)
    prefix = os.path.basename(routeAltfile)
    # prefix = prefix[:prefix.find('.')]
    prefix = prefix[:-12]
    # print('outputPath:', outputPath)
    print('prefix:', prefix)
    outputAltfile = os.path.join(outputPath, prefix + '.rou.galt.xml')
    outputRoufile = os.path.join(outputPath, prefix + '.grou.xml')

    if len(edgesMap) == 0:
        try:
            print('parse network file')
            parse(netfile, netReader(edgesList, edgesMap))
        except AttributeError:
            print("could not parse netfile: " + str(netfile))
        try:
            parse(addWeightsfile, addweightsReader(edgesList, edgesMap))
        except AttributeError:
            print("could not parse weights file: " + str(addWeightsfile))
    else:
        resetEdges(edgesMap)

    fout = open(outputAltfile, 'w')
    foutrout = open(outputRoufile, 'w')
    fout.write('<?xml version="1.0"?>\n')
    fout.write('<!--\n')
    fout.write('route choices are generated with use of %s' %
               os.path.join(os.getcwd(), 'routeChoices.py'))
    fout.write('-->\n')
    fout.write('<route-alternatives>\n')
    foutrout.write('<?xml version="1.0"?>\n')
    foutrout.write('<!--\n')
    foutrout.write('route choices are generated with use of %s' %
                   os.path.join(os.getcwd(), 'routeChoices.py'))
    foutrout.write('-->\n')
    foutrout.write('<routes>')

    print('parse dumpfile')
    print(dumpfile)
    parse(dumpfile, dumpsReader(edgesList, edgesMap))
    # parse routeAltfile from SUMO
    try:
        print('parse routeAltfile:', routeAltfile)
        parse(routeAltfile, routeReader(vehList, vehMap))
    except IOError:
        print('could not parse routeAltfile:', routeAltfile)
    ex_outputAltFile = prefix[
        :prefix.rfind('_')] + '_%03i' % (step - 1) + '.rou.galt.xml'
    try:
        print('parse routeAltfile from externalGawron: ', ex_outputAltFile)
        parse(ex_outputAltFile, vehrouteReader(
            vehList, vehMap, edgesMap, fout, foutrout, ecoMeasure, alpha, beta))
    except IOError:
        print('could not parse routeAltfile from externalGawron:', ex_outputAltFile)
    return outputRoufile, edgesMap


def gawron(r1, r2, alpha):
    a = alpha
    delta = (r2.act_cost - r1.act_cost) / (r1.act_cost + r2.act_cost)
    g = math.exp(a * delta / (1 - delta * delta))
    ex_prob = r1.probability
    r1.probability = (r1.probability * (r1.probability + r2.probability) *
                      g) / (r1.probability * g + r2.probability)  # check together with Eva
    r2.probability = ex_prob + r2.probability - r1.probability


def calFirstRouteProbs(dumpfile, sumoAltFile, addweights, ecoMeasure=None):
    basename = sumoAltFile.split('_')[0]
    outputAltFile = basename + "_001.rou.galt.xml"
    outputRouFile = basename + "_001.rou.alt.xml"
    edgesList = []
    edgesMap = {}
    vehList = []
    vehMap = {}
#    parse(netfile, netReader(edgesList, edgesMap))
    parse(addweights, addweightsReader(edgesList, edgesMap))
    parse(dumpfile, dumpsReader(edgesList, edgesMap))
    parse(sumoAltFile, routeReader(vehList, vehMap))

    fout = open(outputAltFile, 'w')
    foutrout = open(outputRouFile, 'w')
    fout.write('<?xml version="1.0"?>\n')
    fout.write('<!--\n')
    fout.write('route choices are generated with use of %s' %
               os.path.join(os.getcwd(), 'routeChoices.py'))
    fout.write('-->\n')
    fout.write('<route-alternatives>\n')
    foutrout.write('<?xml version="1.0"?>\n')
    foutrout.write('<!--\n')
    foutrout.write('route choices are generated with use of %s' %
                   os.path.join(os.getcwd(), 'routeChoices.py'))
    foutrout.write('-->\n')
    foutrout.write('<routes>')

    for v in vehMap:
        vehObj = vehMap[v]
        for r in vehObj.routesList:
            for e in r.edges.split(' '):
                eObj = edgesMap[e]
                if ecoMeasure != 'fuel' and eObj.traveltime == 0.:
                    r.act_cost += eObj.freetraveltime
                    r.ex_cost += eObj.freetraveltime
                elif ecoMeasure != 'fuel' and eObj.traveltime > 0.:
                    r.act_cost += eObj.traveltime
                    r.ex_cost += eObj.freetraveltime
                elif ecoMeasure == 'fuel' and eObj.fuel_perVeh == 0.:
                    r.act_cost += eObj.fuel_perVeh_default
                    r.ex_cost += eObj.fuel_perVeh_default
                elif ecoMeasure == 'fuel' and eObj.fuel_perVeh > 0.:
                    r.act_cost += eObj.fuel_perVeh
                    r.ex_cost += eObj.fuel_perVeh
        costSum = 0.
        for r in vehObj.routesList:
            costSum += r.ex_cost
        for r in vehObj.routesList:
            r.ex_probability = r.ex_cost / costSum

        randProb = random.random()
        selected = 0
        if len(vehObj.routesList) > 1:
            cumulatedProbs = 0.
            for i, r in enumerate(vehObj.routesList):
                cumulatedProbs += r.ex_probability
                if cumulatedProbs >= randProb:
                    selected = i
                    break

        # generate the *.rou.xml
        foutrout.write('    <vehicle id="%s" depart="%.2f" departLane="%s" departPos="%s" departSpeed="%s">\n'
                       % (vehObj.label, vehObj.depart, vehObj.departlane, vehObj.departpos, vehObj.departspeed))
        foutrout.write(
            '        <route edges="%s"/>\n' % vehObj.routesList[selected].edges)
        foutrout.write('    </vehicle> \n')

        # generate the *.rou.alt.xml
        fout.write('    <vehicle id="%s" depart="%.2f" departLane="%s" departPos="%s" departSpeed="%s">\n'
                   % (vehObj.label, vehObj.depart, vehObj.departlane, vehObj.departpos, vehObj.departspeed))
        fout.write('        <routeDistribution last="%s">\n' % selected)

        for route in vehObj.routesList:
            fout.write('            <route cost="%.4f" probability="%s" edges="%s"/>\n' %
                       (route.act_cost, route.ex_probability, route.edges))
        fout.write('        </routeDistribution>\n')
        fout.write('    </vehicle> \n')
    fout.write('</route-alternatives>\n')
    fout.close()
    foutrout.write('</routes>\n')
    foutrout.close()
