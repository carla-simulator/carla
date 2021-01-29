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

# @file    route_departOffset.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    11.09.2009

from __future__ import absolute_import
from __future__ import print_function
import os
import sys
import codecs
import optparse

if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(os.path.join(tools))
    from sumolib.output import parse
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")


def intIfPossible(val):
    if int(val) == val:
        return int(val)
    else:
        return val


def get_options(args=None):
    optParser = optparse.OptionParser()
    optParser.add_option("-r", "--input-file", dest="infile",
                         help="the input route file (mandatory)")
    optParser.add_option("-o", "--output-file", dest="outfile",
                         help="the output route file (mandatory)")
    optParser.add_option("-d", "--depart-offset", dest="offset",
                         type="float", help="the depart offset to apply")
    optParser.add_option("-i", "--depart-interval", dest="interval",
                         help="time intervals a,b,c,d where all vehicles departing in the interval" +
                         "[a,b[ are mapped to the interval [c,d[")
    optParser.add_option("--modify-ids", dest="modify_ids", action="store_true",
                         default=False, help="whether ids should be modified as well")
    optParser.add_option("--heterogeneous", dest="heterogeneous",
                         action="store_true", default=False, help="whether heterogeneous objects shall be parsed " +
                                                                  "(i.e. vehicles with embeded and referenced routes)")
    optParser.add_option("--depart-edges", dest="depart_edges",
                         help="only modify departure times of vehicles departing on the given edges")
    optParser.add_option("--depart-edges.file", dest="depart_edges_file",
                         help="only modify departure times of vehicles departing on edges or lanes in the " +
                              "given selection file")
    optParser.add_option("--arrival-edges", dest="arrival_edges",
                         help="only modify departure times of vehicles arriving on the given edges")
    optParser.add_option("--arrival-edges.file", dest="arrival_edges_file",
                         help="only modify departure times of vehicles arriving on edges or lanes in the " +
                              "given selection file")

    (options, args) = optParser.parse_args(args=args)
    if options.infile is None or options.outfile is None:
        optParser.print_help()
        sys.exit()

    if ((options.offset is None and options.interval is None) or
            (options.offset is not None and options.interval is not None)):
        print(
            "Either one of the options --depart-offset or --depart-interval must be given")
        sys.exit()

    if options.offset is not None:
        options.name_suffix = "_%s" % intIfPossible(options.offset)
    else:
        options.interval = tuple(map(float, options.interval.split(',')))
        options.name_suffix = "_%s_%s_%s_%s" % options.interval

    if options.depart_edges is not None:
        options.depart_edges = options.depart_edges.split(',')

    if options.depart_edges_file is not None:
        if options.depart_edges is None:
            options.depart_edges = []
        for line in open(options.depart_edges_file):
            line = line.strip()
            if line.startswith("edge:"):
                options.depart_edges.append(line[5:])
            elif line.startswith("lane:"):
                options.depart_edges.append(line[5:-2])
            else:
                options.depart_edges.append(line)

    if options.arrival_edges is not None:
        options.arrival_edges = options.arrival_edges.split(',')

    if options.arrival_edges_file is not None:
        if options.arrival_edges is None:
            options.arrival_edges = []
        for line in open(options.arrival_edges_file):
            line = line.strip()
            if line.startswith("edge:"):
                options.arrival_edges.append(line[5:])
            elif line.startswith("lane:"):
                options.arrival_edges.append(line[5:-2])
            else:
                options.arrival_edges.append(line)

    return options


def shiftInterval(val, interval):
    val = float(val)
    if interval[0] <= val < interval[1]:
        val = (val - interval[0]) / (interval[1] - interval[0]
                                     ) * (interval[3] - interval[2]) + interval[2]
    return str(intIfPossible(val))


def main(options):
    # cache stand-alone routes
    routesDepart = {}  # first edge for each route
    routesArrival = {}  # last edge for each route

    with codecs.open(options.outfile, 'w', encoding='utf8') as out:
        out.write("<routes>\n")
        for route in parse(options.infile, "route"):
            if route.hasAttribute('id') and route.id is not None:
                routesDepart[route.id] = route.edges.split()[0]
                routesArrival[route.id] = route.edges.split()[-1]
                out.write(route.toXML('    '))

        for obj in parse(options.infile, ['vehicle', 'trip', 'flow', 'vType'],
                         heterogeneous=options.heterogeneous, warn=False):
            if obj.name == 'vType':
                # copy
                pass
            else:
                if options.modify_ids:
                    obj.id += options.name_suffix

                # compute depart-edge filter
                departEdge = None
                if options.depart_edges is not None:
                    # determine the departEdge of the current vehicle
                    if obj.name == 'trip':
                        departEdge = obj.attr_from
                    elif obj.name == 'vehicle':
                        if obj.hasAttribute('route') and obj.route is not None:
                            departEdge = routesDepart[obj.route]
                        else:
                            # route child element
                            departEdge = obj.route[0].edges.split()[0]
                    elif obj.name == 'flow':
                        if obj.hasAttribute('attr_from') and obj.attr_from is not None:
                            departEdge = obj.attr_from
                        elif obj.hasAttribute('route') and obj.route is not None:
                            departEdge = routesDepart[obj.route]
                        else:
                            # route child element
                            departEdge = obj.route[0].edges.split()[0]

                # compute arrival-edge filter
                arrivalEdge = None
                if options.arrival_edges is not None:
                    # determine the arrivalEdge of the current vehicle
                    if obj.name == 'trip':
                        arrivalEdge = obj.to
                    elif obj.name == 'vehicle':
                        if obj.hasAttribute('route') and obj.route is not None:
                            arrivalEdge = routesArrival[obj.route]
                        else:
                            # route child element
                            arrivalEdge = obj.route[0].edges.split()[-1]
                    elif obj.name == 'flow':
                        if obj.hasAttribute('to') and obj.attr_from is not None:
                            arrivalEdge = obj.to
                        elif obj.hasAttribute('route') and obj.route is not None:
                            arrivalEdge = routesArrival[obj.route]
                        else:
                            # route child element
                            arrivalEdge = obj.route[0].edges.split()[-1]

                # modify departure time
                if ((departEdge is None or departEdge in options.depart_edges) and
                        (arrivalEdge is None or arrivalEdge in options.arrival_edges)):
                    if options.offset is not None:
                        # shift by offset
                        if obj.name in ['trip', 'vehicle']:
                            obj.depart = str(intIfPossible(
                                float(obj.depart) + options.offset))
                        else:
                            obj.begin = str(intIfPossible(
                                float(obj.begin) + options.offset))
                            obj.end = str(intIfPossible(
                                float(obj.end) + options.offset))
                    else:
                        # shift by interval
                        if obj.name in ['trip', 'vehicle']:
                            obj.depart = shiftInterval(
                                obj.depart, options.interval)
                        else:
                            obj.begin = shiftInterval(
                                obj.begin, options.interval)
                            obj.end = shiftInterval(obj.end, options.interval)

            out.write(obj.toXML('    '))
        out.write("</routes>\n")


if __name__ == "__main__":
    main(get_options(sys.argv))
