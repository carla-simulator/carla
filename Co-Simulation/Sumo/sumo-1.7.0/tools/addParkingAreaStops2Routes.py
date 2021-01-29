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

# @file    addParkingAreaStops2Routes.py
# @author  Evamarie Wiessner
# @date    2017-01-09

"""
add stops to vehicle routes
"""

from __future__ import print_function
from __future__ import absolute_import
import os
import sys
import optparse

if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(tools)
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")

import sumolib  # noqa


def get_options(args=None):
    optParser = optparse.OptionParser()
    optParser.add_option("-r", "--route-file", dest="routefile", help="define the route file")
    optParser.add_option("-o", "--output-file", dest="outfile", help="output route file including parking")
    optParser.add_option("-p", "--parking-areas", dest="parking",
                         help="define the parking areas seperated by comma")
    optParser.add_option("-d", "--parking-duration", dest="duration",
                         help="define the parking duration (in seconds)", default=3600)
    optParser.add_option("-v", "--verbose", dest="verbose", action="store_true",
                         default=False, help="tell me what you are doing")
    (options, args) = optParser.parse_args(args=args)
    if not options.routefile or not options.parking:
        optParser.print_help()
        sys.exit()
    return options


def main(options):
    infile = options.routefile
    if not options.outfile:
        outfile = infile.replace(".xml", ".parking.xml")

    with open(outfile, 'w') as outf:
        outf.write("<?xml version= \"1.0\" encoding=\"UTF-8\"?>\n\n")
        outf.write("<routes>\n")
        for veh in sumolib.xml.parse(infile, "vehicle"):
            stops = [x for x in options.parking.split(',') if x in veh.id]
            for stop in stops:
                veh.addChild("stop", {"parkingArea": stop, "duration": int(options.duration)})
                veh.setAttribute("arrivalPos", -2)
            outf.write(veh.toXML(initialIndent="    "))
        outf.write("</routes>\n")


if __name__ == "__main__":
    options = get_options(sys.argv)
    main(options)
