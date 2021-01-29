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

# @file    statisticsElements.py
# @author  Yun-Pang Floetteroed
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2008-07-10

"""
This script is to define classes, functions, parse-reader and output files/format for
- calculating network performances
- conducting significance tests (either t-Test or Kruskal-Wallis-Test) and
- writing results into files.
"""
from __future__ import absolute_import

from xml.sax import handler


# This class is for storing vehicle information, such as departure time,
# route and travel time.
class Vehicle:

    def __init__(self, label):
        self.label = label
        self.method = None
        self.depart = 0.
        self.arrival = 0.
        self.speed = 0.
        self.route = []
        self.traveltime = 0.
        self.travellength = 0.
        self.departdelay = 0.
        self.waittime = 0.
        self.rank = 0.

    def __repr__(self):
        return "%s_%s_%s_%s_%s_%s<%s>" % (self.label, self.depart, self.arrival,
                                          self.speed, self.traveltime, self.travellength,
                                          self.route)

# This class is used in the significance test.


class Assign:

    def __init__(self, method, totalVeh, totalTravelTime, totalTravelLength, totalDepartDelay,
                 totalWaitTime, avgTravelTime, avgTravelLength, avgTravelSpeed, avgDepartDelay,
                 avgWaitTime, SDTravelTime, SDLength, SDSpeed, SDWaitTime):
        self.label = method
        self.totalVeh = totalVeh
        self.totalTravelTime = totalTravelTime
        self.totalTravelLength = totalTravelLength
        self.totalDepartDelay = totalDepartDelay
        self.totalWaitTime = totalWaitTime
        self.avgTravelTime = avgTravelTime
        self.avgTravelLength = avgTravelLength
        self.avgTravelSpeed = avgTravelSpeed
        self.avgDepartDelay = avgDepartDelay
        self.avgWaitTime = avgWaitTime
        self.SDTravelTime = SDTravelTime
        self.SDLength = SDLength
        self.SDSpeed = SDSpeed
        self.SDWaitTime = SDWaitTime
        self.sumrank = 0.

    def __repr__(self):
        return "%s_<%s|%s|%s|%s|%s|%s|%s|%s|%s>" % (self.label, self.totalVeh, self.avgTravelTime,
                                                    self.avgTravelLength, self.avgTravelSpeed,
                                                    self.avgWaitTime, self.SDTravelTime, self.SDLength,
                                                    self.SDSpeed, self.SDWaitTime)

# This cloass is used for the t test in the significance test.


class T_Value:

    def __init__(self, avgtraveltime, avgtravelspeed, avgtravellength, avgwaittime, lowvalue, highvalue):
        self.avgtraveltime = avgtraveltime
        self.avgtravelspeed = avgtravelspeed
        self.avgtravellength = avgtravellength
        self.avgwaittime = avgwaittime
        self.lowtvalue = lowvalue
        self.hightvalue = highvalue

    def __repr__(self):
        return "%<%s|%s|%s|%s>" % (self.avgtraveltime, self.avgtravelspeed, self.avgtravellength, self.avgwaittime)

# This class is used for the Kruskal-Wallis test in the significance test.


class H_Value:

    def __init__(self, label, lowvalue, highvalue):
        self.label = label
        self.traveltime = 0.
        self.travelspeed = 0.
        self.travellength = 0.
        self.waittime = 0.
        # 95% confidence interval
        self.lowchivalue = lowvalue
        # 99% confidence interval
        self.highchivalue = highvalue

    def __repr__(self):
        return "%<%s|%s|%s|%s|%s|%s>" % (self.traveltime, self.travelspeed, self.travellength, self.waittime,
                                         self.lowchivalue, self.highchivalue)

# The class is for parsing the XML input file (vehicle information). This class is used in the networkStatistics.py for
# calculating the gloabal network performances, e.g. avg. travel time and
# avg. travel speed.


class VehInformationReader(handler.ContentHandler):

    def __init__(self, vehList):
        self._vehList = vehList
        self._Vehicle = None
        self._routeString = ''

    def startElement(self, name, attrs):
        if name == 'tripinfo':
            self._Vehicle = Vehicle(attrs['id'])
            self._Vehicle.traveltime = float(attrs['duration'])
            self._Vehicle.travellength = float(attrs['routeLength'])
            self._Vehicle.departdelay = float(attrs['departDelay'])
            self._Vehicle.waittime = float(
                attrs['departDelay']) + float(attrs['waitingTime'])
            self._vehList.append(self._Vehicle)

# output the network statistics based on the sumo-simulation results


def getStatisticsOutput(assignments, outputfile):
    foutveh = open(outputfile, 'w')
    foutveh.write(
        'average vehicular travel time(s) = the sum of all vehicular travel times / the number of vehicles\n')
    foutveh.write(
        'average vehicular travel length(m) = the sum of all vehicular travel lengths / the number of vehicles\n')
    foutveh.write(
        'average vehicular travel speed(m/s) = the sum of all vehicular travel speeds / the number of vehicles\n')
    for method in assignments.values():
        foutveh.write('\nAssignment Method:%s\n' % method.label)
        foutveh.write('- total number of vehicles:%s\n' % method.totalVeh)
        foutveh.write('- total departure delay(s):%s, ' %
                      method.totalDepartDelay)
        foutveh.write('- average departure delay(s):%s\n' %
                      method.avgDepartDelay)
        foutveh.write('- total waiting time(s):%s, ' % method.totalWaitTime)
        foutveh.write('- average vehicular waiting time(s):%s\n' %
                      method.avgWaitTime)
        foutveh.write('- total travel time(s):%s, ' % method.totalTravelTime)
        foutveh.write('- average vehicular travel time(s):%s\n' %
                      method.avgTravelTime)
        foutveh.write('- total travel length(m):%s, ' %
                      method.totalTravelLength)
        foutveh.write('- average vehicular travel length(m):%s\n' %
                      method.avgTravelLength)
        foutveh.write('- average vehicular travel speed(m/s):%s\n' %
                      method.avgTravelSpeed)
    foutveh.close()

# output the results of the significance tests according to the
# sumo-simulation results


def getSignificanceTestOutput(assignments, tTest, tValueAvg, hValues, outputfile):
    foutSGtest = open(outputfile, 'w')
    if tTest:
        foutSGtest.write(
            'The significances of the performance averages among the used assignment models ' +
            'are examined with the t test.\n')
        assignlist = list(assignments.itervalues())
        for num, A in enumerate(assignlist):
            for B in assignlist[num + 1:]:
                foutSGtest.write('\nmethod:%s' % A.label)
                foutSGtest.write('\nmethod:%s' % B.label)
                foutSGtest.write(
                    '\n   t-value for the avg. travel time:%s' % tValueAvg[A][B].avgtraveltime)
                foutSGtest.write(
                    '\n   t-value for the avg. travel length:%s' % tValueAvg[A][B].avgtravellength)
                foutSGtest.write(
                    '\n   t-value for the avg.travel speed:%s' % tValueAvg[A][B].avgtravelspeed)
                foutSGtest.write(
                    '\n   t-value for the avg. wait time:%s\n' % tValueAvg[A][B].avgwaittime)
                foutSGtest.write('\n95 t-value:%s' % tValueAvg[A][B].lowtvalue)
                foutSGtest.write('\n99 t-value:%s\n' %
                                 tValueAvg[A][B].hightvalue)

    foutSGtest.write(
        'The significance test among the different assignment methods is also done with the Kruskal-Wallis test.\n')
    for h in hValues:
        foutSGtest.write('\n\nmethods:%s' % h.label)
        foutSGtest.write('\nH_traveltime:%s' % h.traveltime)
        foutSGtest.write('\nH_travelspeed:%s' % h.travelspeed)
        foutSGtest.write('\nH_travellength:%s' % h.travellength)
        foutSGtest.write('\nH_waittime:%s\n' % h.waittime)
        foutSGtest.write('\n95 chi-square value:%s' % h.lowchivalue)
        foutSGtest.write('\n99 chi-square value:%s\n' % h.highchivalue)
    foutSGtest.close()
