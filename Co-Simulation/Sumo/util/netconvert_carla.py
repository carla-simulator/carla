#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
"""
Script to generate sumo nets based on opendrive files. Internally, it uses netconvert to generate
the net and inserts, manually, the traffic light landmarks retrieved from the opendrive.
"""

# ==================================================================================================
# -- imports ---------------------------------------------------------------------------------------
# ==================================================================================================

import argparse
import bisect
import collections
import logging
import os
import shutil
import subprocess
import sys
import tempfile

import lxml.etree as ET  # pylint: disable=import-error

# ==================================================================================================
# -- find sumo modules -----------------------------------------------------------------------------
# ==================================================================================================

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")

# ==================================================================================================
# -- imports ---------------------------------------------------------------------------------------
# ==================================================================================================

import carla
import sumolib

# ==================================================================================================
# -- topology --------------------------------------------------------------------------------------
# ==================================================================================================


class SumoTopology(object):
    """
    This object holds the topology of a sumo net. Internally, the information is structured as
    follows:

        - topology: {
            (road_id, lane_id): [(successor_road_id, succesor_lane_id), ...], ...}
        - paths: {
            (road_id, lane_id): [
                ((in_road_id, in_lane_id), (out_road_id, out_lane_id)), ...
            ], ...}
        - odr2sumo_ids: {
            (odr_road_id, odr_lane_id): [(sumo_edge_id, sumo_lane_id), ...], ...}
    """
    def __init__(self, topology, paths, odr2sumo_ids):
        # Contains only standard roads.
        self._topology = topology
        # Contaions only roads that belong to a junction.
        self._paths = paths
        # Mapped ids between sumo and opendrive.
        self._odr2sumo_ids = odr2sumo_ids

    # http://sumo.sourceforge.net/userdoc/Networks/Import/OpenDRIVE.html#dealing_with_lane_sections
    def get_sumo_id(self, odr_road_id, odr_lane_id, s=0):
        """
        Returns the pair (sumo_edge_id, sumo_lane index) corresponding to the provided odr pair. The
        argument 's' allows selecting the better sumo edge when it has been split into different
        edges due to different odr lane sections.
        """
        if (odr_road_id, odr_lane_id) not in self._odr2sumo_ids:
            return None

        sumo_ids = list(self._odr2sumo_ids[(odr_road_id, odr_lane_id)])

        if (len(sumo_ids)) == 1:
            return sumo_ids[0]

        # The edge is split into different lane sections. We return the nearest edge based on the
        # s coordinate of the provided landmark.
        else:
            # Ensures that all the related sumo edges belongs to the same opendrive road but to
            # different lane sections.
            assert set([edge.split('.', 1)[0] for edge, lane_index in sumo_ids]) == 1

            s_coords = [float(edge.split('.', 1)[1]) for edge, lane_index in sumo_ids]

            s_coords, sumo_ids = zip(*sorted(zip(s_coords, sumo_ids)))
            index = bisect.bisect_left(s_coords, s, lo=1) - 1
            return sumo_ids[index]

    def is_junction(self, odr_road_id, odr_lane_id):
        """
        Checks whether the provided pair (odr_road_id, odr_lane_id) belongs to a junction.
        """
        return (odr_road_id, odr_lane_id) in self._paths

    def get_successors(self, sumo_edge_id, sumo_lane_index):
        """
        Returns the successors (standard roads) of the provided pair (sumo_edge_id, sumo_lane_index)
        """
        if self.is_junction(sumo_edge_id, sumo_lane_index):
            return []

        return list(self._topology.get((sumo_edge_id, sumo_lane_index), set()))

    def get_incoming(self, odr_road_id, odr_lane_id):
        """
        If the pair (odr_road_id, odr_lane_id) belongs to a junction, returns the incoming edges of
        the path. Otherwise, return and empty list.
        """
        if not self.is_junction(odr_road_id, odr_lane_id):
            return []

        result = set([(connection[0][0], connection[0][1])
                      for connection in self._paths[(odr_road_id, odr_lane_id)]])
        return list(result)

    def get_outgoing(self, odr_road_id, odr_lane_id):
        """
        If the pair (odr_road_id, odr_lane_id) belongs to a junction, returns the outgoing edges of
        the path. Otherwise, return and empty list.
        """
        if not self.is_junction(odr_road_id, odr_lane_id):
            return []

        result = set([(connection[1][0], connection[1][1])
                      for connection in self._paths[(odr_road_id, odr_lane_id)]])
        return list(result)

    def get_path_connectivity(self, odr_road_id, odr_lane_id):
        """
        Returns incoming and outgoing roads of the pair (odr_road_id, odr_lane_id). If the provided
        pair not belongs to a junction, returns an empty list.
        """
        return list(self._paths.get((odr_road_id, odr_lane_id), set()))


def build_topology(sumo_net):
    """
    Builds sumo topology.
    """
    # --------------------------
    # OpenDrive->Sumo mapped ids
    # --------------------------
    # Only takes into account standard roads.
    #
    #   odr2sumo_ids = {(odr_road_id, odr_lane_id) : [(sumo_edge_id, sumo_lane_index), ...], ...}
    odr2sumo_ids = {}
    for edge in sumo_net.getEdges():
        for lane in edge.getLanes():
            if lane.getParam('origId') is None:
                raise RuntimeError(
                    'Sumo lane {} does not have "origId" parameter. Make sure that the --output.original-names parameter is active when running netconvert.'
                    .format(lane.getID()))

            if len(lane.getParam('origId').split()) > 1:
                logging.warning('[Building topology] Sumo net contains joined opendrive roads.')

            for odr_id in lane.getParam('origId').split():
                odr_road_id, odr_lane_id = odr_id.split('_')
                if (odr_road_id, int(odr_lane_id)) not in odr2sumo_ids:
                    odr2sumo_ids[(odr_road_id, int(odr_lane_id))] = set()
                odr2sumo_ids[(odr_road_id, int(odr_lane_id))].add((edge.getID(), lane.getIndex()))

    # -----------
    # Connections
    # -----------
    #
    #   topology -- {(sumo_road_id, sumo_lane_index): [(sumo_road_id, sumo_lane_index), ...], ...}
    #   paths    -- {(odr_road_id, odr_lane_id): [
    #                   ((sumo_edge_id, sumo_lane_index), (sumo_edge_id, sumo_lane_index))
    #               ]}
    topology = {}
    paths = {}

    for from_edge in sumo_net.getEdges():
        for to_edge in sumo_net.getEdges():
            connections = from_edge.getConnections(to_edge)
            for connection in connections:
                from_ = connection.getFromLane()
                to_ = connection.getToLane()
                from_edge_id, from_lane_index = from_.getEdge().getID(), from_.getIndex()
                to_edge_id, to_lane_index = to_.getEdge().getID(), to_.getIndex()

                if (from_edge_id, from_lane_index) not in topology:
                    topology[(from_edge_id, from_lane_index)] = set()

                topology[(from_edge_id, from_lane_index)].add((to_edge_id, to_lane_index))

                # Checking if the connection is an opendrive path.
                conn_odr_ids = connection.getParam('origId')
                if conn_odr_ids is not None:
                    if len(conn_odr_ids.split()) > 1:
                        logging.warning(
                            '[Building topology] Sumo net contains joined opendrive paths.')

                    for odr_id in conn_odr_ids.split():

                        odr_road_id, odr_lane_id = odr_id.split('_')
                        if (odr_road_id, int(odr_lane_id)) not in paths:
                            paths[(odr_road_id, int(odr_lane_id))] = set()

                        paths[(odr_road_id, int(odr_lane_id))].add(
                            ((from_edge_id, from_lane_index), (to_edge_id, to_lane_index)))

    return SumoTopology(topology, paths, odr2sumo_ids)


# ==================================================================================================
# -- sumo definitions ------------------------------------------------------------------------------
# ==================================================================================================


class SumoTrafficLight(object):
    """
    SumoTrafficLight holds all the necessary data to define a traffic light in sumo:

        * connections (tlid, from_road, to_road, from_lane, to_lane, link_index).
        * phases (duration, state, min_dur, max_dur, nex, name).
        * parameters.
    """
    DEFAULT_DURATION_GREEN_PHASE = 42
    DEFAULT_DURATION_YELLOW_PHASE = 3
    DEFAULT_DURATION_RED_PHASE = 3

    Phase = collections.namedtuple('Phase', 'duration state min_dur max_dur next name')
    Connection = collections.namedtuple('Connection',
                                        'tlid from_road to_road from_lane to_lane link_index')

    def __init__(self, tlid, program_id='0', offset=0, tltype='static'):
        self.id = tlid
        self.program_id = program_id
        self.offset = offset
        self.type = tltype

        self.phases = []
        self.parameters = set()
        self.connections = set()

    @staticmethod
    def generate_tl_id(from_edge, to_edge):
        """
        Generates sumo traffic light id based on the junction connectivity.
        """
        return '{}:{}'.format(from_edge, to_edge)

    @staticmethod
    def generate_default_program(tl):
        """
        Generates a default program for the given sumo traffic light
        """
        incoming_roads = [connection.from_road for connection in tl.connections]
        for road in set(incoming_roads):
            phase_green = ['r'] * len(tl.connections)
            phase_yellow = ['r'] * len(tl.connections)
            phase_red = ['r'] * len(tl.connections)

            for connection in tl.connections:
                if connection.from_road == road:
                    phase_green[connection.link_index] = 'g'
                    phase_yellow[connection.link_index] = 'y'

            tl.add_phase(SumoTrafficLight.DEFAULT_DURATION_GREEN_PHASE, ''.join(phase_green))
            tl.add_phase(SumoTrafficLight.DEFAULT_DURATION_YELLOW_PHASE, ''.join(phase_yellow))
            tl.add_phase(SumoTrafficLight.DEFAULT_DURATION_RED_PHASE, ''.join(phase_red))

    def add_phase(self, duration, state, min_dur=-1, max_dur=-1, next_phase=None, name=''):
        """
        Adds a new phase.
        """
        self.phases.append(
            SumoTrafficLight.Phase(duration, state, min_dur, max_dur, next_phase, name))

    def add_parameter(self, key, value):
        """
        Adds a new parameter.
        """
        self.parameters.add((key, value))

    def add_connection(self, connection):
        """
        Adds a new connection.
        """
        self.connections.add(connection)

    def add_landmark(self,
                     landmark_id,
                     tlid,
                     from_road,
                     to_road,
                     from_lane,
                     to_lane,
                     link_index=-1):
        """
        Adds a new landmark.

        Returns True if the landmark is successfully included. Otherwise, returns False.
        """
        if link_index == -1:
            link_index = len(self.connections)

        def is_same_connection(c1, c2):
            return c1.from_road == c2.from_road and c1.to_road == c2.to_road and \
                   c1.from_lane == c2.from_lane and c1.to_lane == c2.to_lane

        connection = SumoTrafficLight.Connection(tlid, from_road, to_road, from_lane, to_lane,
                                                 link_index)
        if any([is_same_connection(connection, c) for c in self.connections]):
            logging.warning(
                'Different landmarks controlling the same connection. Only one will be included.')
            return False

        self.add_connection(connection)
        self.add_parameter(link_index, landmark_id)
        return True

    def to_xml(self):
        info = {
            'id': self.id,
            'type': self.type,
            'programID': self.program_id,
            'offset': str(self.offset)
        }

        xml_tag = ET.Element('tlLogic', info)
        for phase in self.phases:
            ET.SubElement(xml_tag, 'phase', {'state': phase.state, 'duration': str(phase.duration)})
        for parameter in sorted(self.parameters, key=lambda x: x[0]):
            ET.SubElement(xml_tag, 'param', {
                'key': 'linkSignalID:' + str(parameter[0]),
                'value': str(parameter[1])
            })

        return xml_tag


# ==================================================================================================
# -- main ------------------------------------------------------------------------------------------
# ==================================================================================================


def _netconvert_carla_impl(xodr_file, output, tmpdir, guess_tls=False):
    """
    Implements netconvert carla.
    """
    # ----------
    # netconvert
    # ----------
    basename = os.path.splitext(os.path.basename(xodr_file))[0]
    tmp_sumo_net = os.path.join(tmpdir, basename + '.net.xml')

    try:
        basedir = os.path.dirname(os.path.realpath(__file__))
        result = subprocess.call(['netconvert',
            '--opendrive', xodr_file,
            '--output-file', tmp_sumo_net,
            '--geometry.min-radius.fix',
            '--geometry.remove',
            '--opendrive.curve-resolution', '1',
            '--opendrive.import-all-lanes',
            '--type-files', os.path.join(basedir, 'data/opendrive_netconvert.typ.xml'),
            # Necessary to link odr and sumo ids.
            '--output.original-names',
            # Discard loading traffic lights as them will be inserted manually afterwards.
            '--tls.discard-loaded', 'true',
        ])
    except subprocess.CalledProcessError:
        raise RuntimeError('There was an error when executing netconvert.')
    else:
        if result != 0:
            raise RuntimeError('There was an error when executing netconvert.')

    # --------
    # Sumo net
    # --------
    sumo_net = sumolib.net.readNet(tmp_sumo_net)
    sumo_topology = build_topology(sumo_net)

    # ---------
    # Carla map
    # ---------
    with open(xodr_file, 'r') as f:
        carla_map = carla.Map('netconvert', str(f.read()))

    # ---------
    # Landmarks
    # ---------
    tls = {}  # {tlsid: SumoTrafficLight}

    landmarks = carla_map.get_all_landmarks_of_type('1000001')
    for landmark in landmarks:
        if landmark.name == '':
            # This is a workaround to avoid adding traffic lights without controllers.
            logging.warning('Landmark %s has not a valid name.', landmark.name)
            continue

        road_id = str(landmark.road_id)
        for from_lane, to_lane in landmark.get_lane_validities():
            for lane_id in range(from_lane, to_lane + 1):
                if lane_id == 0:
                    continue

                wp = carla_map.get_waypoint_xodr(landmark.road_id, lane_id, landmark.s)
                if wp is None:
                    logging.warning(
                        'Could not find waypoint for landmark {} (road_id: {}, lane_id: {}, s:{}'.
                        format(landmark.id, landmark.road_id, lane_id, landmark.s))
                    continue

                # When the landmark belongs to a junction, we place te traffic light at the
                # entrance of the junction.
                if wp.is_junction and sumo_topology.is_junction(road_id, lane_id):
                    tlid = str(wp.get_junction().id)
                    if tlid not in tls:
                        tls[tlid] = SumoTrafficLight(tlid)
                    tl = tls[tlid]

                    if guess_tls:
                        for from_edge, from_lane in sumo_topology.get_incoming(road_id, lane_id):
                            successors = sumo_topology.get_successors(from_edge, from_lane)
                            for to_edge, to_lane in successors:
                                tl.add_landmark(landmark.id, tl.id, from_edge, to_edge, from_lane,
                                                to_lane)

                    else:
                        connections = sumo_topology.get_path_connectivity(road_id, lane_id)
                        for from_, to_ in connections:
                            from_edge, from_lane = from_
                            to_edge, to_lane = to_

                            tl.add_landmark(landmark.id, tl.id, from_edge, to_edge, from_lane,
                                            to_lane)

                # When the landmarks does not belong to a junction (i.e., belongs to a std road),
                # we place the traffic light between that std road and its successor.
                elif not wp.is_junction and not sumo_topology.is_junction(road_id, lane_id):
                    from_edge, from_lane = sumo_topology.get_sumo_id(road_id, lane_id, landmark.s)

                    for to_edge, to_lane in sumo_topology.get_successors(from_edge, from_lane):
                        tlid = SumoTrafficLight.generate_tl_id(from_edge, to_edge)
                        if tlid not in tls:
                            tls[tlid] = SumoTrafficLight(tlid)
                        tl = tls[tlid]

                        tl.add_landmark(landmark.id, tl.id, from_edge, to_edge, from_lane, to_lane)

                else:
                    logging.warning('Landmark %s could not be added.', landmark.id)

    # ---------------
    # Modify sumo net
    # ---------------
    parser = ET.XMLParser(remove_blank_text=True)
    tree = ET.parse(tmp_sumo_net, parser)
    root = tree.getroot()

    for tl in tls.values():
        SumoTrafficLight.generate_default_program(tl)
        edges_tags = tree.xpath('//edge')
        if not edges_tags:
            raise RuntimeError('No edges found in sumo net.')
        root.insert(root.index(edges_tags[-1]) + 1, tl.to_xml())

        for connection in tl.connections:
            tags = tree.xpath(
                '//connection[@from="{}" and @to="{}" and @fromLane="{}" and @toLane="{}"]'.format(
                    connection.from_road, connection.to_road, connection.from_lane,
                    connection.to_lane))

            if tags:
                if len(tags) > 1:
                    logging.warning(
                        'Found repeated connections from={} to={} fromLane={} toLane={}.'.format(
                            connection.from_road, connection.to_road, connection.from_lane,
                            connection.to_lane))

                tags[0].set('tl', str(connection.tlid))
                tags[0].set('linkIndex', str(connection.link_index))
            else:
                logging.warning('Not found connection from={} to={} fromLane={} toLane={}.'.format(
                    connection.from_road, connection.to_road, connection.from_lane,
                    connection.to_lane))

    tree.write(output, pretty_print=True, encoding='UTF-8', xml_declaration=True)


def netconvert_carla(xodr_file, output, guess_tls=False):
    """
    Generates sumo net.

        :param xodr_file: opendrive file (*.xodr)
        :param output: output file (*.net.xml)
        :param guess_tls: guess traffic lights at intersections.
        :returns: path to the generated sumo net.
    """
    try:
        tmpdir = tempfile.mkdtemp()
        _netconvert_carla_impl(xodr_file, output, tmpdir, guess_tls)

    finally:
        if os.path.exists(tmpdir):
            shutil.rmtree(tmpdir)


if __name__ == '__main__':
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument('xodr_file', help='opendrive file (*.xodr')
    argparser.add_argument('--output',
                           '-o',
                           default='net.net.xml',
                           type=str,
                           help='output file (default: net.net.xml)')
    argparser.add_argument('--guess-tls',
                           action='store_true',
                           help='guess traffic lights at intersections (default: False)')
    args = argparser.parse_args()

    netconvert_carla(args.xodr_file, args.output, args.guess_tls)
