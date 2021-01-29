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

# @file    generateMeanDataDefinitions.py
# @author  Karol Stosiek
# @author  Michael Behrisch
# @date    2011-10-25

from __future__ import absolute_import

import xml.dom.minidom

import logging
import optparse
import sys


def generate_mean_data_xml(detectors_xml,
                           detectors_type,
                           detectors_frequency,
                           detectors_suffix,
                           detectors_output_type):
    """ Generates mean data definitions in XML format.
        - detectors_xml is the detectors XML read by xml.dom.minidom.
        - detectors_type is one of the supported detectors: 'e1', 'e2' or 'e3'
        - detectors_frequency is either an empty string or a positive integer.
        - detectors_suffix is the suffix appended to each detector id to form
            a detector's aggregated results filename. It's appended with .xml
            string.
        """

    meandata_xml = xml.dom.minidom.getDOMImplementation().createDocument(
        None, 'additional', None)

    for detector_xml in detectors_xml.getElementsByTagName(
            detectors_type + "Detector"):

        detector_id = detector_xml.getAttribute('id')
        meandata_element = meandata_xml.createElement(detectors_output_type)
        meandata_element.setAttribute("id", detector_id)
        meandata_element.setAttribute("freq", str(detectors_frequency))
        meandata_element.setAttribute("file", detector_id +
                                      detectors_suffix + ".xml")
        meandata_xml.documentElement.appendChild(meandata_element)

    return meandata_xml


if __name__ == "__main__":
    # pylint: disable-msg=C0103

    def get_detector_file(provided_options):
        """ Returns validated detector file name located in
            provided_options. Exits, if the provided
            detector file is invalid (None or empty). """

        if (provided_options.detector_file is None or
                provided_options.detector_file == ""):
            logging.fatal("Invalid input file. \n" +
                          option_parser.format_help())
            exit()
        return xml.dom.minidom.parse(provided_options.detector_file)

    def get_detector_type(provided_options):
        """ Returns validated detector type located in provided_options.
            Checks if the detector type is one of e1, e2 or e3. """

        if provided_options.detector_type not in ('e1', 'e2', 'e3'):
            logging.fatal("Invalid detector type.\n" +
                          option_parser.format_help())
            exit()
        return provided_options.detector_type

    def get_detector_frequency(provided_options):
        """ Returns validated detector frequency located in provided_options.
            Validated frequency is either an empty string or is a positive
            integer. """

        if provided_options.frequency != "":
            try:
                frequency = int(provided_options.frequency)
                if frequency < 0:
                    raise ValueError
                return frequency
            except ValueError:
                logging.fatal("Invalid time range length specified.\n" +
                              option_parser.format_help())
                exit()
        return ""

    def get_detector_suffix(provided_options):
        """ Returns detector suffix located in provided_options. """

        return provided_options.output_suffix

    def get_detector_output_type(provided_options):
        """If provided_options indicated that edge-based traffic should be
           created, then returns \"edgeData\"; returns \"laneData\" otherwise.
        """

        if provided_options.edge_based_dump:
            return "edgeData"
        else:
            return "laneData"

    logging.basicConfig()

    option_parser = optparse.OptionParser()
    option_parser.add_option("-d", "--detector-file",
                             help="Input detector FILE",
                             dest="detector_file",
                             type="string")
    option_parser.add_option("-t", "--detector-type",
                             help="Type of detectors defined in the input. "
                                  "Allowed values: e1, e2, e3. Mandatory.",
                             dest="detector_type",
                             type="string")
    option_parser.add_option("-f", "--frequency",
                             help="The aggregation period the values the "
                                  "detector collects shall be summed up. "
                                  "If not given, the whole time interval "
                                  "from begin to end is aggregated, which is "
                                  "the default. If specified, must be a "
                                  "positive integer (seconds) representing "
                                  "time range length.",
                             dest="frequency",
                             type="string",
                             default="")
    option_parser.add_option("-l", "--lane-based-dump",
                             help="Generate lane based dump instead of "
                                  "edge-based dump.",
                             dest="edge_based_dump",
                             action="store_false")
    option_parser.add_option("-e", "--edge-based-dump",
                             help="Generate edge-based dump instead of "
                                  "lane-based dump. This is the default.",
                             dest="edge_based_dump",
                             action="store_true",
                             default=True)
    option_parser.add_option("-p", "--output-suffix",
                             help="Suffix to append to aggregated detector "
                                  "output. For each detector, the detector's "
                                  "aggregated results file with have the name "
                                  "build from the detector's ID and this "
                                  "suffix, with '.xml' extension. Defaults "
                                  "to -results-aggregated.",
                             dest="output_suffix",
                             default="-results-aggregated")
    option_parser.add_option("-o", "--output",
                             help="Output to write the mean data definition "
                                  "to. Defaults to stdout.",
                             dest="output",
                             type="string")

    (options, args) = option_parser.parse_args()

    output = sys.stdout
    if options.output is not None:
        output = open(options.output, "w")

    output.write(
        generate_mean_data_xml(
            get_detector_file(options),
            get_detector_type(options),
            get_detector_frequency(options),
            get_detector_suffix(options),
            get_detector_output_type(options)).toprettyxml())

    output.close()
