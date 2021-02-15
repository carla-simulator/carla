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

# @file    generateTLSE1Detectors.py
# @author  Daniel Krajzewicz
# @author  Karol Stosiek
# @author  Michael Behrisch
# @date    2011-10-07

from __future__ import absolute_import
from __future__ import print_function

import logging
import optparse
import os
import sys
from generateTLSE2Detectors import adjust_detector_position

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import sumolib  # noqa


if __name__ == "__main__":
    # pylint: disable-msg=C0103

    logging.basicConfig(level="INFO")

    option_parser = optparse.OptionParser()
    option_parser.add_option("-n", "--net-file",
                             dest="net_file",
                             help="Network file to work with. Mandatory.",
                             type="string")
    option_parser.add_option("-d", "--distance-to-TLS",
                             dest="requested_distance_to_tls",
                             help="Distance of the detector to the traffic "
                             "light in meters. Defaults to 0.1m.",
                             type="float",
                             default=.1)
    option_parser.add_option("-f", "--frequency",
                             dest="frequency",
                             help="Detector's frequency. Defaults to 60.",
                             type="int",
                             default=60)
    option_parser.add_option("-o", "--output",
                             dest="output",
                             help="The name of the file to write the detector "
                             "definitions into. Defaults to e1.add.xml.",
                             type="string",
                             default="e1.add.xml")
    option_parser.add_option("-r", "--results-file",
                             dest="results",
                             help="The name of the file the detectors write "
                             "their output into. Defaults to e1output.xml.",
                             type="string",
                             default="e1output.xml")
    option_parser.set_usage("generateTLSE1Detectors.py -n example.net.xml -d .1 -f 60")

    (options, args) = option_parser.parse_args()
    if not options.net_file:
        print("Missing arguments")
        option_parser.print_help()
        exit()

    logging.info("Reading net...")
    net = sumolib.net.readNet(options.net_file)

    logging.info("Generating detectors...")
    detectors_xml = sumolib.xml.create_document("additional")
    lanes_with_detectors = set()
    for tls in net._tlss:
        for connection in tls._connections:
            lane = connection[0]
            lane_length = lane.getLength()
            lane_id = lane.getID()

            logging.debug("Creating detector for lane %s" % (str(lane_id)))

            if lane_id in lanes_with_detectors:
                logging.warning("Detector for lane %s already generated" % lane_id)
                continue

            lanes_with_detectors.add(lane_id)

            final_detector_position = adjust_detector_position(
                0,
                options.requested_distance_to_tls,
                lane_length)

            detector_xml = detectors_xml.addChild("e1Detector")
            detector_xml.setAttribute("file", options.results)
            detector_xml.setAttribute("freq", str(options.frequency))
            detector_xml.setAttribute("friendlyPos", "x")
            detector_xml.setAttribute("id", "e1det_" + str(lane_id))
            detector_xml.setAttribute("lane", str(lane_id))
            detector_xml.setAttribute("pos", str(final_detector_position))

    detector_file = open(options.output, 'w')
    detector_file.write(detectors_xml.toXML())
    detector_file.close()

    logging.info("%d e1 detectors generated!" % len(lanes_with_detectors))
