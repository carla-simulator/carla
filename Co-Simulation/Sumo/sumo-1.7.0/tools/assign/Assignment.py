#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2008-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    Assignment.py
# @author  Jakob Erdmann
# @author  Yun-Pang Floetteroed
# @author  Michael Behrisch
# @date    2008-03-28

"""
This script is for executing the traffic assignment.
Three assignment models are available:
- incremental
- c-logit
- lohse

The c-logit model are set as default.
"""
from __future__ import absolute_import
from __future__ import print_function

import os
import sys
import datetime
import math
import operator
from xml.sax import make_parser
from optparse import OptionParser

sys.path.append(os.path.join(os.environ["SUMO_HOME"], 'tools'))
import sumolib.net  # noqa

from network import Net, DistrictsReader, ExtraSignalInformationReader  # noqa
from dijkstra import dijkstraBoost, dijkstraPlain, dijkstra  # noqa
from inputs import getMatrix, getConnectionTravelTime  # noqa
from outputs import timeForInput, outputODZone, outputNetwork, outputStatistics, sortedVehOutput, linkChoicesOutput  # noqa
from assign import doSUEAssign, doLohseStopCheck, doSUEVehAssign, doIncAssign  # noqa
from tables import updateCurveTable  # noqa


def initLinkChoiceMap(net, startVertices, endVertices, matrixPshort, linkChoiceMap, odPairsMap):
    odpairCounts = 0
    for start, startVertex in enumerate(startVertices):
        odPairsMap[startVertex.label] = {}
        for end, endVertex in enumerate(endVertices):
            if startVertex.label != endVertex.label and matrixPshort[start][end] > 0.:
                odpairCounts += 1
                odPairsMap[startVertex.label][endVertex.label] = odpairCounts

                for e in net._detectedEdges:
                    if e.detected not in linkChoiceMap:
                        linkChoiceMap[e.detected] = {}
                    linkChoiceMap[e.detected][odpairCounts] = 0.

    return linkChoiceMap


def main():
    # for measuring the required time for reading input files
    inputreaderstart = datetime.datetime.now()
    foutlog = open('%s_log.txt' % options.type, 'w')
    foutlog.write(
        'The stochastic user equilibrium traffic assignment will be executed with the %s model.\n' % options.type)
    foutlog.write(
        'All vehicular releasing times are determined randomly(uniform).\n')

    matrices = options.mtxpsfile.split(",")
    parser = make_parser()

    if options.verbose:
        print("Reading net")
    print('net file:', options.netfile)
    net = Net()
    sumolib.net.readNet(options.netfile, net=net)
    parser.setContentHandler(DistrictsReader(net))
    parser.parse(options.confile)
    if options.sigfile:
        parser.setContentHandler(ExtraSignalInformationReader(net))
        parser.parse(options.sigfile)
    foutlog.write('- Reading network: done.\n')
    foutlog.write('number of total startVertices:%s\n' % net.getstartCounts())
    foutlog.write('number of total endVertices:%s\n' % net.getendCounts())
    if options.verbose:
        print(net.getfullEdgeCounts(), "edges read (internal edges included)")

    if options.curvefile:
        updateCurveTable(options.curvefile)

    if options.hours == 24.:
        assignHours = 16.
    else:
        assignHours = options.hours

    for edge in net.getEdges():
        if edge._lanes:
            edge.getCapacity()
            edge.getAdjustedCapacity(net)
            edge.estcapacity *= assignHours
            edge.getConflictLink()

    if options.dijkstra == 'boost':
        net.createBoostGraph()
    if options.verbose:
        print("after link reduction:", net.getfullEdgeCounts(), "edges read")

    # calculate link travel time for all district connectors
    getConnectionTravelTime(net._startVertices, net._endVertices)

    foutlog.write('- Initial calculation of link parameters : done.\n')
    # the required time for reading the network
    timeForInput(inputreaderstart)

    if options.debug:
        outputNetwork(net)

    # initialize the map for recording the number of the assigned vehicles
    AssignedVeh = {}
    # initialize the map for recording the number of the assigned trips
    AssignedTrip = {}
    smallDemand = []
    linkChoiceMap = {}
    odPairsMap = {}
    for start, startVertex in enumerate(net._startVertices):
        AssignedVeh[startVertex] = {}
        AssignedTrip[startVertex] = {}
        smallDemand.append([])
        for end, endVertex in enumerate(net._endVertices):
            AssignedVeh[startVertex][endVertex] = 0
            AssignedTrip[startVertex][endVertex] = 0.
            smallDemand[-1].append(0.)

    # initialization
    vehID = 0
    matrixSum = 0.0
    lohse = (options.type == "lohse")
    incremental = (options.type == "incremental")
    checkKPaths = False

    if not incremental and options.kPaths > 1:
        checkKPaths = True
    if not incremental:
        net.initialPathSet()

    starttime = datetime.datetime.now()
    # initialize the file for recording the routes
    if options.odestimation:
        net.getDetectedEdges(options.outputdir)
    else:
        foutroute = open('routes.rou.xml', 'w')
        print('<?xml version="1.0"?>\n<!-- generated on %s by $Id: Assignment.py v1_3_1+0411-36956f96df michael.behrisch@dlr.de 2019-01-23 11:12:48 +0000 $ -->\n<routes>' % starttime, file=foutroute)  # noqa

    # for counter in range (0, len(matrices)):
    for counter, matrix in enumerate(matrices):
        # delete all vehicle information related to the last matrix for saving
        # the disk space
        vehicles = []
        iterInterval = 0
        matrixPshort, startVertices, endVertices, CurrentMatrixSum, begintime, assignPeriod, Pshort_EffCells, \
            matrixSum, smallDemandRatio = getMatrix(net, options.verbose, matrix, matrixSum, options.demandscale)
        options.hours = float(assignPeriod)
        smallDemandPortion = math.ceil(
            float(options.maxiteration) / 2. * smallDemandRatio)
        if float(smallDemandPortion) != 0.:
            iterInterval = math.ceil(
                float(options.maxiteration) / float(smallDemandPortion))

        departtime = begintime * 3600

        if options.verbose:
            print('the analyzed matrices:', counter)
            print('Begintime:', begintime, "O'Clock")
            print('departtime', departtime)
            print('Matrix und OD Zone already read for Interval', counter)
            print('CurrentMatrixSum:', CurrentMatrixSum)

        foutlog.write('Reading matrix and O-D zones: done.\n')
        foutlog.write(
            'Matrix und OD Zone already read for Interval:%s\n' % counter)
        foutlog.write('CurrentMatrixSum:%s\n' % CurrentMatrixSum)
        foutlog.write('number of current startVertices:%s\n' %
                      len(startVertices))
        foutlog.write('number of current endVertices:%s\n' % len(endVertices))

        if options.odestimation:
            linkChoiceMap.clear()
            odPairsMap.clear()
            linkChoiceMap = initLinkChoiceMap(
                net, startVertices, endVertices, matrixPshort, linkChoiceMap, odPairsMap)

        for edge in net.getEdges():
            edge.flow = 0.
            edge.helpflow = 0.
            edge.actualtime = edge.freeflowtime
            edge.helpacttime = edge.freeflowtime
            edge.fTT = 0.
            edge.TT = 0.
            edge.delta = 0.
            edge.helpacttimeEx = 0.

        # the number of origins, the umber of destinations and the number of
        # the OD pairs
        len(startVertices)
        len(endVertices)

        # output the origin and destination zones and the number of effective
        # OD pairs
        if options.debug:
            # matrixCounter)
            outputODZone(startVertices, endVertices, Pshort_EffCells, counter)

        if incremental:
            print('begin the incremental assignment!')
            iter = 0
            options.lamda = 0.
            while iter < options.maxiteration:
                foutlog.write(
                    '- Current iteration(not executed yet):%s\n' % iter)
                iter += 1
                if iterInterval != 0 and operator.mod(iter, iterInterval) == 0:
                    assignSmallDemand = True
                else:
                    assignSmallDemand = False
                for start, startVertex in enumerate(startVertices):
                    targets = set()
                    for end, endVertex in enumerate(endVertices):
                        if assignSmallDemand and matrixPshort[start][end] > 0. and matrixPshort[start][end] < 1.:
                            smallDemand[start][end] = matrixPshort[
                                start][end] / float(smallDemandPortion)

                        if matrixPshort[start][end] > 1. or (assignSmallDemand and smallDemand[start][end] > 0.):
                            targets.add(endVertex)

                    if len(targets) > 0:
                        if options.dijkstra == 'boost':
                            D, P = dijkstraBoost(
                                net._boostGraph, startVertex.boost)
                        elif options.dijkstra == 'plain':
                            D, P = dijkstraPlain(startVertex, targets)
                        elif options.dijkstra == 'extend':
                            D, P = dijkstra(startVertex, targets)
                        vehID, smallDemand, linkChoiceMap = doIncAssign(
                            net, vehicles, options.verbose, options.maxiteration, options.odestimation,
                            endVertices, start, startVertex, matrixPshort, smallDemand,
                            D, P, AssignedVeh, AssignedTrip, vehID, assignSmallDemand, linkChoiceMap, odPairsMap)

                if options.dijkstra != 'extend':
                    linkMap = net._fullEdges
                else:
                    linkMap = net._edges
                for edge in linkMap.itervalues():
                    edge.getActualTravelTime(options, False)
                    if options.dijkstra == 'boost':
                        edge.boost.weight = edge.helpacttime
        else:
            print('begin the', options.type, " assignment!")
            # initialization for the clogit and the lohse assignment model
            iter_outside = 1
            newRoutes = 1
            stable = False
            first = True
            # begin the traffic Assignment
            while newRoutes > 0:
                foutlog.write('- SUE iteration:%s\n' % iter_outside)
                # Generate the effective routes als intital path solutions,
                # when considering k shortest paths (k is defined by the user.)
                if checkKPaths:
                    checkPathStart = datetime.datetime.now()
                    newRoutes = net.calcKPaths(
                        options.verbose, options.kPaths, newRoutes, startVertices, endVertices, matrixPshort,
                        options.gamma)
                    checkPathEnd = datetime.datetime.now() - checkPathStart
                    foutlog.write(
                        '- Time for finding the k-shortest paths: %s\n' % checkPathEnd)
                    foutlog.write(
                        '- Finding the k-shortest paths for each OD pair: done.\n')
                    if options.verbose:
                        print('iter_outside:', iter_outside)
                        print('number of k shortest paths:', options.kPaths)
                        print('number of new routes:', newRoutes)

                elif not checkKPaths and iter_outside == 1 and counter == 0:
                    print('search for the new path')
                    newRoutes = net.findNewPath(
                        startVertices, endVertices, newRoutes, matrixPshort, options.gamma, lohse, options.dijkstra)

                checkKPaths = False

                if options.verbose:
                    print('iter_outside:', iter_outside)
                    print('number of new routes:', newRoutes)

                stable = False
                iter_inside = 1
                while not stable:
                    if options.verbose:
                        print('iter_inside:', iter_inside)
                    stable = doSUEAssign(
                        net, options, startVertices, endVertices, matrixPshort, iter_inside, lohse, first)
                    # The matrixPlong and the matrixTruck should be added when
                    # considering the long-distance trips and the truck trips.
                    if lohse:
                        stable = doLohseStopCheck(
                            net, options, stable, iter_inside, options.maxiteration, foutlog)

                    iter_inside += 1

                    if options.verbose:
                        print('stable:', stable)

                newRoutes = net.findNewPath(
                    startVertices, endVertices, newRoutes, matrixPshort, options.gamma, lohse, options.dijkstra)

                first = False
                iter_outside += 1

                if newRoutes < 3 and iter_outside > int((options.maxiteration) / 2):
                    newRoutes = 0

                if iter_outside > options.maxiteration:
                    print('The max. number of iterations is reached!')
                    foutlog.write(
                        'The max. number of iterations is reached!\n')
                    foutlog.write(
                        'The number of new routes and the parameter stable will be set to zero and ' +
                        'True respectively.\n')
                    print('newRoutes:', newRoutes)
                    stable = True
                    newRoutes = 0

            # update the path choice probability and the path flows as well as
            # generate vehicle data
            vehID = doSUEVehAssign(net, vehicles, options, counter, matrixPshort,
                                   startVertices, endVertices, AssignedVeh, AssignedTrip, vehID, lohse)

        # output the generated vehicular releasing times and routes, based on
        # the current matrix
        print('done with the assignment')  # debug
        if options.odestimation:
            linkChoicesOutput(net, startVertices, endVertices, matrixPshort,
                              linkChoiceMap, odPairsMap, options.outputdir, starttime)
        else:
            sortedVehOutput(vehicles, departtime, options, foutroute)

    if not options.odestimation:
        foutroute.write('</routes>\n')
        foutroute.close()

    # output the global performance indices
    assigntime = outputStatistics(net, starttime, len(matrices))

    foutlog.write(
        '- Assignment is completed and all required information is generated. ')
    foutlog.close()

    if options.verbose:
        print('Duration for traffic assignment:', assigntime)
        print('Total assigned vehicles:', vehID)
    print('Total number of the assigned trips:', matrixSum)


optParser = OptionParser()
optParser.add_option("-m", "--matrix-file", dest="mtxpsfile",
                     help="read OD matrix for passenger vehicles from FILE (mandatory)", metavar="FILE")
optParser.add_option("-G", "--globalmatrix-file", dest="glbmtxfile",
                     help="read daily OD matrix for passenger vehicles from FILE (mandatory)", metavar="FILE")
optParser.add_option("-n", "--net-file", dest="netfile",
                     help="read SUMO network from FILE (mandatory)", metavar="FILE")
optParser.add_option("-d", "--district-file", dest="confile",
                     help="read OD Zones from FILE (mandatory)", metavar="FILE")
optParser.add_option("-s", "--extrasignal-file", dest="sigfile",
                     help="read extra/updated signal timing plans from FILE", metavar="FILE")
optParser.add_option("-u", "--crCurve-file", dest="curvefile",
                     help="read parameters used in cost functions from FILE", metavar="FILE")
optParser.add_option("-k", "--k-shortest-paths", dest="kPaths", type="int",
                     default=8, help="number of the paths should be found at the first iteration")
optParser.add_option("-i", "--max-sue-iteration", dest="maxiteration", type="int",
                     default=20, help="maximum number of the assignment iterations")
optParser.add_option("-t", "--sue-tolerance", dest="sueTolerance", type="float",
                     default=0.001, help="difference tolerance for the convergence in the c-logit model")
optParser.add_option("-a", "--alpha", dest="alpha", type="float",
                     default=0.15, help="alpha value to determine the commonality factor")
optParser.add_option("-g", "--gamma", dest="gamma", type="float",
                     default=1., help="gamma value to determine the commonality factor")
optParser.add_option("-l", "--lambda", dest="lamda", type="float",
                     default=0.3, help="lambda value to determine the penalty time due to queue")
optParser.add_option("-U", "--under-value", dest="under", type="float",
                     default=0.15, help="parameter 'under' to determine auxiliary link cost")
optParser.add_option("-p", "--upper-value", dest="upper", type="float",
                     default=0.5, help="parameter 'upper' to determine auxiliary link cost")
optParser.add_option("-X", "--parameter-1", dest="v1", type="float",
                     default=2.5, help="parameter 'v1' to determine auxiliary link cost in the lohse model")
optParser.add_option("-y", "--parameter-2", dest="v2", type="float",
                     default=4., help="parameter 'v2' to determine auxiliary link cost in the lohse model")
optParser.add_option("-z", "--parameter-3", dest="v3", type="float",
                     default=0.002, help="parameter 'v3' to determine auxiliary link cost in the lohse model")
optParser.add_option("-c", "--convergence-parameter-1", dest="cvg1", type="float",
                     default=1., help="parameter 'cvg1' to calculate the convergence value in the lohse model")
optParser.add_option("-o", "--convergence-parameter-2", dest="cvg2", type="float",
                     default=1., help="parameter 'cvg2' to calculate the convergence value in the lohse model")
optParser.add_option("-q", "--convergence-parameter-3", dest="cvg3", type="float",
                     default=10., help="parameter 'cvg3' to calculate the convergence value in the lohse model")
optParser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                     default=False, help="tell me what you are doing")
optParser.add_option("-b", "--debug", action="store_true", dest="debug",
                     default=False, help="debug the program")
optParser.add_option("-e", "--type", dest="type", type="choice",
                     choices=('clogit', 'lohse', 'incremental'),
                     default="clogit", help="type of assignment [default: %default]")
optParser.add_option("-H", "--hours", dest="hours", type="float",
                     default=1., help="the analysing period(hours)")
optParser.add_option("-r", "--profile", action="store_true", dest="profile",
                     default=False, help="writing profiling info")
optParser.add_option("-+", "--dijkstra", dest="dijkstra", type="choice",
                     choices=('extend', 'plain', 'boost'),
                     default="plain", help="use penalty, plain(original) or boost in dijkstra implementation " +
                                           "[default: %default]")
optParser.add_option("-x", "--odestimation", action="store_true", dest="odestimation",
                     default=False, help="generate trips for OD estimation")
optParser.add_option("-f", "--scale-factor", dest="demandscale",
                     type="float", default=1., help="scale demand by ")
optParser.add_option("-O", "--output-dir", dest="outputdir",
                     default=os.getcwd(), help="define the output directory name and path")
(options, args) = optParser.parse_args()

if not options.netfile or not options.confile or not options.mtxpsfile:
    optParser.print_help()
    sys.exit()

if options.profile:
    import hotshot
    import hotshot.stats
    hotshotFile = "hotshot_%s_stats" % options.type
    prof = hotshot.Profile(hotshotFile)
    prof.runcall(main)
    prof.close()
    s = hotshot.stats.load(hotshotFile)
    s.strip_dirs().sort_stats("time").print_stats(20)
else:
    main()
