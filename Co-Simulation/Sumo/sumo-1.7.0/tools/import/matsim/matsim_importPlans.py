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

# @file    matsim_importPlans.py
# @author  Jakob Erdmann
# @author  Camillo Fillinger
# @date    2019-09-27

"""
Import person plans from MATSim
"""
from __future__ import absolute_import
from __future__ import print_function

import os
import sys
import optparse
try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
import sumolib  # noqa


def get_options(args=None):
    optParser = optparse.OptionParser()
    optParser.add_option("-p", "--plan-file", dest="planfile",
                         help="define the route file (mandatory)")
    optParser.add_option("-o", "--out-file", dest="outfile",
                         help="Output file (mandatory)")
    optParser.add_option("--vehicles-only", dest="carsOnly", action="store_true",
                         default=False, help="Import only vehicles instead of persons")
    optParser.add_option("--default-start", dest="defaultStart", default="0:0:0",
                         help="default start time for the first activity")
    optParser.add_option("--default-end", dest="defaultEnd", default="24:0:0",
                         help="default end time for the last activity")
    optParser.add_option("-v", "--verbose", dest="verbose", action="store_true",
                         default=False, help="tell me what you are doing")

    (options, args) = optParser.parse_args(args=args)

    if not options.planfile or not options.outfile:
        optParser.print_help()
        sys.exit()

    return options


def writeLeg(outf, options, idveh, leg, startLink, endLink):
    depart = leg.dep_time if options.carsOnly else "triggered"
    if leg.route is None or leg.route[0].distance == "NaN":
        outf.write('   <trip id="%s" depart="%s" from="%s" to="%s"/>\n'
                   % (idveh, depart, startLink, endLink))
    else:
        outf.write('   <vehicle id="%s" depart="%s" >\n' % (idveh, depart))
        outf.write('        <route edges="%s"/>\n' % (leg.route[0].getText()))
        outf.write('   </vehicle>\n')


def main(options):
    persons = []  # (depart, xmlsnippet)
    for person in sumolib.xml.parse(options.planfile, 'person'):
        outf = StringIO()
        vehIndex = 0
        plan = person.plan[0]
        if len(plan.getChildList()) == 0:
            continue
        firstAct = plan.getChildList()[0]
        depart = firstAct.start_time
        if depart is None:
            depart = options.defaultStart
        attributes = person.attributes[0] if person.attributes else None
        # write vehicles
        vehicleslist = []
        untillist = []
        lastAct = None
        lastLeg = None
        for item in plan.getChildList():
            leg = None
            idveh = "%s_%s" % (person.id, vehIndex)
            if "act" in item.name:  # act or activity
                if lastLeg is not None:
                    leg = lastLeg
                    leg.dep_time = lastAct.end_time
                    writeLeg(outf, options, idveh, leg, lastAct.link, item.link)
                    lastLeg = None
                lastAct = item
            if item.name == "leg":
                if item.route is None:
                    lastLeg = item
                else:
                    leg = item
                    writeLeg(outf, options, idveh, leg, leg.route[0].start_link, leg.route[0].end_link)
            if leg:
                untillist.append(leg.dep_time)
                vehicleslist.append(idveh)
                vehIndex = vehIndex+1
        untillist.append(lastAct.end_time if lastAct.end_time else options.defaultEnd)
        # write person
        if not options.carsOnly:
            vehIndex = 0
            outf.write('   <person id="%s" depart="%s">\n' % (person.id, depart))
            if attributes is not None:
                for attr in attributes.attribute:
                    outf.write('       <param key="%s" value="%s"/>\n' % (attr.attr_name, attr.getText()))

            lastLeg = None
            for item in plan.getChildList():
                if "act" in item.name:  # act or activity
                    if lastLeg is not None:
                        outf.write('       <ride lines="%s" to="%s"  />\n' % (vehicleslist[vehIndex], item.link))
                        vehIndex = vehIndex+1
                    outf.write('       <stop lane="%s_0" until="%s" actType="%s" />\n' %
                               (item.link, untillist[vehIndex], item.type))
                if item.name == "leg":
                    lastLeg = item
            outf.write('   </person>\n')
        persons.append((sumolib.miscutils.parseTime(depart), outf.getvalue()))

    persons.sort()
    with open(options.outfile, 'w') as outf:
        sumolib.writeXMLHeader(outf, "$Id: matsim_importPlans.py v1_3_1+0713-63b241ac79 michael.behrisch@dlr.de 2019-10-20 15:41:56 +0200 $", "routes")  # noqa
        for depart, xml in persons:
            outf.write(xml)
        outf.write('</routes>\n')
    outf.close()


if __name__ == "__main__":
    options = get_options(sys.argv)
    main(options)
