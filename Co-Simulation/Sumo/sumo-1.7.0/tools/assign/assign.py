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

# @file    assign.py
# @author  Yun-Pang Floetteroed
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2007-11-25

"""
This script is for executing traffic assignment according to the required assignment model.
The incremental assignment model, the C-Logit assignment model and the Lohse assignment model
are included in this script.
"""
from __future__ import absolute_import
from __future__ import print_function

import math
import operator
import elements
from elements import Vehicle


def doIncAssign(net, vehicles, verbose, iteration, odestimation, endVertices, start, startVertex, matrixPshort,
                smallDemand, D, P, AssignedVeh, AssignedTrip, vehID, assignSmallDemand, linkChoiceMap, odPairsMap):

    for end, endVertex in enumerate(endVertices):
        getlinkChoices = False
        if ((odestimation and matrixPshort[start][end] > 0.) or
                (matrixPshort[start][end] > 1. or (assignSmallDemand and smallDemand[start][end] > 0.))):
            getlinkChoices = True

        if startVertex._id != endVertex._id and getlinkChoices:
            # if matrixPling and the matrixTruck exist, matrixPlong[start][end]
            # > 0.0 or matrixTruck[start][end] > 0.0): should be added.
            helpPath = []
            vertex = endVertex
            demand = 0.
            if matrixPshort[start][end] > 1. or odestimation:
                demand = matrixPshort[start][end] / float(iteration)
            if assignSmallDemand and not odestimation:
                demand += smallDemand[start][end]

            while vertex != startVertex:
                if P[vertex].kind == "real":
                    helpPath.append(P[vertex])
                    P[vertex].flow += demand
                    if getlinkChoices and P[vertex] in net._detectedEdges:
                        odIndex = odPairsMap[startVertex._id][endVertex._id]
                        linkChoiceMap[P[vertex].detected][odIndex] += demand

                vertex = P[vertex].source
            helpPath.reverse()

            # the amount of the pathflow, which will be released at this
            # iteration
            if assignSmallDemand:
                smallDemand[start][end] = 0.

            if not odestimation:
                AssignedTrip[startVertex][endVertex] += demand
                vehID = assignVeh(
                    verbose, vehicles, startVertex, endVertex, helpPath, AssignedVeh, AssignedTrip, vehID)
    return vehID, smallDemand, linkChoiceMap

# execute the SUE model with the given path set


def doSUEAssign(net, options, startVertices, endVertices, matrixPshort, iter, lohse, first):
    if lohse:
        if options.verbose:
            foutassign = open('lohse_pathSet.txt', 'a')
            foutassign.write('\niter:%s\n' % iter)

    # matrixPlong and matrixTruck should be added if available.
    if options.verbose:
        print('pathNum in doSUEAssign:', elements.pathNum)
    # calculate the overlapping factors between any two paths of a given OD
    # pair
    for start, startVertex in enumerate(startVertices):
        for end, endVertex in enumerate(endVertices):
            cumulatedflow = 0.
            pathcount = 0
            if matrixPshort[start][end] > 0. and startVertex._id != endVertex._id:
                ODPaths = net._paths[startVertex][endVertex]

                for path in ODPaths:
                    path.getPathTimeUpdate()
                calCommonalityAndChoiceProb(ODPaths, options.alpha, lohse)

                # calculate the path choice probabilities and the path flows
                # for the given OD Pair
                for path in ODPaths:
                    pathcount += 1
                    if pathcount < len(ODPaths):
                        path.helpflow = matrixPshort[
                            start][end] * path.choiceprob
                        cumulatedflow += path.helpflow
                        if lohse and options.verbose:
                            foutassign.write('    path:%s\n' % path.label)
                            foutassign.write(
                                '    path.choiceprob:%s\n' % path.choiceprob)
                            foutassign.write(
                                '    path.helpflow:%s\n' % path.helpflow)
                            foutassign.write(
                                '    cumulatedflow:%s\n' % cumulatedflow)
                    else:
                        path.helpflow = matrixPshort[
                            start][end] - cumulatedflow
                        if lohse and options.verbose:
                            foutassign.write(
                                '    last_path.helpflow:%s\n' % path.helpflow)
                    if first and iter == 1:
                        for edge in path.edges:
                            edge.flow += path.helpflow
                    else:
                        for edge in path.edges:
                            edge.helpflow += path.helpflow

    # Reset the convergence index for the C-Logit model
    notstable = 0
    stable = False
    # link travel times and link flows will be updated according to the latest
    # traffic assingment
    for edge in net._edges:
        if (first and iter > 1) or (not first):
            exflow = edge.flow
            edge.flow = edge.flow + (1. / iter) * (edge.helpflow - edge.flow)

            if not lohse:
                if edge.flow > 0.:
                    if abs(edge.flow - exflow) / edge.flow > options.sueTolerance:
                        notstable += 1
                elif edge.flow == 0.:
                    if exflow != 0. and (abs(edge.flow - exflow) / exflow > options.sueTolerance):
                        notstable += 1
                elif edge.flow < 0.:
                    notstable += 1
                    edge.flow = 0.
            else:
                if edge.flow < 0.:
                    edge.flow = 0.
        # reset the edge.helpflow for the next iteration
        edge.helpflow = 0.0
        edge.getActualTravelTime(options, lohse)
        if options.dijkstra == 'boost':
            edge.boost.weight = edge.helpacttime
        if edge.queuetime > 1.:
            notstable += 1
    if lohse and options.verbose:
        foutassign.close()

    if not lohse and iter > 5:
        if notstable == 0:
            stable = True
        elif notstable < math.ceil(net.geteffEdgeCounts() * 0.005) or notstable < 3:
            stable = True

        if iter > options.maxiteration:
            stable = True
            print('Number of max. iterations is reached!')
            print('stable:', stable)

    return stable

# calculate the commonality factors in the C-Logit model


def calCommonalityAndChoiceProb(ODPaths, alpha, lohse):
    if len(ODPaths) > 1:
        for path in ODPaths:
            if not lohse:
                path.utility = path.actpathtime + \
                    alpha * math.log(path.sumOverlap)
            else:
                path.utility = path.pathhelpacttime + \
                    alpha * math.log(path.sumOverlap)

        if lohse:
            minpath = min(ODPaths, key=operator.attrgetter('pathhelpacttime'))
            beta = 12. / (1. + math.exp(0.7 - 0.015 * minpath.pathhelpacttime))
        else:
            theta = getThetaForCLogit(ODPaths)

        for pathone in ODPaths:
            sum_exputility = 0.
            for pathtwo in ODPaths:
                if pathone != pathtwo:
                    if not lohse:
                        sum_exputility += math.exp(theta *
                                                   (pathone.utility - pathtwo.utility))
                    else:
                        pathtwoPart = beta * \
                            (pathtwo.utility / minpath.utility - 1.)
                        pathonePart = beta * \
                            (pathone.utility / minpath.utility - 1.)
                        sum_exputility += math.exp(-(pathtwoPart *
                                                     pathtwoPart) + pathonePart * pathonePart)
            pathone.choiceprob = 1. / (1. + sum_exputility)
    else:
        for path in ODPaths:
            path.choiceprob = 1.

# calculate the path choice probabilities and the path flows and generate
# the vehicular data for each OD Pair


def doSUEVehAssign(net, vehicles, options, counter, matrixPshort, startVertices, endVertices, AssignedVeh,
                   AssignedTrip, vehID, lohse):
    if options.verbose:
        if counter == 0:
            foutpath = open('paths.txt', 'w')
            fouterror = open('errors.txt', 'w')
        else:
            foutpath = open('paths.txt', 'a')
            fouterror = open('errors.txt', 'a')
        if lohse:
            foutpath.write(
                'begin the doSUEVehAssign based on the lohse assignment model!')
        else:
            foutpath.write(
                'begin the doSUEVehAssign based on the c-logit model!')
        foutpath.write('the analyzed matrix=%s' % counter)

    TotalPath = 0

    for start, startVertex in enumerate(startVertices):
        if options.verbose:
            foutpath.write('\norigin=%s, ' % startVertex)
        for end, endVertex in enumerate(endVertices):
            pathcount = 0
            cumulatedflow = 0.
            if matrixPshort[start][end] > 0. and startVertex._id != endVertex._id:
                if options.verbose:
                    foutpath.write('destination=%s' % endVertex)
                ODPaths = net._paths[startVertex][endVertex]

                for path in ODPaths:
                    TotalPath += 1
                    path.getPathTimeUpdate()
                    if lohse:
                        path.pathhelpacttime = path.actpathtime

                calCommonalityAndChoiceProb(ODPaths, options.alpha, lohse)

                for path in ODPaths:
                    pathcount += 1
                    if pathcount < len(ODPaths):
                        path.pathflow = matrixPshort[
                            start][end] * path.choiceprob
                        cumulatedflow += path.pathflow
                    else:
                        path.pathflow = matrixPshort[
                            start][end] - cumulatedflow
                        if options.verbose and path.pathflow < 0.:
                            fouterror.write(
                                '*********************** the path flow on the path:%s < 0.!!' % path.label)
                    if options.verbose:
                        foutpath.write('\npathID= %s, path flow=%4.4f, actpathtime=%4.4f, choiceprob=%4.4f, edges='
                                       % (path.label, path.pathflow, path.actpathtime, path.choiceprob))
                        for item in path.edges:
                            foutpath.write('%s, ' % (item._id))

                    AssignedTrip[startVertex][endVertex] += path.pathflow
                    edges = []
                    for link in path.edges:
                        edges.append(link)
                    vehID = assignVeh(
                        options.verbose, vehicles, startVertex, endVertex, edges, AssignedVeh, AssignedTrip, vehID)
                if options.verbose:
                    foutpath.write('\n')
    if options.verbose:
        print(
            'total Number of the used paths for the current matrix:', TotalPath)
        foutpath.write(
            '\ntotal Number of the used paths for the current matrix:%s' % TotalPath)
        foutpath.close()
        fouterror.close()
    return vehID


def assignVeh(verbose, vehicles, startVertex, endVertex, edges, AssignedVeh, AssignedTrip, vehID):
    while AssignedVeh[startVertex][endVertex] < int(round(AssignedTrip[startVertex][endVertex])):
        vehID += 1
        newVehicle = Vehicle(str(vehID))
        newVehicle.route = edges
        vehicles.append(newVehicle)

        AssignedVeh[startVertex][endVertex] += 1
    if verbose:
        print('vehID:', vehID)
        print('AssignedTrip[start][end]', AssignedTrip[startVertex][endVertex])
        print('AssignedVeh[start][end]', AssignedVeh[startVertex][endVertex])

    return vehID


def getThetaForCLogit(ODPaths):
    sum = 0.
    diff = 0.
    minpath = min(ODPaths, key=operator.attrgetter('actpathtime'))

    for path in ODPaths:
        sum += path.actpathtime

    meanpathtime = sum / float(len(ODPaths))

    for path in ODPaths:
        diff += (path.actpathtime - meanpathtime)**2.

    sdpathtime = (diff / float(len(ODPaths)))**0.5

    if sdpathtime > 0.04:
        theta = math.pi / (pow(6., 0.5) * sdpathtime * minpath.actpathtime)
    else:
        theta = 1.

    return theta


def doLohseStopCheck(net, options, stable, iter, maxIter, foutlog):
    stable = False
    # Check if the convergence reaches.
    if iter > 1:
        counts = 0
        for edge in net._edges.itervalues():
            stop = edge.stopCheck(options)
            if stop:
                counts += 1
        if counts == net.geteffEdgeCounts():
            stable = True
            foutlog.write(
                'The defined convergence is reached. The number of the required iterations:%s\n' % iter)
        elif counts < int(net.geteffEdgeCounts() * 0.05) and float(iter) > options.maxiteration * 0.85:
            stable = True
            foutlog.write(
                ('The number of the links with convergence is 95% of the total links.' +
                 'The number of executed iterations:%s\n') % iter)

    if iter >= maxIter:
        print('The max. number of iterations is reached!')
        foutlog.write('The max. number(%s) of iterations is reached!\n' % iter)
        foutlog.write(
            'The number of new routes will be set to 0, since the max. number of iterations is reached.')
        stable = True
        print('stop?:', stable)
        print('iter_inside:', iter)
    return stable
