#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2009-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    generateITetrisNetworkMetrics.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2007-10-25

from __future__ import absolute_import
from __future__ import print_function
from optparse import OptionParser
import os
import sys
import numpy
from xml.sax import make_parser, handler
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import sumolib.net  # noqa


def quantil(a, alpha):
    asort = numpy.sort(a)
    n = len(asort)
    j = int(n * alpha)
    if (j * alpha == n):
        return 0.5 * (asort[j - 1] + asort[j])
    else:
        return asort[j]

# This class is for storing vehicle information, such as departure time,
# route and travel time.


class Vehicle:

    def __init__(self, label):
        self.label = label
        self.depart = 0.
        self.arrival = 0.
        self.speed = 0.
        self.traveltime = 0.
        self.travellength = 0.
        self.departdelay = 0.
        self.waittime = 0.
        self.fuel_consumption = 0.
        self.co = 0.
        self.co2 = 0.
        self.hc = 0.
        self.pmx = 0.
        self.nox = 0.
        self.vtype = None

    def __repr__(self):
        return "%s_%s_%s_%s_%s_%s<%s>" % (
            self.label, self.depart, self.arrival, self.speed, self.traveltime, self.travellength)


class VehInformationReader(handler.ContentHandler):

    def __init__(self, vehList):
        self._vehList = vehList
        self._Vehicle = None

    def startElement(self, name, attrs):
        if name == 'tripinfo':
            self._Vehicle = Vehicle(attrs['id'])
            self._Vehicle.vtype = attrs['vtype']
            self._Vehicle.traveltime = float(attrs['duration'])
            self._Vehicle.travellength = float(attrs['routeLength'])
            self._Vehicle.departdelay = float(attrs['departDelay'])
            self._Vehicle.waittime = float(
                attrs['departDelay']) + float(attrs['waitingTime'])
            self._Vehicle.depart = float(attrs['depart'])
            self._vehList.append(self._Vehicle)
        if name == 'emissions':
            self._Vehicle.fuel_consumption = float(attrs['fuel_abs'])
            self._Vehicle.co = float(attrs['CO_abs'])
            self._Vehicle.co2 = float(attrs['CO2_abs'])
            self._Vehicle.hc = float(attrs['HC_abs'])
            self._Vehicle.pmx = float(attrs['PMx_abs'])
            self._Vehicle.nox = float(attrs['NOx_abs'])


class VehRoutesReader(handler.ContentHandler):

    def __init__(self):
        self._routes = []
        self._depart = 0
        self._arrival = 0

    def startElement(self, name, attrs):
        if name == 'vehicle':
            self._depart = int(attrs['depart'])
            self._arrival = int(attrs['arrival'])
        if name == 'route':
            self._routes.append((attrs['edges'], self._arrival - self._depart))


def getAvgNrLanesPerStreet(netfile):
    net = sumolib.net.readNet(netfile)
    nrLanes = 0
    for edge in net._edges:
        nrLanes += len(edge._lanes)

    return 1.0 * nrLanes / len(net._edges)


def getRouteDistributions(vehroutesFile):
    parser = make_parser()
    viReader = VehRoutesReader()
    parser.setContentHandler(viReader)
    parser.parse(vehroutesFile)

    routes = viReader._routes

    startEnd = {}
    for route in routes:
        routeSplit = route[0].split(' ')
        if ((routeSplit[0], routeSplit[-1]) not in startEnd):
            startEnd[routeSplit[0], routeSplit[-1]] = {}
        if (route[0] not in startEnd[routeSplit[0], routeSplit[-1]]):
            startEnd[routeSplit[0], routeSplit[-1]][route[0]] = [0, 0]
        startEnd[routeSplit[0], routeSplit[-1]][route[0]][0] += 1
        startEnd[routeSplit[0], routeSplit[-1]][route[0]][1] += route[1]
    return startEnd


def getBasicStats(vehicles):
    totalVeh = 0.
    aTravelTime = []
    aTravelLength = []
    aTravelSpeed = []
    aWaitTime = []
    aDepartDelay = []
    aFuelConsumption = []
    aCO = []
    aCO2 = []
    aHC = []
    aPMx = []
    aNOx = []

    for veh in vehicles:
        totalVeh += 1
        # unit: speed - m/s; traveltime - s; travel length - m
        veh.speed = veh.travellength / veh.traveltime
        aTravelTime.append(veh.traveltime)
        aTravelLength.append(veh.travellength)
        aWaitTime.append(veh.waittime)
        aTravelSpeed.append(veh.speed)
        aDepartDelay.append(veh.departdelay)
        aFuelConsumption.append(veh.fuel_consumption)
        aCO.append(veh.co)
        aCO2.append(veh.co2)
        aHC.append(veh.hc)
        aPMx.append(veh.pmx)
        aNOx.append(veh.nox)

    assignments = {}
    assignments['totalVeh'] = totalVeh
    assignments['totalTravelTime'] = sum(aTravelTime)
    assignments['totalTravelLength'] = sum(aTravelLength)
    assignments['totalDepartDelay'] = sum(aDepartDelay)
    assignments['totalWaitTime'] = sum(aWaitTime)
    assignments['totalFuelConsumption'] = sum(aFuelConsumption)
    assignments['totalCO'] = sum(aCO)
    assignments['totalCO2'] = sum(aCO2)
    assignments['totalHC'] = sum(aHC)
    assignments['totalPMx'] = sum(aPMx)
    assignments['totalNOx'] = sum(aNOx)

    assignments['avgTravelTime'] = numpy.mean(aTravelTime)
    assignments['avgTravelLength'] = numpy.mean(aTravelLength)
    assignments['avgTravelSpeed'] = numpy.mean(aTravelSpeed)
    assignments['avgDepartDelay'] = numpy.mean(aDepartDelay)
    assignments['avgWaitTime'] = numpy.mean(aWaitTime)
    assignments['avgFuelConsumption'] = numpy.mean(aFuelConsumption)
    assignments['avgCO'] = numpy.mean(aCO)
    assignments['avgCO2'] = numpy.mean(aCO2)
    assignments['avgHC'] = numpy.mean(aHC)
    assignments['avgPMx'] = numpy.mean(aPMx)
    assignments['avgNOx'] = numpy.mean(aNOx)

    assignments['SDTravelTime'] = numpy.std(aTravelTime)
    assignments['SDLength'] = numpy.std(aTravelLength)
    assignments['SDSpeed'] = numpy.std(aTravelSpeed)
    assignments['SDWaitTime'] = numpy.std(aWaitTime)
    assignments['SDFuelConsumption'] = numpy.std(aFuelConsumption)
    assignments['SDCO'] = numpy.std(aCO)
    assignments['SDCO2'] = numpy.std(aCO2)
    assignments['SDHC'] = numpy.std(aHC)
    assignments['SDPMx'] = numpy.std(aPMx)
    assignments['SDNOx'] = numpy.std(aNOx)

    assignments['quartil25TravelTime'] = quantil(aTravelTime, 0.25)
    assignments['quartil25Length'] = quantil(aTravelLength, 0.25)
    assignments['quartil25Speed'] = quantil(aTravelSpeed, 0.25)
    assignments['quartil25WaitTime'] = quantil(aWaitTime, 0.25)
    assignments['quartil25FuelConsumption'] = quantil(aFuelConsumption, 0.25)
    assignments['quartil25CO'] = quantil(aCO, 0.25)
    assignments['quartil25CO2'] = quantil(aCO2, 0.25)
    assignments['quartil25HC'] = quantil(aHC, 0.25)
    assignments['quartil25PMx'] = quantil(aPMx, 0.25)
    assignments['quartil25NOx'] = quantil(aNOx, 0.25)

    assignments['quartil75TravelTime'] = quantil(aTravelTime, 0.75)
    assignments['quartil75Length'] = quantil(aTravelLength, 0.75)
    assignments['quartil75Speed'] = quantil(aTravelSpeed, 0.75)
    assignments['quartil75WaitTime'] = quantil(aWaitTime, 0.75)
    assignments['quartil75FuelConsumption'] = quantil(aFuelConsumption, 0.75)
    assignments['quartil75CO'] = quantil(aCO, 0.75)
    assignments['quartil75CO2'] = quantil(aCO2, 0.75)
    assignments['quartil75HC'] = quantil(aHC, 0.75)
    assignments['quartil75PMx'] = quantil(aPMx, 0.75)
    assignments['quartil75NOx'] = quantil(aNOx, 0.75)

    assignments['medianTravelTime'] = quantil(aTravelTime, 0.5)
    assignments['medianLength'] = quantil(aTravelLength, 0.5)
    assignments['medianSpeed'] = quantil(aTravelSpeed, 0.5)
    assignments['medianWaitTime'] = quantil(aWaitTime, 0.5)
    assignments['medianFuelConsumption'] = quantil(aFuelConsumption, 0.5)
    assignments['medianCO'] = quantil(aCO, 0.5)
    assignments['medianCO2'] = quantil(aCO2, 0.5)
    assignments['medianHC'] = quantil(aHC, 0.5)
    assignments['medianPMx'] = quantil(aPMx, 0.5)
    assignments['medianNOx'] = quantil(aNOx, 0.5)

    return assignments


# output the network statistics based on the sumo-simulation results
def writeStatistics(assignments, out):
    out.write('mean number of lanes per street : %f\n\n' %
              assignments['avgNrLanes'])

    out.write('total number of vehicles : %f\n\n' % assignments['totalVeh'])

    out.write('travel time\n')
    out.write('===========\n')
    out.write('total       : %f\n' % assignments['totalTravelTime'])
    out.write('mean        : %f\n' % assignments['avgTravelTime'])
    out.write('std         : %f\n' % assignments['SDTravelTime'])
    out.write('1/4-quantil : %f\n' % assignments['quartil25TravelTime'])
    out.write('median      : %f\n' % assignments['medianTravelTime'])
    out.write('3/4-quantil : %f\n\n' % assignments['quartil75TravelTime'])

    out.write('travel speed\n')
    out.write('============\n')
    out.write('mean        : %f\n' % assignments['avgTravelSpeed'])
    out.write('std         : %f\n' % assignments['SDSpeed'])
    out.write('1/4-quantil : %f\n' % assignments['quartil25Speed'])
    out.write('median      : %f\n' % assignments['medianSpeed'])
    out.write('3/4-quantil : %f\n\n' % assignments['quartil75Speed'])

    out.write('waiting time\n')
    out.write('============\n')
    out.write('total       : %f\n' % assignments['totalWaitTime'])
    out.write('mean        : %f\n' % assignments['avgWaitTime'])
    out.write('std         : %f\n' % assignments['SDWaitTime'])
    out.write('1/4-quantil : %f\n' % assignments['quartil25WaitTime'])
    out.write('median      : %f\n' % assignments['medianWaitTime'])
    out.write('3/4-quantil : %f\n\n' % assignments['quartil75WaitTime'])

    out.write('distance travelled\n')
    out.write('==================\n')
    out.write('total       : %f\n' % assignments['totalTravelLength'])
    out.write('mean        : %f\n' % assignments['avgTravelLength'])
    out.write('std         : %f\n' % assignments['SDLength'])
    out.write('1/4-quantil : %f\n' % assignments['quartil25Length'])
    out.write('median      : %f\n' % assignments['medianLength'])
    out.write('3/4-quantil : %f\n\n' % assignments['quartil75Length'])

    out.write('fuel consumption\n')
    out.write('================\n')
    out.write('total       : %f\n' % assignments['totalFuelConsumption'])
    out.write('mean        : %f\n' % assignments['avgFuelConsumption'])
    out.write('std         : %f\n' % assignments['SDFuelConsumption'])
    out.write('1/4-quantil : %f\n' % assignments['quartil25FuelConsumption'])
    out.write('median      : %f\n' % assignments['medianFuelConsumption'])
    out.write('3/4-quantil : %f\n\n' % assignments['quartil75FuelConsumption'])

    out.write('CO emissions\n')
    out.write('============\n')
    out.write('total       : %f\n' % assignments['totalCO'])
    out.write('mean        : %f\n' % assignments['avgCO'])
    out.write('std         : %f\n' % assignments['SDCO'])
    out.write('1/4-quantil : %f\n' % assignments['quartil25CO'])
    out.write('median      : %f\n' % assignments['medianCO'])
    out.write('3/4-quantil : %f\n\n' % assignments['quartil75CO'])

    out.write('CO2 emissions\n')
    out.write('=============\n')
    out.write('total       : %f\n' % assignments['totalCO2'])
    out.write('mean        : %f\n' % assignments['avgCO2'])
    out.write('std         : %f\n' % assignments['SDCO2'])
    out.write('1/4-quantil : %f\n' % assignments['quartil25CO2'])
    out.write('median      : %f\n' % assignments['medianCO2'])
    out.write('3/4-quantil : %f\n\n' % assignments['quartil75CO2'])

    out.write('HC emissions\n')
    out.write('============\n')
    out.write('total       : %f\n' % assignments['totalHC'])
    out.write('mean        : %f\n' % assignments['avgHC'])
    out.write('std         : %f\n' % assignments['SDHC'])
    out.write('1/4-quantil : %f\n' % assignments['quartil25HC'])
    out.write('median      : %f\n' % assignments['medianHC'])
    out.write('3/4-quantil : %f\n\n' % assignments['quartil75HC'])

    out.write('PMx emissions\n')
    out.write('=============\n')
    out.write('total       : %f\n' % assignments['totalPMx'])
    out.write('mean        : %f\n' % assignments['avgPMx'])
    out.write('std         : %f\n' % assignments['SDPMx'])
    out.write('1/4-quantil : %f\n' % assignments['quartil25PMx'])
    out.write('median      : %f\n' % assignments['medianPMx'])
    out.write('3/4-quantil : %f\n\n' % assignments['quartil75PMx'])

    out.write('NOx emissions\n')
    out.write('=============\n')
    out.write('total       : %f\n' % assignments['totalNOx'])
    out.write('mean        : %f\n' % assignments['avgNOx'])
    out.write('std         : %f\n' % assignments['SDNOx'])
    out.write('1/4-quantil : %f\n' % assignments['quartil25NOx'])
    out.write('median      : %f\n' % assignments['medianNOx'])
    out.write('3/4-quantil : %f\n\n' % assignments['quartil75NOx'])


# output the network statistics based on the sumo-simulation results
def getStatisticsOutput(assignments, outputfile):
    foutveh = open(outputfile, 'w')
    writeStatistics(assignments, foutveh)
    foutveh.write('\n\nRoute distribution:\n')
    foutveh.write('==================:\n')
    startEnd = assignment['routeDistr']
    for se in startEnd:
        for r in startEnd[se]:
            foutveh.write('%s: Number of vehicles: %d, Avg Travel Time: %d\n' % (
                r, startEnd[se][r][0], 1.0 * startEnd[se][r][1] / startEnd[se][r][0]))
        foutveh.write('\n')
    foutveh.close()
    writeStatistics(assignments, sys.stdout)


def getCSVOutput(assignments, path, veh_types, interval):
    f_mean_travel_time = open(
        os.path.join(options.path, 'mean_travel_time.csv'), 'w')
    f_mean_speed = open(os.path.join(options.path, 'mean_speed.csv'), 'w')
    f_mean_waiting_time = open(
        os.path.join(options.path, 'mean_waiting_time.csv'), 'w')
    f_mean_distance_travelled = open(
        os.path.join(options.path, 'mean_distance_travelled.csv'), 'w')
    f_mean_fuel_consumption = open(
        os.path.join(options.path, 'mean_fuel_consumption.csv'), 'w')
    f_mean_CO_emissions = open(
        os.path.join(options.path, 'mean_CO_emissions.csv'), 'w')
    f_mean_CO2_emissions = open(
        os.path.join(options.path, 'mean_CO2_emissions.csv'), 'w')
    f_mean_HC_emissions = open(
        os.path.join(options.path, 'mean_HC_emissions.csv'), 'w')
    f_mean_PMx_emissions = open(
        os.path.join(options.path, 'mean_PMx_emissions.csv'), 'w')
    f_mean_NOx_emissions = open(
        os.path.join(options.path, 'mean_NOx_emissions.csv'), 'w')
    f_abs_travel_time = open(
        os.path.join(options.path, 'abs_travel_time.csv'), 'w')
    f_abs_waiting_time = open(
        os.path.join(options.path, 'abs_waiting_time.csv'), 'w')
    f_abs_distance_travelled = open(
        os.path.join(options.path, 'abs_distance_travelled.csv'), 'w')
    f_abs_fuel_consumption = open(
        os.path.join(options.path, 'abs_fuel_consumption.csv'), 'w')
    f_abs_CO_emissions = open(
        os.path.join(options.path, 'abs_CO_emissions.csv'), 'w')
    f_abs_CO2_emissions = open(
        os.path.join(options.path, 'abs_CO2_emissions.csv'), 'w')
    f_abs_HC_emissions = open(
        os.path.join(options.path, 'abs_HC_emissions.csv'), 'w')
    f_abs_PMx_emissions = open(
        os.path.join(options.path, 'abs_PMx_emissions.csv'), 'w')
    f_abs_NOx_emissions = open(
        os.path.join(options.path, 'abs_NOx_emissions.csv'), 'w')

    files = []
    files.append(f_mean_travel_time)
    files.append(f_mean_speed)
    files.append(f_mean_waiting_time)
    files.append(f_mean_distance_travelled)
    files.append(f_mean_fuel_consumption)
    files.append(f_mean_CO_emissions)
    files.append(f_mean_CO2_emissions)
    files.append(f_mean_HC_emissions)
    files.append(f_mean_PMx_emissions)
    files.append(f_mean_NOx_emissions)
    files.append(f_abs_travel_time)
    files.append(f_abs_waiting_time)
    files.append(f_abs_distance_travelled)
    files.append(f_abs_fuel_consumption)
    files.append(f_abs_CO_emissions)
    files.append(f_abs_CO2_emissions)
    files.append(f_abs_HC_emissions)
    files.append(f_abs_PMx_emissions)
    files.append(f_abs_NOx_emissions)

    for f in files:
        f.write(';')

    for veh_type in veh_types:
        head = veh_type + '(mean);' + veh_type + '(std);' + veh_type + \
            '(1/4-quantil);' + veh_type + \
            '(median);' + veh_type + '(3/4-quantil);'
        f_mean_travel_time.write(head)
        f_mean_speed.write(head)
        f_mean_waiting_time.write(head)
        f_mean_distance_travelled.write(head)
        f_mean_fuel_consumption.write(head)
        f_mean_CO_emissions.write(head)
        f_mean_CO2_emissions.write(head)
        f_mean_HC_emissions.write(head)
        f_mean_PMx_emissions.write(head)
        f_mean_NOx_emissions.write(head)
        f_abs_travel_time.write(veh_type + ';')
        f_abs_waiting_time.write(veh_type + ';')
        f_abs_distance_travelled.write(veh_type + ';')
        f_abs_fuel_consumption.write(veh_type + ';')
        f_abs_CO_emissions.write(veh_type + ';')
        f_abs_CO2_emissions.write(veh_type + ';')
        f_abs_HC_emissions.write(veh_type + ';')
        f_abs_PMx_emissions.write(veh_type + ';')
        f_abs_NOx_emissions.write(veh_type + ';')

    for f in files:
        f.write('\n')

    t = 0
    while t in assignments:
        for f in files:
            f.write('[' + str(t) + ':' + str(t + interval - 1) + '];')
        for veh_type in assignments[t].itervalues():
            f_mean_travel_time.write(str(veh_type['avgTravelTime']) + ";" + str(veh_type['SDTravelTime']) + ";" +
                                     str(veh_type['quartil25TravelTime']) + ";" + str(veh_type['medianTravelTime']) +
                                     ";" + str(veh_type['quartil75TravelTime']) + ";")
            f_mean_speed.write(str(veh_type['avgTravelSpeed']) + ";" + str(veh_type['SDSpeed']) + ";" +
                               str(veh_type['quartil25Speed']) + ";" + str(veh_type['medianSpeed']) + ";" +
                               str(veh_type['quartil75Speed']) + ";")
            f_mean_waiting_time.write(str(veh_type['avgWaitTime']) + ";" + str(veh_type['SDWaitTime']) + ";" +
                                      str(veh_type['quartil25WaitTime']) + ";" + str(veh_type['medianWaitTime']) +
                                      ";" + str(veh_type['quartil75WaitTime']) + ";")
            f_mean_distance_travelled.write(str(veh_type['avgTravelLength']) + ";" + str(veh_type['SDLength']) + ";" +
                                            str(veh_type['quartil25Length']) + ";" + str(veh_type['medianLength']) +
                                            ";" + str(veh_type['quartil75Length']) + ";")
            f_mean_fuel_consumption.write(str(veh_type['avgFuelConsumption']) + ";" +
                                          str(veh_type['SDFuelConsumption']) + ";" +
                                          str(veh_type['quartil25FuelConsumption']) + ";" +
                                          str(veh_type['medianFuelConsumption']) + ";" +
                                          str(veh_type['quartil75FuelConsumption']) + ";")
            f_mean_CO_emissions.write(str(veh_type['avgCO']) + ";" + str(veh_type['SDCO']) + ";" + str(
                veh_type['quartil25CO']) + ";" + str(veh_type['medianCO']) + ";" + str(veh_type['quartil75CO']) + ";")
            f_mean_CO2_emissions.write(str(veh_type['avgCO2']) + ";" + str(veh_type['SDCO2']) + ";" + str(
                veh_type['quartil25CO2']) + ";" + str(veh_type['medianCO2']) + ";" + str(
                veh_type['quartil75CO2']) + ";")
            f_mean_HC_emissions.write(str(veh_type['avgHC']) + ";" + str(veh_type['SDHC']) + ";" + str(
                veh_type['quartil25HC']) + ";" + str(veh_type['medianHC']) + ";" + str(veh_type['quartil75HC']) + ";")
            f_mean_PMx_emissions.write(str(veh_type['avgPMx']) + ";" + str(veh_type['SDPMx']) + ";" + str(
                veh_type['quartil25PMx']) + ";" + str(veh_type['medianPMx']) + ";" + str(
                veh_type['quartil75PMx']) + ";")
            f_mean_NOx_emissions.write(str(veh_type['avgNOx']) + ";" + str(veh_type['SDNOx']) + ";" + str(
                veh_type['quartil25NOx']) + ";" + str(veh_type['medianNOx']) + ";" + str(
                veh_type['quartil75NOx']) + ";")
            f_abs_travel_time.write(str(veh_type['totalTravelTime']) + ";")
            f_abs_waiting_time.write(str(veh_type['totalWaitTime']) + ";")
            f_abs_distance_travelled.write(
                str(veh_type['totalTravelLength']) + ";")
            f_abs_fuel_consumption.write(
                str(veh_type['totalFuelConsumption']) + ";")
            f_abs_CO_emissions.write(str(veh_type['totalCO']) + ";")
            f_abs_CO2_emissions.write(str(veh_type['totalCO2']) + ";")
            f_abs_HC_emissions.write(str(veh_type['totalHC']) + ";")
            f_abs_PMx_emissions.write(str(veh_type['totalPMx']) + ";")
            f_abs_NOx_emissions.write(str(veh_type['totalNOx']) + ";")
        for f in files:
            f.write('\n')
        t += interval

    for f in files:
        f.close()


# initialise
optParser = OptionParser()
optParser.add_option("-n", "--netfile", dest="netfile",
                     help="name of the netfile (f.e. 'inputs\\pasubio\\a_costa.net.xml')",
                     metavar="<FILE>", type="string")
optParser.add_option("-p", "--path", dest="path",
                     help="name of folder to work with (f.e. 'outputs\\a_costa\\')", metavar="<FOLDER>", type="string")
optParser.add_option("-t", "--vehicle-types", dest="vehicle_types",
                     help="vehicle-types for which the values shall be generated",
                     metavar="<VEHICLE_TYPE>[,<VEHICLE_TYPE>]", type="string")
optParser.add_option("-i", "--intervals", dest="interval",
                     help="intervals to be generated ([0:<TIME>-1], [<TIME>:2*<TIME>-1], ...)",
                     metavar="<TIME>", type="int")

optParser.set_usage('\ngenerateITetrisNetworkMetrics.py -n inputs\\a_costa\\acosta.net.xml -p outputs\\a_costa\\ -t ' +
                    'passenger2a,passenger5,passenger1,passenger2b,passenger3,passenger4 -i 500 \n' +
                    'generateITetrisNetworkMetrics.py -n inputs\\a_costa\\acosta.net.xml -p outputs\\a_costa\\ ' +
                    '-i 500\n' + 'generateITetrisNetworkMetrics.py -n inputs\\a_costa\\acosta.net.xml ' +
                    '-p outputs\\a_costa\\')
# parse options
(options, args) = optParser.parse_args()
if not options.netfile or not options.path:
    print("Missing arguments")
    optParser.print_help()
    exit()

interval = options.interval
netfile = options.netfile
vehroutefile = os.path.join(options.path, 'vehroutes.xml')

if interval is None:
    interval = 10000000


vehicles = []
parser = make_parser()
parser.setContentHandler(VehInformationReader(vehicles))
parser.parse(os.path.join(options.path, 'tripinfos.xml'))

if options.vehicle_types is None:
    vehicle_types = ['all']
else:
    vehicle_types = options.vehicle_types.split(',')

vehicles_of_type_interval = {}
for veh in vehicles:
    t = int(veh.depart / interval) * interval
    if (t not in vehicles_of_type_interval):
        vehicles_of_type_interval[t] = {}
        for veh_type in vehicle_types:
            vehicles_of_type_interval[t][veh_type] = []
    if vehicle_types[0] == 'all':
        vehicles_of_type_interval[t]['all'].append(veh)
    else:
        if (veh.vtype in vehicle_types):
            vehicles_of_type_interval[t][veh.vtype].append(veh)
assignments = {}
for t in vehicles_of_type_interval.keys():
    assignments[t] = {}
    for veh_type in vehicle_types:
        assignments[t][veh_type] = getBasicStats(
            vehicles_of_type_interval[t][veh_type])

getCSVOutput(assignments, options.path, vehicle_types, interval)

assignment = getBasicStats(vehicles)
assignment['avgNrLanes'] = getAvgNrLanesPerStreet(netfile)
assignment['routeDistr'] = getRouteDistributions(vehroutefile)
getStatisticsOutput(
    assignment, os.path.join(options.path, "network_metrics_summary.txt"))

print('The calculation is done!')
