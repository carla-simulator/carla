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

# @file    cutTrips.py
# @author  Jakob Erdmann
# @date    2017-04-11

"""
Cut down trips from a large scenario to a sub-scenario.
Only trips that start and end in the sub-scenario network are kept
(This differs from cutRoutes.py which also keeps routes passing throu the
sub-scenario network)
"""
from __future__ import absolute_import
from __future__ import print_function

import os
import sys
import codecs

from optparse import OptionParser
from collections import defaultdict
import sort_routes

if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(os.path.join(tools))
    from sumolib.output import parse, parse_fast  # noqa
    from sumolib.net import readNet  # noqa
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")


def get_options(args=sys.argv[1:]):
    USAGE = """Usage %prog [options] <new_net.xml> <trips> [<trips2> ...]
If the given routes contain exit times these will be used to compute new
departure times. If the option --orig-net is given departure times will be
extrapolated based on edge-lengths and maximum speeds multiplied with --speed-factor"""
    optParser = OptionParser(usage=USAGE)
    optParser.add_option("-v", "--verbose", action="store_true",
                         default=False, help="Give more output")
    optParser.add_option("-o", "--trips-output", help="output trip file")
    optParser.add_option("-a", "--additional-input", help="additional file for taz (must already be cut)")
    optParser.add_option("-b", "--big", action="store_true", default=False,
                         help="Perform out-of-memory sort using module sort_routes (slower but more memory efficient)")
    options, args = optParser.parse_args(args=args)
    try:
        options.network = args[0]
        options.routeFiles = args[1:]
    except Exception:
        sys.exit(USAGE.replace('%prog', os.path.basename(__file__)))
    options.output = options.trips_output
    return options


def cut_trips(aEdges, options, validTaz):
    areaEdges = set(aEdges)
    num_trips = 0
    num_returned = 0

    for routeFile in options.routeFiles:
        print("Parsing trips from %s" % routeFile)
        for trip in parse(routeFile, 'trip'):
            num_trips += 1
            if trip.attr_from is not None and trip.attr_from not in areaEdges:
                continue
            if trip.to is not None and trip.to not in areaEdges:
                continue
            if trip.fromTaz is not None and trip.fromTaz not in validTaz:
                continue
            if trip.toTaz is not None and trip.toTaz not in validTaz:
                continue
            yield float(trip.depart), trip
            num_returned += 1

        print("Parsing persontrips from %s" % routeFile)
        ignored_planitems = defaultdict(lambda: 0)
        num_persons = 0
        num_persontrips = 0
        from_ok = 0
        to_ok = 0
        for person in parse(routeFile, 'person'):
            num_persons += 1
            if person.walk is not None:
                ignored_planitems['walk'] += len(person.walk)
                del person.walk
            if person.stop is not None:
                ignored_planitems['stop'] += len(person.stop)
                del person.stop
            if person.ride is not None:
                ignored_planitems['ride'] += len(person.ride)
                del person.ride
            if person.personTrip is not None:
                kept_pt = []
                for pt in person.personTrip:
                    skip = False
                    if pt.attr_from in areaEdges:
                        from_ok += 1
                    else:
                        skip = True
                    if pt.to in areaEdges:
                        to_ok += 1
                    else:
                        skip = True

                    if skip:
                        continue
                    kept_pt.append(pt)
                    num_persontrips += 1
                if kept_pt:
                    person.personTrip = kept_pt
                    yield float(person.depart), person

    print("Parsed %s trips and kept %s" % (num_trips, num_returned))
    if num_persons > 0:
        print("Parsed %s persons and kept %s persontrips" % (num_trips,
                                                             num_persontrips))
        print("Discared %s person that departed in the area and %s persons that arrived in the area" % (
            from_ok, to_ok))
        if ignored_planitems:
            print("Ignored plan items:")
            for itemtype, count in ignored_planitems.items():
                print("  %s %ss" % (count, itemtype))


def writer(file, trip):
    file.write(trip.toXML('    '))


def main(options):
    net = readNet(options.network)
    edges = set([e.getID() for e in net.getEdges()])
    print("Valid area contains %s edges" % len(edges))

    def write_to_file(vehicles, f):
        f.write('<!-- generated with %s for %s from %s -->\n' %
                (os.path.basename(__file__), options.network, options.routeFiles))
        f.write(
            ('<routes xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ' +
             'xsi:noNamespaceSchemaLocation="http://sumo.dlr.de/xsd/routes_file.xsd">\n'))
        num_trips = 0
        num_persons = 0
        for _, v in vehicles:
            if v.name == 'trip':
                num_trips += 1
            else:
                num_persons += 1
            writer(f, v)
        f.write('</routes>\n')
        if num_persons > 0:
            print("Wrote %s trips and %s persons" % (num_trips, num_persons))
        else:
            print("Wrote %s trips" % (num_trips))

    validTaz = set()
    if options.additional_input:
        for taz in parse(options.additional_input, 'taz'):
            validTaz.add(taz.id)

    if options.big:
        # write output unsorted
        tmpname = options.output + ".unsorted"
        with codecs.open(tmpname, 'w', encoding='utf8') as f:
            write_to_file(
                cut_trips(edges, options, validTaz), f)
        # sort out of memory
        sort_routes.main([tmpname, '--big', '--outfile', options.output])
    else:
        routes = list(cut_trips(edges, options, validTaz))
        routes.sort(key=lambda v: v[0])
        with codecs.open(options.output, 'w', encoding='utf8') as f:
            write_to_file(routes, f)


if __name__ == "__main__":
    main(get_options())
