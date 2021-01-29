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

# @file    visum_convertRoutes.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2007-02-21


from __future__ import print_function
from __future__ import absolute_import
import os
import sys
import random
from optparse import OptionParser
sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..'))
import sumolib  # noqa


class Statistics:
    found = 0
    foundN = 0
    missing = 0
    missingN = 0


stats = Statistics()
routes = []


def addRouteChecking(route, id, count, ok):
    route = route.strip()
    if route != "":
        if ok:
            if options.distribution and routes:
                distID = routes[0][0][:routes[0][0].rfind("_")]
                if distID != id[:id.rfind("_")]:
                    sum = 0
                    r_max = (None, 0, None)
                    for r in routes:
                        sum += r[1]
                        if r_max[1] < r[1]:
                            r_max = r
                    if sum < options.cutoff:
                        del routes[:]
                        routes.append((r_max[0], sum, r_max[2]))
                    if len(routes) > 1:
                        fdo.write('    <routeDistribution id="%s">\n' % distID)
                    else:
                        routes[0] = (distID, routes[0][1], routes[0][2])
                    for r in routes:
                        if net2:
                            trace = []
                            for e in route.split():
                                edge = net.getEdge(e)
                                numSteps = int(edge.getLength() / options.step)
                                for p in range(numSteps):
                                    trace.append(
                                        sumolib.geomhelper.positionAtShapeOffset(edge.getShape(), p * options.step))
                            path = sumolib.route.mapTrace(
                                trace, net2, options.delta)
                            r = (r[0], r[1], " ".join(path))
                        fdo.write(
                            '        <route id="%s" probability="%s" edges="%s"/>\n' % r)
                    if len(routes) > 1:
                        fdo.write('    </routeDistribution>\n')
                    del routes[:]
            routes.append((id, count, route))
            stats.found += 1
            stats.foundN += count
        else:
            stats.missing += 1
            stats.missingN += count


def sorter(idx):
    def t(i, j):
        if i[idx] < j[idx]:
            return -1
        elif i[idx] > j[idx]:
            return 1
        else:
            return 0


# initialise
optParser = OptionParser()
optParser.add_option("-n", "--net-file", dest="netfile",
                     help="SUMO net file to work with", type="string")
optParser.add_option("-r", "--visum-routes", dest="routes",
                     help="The VISUM-routes files to parse", type="string")
optParser.add_option("-o", "--output",
                     help="Name of the file to write", type="string")
optParser.add_option("-b", "--begin",
                     help="The begin time of the routes to generate", type="int", default=0)
optParser.add_option("-e", "--end",
                     help="The end time (+1) of the routes to generate", type="int", default=3600)
optParser.add_option("-p", "--prefix",
                     help="ID prefix to use", type="string")
optParser.add_option("-t", "--type",
                     help="The type to use for vehicles", type="string")
optParser.add_option("-u", "--uniform",
                     help="Whether departures shall be distributed uniform in each interval", action="store_true",
                     default=False)
optParser.add_option("-l", "--timeline",
                     help="Percentages over a day", type="string")
optParser.add_option("-a", "--tabs", action="store_true",
                     default=False, help="tab separated route file")
optParser.add_option("-v", "--verbose", action="store_true",
                     default=False, help="tell me what you are doing")
optParser.add_option("-2", "--net2",
                     help="immediately match routes to a second network", metavar="FILE")
optParser.add_option("-s", "--step", default="10",
                     type="float", help="distance between successive trace points")
optParser.add_option("-d", "--delta", default="1",
                     type="float", help="maximum distance between edge and trace points when matching " +
                                        " to the second net")
optParser.add_option("-i", "--distribution", action="store_true",
                     default=False, help="write route distributions only")
optParser.add_option("-c", "--cutoff",
                     help="Keep only one route when less than CUTOFF vehicles drive the OD", type="int",
                     default=0)

optParser.set_usage(
    '\nvisum_convertRoutes.py -n visum.net.xml -r visum_routes.att -o visum.rou.xml')
# parse options
(options, args) = optParser.parse_args()
if not options.netfile or not options.routes or not options.output:
    print("Missing arguments")
    optParser.print_help()
    exit()

if options.verbose:
    print("Reading net...")
net = sumolib.net.readNet(options.netfile)
net2 = None
if options.net2:
    net.move(-net.getLocationOffset()[0], -net.getLocationOffset()[1])
    net2 = sumolib.net.readNet(options.net2)
    net2.move(-net2.getLocationOffset()[0], -net2.getLocationOffset()[1])

# initialise nodes/edge map
emap = {}
for e in net._edges:
    if e._from._id not in emap:
        emap[e._from._id] = {}
    if e._to._id not in emap[e._from._id]:
        emap[e._from._id][e._to._id] = e._id

# fill with read values
if options.verbose:
    print("Reading routes...")
separator = "\t" if options.tabs else ";"
parse = False
ok = True
route = ""
id = ""
attributes = []
count = 0
no = 0
fd = open(options.routes)
fdo = open(options.output, "w")
fdo.write("<routes>\n")
for idx, line in enumerate(fd):
    if options.verbose and idx % 10000 == 0:
        sys.stdout.write("%s lines read\r" % "{:,}".format(idx))
    if line.find("$") == 0 or line.find("*") == 0 or line.find(separator) < 0:
        parse = False
        addRouteChecking(route, id, count, ok)
    if parse:
        values = line.strip('\n\r').split(separator)
        amap = {}
        for i in range(0, len(attributes)):
            amap[attributes[i]] = values[i]
        if amap["origzoneno"] != "":
            # route begin (not the route)
            addRouteChecking(route, id, count, ok)
            id = amap["origzoneno"] + "_" + \
                amap["destzoneno"] + "_" + amap["pathindex"]
            count = float(amap["prtpath\\vol(ap)"])
            route = " "
            ok = True
        else:
            if not ok:
                continue
            fromnode = amap["fromnodeno"]
            tonode = amap["tonodeno"]
            link = amap["linkno"]
            if fromnode not in emap:
                if no != 0:
                    print("Missing from-node '" + fromnode + "'; skipping")
                ok = False
                continue
            if tonode not in emap[fromnode]:
                if no != 0:
                    print("No connection between from-node '" +
                          fromnode + "' and to-node '" + tonode + "'; skipping")
                ok = False
                continue
            edge = emap[fromnode][tonode]
            if link != edge and link != edge[1:]:
                if no != 0:
                    print("Mismatching edge '" + link + "' (from '" +
                          fromnode + "', to '" + tonode + "'); skipping")
                ok = False
                continue
            route = route + edge + " "

    if line.find("$PRTPATHLINK:") == 0 or line.find("$IVTEILWEG:") == 0:
        attributes = line[line.find(":") + 1:].strip().lower().split(separator)
        for i in range(0, len(attributes)):
            if attributes[i] == "qbeznr":
                attributes[i] = "origzoneno"
            if attributes[i] == "zbeznr":
                attributes[i] = "destzoneno"
            if attributes[i] == "iv-weg\\bel(ap)":
                attributes[i] = "prtpath\\vol(ap)"
            if attributes[i] == "wegind":
                attributes[i] = "pathindex"
            if attributes[i] == "vonknotnr":
                attributes[i] = "fromnodeno"
            if attributes[i] == "nachknotnr":
                attributes[i] = "tonodeno"
            if attributes[i] == "strnr":
                attributes[i] = "linkno"
        parse = True

addRouteChecking(route, id, count, ok)
fd.close()

if options.verbose:
    print(" %s routes found (%s vehs)" % (stats.found, stats.foundN))
    print(" %s routes missing (%s vehs)" % (stats.missing, stats.missingN))

if options.distribution:
    if routes:
        distID = routes[0][0][:routes[0][0].rfind("_")]
        fdo.write('    <routeDistribution id="%s">\n' % distID)
        for r in routes:
            fdo.write(
                '        <route id="%s" probability="%s" edges="%s"/>\n' % r)
        fdo.write('    </routeDistribution>\n')
    fdo.write("</routes>\n")
    fdo.close()
    exit()

timeline = None
# apply timeline
if options.timeline:
    timeline = []
    nNo = 0
    vals = options.timeline.split(",")
    sum = 0
    for v in vals:
        timeline.append(float(v))
        sum += float(v)
    if len(timeline) != 24:
        print("The timeline must have 24 entries")
        sys.exit()
    nRoutes = []

# convert to vehicles
if options.verbose:
    print("Generating vehicles...")
emissions = []
begin = options.begin
end = options.end

if not timeline:
    for r in routes:
        for i in range(0, int(r[1])):
            if options.uniform:
                t = float(begin) + float(end - begin) / float(r[1]) * float(i)
            else:
                t = float(begin) + float(end - begin) * random.random()
            emissions.append((int(t), r[0] + "__" + str(i), r[2]))
else:
    for r in routes:
        left = 0.
        tbeg = 0
        j = 0
        for t in timeline:
            fno = (float(r[1]) + left) * t / 100.
            no = int(fno)
            left += fno - no
            if left >= 1.:
                left -= 1
                no += 1
            for i in range(0, no):
                if options.uniform:
                    t = tbeg + float(3600) / float(r[1]) * float(i)
                else:
                    t = tbeg + float(3600) * random.random()
                emissions.append((int(t), r[0] + "__" + str(j), r[2]))
                j = j + 1
            nNo += no
            tbeg += 3600
    if options.verbose:
        print(" %s vehicles after applying timeline" % nNo)


# sort emissions
if options.verbose:
    print("Sorting routes...")
emissions.sort(sorter(0))

# save emissions
if options.verbose:
    print("Writing routes...")
for emission in emissions:
    fdo.write('    <vehicle id="')
    if options.prefix:
        fdo.write(options.prefix + "_")
    fdo.write(emission[1] + '" depart="' + str(emission[0]) + '"')
    if options.type:
        fdo.write(' type="' + options.type + '"')
    fdo.write('><route edges="' + emission[2] + '"/></vehicle>\n')
fdo.write("</routes>\n")
fdo.close()
if options.verbose:
    print(" %s vehicles written" % len(emissions))
