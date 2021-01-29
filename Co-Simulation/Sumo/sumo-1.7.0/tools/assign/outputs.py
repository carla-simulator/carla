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

# @file    outputs.py
# @author  Yun-Pang Floetteroed
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2007-12-25

from __future__ import absolute_import
from __future__ import print_function

import os
import random
import datetime
import operator

# calculate the time for reading the input data (matrix data are excluded.)


def timeForInput(inputreaderstart):
    fouttime = open('timeforinput.txt', 'w')
    inputreadtime = datetime.datetime.now() - inputreaderstart
    fouttime.write('Time for reading input files:%s\n' % inputreadtime)
    fouttime.close()

# output the input matrices, origins, destinations and the number of OD
# pairsdemand > 0)


def outputODZone(startVertices, endVertices, Pshort_EffCells, MatrixCounter):
    foutmatrixstart = open('origins.txt', 'a')
    foutmatrixend = open('destinations.txt', 'a')
    foutmatrixstart.write('Interval =%s\n' % (MatrixCounter))
    foutmatrixstart.write('number of origins=%s\n' % len(startVertices))
    foutmatrixstart.write(
        'number of effective OD cells for Passenger vehicles=%s\n' % (Pshort_EffCells))
#    foutmatrixstart.write('number of effective OD cells for Passenger vehicles(long distances)=%s\n' %(Plong_EffCells))
#    foutmatrixstart.write('number of effective OD cells for trucks=%s\n' %(Truck_EffCells))
    for i in range(0, len(startVertices)):
        foutmatrixstart.write('%s\n' % startVertices[i])

    foutmatrixend.write('number of destinations=%s\n' % len(endVertices))
    for j in range(0, len(endVertices)):
        foutmatrixend.write('%s\n' % endVertices[j])

    foutmatrixstart.close()
    foutmatrixend.close()

# output the network data which is based on the SUMO-network


def outputNetwork(net):
    foutnet = open('network.txt', 'w')
    net.printNet(foutnet)
    foutnet.close()

# ouput the required CPU time for the assignment and the assignment
# results (e.g. link flows, link travel times)


def outputStatistics(net, starttime, periods):
    totaltime = 0.0
    totalflow = 0.0
    assigntime = datetime.datetime.now() - starttime
    foutMOE = open('MOE.txt', 'w')
    foutMOE.write('Number of analyzed periods(hr):%s' % periods)
    # generate the output of the link travel times
    for edge in net._edges:
        if edge.estcapacity > 0.:
            totaltime += edge.flow * edge.actualtime
            totalflow += edge.flow
            foutMOE.write(('\nedge:%s \t from:%s \t to:%s \t freeflowtime(s):%2.2f \t traveltime(s):%2.2f \t ' +
                           'traffic flow(veh):%2.2f \t v/c:%2.2f') % (
                          edge._id, edge._from, edge._to, edge.freeflowtime, edge.actualtime,
                          edge.flow, (edge.flow / edge.estcapacity)))
        if edge.flow > edge.estcapacity and edge.connection == 0:
            foutMOE.write('****overflow!')

    avetime = totaltime / totalflow
    foutMOE.write('\nTotal flow(veh):%2.2f \t average travel time(s):%2.2f\n' % (
        totalflow, avetime))
    foutMOE.write(
        '\nTime for the traffic assignment and reading matrices:%s' % assigntime)
    foutMOE.close()
    return assigntime

# output the releasing time and the route for each vehicle


def sortedVehOutput(vehicles, departtime, options, foutroute):
    random.seed(42)
    for veh in vehicles:
        if veh.depart == 0:
            veh.depart = random.randint(
                departtime, departtime + 3600 * options.hours)
    # sorting by departure times
    vehicles.sort(key=operator.attrgetter('depart'))
    # output the generated routes
    for veh in vehicles:
        foutroute.write('    <vehicle id="%s" depart="%d" departLane="free">\n' % (
            veh.label, veh.depart))
        foutroute.write('        <route>')
        # for generating vehicle routes used in SUMO
        for edge in veh.route[1:-1]:
            foutroute.write('%s ' % edge._id)
        foutroute.write('</route>\n')
        foutroute.write('    </vehicle>\n')

# output the result of the matrix estimation with the traffic counts


def outputMatrix(startVertices, endVertices, estMatrix, daytimeindex):
    filename = 'estimatedMatri-' + daytimeindex + '.fma'
    foutmtx = open(filename, 'w')

    foutmtx.write(
        '$VMR;D2;estimated with the generalized least squares model\n')
    foutmtx.write('* Verkehrsmittelkennung\n')
    foutmtx.write('   1\n')
    foutmtx.write('* Von  Bis\n\n')
    foutmtx.write('* Faktor\n')
    foutmtx.write('1.00\n')
    foutmtx.write('*\n')
    foutmtx.write('* Deutsches Zentrum fuer Luft- und Raumfahrt e.V.\n')
    foutmtx.write('* %s\n' % datetime.datetime.now())
    foutmtx.write('* Anzahl Bezirke\n')
    foutmtx.write('%s\n' % len(startVertices))
    foutmtx.write('*\n')
    for startVertex in startVertices:
        foutmtx.write('%s ' % startVertex.label)
    foutmtx.write('\n*')
    for start, startVertex in enumerate(startVertices):
        count = -1
        foutmtx.write('\n* from: %s\n' % startVertex.label)
        for end, endVertex in enumerate(endVertices):
            count += 1
            if operator.mod(count, 12) != 0:
                foutmtx.write('%s ' % estMatrix[start][end])
            elif count > 12:
                foutmtx.write('\n%s ' % estMatrix[start][end])
    foutmtx.close()


def linkChoicesOutput(net, startVertices, endVertices, matrixPshort, linkChoiceMap, odPairsMap, outputdir, starttime):
    foutchoice = open(os.path.join(outputdir, "linkchoices.xml"), 'w')
    print("""<?xml version="1.0"?>
<!-- generated on %s by $Id$ -->
<edgechoices>""" % starttime, file=foutchoice)
    for e in net._detectedEdges:
        if len(linkChoiceMap[e.detected]) > 0:
            foutchoice.write(
                '    <edge id="%s" flows="%s">\n' % (e.label, e.flow))
            foutchoice.write('        <choiceprobs>\n')
            for start, startVertex in enumerate(startVertices):
                for end, endVertex in enumerate(endVertices):
                    if startVertex.label != endVertex.label and matrixPshort[start][end] > 0.:
                        odIndex = odPairsMap[
                            startVertex.label][endVertex.label]
                        foutchoice.write('            <choice origin="%s" destination="%s" choiceprob="%.5f"/>\n'
                                         % (startVertex.label, endVertex.label,
                                            linkChoiceMap[e.detected][odIndex] / matrixPshort[start][end]))
            foutchoice.write('        </choiceprobs>\n')
            foutchoice.write('    </edge>\n')
    foutchoice.write('</edgechoices>\n')
    foutchoice.close()
