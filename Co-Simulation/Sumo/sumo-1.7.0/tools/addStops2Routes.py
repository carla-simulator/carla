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

# @file    addStops2Routes.py
# @author  Yun-Pang Floetteroed
# @date    2019-04-25

"""
- calculate the turn ratios or turn movements at each node
  with a given route file

- The output can be directly used as input in jtrrouter,
  where the time interval will be set for one day
"""
from __future__ import absolute_import
from __future__ import print_function

import sys
import optparse
import sumolib


def get_options(args=None):
    optParser = optparse.OptionParser()
    optParser.add_option("-n", "--net-file", dest="netfile",
                         help="define the NetEdit filename")
    optParser.add_option("-r", "--route-files", dest="routefiles",
                         help="define the route file seperated by comma(mandatory)")
    optParser.add_option("-o", "--output-file", dest="outfile",
                         help="define the output filename")
    optParser.add_option("-t", "--typesfile", dest="typesfile",
                         help="Give a typesfile")
    optParser.add_option("-d", "--duration",
                         help="Give a time, how long the vehicle stands")
    optParser.add_option("-u", "--until",
                         help="specify a time until the vehicle is parked")
    optParser.add_option("-p", "--parking", dest="parking", action="store_true",
                         default=False, help="where is the vehicle parking")
    optParser.add_option("-v", "--verbose", dest="verbose", action="store_true",
                         default=False, help="tell me what you are doing")

    (options, args) = optParser.parse_args(args=args)

    if not options.routefiles or not options.netfile or not options.outfile or not options.typesfile:
        optParser.print_help()
        sys.exit()

    if not options.duration and not options.until:
        optParser.print_help()
        sys.exit()
    return options


def readTypes(options):
    vtypes = {None: "passenger"}
    for file in options.typesfile.split(','):
        for vtype in sumolib.output.parse(file, 'vType'):
            vtypes[vtype.id] = vtype.vClass
    # print(vtypes)
    return vtypes


def main(options):
    # with io.open(options.outfile, 'w', encoding="utf8") as outf:
    # with open(options.outfile, 'w', encoding="utf8") as outf:
    with open(options.outfile, 'w') as outf:
        net = sumolib.net.readNet(options.netfile)
        vtypes = readTypes(options)
        sumolib.writeXMLHeader(outf, "$Id: addStops2Routes.py v1_3_1+0411-36956f96df michael.behrisch@dlr.de 2019-09-21 21:10:12 +0200 $", "routes")  # noqa
        for file in options.routefiles.split(','):
            for veh in sumolib.output.parse(file, 'vehicle'):
                edgesList = veh.route[0].edges.split()
                lastEdge = net.getEdge(edgesList[-1])
                lanes = lastEdge.getLanes()
                for lane in lanes:
                    if lane.allows(vtypes[veh.type]):
                        stopAttrs = {"lane": lane.getID()}
                        if options.parking:
                            stopAttrs["parking"] = "true"
                        if options.duration:
                            stopAttrs["duration"] = options.duration
                        if options.until:
                            stopAttrs["until"] = options.until
                        veh.addChild("stop", attrs=stopAttrs)
                        break

                outf.write(veh.toXML(' '*4))
        outf.write('</routes>\n')
    outf.close()


if __name__ == "__main__":
    options = get_options(sys.argv)
    main(options)
