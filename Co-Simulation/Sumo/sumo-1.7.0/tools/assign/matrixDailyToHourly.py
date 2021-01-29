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

# @file    matrixDailyToHourly.py
# @author  Yun-Pang Floetteroed
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2008-08-20

"""
This script is to generate hourly matrices from a VISUM daily matrix.
The taffic demand of the traffic zones, which have the same connection links, will be integrated.
"""
from __future__ import absolute_import
from __future__ import print_function

import sys
from optparse import OptionParser
from xml.sax import make_parser, handler

OUTPUTDIR = "./input/"
optParser = OptionParser()
optParser.add_option("-m", "--matrix-file", dest="mtxpsfile",
                     help="read OD matrix for passenger vehicles(long dist.) from FILE (mandatory)", metavar="FILE")
optParser.add_option("-z", "--districts-file", dest="districtsfile",
                     help="read connecting links from FILE (mandatory)", metavar="FILE")
optParser.add_option("-t", "--timeSeries-file", dest="timeseries",
                     help="read hourly traffic demand rate from FILE", metavar="FILE")
optParser.add_option("-d", "--dir", dest="OUTPUTDIR", default=OUTPUTDIR,
                     help="Directory to store the output files. Default: " + OUTPUTDIR)
optParser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                     default=False, help="tell me what you are doing")

(options, args) = optParser.parse_args()


class District:

    def __init__(self, label):
        self.label = label
        self.sourcelink = None
        self.sinklink = None
        self.combinedDistrict = []
        self.alreadyCombined = False

    def __repr__(self):

        return "%s<%s|%s|%s>" % (self.label, self.sourcelink, self.sinklink, self.combinedDistrict)


class DistrictsReader(handler.ContentHandler):

    def __init__(self, districtList):
        self._districtList = districtList
        self._newDistrict = None
        self._district = None

    def startElement(self, name, attrs):
        if name == 'taz':
            self._newDistrict = District(attrs['id'])
            self._district = attrs['id']
            self._districtList.append(self._newDistrict)
        elif name == 'tazSource' and self._district is not None:
            self._newDistrict.sourcelink = attrs['id']
        elif name == 'tazSink' and self._district is not None:
            self._newDistrict.sinklink = attrs['id']

    def endElement(self, name):
        if name == 'taz':
            self._district = None


def combineDemand(matrix, districtList, startVertices, endVertices):
    matrixMap = {}
    combinedCounter = 0
    existCounter = 0
    foutzone = open('combinedZones.txt', 'w')

    for i, start in enumerate(startVertices):
        matrixMap[start] = {}
        for j, end in enumerate(endVertices):
            matrixMap[start][end] = matrix[i][j]

    for district1 in districtList:
        if not district1.alreadyCombined:
            foutzone.write('district:%s\n' % district1.label)
            foutzone.write('combinedDistricts: ')
            for district2 in districtList:
                if not district2.alreadyCombined:
                    if district1.label != district2.label and district1.sourcelink == district2.sourcelink:
                        district1.combinedDistrict.append(district2)
                        district2.alreadyCombined = True
                        foutzone.write('%s, ' % district2.label)
            foutzone.write('\n')

    for start in startVertices:
        for district in districtList:
            if start == district.label and district.combinedDistrict != []:
                existCounter += 1
                combinedCounter += len(district.combinedDistrict)
                for end in endVertices:
                    for zone in district.combinedDistrict:
                        matrixMap[start][end] += matrixMap[zone.label][end]
                        matrixMap[zone.label][end] = 0.

            elif start == district.label and district.combinedDistrict == [] and not district.alreadyCombined:
                existCounter += 1

    for i, start in enumerate(startVertices):
        for j, end in enumerate(endVertices):
            matrix[i][j] = matrixMap[start][end]

    foutzone.close()
    matrixMap.clear()
    print('finish combining zones!')
    print('number of zones (before):', len(startVertices))
    print('number of zones (after):', existCounter)
    print('number of the combined zones:', combinedCounter)

    return matrix
# read the analyzed matrix


def getMatrix(verbose, matrix):  # , mtxplfile, mtxtfile):
    matrixPshort = []
    startVertices = []
    endVertices = []
    Pshort_EffCells = 0
    periodList = []
    MatrixSum = 0.
    if verbose:
        print('matrix:', str(matrix))
    ODpairs = 0
    origins = 0
    dest = 0
    CurrentMatrixSum = 0.0
    skipCount = 0
    zones = 0
    for line in open(matrix):
        if line[0] == '$':
            visumCode = line[1:3]
            if visumCode != 'VM':
                skipCount += 1
        elif line[0] != '*' and line[0] != '$':
            skipCount += 1
            if skipCount == 2:
                for elem in line.split():
                    periodList.append(float(elem))
                print('periodList:', periodList)
            elif skipCount > 3:
                if zones == 0:
                    for elem in line.split():
                        zones = int(elem)
                        print('zones:', zones)
                elif len(startVertices) < zones:
                    for elem in line.split():
                        if len(elem) > 0:
                            startVertices.append(elem)
                            endVertices.append(elem)
                    origins = len(startVertices)
                    dest = len(endVertices)
                elif len(startVertices) == zones:
                    if ODpairs % origins == 0:
                        matrixPshort.append([])
                    for item in line.split():
                        matrixPshort[-1].append(float(item))
                        ODpairs += 1
                        MatrixSum += float(item)
                        CurrentMatrixSum += float(item)
                        if float(item) > 0.0:
                            Pshort_EffCells += 1
    begintime = int(periodList[0])
    if verbose:
        foutlog = open('log.txt', 'w')
        foutlog.write('Number of zones:%s, Number of origins:%s, Number of destinations:%s, begintime:%s, \n' % (
            zones, origins, dest, begintime))
        foutlog.write('CurrentMatrixSum:%s, total O-D pairs:%s, effective O-D pairs:%s\n' %
                      (CurrentMatrixSum, ODpairs, Pshort_EffCells))
        print('Number of zones:', zones)
        print('Number of origins:', origins)
        print('Number of destinations:', dest)
        print('begintime:', begintime)
        print('CurrentMatrixSum:', CurrentMatrixSum)
        print('total O-D pairs:', ODpairs)
        print('Effective O-D Cells:', Pshort_EffCells)
        print('len(startVertices):', len(startVertices))
        print('len(endVertices):', len(endVertices))
        foutlog.close()

    return matrixPshort, startVertices, endVertices, Pshort_EffCells, MatrixSum, CurrentMatrixSum, begintime, zones


def main():
    if not options.mtxpsfile:
        optParser.print_help()
        sys.exit()

    districtList = []
    parser = make_parser()
    parser.setContentHandler(DistrictsReader(districtList))
    parser.parse(options.districtsfile)

    MTX_STUB = "mtx%02i_%02i.fma"
    matrix = options.mtxpsfile
    if options.OUTPUTDIR:
        OUTPUTDIR = options.OUTPUTDIR

    matrix, startVertices, endVertices, Pshort_EffCells, MatrixSum, CurrentMatrixSum, begintime, zones = getMatrix(
        options.verbose, matrix)
    timeSeriesList = []
    hourlyMatrix = []
    subtotal = 0.

    if options.verbose:
        foutlog = open('log.txt', 'a')
    if options.timeseries:
        print('read the time-series profile')
    # combine matrices
    matrix = combineDemand(matrix, districtList, startVertices, endVertices)

    for i in range(0, len(startVertices)):
        hourlyMatrix.append([])
        for j in range(0, len(endVertices)):
            hourlyMatrix[-1].append(0.)

    if options.timeseries:
        for line in open(options.timeseries):
            for elem in line.split():
                timeSeriesList.append(float(elem))
    else:
        factor = 1. / 24.
        for i in range(0, 24):
            timeSeriesList.append(factor)

    for hour in range(0, 24):
        for i in range(0, len(startVertices)):
            for j in range(0, len(endVertices)):
                hourlyMatrix[i][j] = matrix[i][j] * timeSeriesList[0]

        filename = MTX_STUB % (hour, hour + 1)

        foutmatrix = open(OUTPUTDIR + filename, 'w')  # /input/filename

        foutmatrix.write('$VMR;D2\n')
        foutmatrix.write('* Verkehrsmittelkennung\n')
        foutmatrix.write('   1\n')
        foutmatrix.write('*  ZeitIntervall\n')
        foutmatrix.write('    %s.00  %s.00\n' % (hour, hour + 1))
        foutmatrix.write('*  Faktor\n')
        foutmatrix.write('   1.000000\n')
        foutmatrix.write('*  Anzahl Bezirke\n')
        foutmatrix.write('   %s\n' % zones)
        foutmatrix.write('*  BezirksNummern \n')

        for count, start in enumerate(startVertices):
            count += 1
            if count == 1:
                foutmatrix.write('          %s ' % start)
            else:
                foutmatrix.write('%s ' % start)
            if count != 1 and count % 10 == 0:
                foutmatrix.write('\n')
                foutmatrix.write('          ')
            # count == (len(startVertices) -1):
            elif count % 10 != 0 and count == len(startVertices):
                foutmatrix.write('\n')

        for i, start in enumerate(startVertices):
            subtotal = 0.
            foutmatrix.write('*  %s\n' % startVertices[i])
            foutmatrix.write('         ')
            for j, end in enumerate(endVertices):
                k = j + 1
                foutmatrix.write('        %.4f' % hourlyMatrix[i][j])
                subtotal += hourlyMatrix[i][j]
                if k % 10 == 0:
                    foutmatrix.write('\n')
                    foutmatrix.write('         ')
                elif k % 10 != 0 and j == (len(endVertices) - 1):
                    foutmatrix.write('\n')
            if options.verbose:
                print('origin:', startVertices[i])
                print('subtotal:', subtotal)
                foutlog.write('origin:%s, subtotal:%s\n' %
                              (startVertices[i], subtotal))
        foutmatrix.close()
    if options.verbose:
        print('done with generating', filename)

    if options.verbose:
        foutlog.close()


main()
