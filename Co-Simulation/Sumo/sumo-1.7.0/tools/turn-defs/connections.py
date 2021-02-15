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

# @file    connections.py
# @author  Karol Stosiek
# @author  Michael Behrisch
# @date    2011-10-26

from __future__ import absolute_import

import logging
import os
import sys

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")

import sumolib  # noqa


LOGGER = logging.getLogger(__name__)


class UniformDestinationWeightCalculator():

    """Calculates weight for each destination from given lane,
       redistributing weights uniformly: first redistributes the total
       traffic among incoming lanes uniformly and then divides the total
       traffic for a lane among all of the destination lanes.

       For example, assume we  have a junction with one incoming road In1
       with 3 incoming lanes and 2 outgoing roads Out1 and Out2,
       having 2 and 3 outgoing lanes, respectively.

                   ___________________
            lane 0 _______     _______ lane 0
       Out2 lane 1 _______     _______ lane 1  In1
            lane 2 _______     _______ lane 2
                          | | |
                          | | |
                          | | |
                      lane 0 1
                           Out1

      Assume that lanes are connected as follows:
      - In1, lane 0 is connected to Out2, lane 0
      - In1, lane 1 is connected to Out2, lane 1
      - In1, lane 2 is connected to Out2, lane 2

      - In1, lane 1 is connected to Out1, lane 0
      - In1, lane 2 is connected to Out1, lane 1

       This weight calculator will redistribute weights as follows:

       1. Distribute the incoming traffic on In1 uniformly on lanes 0, 1 and 2:
          as a result, each lane has been assigned 33,(3)% of the traffic.
       2. Since In1, lane 0 is connected to Out2, lane 0, whole traffic
          from In1, lane 0 is redirected to Out2, lane 0, which makes 33,(3)%
          of the total incoming traffic from In1.
       3. Since In1, lane 1 is connected both to Out2, lane 1 and Out1, lane 0,
          the traffic on that lane (that is, 33,(3)% of the total traffic) is
          spread uniformly among these two lanes, resulting in 16,(6)% of the total
          traffic for each destination lane.
       4. Similarly, In2, lane 2's traffic is spread uniformly among Out2, lane 2
          and Out 1, lane 1, resulting in 16,(6)% of the total traffic for each
          destination lane.

       This, if we ask the calculator for weight assigned to In1, lane 0, we get
       33,(3) as a result; if we ask for weight assigned to In1, lane 1, we get
       16,(6) as a result.

       Note: If you want to provide your own weight calculator (based on Gaussian
       distribution, for example), simply provide a class with calculate_weight
       method. The method's signature may need to be changed in that case. """

    # pylint: disable=R0903

    logger = logging.getLogger(__name__)

    def __init__(self):
        pass

    def calculate_weight(self,
                         source_lane_no,
                         source_total_lanes,
                         destination_total_lanes):
        """ Calculates the weight assigned to a single destination
            from given lane in an uniform way. See class docs
            for explanation. """

        lane_weight = 100.0 / source_total_lanes
        destination_weight = lane_weight / destination_total_lanes

        self.logger.debug("Destination weight for lane %s/%s connected to %s "
                          "destinations: %f" %
                          (str(source_lane_no), str(source_total_lanes),
                           str(destination_total_lanes), destination_weight))

        return destination_weight


class Connections:

    """ Represents all of the connections in the network we have read. """

    logger = logging.getLogger(__name__)

    def __init__(self,
                 calculator=UniformDestinationWeightCalculator()):
        """ Constructor. Allows providing own destination weight calculator,
            which defaults to UniformDestinationWeightCalculator if not "
            provided. """

        self.connections_map = {}
        self.destination_weight_calculator = calculator

    def add(self,
            source,
            source_lane,
            destination):
        """ Adds a connection. If a connection is readded,
            a warning is issued. """

        self.logger.debug("Adding connection %s (%s) -> %s" %
                          (str(source), str(source_lane), str(destination)))

        if source not in self.connections_map:
            self.logger.debug("Created new mapping for %s" %
                              (str(source)))
            self.connections_map[source] = {}

        if source_lane not in self.connections_map[source]:
            self.logger.debug("Created new mapping for %s / %s" %
                              (str(source), str(source_lane)))
            self.connections_map[source][source_lane] = set()

        if destination in self.connections_map[source][source_lane]:
            self.logger.warning("Destination for %s (lane %s) readded: %s"
                                % (source, source_lane, destination))

        self.connections_map[source][source_lane].add(destination)

    def get_sources(self):
        """ Returns all of the incoming edges that this connections collection
            contains. Incoming edges are sorted alphabetically
            in ascending order. """

        sources = sorted(self.connections_map.keys())
        return sources

    def get_lanes(self, source):
        """ Returns all lanes that have connections for the given edge.
            The connection_source must have been added before. """

        return self.connections_map[source].keys()

    def get_destinations(self, source, source_lane):
        """ Returns all possible destinations that are achievable from given
            lane on given edge. The connection_source
            and connection_source_lane must have been added before. """

        return self.connections_map[source][source_lane]

    def calculate_destination_weight(self,
                                     source,
                                     source_lane,
                                     destination):
        """ Calculates weight assigned to the given destination using
            weight calculator provided in class' constructor.
            The connection_source, connection_source_lane
            and connection_destination must have been added before."""

        weight = self.destination_weight_calculator.calculate_weight(
            source_lane,
            len(self.get_lanes(source)),
            len(self.get_destinations(source, source_lane)))

        self.logger.debug("Destination weight for connection "
                          "%s (%s) -> %s: %f" %
                          (str(source), str(source_lane), str(destination), weight))

        return weight


def from_stream(input_connections):
    """ Constructs Connections object from connections defined in the given
        stream. The input_connections argument may be a filename or an opened
        file. """

    LOGGER.info("Reading connections from input stream")

    connections = Connections()
    for xml_connection in sumolib.xml.parse(input_connections, "connection"):
        connections.add(xml_connection.attr_from,
                        xml_connection.fromLane,
                        xml_connection.to)

    return connections
