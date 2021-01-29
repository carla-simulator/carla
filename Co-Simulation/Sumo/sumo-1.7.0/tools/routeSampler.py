#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    routeSampler.py
# @author  Jakob Erdmann
# @date    2020-02-07

"""
Samples routes from a given set to fullfill specified counting data (edge counts or turn counts)
"""

from __future__ import absolute_import
from __future__ import print_function

import os
import sys
import random
from collections import defaultdict
# multiprocessing imports
import multiprocessing
import numpy as np

try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
import sumolib  # noqa
from sumolib.miscutils import parseTime  # noqa


def _run_func(args):
    func, interval, kwargs, num = args
    kwargs["cpuIndex"] = num
    return num, func(interval=interval, **kwargs)


def multi_process(cpu_num, seed, interval_list, func, outf, mismatchf, **kwargs):
    cpu_count = min(cpu_num, multiprocessing.cpu_count()-1)
    interval_split = np.array_split(interval_list, cpu_count)
    # pool = multiprocessing.Pool(processes=cpu_count)
    with multiprocessing.get_context("spawn").Pool() as pool:
        results = pool.map(_run_func, [(func, interval, kwargs, i) for i, interval in enumerate(interval_split)])
        # pool.close()
        results = sorted(results, key=lambda x: x[0])
        for _, result in results:
            outf.write("".join(result[-2]))
            if mismatchf is not None:
                mismatchf.write("".join(result[-1]))
        return results


def get_options(args=None):
    parser = sumolib.options.ArgumentParser(description="Sample routes to match counts")
    parser.add_argument("-r", "--route-files", dest="routeFiles",
                        help="Input route file")
    parser.add_argument("-t", "--turn-files", dest="turnFiles",
                        help="Input turn-count file")
    parser.add_argument("-d", "--edgedata-files", dest="edgeDataFiles",
                        help="Input edgeData file (for counts)")
    parser.add_argument("-O", "--od-files", dest="odFiles",
                        help="Input edgeRelation file for origin-destination counts")
    parser.add_argument("--edgedata-attribute", dest="edgeDataAttr", default="entered",
                        help="Read edgeData counts from the given attribute")
    parser.add_argument("--turn-attribute", dest="turnAttr", default="count",
                        help="Read turning counts from the given attribute")
    parser.add_argument("--turn-max-gap", type=int, dest="turnMaxGap", default=0,
                        help="Allow at most a gap of INT edges between from-edge and to-edge")
    parser.add_argument("-o", "--output-file", dest="out", default="out.rou.xml",
                        help="Output route file")
    parser.add_argument("--prefix", dest="prefix", default="",
                        help="prefix for the vehicle ids")
    parser.add_argument("-a", "--attributes", dest="vehattrs", default="",
                        help="additional vehicle attributes")
    parser.add_argument("-s", "--seed", type=int, default=42,
                        help="random seed")
    parser.add_argument("--mismatch-output", dest="mismatchOut",
                        help="write cout-data with overflow/underflow information to FILE")
    parser.add_argument("--weighted", dest="weighted", action="store_true", default=False,
                        help="Sample routes according to their probability (or count)")
    parser.add_argument("--optimize",
                        help="set optimization method level (full, INT boundary)")
    parser.add_argument("--optimize-input", dest="optimizeInput", action="store_true", default=False,
                        help="Skip resampling and run optimize directly on the input routes")
    parser.add_argument("--geh-ok", dest="gehOk", default=5,
                        help="threshold for acceptable GEH values")
    parser.add_argument("-f", "--write-flows", dest="writeFlows",
                        help="write flows with the give style instead of vehicles [number|probability]")
    parser.add_argument("-I", "--write-route-ids", dest="writeRouteIDs", action="store_true", default=False,
                        help="write routes with ids")
    parser.add_argument("-u", "--write-route-distribution", dest="writeRouteDist",
                        help="write routeDistribution with the given ID instead of individual routes")
    parser.add_argument("-b", "--begin", help="custom begin time (seconds or H:M:S)")
    parser.add_argument("-e", "--end", help="custom end time (seconds or H:M:S)")
    parser.add_argument("-i", "--interval", help="custom aggregation interval (seconds or H:M:S)")
    parser.add_argument("-v", "--verbose", action="store_true", default=False,
                        help="tell me what you are doing")
    parser.add_argument("-V", "--verbose.histograms", dest="verboseHistogram", action="store_true", default=False,
                        help="print histograms of edge numbers and detector passing count")
    parser.add_argument("--threads", dest="threads", type=int, default=1,
                        help="If parallelization is desired, enter the number of CPUs to use. Set to a value >> then "
                             "your machines CPUs if you want to utilize all CPUs (Default is 1)"
                        )

    options = parser.parse_args(args=args)
    if (options.routeFiles is None or
            (options.turnFiles is None and options.edgeDataFiles is None and options.odFiles is None)):
        parser.print_help()
        sys.exit()
    if options.writeRouteIDs and options.writeRouteDist:
        sys.stderr.write("Only one of the options --write-route-ids and --write-route-distribution may be used")
        sys.exit()
    if options.writeFlows not in [None, "number", "probability"]:
        sys.stderr.write("Options --write-flows only accepts arguments 'number' and 'probability'")
        sys.exit()

    options.routeFiles = options.routeFiles.split(',')
    options.turnFiles = options.turnFiles.split(',') if options.turnFiles is not None else []
    options.edgeDataFiles = options.edgeDataFiles.split(',') if options.edgeDataFiles is not None else []
    options.odFiles = options.odFiles.split(',') if options.odFiles is not None else []
    if options.vehattrs and options.vehattrs[0] != ' ':
        options.vehattrs = ' ' + options.vehattrs

    if options.optimize is not None:
        try:
            import scipy.optimize  # noqa
            if options.optimize != "full":
                try:
                    options.optimize = int(options.optimize)
                except Exception:
                    print("Option optimize requires the value 'full' or an integer", file=sys.stderr)
                    sys.exit(1)
        except ImportError:
            print("Cannot use optimization (scipy not installed)", file=sys.stderr)
            sys.exit(1)

    if options.optimizeInput and type(options.optimize) != int:
        print("Option --optimize-input requires an integer argument for --optimize", file=sys.stderr)
        sys.exit(1)

    if options.threads > 1 and sys.version_info[0] < 3:
        print("Using multiple cpus is only supported for python 3", file=sys.stderr)
        sys.exit(1)

    return options


class CountData:
    def __init__(self, count, edgeTuple, allRoutes, isOD, options):
        self.origCount = count
        self.count = count
        self.edgeTuple = edgeTuple
        self.isOD = isOD
        self.options = options  # multiprocessing had issue with sumolib.options.getOptions().turnMaxGap
        self.routeSet = set()
        for routeIndex, edges in enumerate(allRoutes.unique):
            if self.routePasses(edges,):
                self.routeSet.add(routeIndex)

    def routePasses(self, edges):

        if self.isOD:
            return edges[0] == self.edgeTuple[0] and edges[-1] == self.edgeTuple[-1]
        try:
            i = edges.index(self.edgeTuple[0])
            maxDelta = self.options.turnMaxGap + 1
            for edge in self.edgeTuple[1:]:
                i2 = edges.index(edge, i)
                if i2 - i > maxDelta:
                    return False
                i = i2
        except ValueError:
            # first edge not in route
            return False
        return True

    def addCount(self, count):
        self.count += count
        self.origCount += count

    def sampleOpen(self, rng, openRoutes, routeCounts):
        cands = list(self.routeSet.intersection(openRoutes))
        assert (cands)
        probs = [routeCounts[i] for i in cands]
        x = rng.random() * sum(probs)
        seen = 0
        for route, prob in zip(cands, probs):
            seen += prob
            if seen >= x:
                return route
        assert (False)


def getIntervals(options):
    begin, end, interval = parseTimeRange(options.turnFiles + options.edgeDataFiles + options.odFiles)
    if options.begin is not None:
        begin = parseTime(options.begin)
    if options.end is not None:
        end = parseTime(options.end)
    if options.interval is not None:
        interval = parseTime(options.interval)

    result = []
    while begin < end:
        result.append((begin, begin + interval))
        begin += interval

    return result


def getOverlap(begin, end, iBegin, iEnd):
    """return overlap of the given intervals as fraction"""
    if iEnd <= begin or end <= iBegin:
        return 0  # no overlap
    elif iBegin >= begin and iEnd <= end:
        return 1  # data interval fully within requested interval
    elif iBegin <= begin and iEnd >= end:
        return (end - begin) / (iEnd - iBegin)  # only part of the data interval applies to the requested interval
    elif iBegin <= begin and iEnd <= end:
        return (iEnd - begin) / (iEnd - iBegin)  # partial overlap
    else:
        return (end - iBegin) / (iEnd - iBegin)  # partial overlap


def parseTurnCounts(interval, attr):
    for edgeRel in interval.edgeRelation:
        via = [] if edgeRel.via is None else edgeRel.via.split(' ')
        edges = tuple([edgeRel.attr_from] + via + [edgeRel.to])
        value = getattr(edgeRel, attr)
        yield edges, value


def parseEdgeCounts(interval, attr):
    for edge in interval.edge:
        yield (edge.id,), getattr(edge, attr)


def parseDataIntervals(parseFun, fnames, begin, end, allRoutes, attr, options, isOD=False, warn=False):
    locations = {}  # edges -> CountData
    result = []
    for fname in fnames:
        for interval in sumolib.xml.parse(fname, 'interval', heterogeneous=True):
            overlap = getOverlap(begin, end, parseTime(interval.begin), parseTime(interval.end))
            if overlap > 0:
                # print(begin, end, interval.begin, interval.end, "overlap:", overlap)
                for edges, value in parseFun(interval, attr):
                    try:
                        value = float(value)
                    except TypeError:
                        if warn:
                            print("Warning: Missing '%s' value in file '%s' for edge(s) '%s'" %
                                  (attr, fname, ' '.join(edges)), file=sys.stderr)
                        continue
                    if edges not in locations:
                        result.append(CountData(0, edges, allRoutes, isOD, options))
                        locations[edges] = result[-1]
                    elif isOD != locations[edges].isOD:
                        print("Warning: Edge relation '%s' occurs as turn relation and also as OD-relation" %
                              ' '.join(edges), file=sys.stderr)
                    locations[edges].addCount(int(value * overlap))
    return result


def parseTimeRange(fnames):
    begin = 1e20
    end = 0
    minInterval = 1e20
    for fname in fnames:
        for interval in sumolib.xml.parse(fname, 'interval'):
            iBegin = parseTime(interval.begin)
            iEnd = parseTime(interval.end)
            begin = min(begin, iBegin)
            end = max(end, iEnd)
            minInterval = min(minInterval, iEnd - iBegin)
    return begin, end, minInterval


def hasCapacity(dataIndices, countData):
    for i in dataIndices:
        if countData[i].count == 0:
            return False
    return True


def updateOpenRoutes(openRoutes, routeUsage, countData):
    return set(filter(lambda r: hasCapacity(routeUsage[r], countData), openRoutes))


def updateOpenCounts(openCounts, countData, openRoutes):
    return set(filter(lambda i: countData[i].routeSet.intersection(openRoutes), openCounts))


def optimize(options, countData, routes, usedRoutes, routeUsage):
    """ use relaxtion of the ILP problem for picking the number of times that each route is used
    x = usageCount vector (count for each route index)
    c = weight vector (vector of 1s)
    A_eq = routeUsage encoding
    b_eq = counts

    Rationale:
      c: costs for using each route,
         when minimizing x @ c, routes that pass multiple counting stations are getting an advantage

    """
    import scipy.optimize as opt
    import scipy.version
    import numpy as np

    k = routes.number
    m = len(countData)

    priorRouteCounts = getRouteCounts(routes, usedRoutes)

    if options.optimize == "full":
        # allow changing all prior usedRoutes
        bounds = None
    else:
        u = int(options.optimize)
        if u == 0:
            print("Optimization skipped")
            return
        # limited optimization: change prior routeCounts by at most u per route
        bounds = [(max(0, p - u), p + u) for p in priorRouteCounts] + [(0, None)] * m

    # Ax <= b
    # x + s = b
    # min s
    # -> x2 = [x, s]

    A = np.zeros((m, k))
    for i in range(0, m):
        for j in range(0, k):
            A[i][j] = int(j in countData[i].routeSet)
    A_eq = np.concatenate((A, np.identity(m)), 1)

    # constraint: achieve counts
    b = np.asarray([cd.origCount for cd in countData])

    # minimization objective
    c = np.concatenate((np.zeros(k), np.ones(m)))  # [x, s], only s counts for minimization

    # set x to prior counts and slack to deficit (otherwise solver may fail to any find soluton
    x0 = priorRouteCounts + [cd.origCount - cd.count for cd in countData]

    # print("k=%s" % k)
    # print("m=%s" % m)
    # print("A_eq (%s) %s" % (A_eq.shape, A_eq))
    # print("b (%s) %s" % (len(b), b))
    # print("c (%s) %s" % (len(c), c))
    # print("bounds (%s) %s" % (len(bounds) if bounds is not None else "-", bounds))
    # print("x0 (%s) %s" % (len(x0), x0))

    linProgOpts = {}
    if options.verbose:
        linProgOpts["disp"] = True

    try:
        res = opt.linprog(c, A_eq=A_eq, b_eq=b, bounds=bounds, x0=x0, options=linProgOpts)
    except TypeError:
        if options.verbose:
            print("Warning: Scipy version %s does not support initial guess for opt.linprog. Optimization may fail"
                  % scipy.version.version, file=sys.stderr)
        res = opt.linprog(c, A_eq=A_eq, b_eq=b, bounds=bounds, options=linProgOpts)

    del usedRoutes[:]
    if res.success:
        print("Optimization succeeded")
        routeCounts = res.x[:k]  # cut of slack variables
        # slack = res.x[k:]
        # print("routeCounts (n=%s, sum=%s, intSum=%s, roundSum=%s) %s" % (
        #    len(routeCounts),
        #    sum(routeCounts),
        #    sum(map(int, routeCounts)),
        #    sum(map(round, routeCounts)),
        #    routeCounts))
        # print("slack (n=%s, sum=%s) %s" % (len(slack), sum(slack), slack))
        usedRoutes.extend(sum([[i] * int(round(c)) for i, c in enumerate(routeCounts)], []))
        # print("#usedRoutes=%s" % len(usedRoutes))
        # update countData
    else:
        print("Optimization failed")


def zero():
    # cannot pickle lambda for multiprocessing
    return 0


class Routes:
    def __init__(self, routefiles):
        self.all = []
        self.edgeProbs = defaultdict(zero)
        self.edgeIDs = {}
        self.withProb = 0
        for routefile in routefiles:
            # not all routes may have specified probability, in this case use their number of occurence
            for r in sumolib.xml.parse(routefile, 'route', heterogeneous=True):
                edges = tuple(r.edges.split())
                self.all.append(edges)
                prob = float(r.getAttributeSecure("probability", 1))
                if r.hasAttribute("probability"):
                    self.withProb += 1
                    prob = float(r.probability)
                else:
                    prob = 1
                if prob <= 0:
                    print("Warning: route probability must be positive (edges=%s)" % r.edges, file=sys.stderr)
                    prob = 0
                if r.hasAttribute("id"):
                    self.edgeIDs[edges] = r.id
                self.edgeProbs[edges] += prob
        self.unique = sorted(list(self.edgeProbs.keys()))
        self.number = len(self.unique)
        self.edges2index = dict([(e, i) for i, e in enumerate(self.unique)])
        if len(self.unique) == 0:
            print("Error: no input routes loaded", file=sys.stderr)
            sys.exit()
        self.probabilities = np.array([self.edgeProbs[e] for e in self.unique], dtype=np.float64)


def resetCounts(usedRoutes, routeUsage, countData):
    for cd in countData:
        cd.count = cd.origCount
    for r in usedRoutes:
        for i in routeUsage[r]:
            countData[i].count -= 1


def getRouteCounts(routes, usedRoutes):
    result = [0] * routes.number
    for r in usedRoutes:
        result[r] += 1
    return result


def getRouteUsage(routes, countData):
    # store which counting locations are used by each route (using countData index)
    routeUsage = [set() for r in routes.unique]
    for i, cd in enumerate(countData):
        for routeIndex in cd.routeSet:
            routeUsage[routeIndex].add(i)
    return routeUsage


def main(options):
    if options.seed:
        random.seed(options.seed)
        np.random.seed(options.seed)

    routes = Routes(options.routeFiles)

    intervals = getIntervals(options)

    # preliminary integrity check for the whole time range
    b = intervals[0][0]
    e = intervals[-1][-1]
    countData = (parseDataIntervals(parseTurnCounts, options.turnFiles, b, e,
                                    routes, options.turnAttr, options=options, warn=True) +
                 parseDataIntervals(parseEdgeCounts, options.edgeDataFiles, b, e,
                                    routes, options.edgeDataAttr, options=options, warn=True) +
                 parseDataIntervals(parseTurnCounts, options.odFiles, b, e,
                                    routes, options.turnAttr, options=options, isOD=True, warn=True))
    routeUsage = getRouteUsage(routes, countData)

    for cd in countData:
        if cd.count > 0 and not cd.routeSet:
            print("Warning: no routes pass edge '%s' (count %s)" % (
                ' '.join(cd.edgeTuple), cd.count), file=sys.stderr)

    if options.verbose:
        print("Loaded %s routes (%s distinct)" % (len(routes.all), routes.number))
        if options.weighted:
            print("Loaded probability for %s routes" % routes.withProb)
        if options.verboseHistogram:
            edgeCount = sumolib.miscutils.Statistics("route edge count", histogram=True)
            detectorCount = sumolib.miscutils.Statistics("route detector count", histogram=True)
            for i, edges in enumerate(routes.unique):
                edgeCount.add(len(edges), i)
                detectorCount.add(len(routeUsage[i]), i)
            print("input %s" % edgeCount)
            print("input %s" % detectorCount)

    mismatchf = None
    if options.mismatchOut:
        mismatchf = open(options.mismatchOut, 'w')
        sumolib.writeXMLHeader(mismatchf, "$Id$")  # noqa
        mismatchf.write('<data>\n')

    underflowSummary = sumolib.miscutils.Statistics("all interval underflow")
    overflowSummary = sumolib.miscutils.Statistics("all interval overflow")
    gehSummary = sumolib.miscutils.Statistics("all interval GEH%")

    with open(options.out, 'w') as outf:
        sumolib.writeXMLHeader(outf, "$Id$", "routes")  # noqa
        if options.threads > 1:
            # call the multiprocessing function
            results = multi_process(options.threads, options.seed, intervals,
                                    _solveIntervalMP, outf, mismatchf, options=options, routes=routes)
            # handle the uFlow, oFlow and GEH
            for _, result in results:
                for i, begin in enumerate(result[0]):
                    underflowSummary.add(result[1][i], begin)
                    overflowSummary.add(result[2][i], begin)
                    gehSummary.add(result[3][i], begin)
        else:
            for begin, end in intervals:
                intervalPrefix = "" if len(intervals) == 1 else "%s_" % int(begin)
                uFlow, oFlow, gehOK, _ = solveInterval(options, routes, begin, end, intervalPrefix, outf, mismatchf)
                underflowSummary.add(uFlow, begin)
                overflowSummary.add(oFlow, begin)
                gehSummary.add(gehOK, begin)
        outf.write('</routes>\n')

    if options.mismatchOut:
        mismatchf.write('</data>\n')
        mismatchf.close()

    if len(intervals) > 1:
        print(underflowSummary)
        print(overflowSummary)
        print(gehSummary)


def _sample(sampleSet, rng):
    population = tuple(sampleSet)
    return population[(int)(rng.random() * len(population))]


def _sample_skewed(sampleSet, rng, probabilityMap):
    # build cumulative distribution function for weighted sampling
    cdf = []
    total = 0
    population = tuple(sampleSet)
    for element in population:
        total += probabilityMap[element]
        cdf.append(total)

    value = random.random() * total
    return population[np.searchsorted(cdf, value)]


def _solveIntervalMP(options, routes, interval, cpuIndex):
    output_list = []
    rng = np.random.RandomState(options.seed + cpuIndex)
    for begin, end in interval:
        local_outf = StringIO()
        local_mismatch_outf = StringIO() if options.mismatchOut else None
        intervalPrefix = "%s_" % int(begin)
        uFlow, oFlow, gehOKNum, local_outf = solveInterval(
            options, routes, begin, end, intervalPrefix, local_outf, local_mismatch_outf, rng=rng)
        output_list.append([begin, uFlow, oFlow, gehOKNum, local_outf.getvalue(),
                            local_mismatch_outf.getvalue() if options.mismatchOut else None])
    output_lst = list(zip(*output_list))
    return output_lst


def solveInterval(options, routes, begin, end, intervalPrefix, outf, mismatchf, rng=random):
    # store which routes are passing each counting location (using route index)
    countData = (parseDataIntervals(parseTurnCounts, options.turnFiles, begin, end, routes, options.turnAttr,
                                    options=options)
                 + parseDataIntervals(parseEdgeCounts, options.edgeDataFiles, begin, end, routes,
                                      options.edgeDataAttr,
                                      options=options)
                 + parseDataIntervals(parseTurnCounts, options.odFiles, begin, end, routes, options.turnAttr,
                                      isOD=True,
                                      options=options)
                 )

    routeUsage = getRouteUsage(routes, countData)
    unrestricted = set([r for r, usage in enumerate(routeUsage) if len(usage) == 0])
    if options.verbose and len(unrestricted) > 0:
        print("Ignored %s routes which do not pass any counting location" % len(unrestricted))

    # pick a random counting location and select a new route that passes it until
    # all counts are satisfied or no routes can be used anymore
    openRoutes = set(range(0, routes.number))
    openCounts = set(range(0, len(countData)))
    openRoutes = updateOpenRoutes(openRoutes, routeUsage, countData)
    openCounts = updateOpenCounts(openCounts, countData, openRoutes)
    openRoutes = openRoutes.difference(unrestricted)

    usedRoutes = []
    if options.optimizeInput:
        usedRoutes = [routes.edges2index[e] for e in routes.all]
        resetCounts(usedRoutes, routeUsage, countData)
    else:
        while openCounts:
            if options.weighted:
                routeIndex = _sample_skewed(openRoutes, rng, routes.probabilities)
            else:
                # sampling equally among open counting locations appears to
                # improve GEH but it would also introduce a bias in the loaded
                # route probabilities
                cd = countData[_sample(openCounts, rng)]
                routeIndex = _sample(cd.routeSet.intersection(openRoutes), rng)
            usedRoutes.append(routeIndex)
            for dataIndex in routeUsage[routeIndex]:
                countData[dataIndex].count -= 1
            openRoutes = updateOpenRoutes(openRoutes, routeUsage, countData)
            openCounts = updateOpenCounts(openCounts, countData, openRoutes)

    totalMismatch = sum([cd.count for cd in countData])
    if totalMismatch > 0 and options.optimize is not None:
        if options.verbose:
            print("Starting optimization for interval [%s, %s] (mismatch %s)" % (
                begin, end, totalMismatch))
        optimize(options, countData, routes, usedRoutes, routeUsage)
        resetCounts(usedRoutes, routeUsage, countData)
    # avoid bias from sampling order / optimization
    random.shuffle(usedRoutes, rng.random)

    if usedRoutes:
        outf.write('<!-- begin="%s" end="%s" -->\n' % (begin, end))
        period = (end - begin) / len(usedRoutes)
        depart = begin
        routeCounts = getRouteCounts(routes, usedRoutes)
        if options.writeRouteIDs:
            for routeIndex in sorted(set(usedRoutes)):
                edges = routes.unique[routeIndex]
                routeIDComment = ""
                if edges in routes.edgeIDs:
                    routeIDComment = " (%s)" % routes.edgeIDs[edges]
                outf.write('    <route id="%s%s" edges="%s"/> <!-- %s%s -->\n' % (
                    intervalPrefix, routeIndex, ' '.join(edges),
                    routeCounts[routeIndex], routeIDComment))
            outf.write('\n')
        elif options.writeRouteDist:
            outf.write('    <routeDistribution id="%s%s"/>\n' % (intervalPrefix, options.writeRouteDist))
            for routeIndex in sorted(set(usedRoutes)):
                outf.write('        <route id="%s%s" edges="%s" probability="%s"/>\n' % (
                    intervalPrefix, routeIndex, ' '.join(routes.unique[routeIndex]), routeCounts[routeIndex]))
            outf.write('    </routeDistribution>\n\n')

        routeID = options.writeRouteDist
        if options.writeFlows is None:
            for i, routeIndex in enumerate(usedRoutes):
                if options.writeRouteIDs:
                    routeID = routeIndex
                vehID = options.prefix + intervalPrefix + str(i)
                if routeID is not None:
                    outf.write('    <vehicle id="%s" depart="%.2f" route="%s%s"%s/>\n' % (
                        vehID, depart, intervalPrefix, routeID, options.vehattrs))
                else:
                    outf.write('    <vehicle id="%s" depart="%.2f"%s>\n' % (
                        vehID, depart, options.vehattrs))
                    outf.write('        <route edges="%s"/>\n' % ' '.join(routes.unique[routeIndex]))
                    outf.write('    </vehicle>\n')
                depart += period
        else:
            routeDeparts = defaultdict(list)
            for routeIndex in usedRoutes:
                routeDeparts[routeIndex].append(depart)
                depart += period
            if options.writeRouteDist:
                totalCount = sum(routeCounts)
                probability = totalCount / (end - begin)
                flowID = options.prefix + intervalPrefix + options.writeRouteDist
                if options.writeFlows == "number" or probability > 1.001:
                    repeat = 'number="%s"' % totalCount
                    if options.writeFlows == "probability":
                        sys.stderr.write("Warning: could not write flow %s with probability %.2f\n" %
                                         (flowID, probability))
                else:
                    repeat = 'probability="%s"' % probability
                outf.write('    <flow id="%s" begin="%.2f" end="%.2f" %s route="%s"%s/>\n' % (
                    flowID, begin, end, repeat,
                    options.writeRouteDist, options.vehattrs))
            else:
                # ensure flows are sorted
                flows = []
                for routeIndex in sorted(set(usedRoutes)):
                    outf2 = StringIO()
                    fBegin = min(routeDeparts[routeIndex])
                    fEnd = max(routeDeparts[routeIndex] + [fBegin + 1.0])
                    probability = routeCounts[routeIndex] / (fEnd - fBegin)
                    flowID = "%s%s%s" % (options.prefix, intervalPrefix, routeIndex)
                    if options.writeFlows == "number" or probability > 1.001:
                        repeat = 'number="%s"' % routeCounts[routeIndex]
                        if options.writeFlows == "probability":
                            sys.stderr.write("Warning: could not write flow %s with probability %.2f\n" % (
                                flowID, probability))
                    else:
                        repeat = 'probability="%s"' % probability
                    if options.writeRouteIDs:
                        outf2.write('    <flow id="%s" begin="%.2f" end="%.2f" %s route="%s%s"%s/>\n' % (
                            flowID, fBegin, fEnd, repeat,
                            intervalPrefix, routeIndex, options.vehattrs))
                    else:
                        outf2.write('    <flow id="%s" begin="%.2f" end="%.2f" %s%s>\n' % (
                            flowID, fBegin, fEnd, repeat, options.vehattrs))
                        outf2.write('        <route edges="%s"/>\n' % ' '.join(routes.unique[routeIndex]))
                        outf2.write('    </flow>\n')
                    flows.append((fBegin, outf2))
                flows.sort()
                for fBegin, outf2 in flows:
                    outf.write(outf2.getvalue())

    underflow = sumolib.miscutils.Statistics("underflow locations")
    overflow = sumolib.miscutils.Statistics("overflow locations")
    gehStats = sumolib.miscutils.Statistics("GEH")
    numGehOK = 0.0
    hourFraction = (end - begin) / 3600.0
    totalCount = 0
    for cd in countData:
        localCount = cd.origCount - cd.count
        totalCount += localCount
        if cd.count > 0:
            underflow.add(cd.count, cd.edgeTuple)
        elif cd.count < 0:
            overflow.add(cd.count, cd.edgeTuple)
        origHourly = cd.origCount / hourFraction
        localHourly = localCount / hourFraction
        geh = sumolib.miscutils.geh(origHourly, localHourly)
        if geh < options.gehOk:
            numGehOK += 1
        gehStats.add(geh, "[%s] %s %s" % (
            ' '.join(cd.edgeTuple), int(origHourly), int(localHourly)))

    outputIntervalPrefix = "" if intervalPrefix == "" else "%s: " % int(begin)
    gehOKNum = (100 * numGehOK / len(countData)) if countData else 100
    gehOK = "%.2f%%" % gehOKNum if countData else "-"
    print("%sWrote %s routes (%s distinct) achieving total count %s at %s locations. GEH<%s for %s" % (
        outputIntervalPrefix,
        len(usedRoutes), len(set(usedRoutes)), totalCount, len(countData),
        options.gehOk, gehOK))

    if options.verboseHistogram:
        edgeCount = sumolib.miscutils.Statistics("route edge count", histogram=True)
        detectorCount = sumolib.miscutils.Statistics("route detector count", histogram=True)
        for i, r in enumerate(usedRoutes):
            edgeCount.add(len(routes.unique[r]), i)
            detectorCount.add(len(routeUsage[r]), i)
        print("result %s" % edgeCount)
        print("result %s" % detectorCount)
        print(gehStats)

    if underflow.count() > 0:
        print("Warning: %s (total %s)" % (underflow, sum(underflow.values)))
    if overflow.count() > 0:
        print("Warning: %s (total %s)" % (overflow, sum(overflow.values)))
    sys.stdout.flush()  # needed for multiprocessing

    if mismatchf:
        mismatchf.write('    <interval id="deficit" begin="%s" end="%s">\n' % (begin, end))
        for cd in countData:
            if len(cd.edgeTuple) == 1:
                mismatchf.write('        <edge id="%s" measuredCount="%s" deficit="%s"/>\n' % (
                    cd.edgeTuple[0], cd.origCount, cd.count))
            elif len(cd.edgeTuple) == 2:
                mismatchf.write('        <edgeRelation from="%s" to="%s" measuredCount="%s" deficit="%s"/>\n' % (
                    cd.edgeTuple[0], cd.edgeTuple[1], cd.origCount, cd.count))
            else:
                print("Warning: output for edge relations with more than 2 edges not supported (%s)" % cd.edgeTuple,
                      file=sys.stderr)
        mismatchf.write('    </interval>\n')

    return sum(underflow.values), sum(overflow.values), gehOKNum, outf


if __name__ == "__main__":
    main(get_options())
