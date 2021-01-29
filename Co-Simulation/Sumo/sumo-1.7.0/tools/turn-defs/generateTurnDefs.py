#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2011-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    generateTurnDefs.py
# @author  Karol Stosiek
# @date    2011-10-26

from __future__ import absolute_import

import os
import sys
import logging
import optparse
sys.path.append(os.path.join(os.environ["SUMO_HOME"], "tools"))
import connections  # noqa
import turndefinitions  # noqa

LOGGER = logging.getLogger(__name__)

if __name__ == "__main__":

    # pylint: disable-msg=C0103

    logging.basicConfig(level="INFO")

    option_parser = optparse.OptionParser()
    option_parser.add_option("-c", "--connections-file",
                             dest="connections_file",
                             help="Read connections defined in CONNECTIONS_FILE. Mandatory.",
                             metavar="CONNECTIONS_FILE")
    option_parser.add_option("-t", "--turn-definitions-file",
                             dest="turn_definitions_file",
                             help="Write the resulting turn definitions to TURN_DEFINITIONS_FILE. "
                                  "Mandatory.",
                             metavar="TURN_DEFINITIONS_FILE")
    option_parser.add_option("-b", "--begin",
                             help="Generate turn definitions for interval starting at BEGIN seconds. "
                                  "Defaults to 0.",
                             default="0")
    option_parser.add_option("-e", "--end",
                             help="Generate turn definitions for interval ending at END seconds. "
                                  "Defaults to 3600.",
                             default="3600")

    (options, args) = option_parser.parse_args()

    if options.connections_file is None:
        LOGGER.fatal("Missing CONNECTIONS_FILE.\n" +
                     option_parser.format_help())
        exit()

    if options.turn_definitions_file is None:
        LOGGER.fatal("Missing TURN_DEFINITIONS_FILE.\n" +
                     option_parser.format_help())
        exit()

    connections_file = open(options.connections_file, "r")
    turn_definitions_file = open(options.turn_definitions_file, "w")

    connections = connections.from_stream(connections_file)
    turn_definitions = turndefinitions.from_connections(connections)
    turn_definitions_xml = turndefinitions.to_xml(turn_definitions,
                                                  options.begin,
                                                  options.end)
    turn_definitions_file.write(turn_definitions_xml)

    connections_file.close()
    turn_definitions_file.close()
