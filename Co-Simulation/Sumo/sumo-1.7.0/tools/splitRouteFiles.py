#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    splitRouteFiles.py
# @author  Michael Behrisch
# @date    2007-09-10

"""
Splits a list of route files (e.g. coming from the duarouter)
by start time and by edges which contain detectors from an optional
detector file.
"""
from __future__ import print_function
import os
import sys
import re
import random
import glob
import pickle
import bisect
import ctypes

from xml.sax import make_parser, handler
from collections import defaultdict
from optparse import OptionParser


def getFreeSpace(folder):
    """ Return folder/drive free space (in bytes)
    """
    if os.name == "posix":
        s = os.statvfs(folder)
        return s.f_bfree * s.f_bsize
    else:
        free_bytes = ctypes.c_ulonglong(0)
        ctypes.windll.kernel32.GetDiskFreeSpaceExW(ctypes.c_wchar_p(folder), None, None, ctypes.pointer(free_bytes))
        return free_bytes.value


def checkDirOpen(path, mode='w'):
    dirName = os.path.dirname(path)
    if dirName != '' and not os.path.exists(dirName):
        os.makedirs(dirName)
    return open(path, mode)


class RouteReader(handler.ContentHandler):

    def __init__(self, collectFile, edgeCountFile, pythonEdgeFile, collectAll=False):
        """when parsing, collects all routes with their multiplicities in _routeOccurences.
        when closeAll() is called the edge distributions are created"""
        handler.ContentHandler.__init__(self)
        self._routeOccurences = defaultdict(lambda: 0)  # listOfEdges -> count
        self._vehID = None
        self._routeString = ''
        self._routeDistributions = {}
        self._routeIDs = []
        self._calibRoutes = checkDirOpen(collectFile)
        print("<routes>", file=self._calibRoutes)
        self._edgeCountFile = edgeCountFile
        self._pythonEdgeFile = pythonEdgeFile
        self._collectAll = collectAll

    def addEdge(self, edge):
        dirName = ""
        if len(edge) > 2:
            dirName = edge[:2]
            if edge[0] == "-":
                dirName = edge[1:3]
        fileName = os.path.join(os.path.dirname(self._calibRoutes.name), dirName, edge)
        f = checkDirOpen(fileName)
        f.write('    <routeDistribution id="routedist_%s" ' % edge)
        f.close()
        self._routeDistributions[edge] = fileName

    def startElement(self, name, attrs):
        if name == 'route':
            self._routeString = ''
            if 'edges' in attrs:
                self._routeString = attrs['edges']

    def characters(self, content):
        self._routeString += content

    def endElement(self, name):
        if name == 'route':
            self._routeOccurences[self._routeString] += 1

    def closeAll(self):
        """build edge distributions from self._routeOccurences"""
        # build distributions
        # edge -> (route -> prob)
        edgeCount = defaultdict(lambda: 0)
        routeProbs = defaultdict(dict)
        numRoutesTotal = 0
        for index, (edgeString, count) in enumerate(self._routeOccurences.items()):
            edges = edgeString.split()
            routeID = 'r%s' % index
            numRoutesTotal += count
            print('    <route id="%s" edges="%s"/>' % (routeID, ' '.join(edges)), file=self._calibRoutes)
            for edge in edges:
                edgeCount[edge] += count
                if self._collectAll and edge not in self._routeDistributions:
                    self.addEdge(edge)
                if edge in self._routeDistributions:
                    routeProbs[edge][routeID] = count
        print(("writing distributions for %s routes (%s unique)" % (
            numRoutesTotal, len(self._routeOccurences))))
        # write distributions
        for edge, filename in self._routeDistributions.items():
            if edge in routeProbs:
                with open(filename, 'a') as f:
                    print(('routes="%s" probabilities="%s"/>' % (
                        ' '.join(routeProbs[edge].keys()),
                        ' '.join(map(str, routeProbs[edge].values())))), file=f)
            else:
                print("Warning! No routes for %s." % edge, file=sys.stderr)
                os.remove(filename)
        print("</routes>", file=self._calibRoutes)
        self._calibRoutes.close()

        if self._edgeCountFile:
            edgeCountOut = checkDirOpen(self._edgeCountFile)
            print("<netstats>", file=edgeCountOut)
            print('   <interval begin="0" end="%s">' % (24*3600), file=edgeCountOut)
            for edge, weight in edgeCount.items():
                print('      <edge id="%s" traveltime="%s"/>' % (edge, weight), file=edgeCountOut)
            print('   </interval>', file=edgeCountOut)
            print("</netstats>", file=edgeCountOut)
            edgeCountOut.close()
        if self._pythonEdgeFile:
            pythonOut = checkDirOpen(self._pythonEdgeFile, 'wb')
            pickle.dump(set(edgeCount.keys()), pythonOut)
            pythonOut.close()


def splitFiles(routeFiles, typesFile, routesPrefix, step, verbose, modifyID,
               safactor, sufactor):
    if verbose:
        print("Writing types to file", os.path.basename(typesFile))
        print("... in dir", os.path.dirname(typesFile), "TEXTTEST_IGNORE")
    vtypes = checkDirOpen(typesFile)
    print("""<vtypes>
   <vType id="PKW" accel="2.6" decel="4.5" sigma="0.5" length="7" maxspeed="41.6" color="0,1,0"/>
   <vType id="LKW" accel="2.6" decel="4.5" sigma="0.5" length="15" maxspeed="25" color="1,0,0"/>""", file=vtypes)
    currentTime = 0
    out = {"mofr": None, "sa": None, "so": None}
    prefix = {"mofr": routesPrefix, "sa": None, "so": None}
    factor = {"mofr": 1.0, "sa": safactor, "so": sufactor}
    if safactor > 0.0:
        prefix["sa"] = routesPrefix.replace("mofr", "sa")
    if sufactor > 0.0:
        prefix["so"] = routesPrefix.replace("mofr", "so")
    files = []
    sortedDeparts = []
    pattern = re.compile('depart="([^"]+)"')
    # pattern = re.compile('<vehicle.*depart="([0-9]+(\.[0-9]*)?)"')
    for routesIn in routeFiles:
        if verbose:
            print("Reading routes from", routesIn)
        f = open(routesIn, 'rb')
        while True:
            pos = f.tell()
            line = f.readline()
            if not line:
                break
            match = pattern.search(line)
            if match:
                entry = (float(match.group(1)), len(files), pos)
                sortedDeparts.insert(bisect.bisect(sortedDeparts, entry), entry)
            elif '<vtype' in line:
                print("Warning. Route files use outdated tag 'vtype'")
                print(line, file=vtypes)
            elif '<vType' in line:
                print(line, file=vtypes)
        files.append(f)
    for depart, idx, pos in sortedDeparts:
        f = files[idx]
        f.seek(pos)
        line = f.readline().rstrip()
        while depart >= currentTime or not out["mofr"]:
            for day in out.keys():
                if out[day]:
                    print("</routes>", file=out[day])
                    out[day].close()
                if prefix[day]:
                    routeFilename = prefix[day] + "%s.rou.xml" % currentTime
                    if verbose:
                        print("Writing file", os.path.basename(routeFilename))
                        print("... in dir", os.path.dirname(routeFilename), "TEXTTEST_IGNORE")
                    out[day] = checkDirOpen(routeFilename)
                    print("<routes>", file=out[day])
            currentTime += step
        if modifyID:
            line = re.sub('id="([^"]*)"', 'id="\\1_%s"' % idx, line)
        nextLine = ""
        while "</vehicle>" not in nextLine:
            nextLine = f.readline().rstrip()
            if nextLine.strip():
                line += os.linesep + nextLine
        for day in out.keys():
            if out[day] and random.random() < factor[day]:
                print(line, file=out[day])
    for day in out.keys():
        if out[day]:
            print("</routes>", file=out[day])
            out[day].close()
    print("</vtypes>", file=vtypes)
    vtypes.close()


class DepartChanger(handler.ContentHandler):
    def __init__(self, prefix, beg):
        self._offset = 86400
        self._indent = ""
        self._ofile = open("%s%s.rou.xml" % (prefix, beg+self._offset), "w")
        parser = make_parser()
        parser.setContentHandler(self)
        parser.parse("%s%s.rou.xml" % (prefix, beg))
        self._ofile.close()

    def startElement(self, name, attrs):
        print("%s<%s" % (self._indent, name), end=' ', file=self._ofile)
        for attr in attrs.keys():
            if attr == "depart":
                print('%s="%s"' % (attr, float(attrs[attr])+self._offset), end=' ', file=self._ofile)
            else:
                print('%s="%s"' % (attr, attrs[attr]), end=' ', file=self._ofile)
        print(">", file=self._ofile)
        self._indent += "    "

    def endElement(self, name):
        self._indent = self._indent[:-4]
        print("%s</%s>" % (self._indent, name), file=self._ofile)

    def characters(self, chars):
        self._ofile.write(chars)


def main(args=None):
    optParser = OptionParser(usage="usage: %prog [options] <routefiles>")
    optParser.add_option("-f", "--detector-file", dest="detfile",
                         help="read detectors from FILE", metavar="FILE")
    optParser.add_option("-d", "--detector-db", dest="detconn",
                         help="read detectors from database connection",
                         metavar="user:passwd:host:db")
    optParser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                         default=False, help="tell me what you are doing")
    optParser.add_option("-s", "--step", dest="step", type="int", default=900,
                         help="time split step in seconds")
    optParser.add_option("-n", "--next-day", dest="nextday", type="int", default=2700,
                         help="how far to go into the next day in seconds")
    optParser.add_option("-t", "--types-file", dest="typesfile", default="vehtypes.xml",
                         help="write vehicle types to FILE", metavar="FILE")
    optParser.add_option("-c", "--route-collection", dest="collectfile",
                         help="write route collection for the distributions to FILE", metavar="FILE")
    optParser.add_option("-o", "--routes-prefix", dest="routesprefix", default="validate/validate_mofr_",
                         help="let time splitted route files start with PREFIX", metavar="PREFIX")
    optParser.add_option("-e", "--edge-count", dest="edgecount",
                         help="dump number of routes for each edge to FILE", metavar="FILE")
    optParser.add_option("-p", "--pickle-edges", dest="pickleedge",
                         help="dump used edges as pickled set to FILE", metavar="FILE")
    optParser.add_option("-a", "--saturday-factor", dest="safact", type="float", default=0.,
                         help="generate saturday files scaled down by FACTOR", metavar="FACTOR")
    optParser.add_option("-u", "--sunday-factor", dest="sufact", type="float", default=0.,
                         help="generate sunday files scaled down by FACTOR", metavar="FACTOR")
    optParser.add_option("-m", "--modify-id", action="store_true", dest="modifyid",
                         default=False, help="try to make vehicle and route ids unique")
    (options, args) = optParser.parse_args(args=args)
    if len(args) == 0:
        optParser.print_help()
        sys.exit()
    if os.name == "posix":
        expandedArgs = args
    else:
        expandedArgs = []
        for arg in args:
            expandedArgs += glob.glob(arg)
    tempPrefix = options.routesprefix
    reader = None
    if options.detfile or options.detconn:
        if not options.collectfile:
            options.collectfile = "calibrator_routes.rou.xml",
        if options.verbose:
            print("Reading detectors")
        reader = RouteReader(options.collectfile, options.edgecount, options.pythonedge)
        conn = None
        if options.detconn:
            import MySQLdb
            from detector import readDetectorDB
            dbargs = options.detconn.split(":")
            conn = MySQLdb.connect(host=dbargs[2], user=dbargs[0],
                                   passwd=dbargs[1], db=dbargs[3])
            detReader = readDetectorDB(conn)
            conn.close()
        else:
            detReader = DetectorReader(options.detfile)
        for edge, group in detReader.getEdgeDataIterator():
            reader.addEdge(edge)
    elif options.collectfile:
        reader = RouteReader(options.collectfile, options.edgecount, options.pickleedge, True)
    splitFiles(expandedArgs, options.typesfile, tempPrefix, options.step,
               options.verbose, options.modifyid, options.safact, options.sufact)
    if reader:
        parser = make_parser()
        parser.setContentHandler(reader)
        if options.detfile:
            parser.parse(options.detfile)
        for time in range(0, 24*3600, options.step):
            routeFilename = tempPrefix+"%s.rou.xml" % time
            if os.path.exists(routeFilename):
                if options.verbose:
                    print("Parsing file", os.path.basename(routeFilename))
                    print("... in dir", os.path.dirname(routeFilename), "TEXTTEST_IGNORE")
                parser.parse(routeFilename)
        reader.closeAll()
    for i in range(0, options.nextday, options.step):
        DepartChanger(options.routesprefix, i)
        if options.safact > 0.0:
            DepartChanger(options.routesprefix.replace("mofr", "sa"), i)
        if options.sufact > 0.0:
            DepartChanger(options.routesprefix.replace("mofr", "so"), i)


if __name__ == '__main__':
    sys.path.append(os.path.join(os.path.dirname(sys.argv[0]), "..", "loop"))
    from detector import DetectorReader
    main()
