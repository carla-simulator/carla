# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2016-2020 German Aerospace Center (DLR) and others.
# SUMOPy module
# Copyright (C) 2012-2017 University of Bologna - DICAM
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    netconvert.py
# @author  Joerg Schweizer
# @date

import os
import subprocess
import sys

import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
import agilepy.lib_base.xmlman as xm

from agilepy.lib_base.processes import Process, CmlMixin
from agilepy.lib_base.misc import filepathlist_to_filepathstring, filepathstring_to_filepathlist


class NetConvertMixin(CmlMixin, Process):
    def init_common_netconvert(self, ident, net, netfilepath=None,
                               name='Netconvert',
                               info='Generates and transforms SUMO networks from different formats',
                               logger=None,
                               cml='netconvert'):

        # Default values:
        ## proj = '',
        ##             is_import_elevation_osm = False,
        ##             typefilepath = None,
        # roadtypes = 'ordinary roads+bikeways',#ordinary roads+bikeways+footpath
        ##             n_lanes_default = 0,
        ##             edgespeed_default = 13.9,
        ##             priority_default = -1,
        ##             is_remove_isolated_edges = True,
        ##             factor_edge_speed = 1.0,
        ##             is_guess_sidewalks = False,
        ##             edgespeed_min_sidewalks = 5.8,
        ##             edgespeed_max_sidewalks = 13.89,
        ##             is_guess_sidewalks_from_permission = False,
        ##             width_sidewalks_default = 2.0,
        ##             is_guess_crossings = False,
        ##             edgespeed_max_crossings = 13.89,
        ##             is_join_nodes = True,
        ##             dist_join_nodes = 15.0,
        ##             is_keep_nodes_clear = True,
        ##             is_keep_nodes_unregulated = False,
        ##             is_guess_tls = False,
        ##             is_join_tls = False,
        ##             joindist_tls = 20.0,
        ##             is_uncontrolled_within_tls = False,
        ##             is_guess_signals_tls = False,
        ##             dist_guess_signal_tls = 20.0,
        # time_green_tls = 31,
        ##             time_yellow_tls = -1,
        ##             accel_min_yellow_tls = -1.0,
        ##             is_no_turnarounds = False,
        ##             is_no_turnarounds_tls = False,
        ##             is_check_lane_foes = False,
        ##             is_roundabouts_guess = True,
        ##             is_check_lane_foes_roundabout = False,
        ##             is_no_left_connections = False,
        ##             is_geometry_split = False,
        ##             is_geometry_remove = True,
        ##             length_max_segment = -1.0,
        ##             dist_min_geometry = -1.0,
        ##             is_guess_ramps = True,
        ##             rampspeed_max = -1,
        ##             highwayspeed_min = 21.9444,
        ##             ramplength = 100,
        ##             is_no_split_ramps = False,
        ##             #
        ##             is_clean_nodes = False,

        self._init_common(ident, name=name,
                          parent=net,
                          logger=logger,
                          info=info,
                          )
        self.init_cml(cml)  # pass main shell command

        if netfilepath is None:
            netfilepath = net.get_filepath()

        self.add_option('netfilepath', netfilepath,
                        groupnames=[],  # ['_private'],#
                        cml='--output-file',
                        perm='r',
                        name='Net file',
                        wildcards='Net XML files (*.net.xml)|*.net.xml',
                        metatype='filepath',
                        info='SUMO Net file in XML format.',
                        )

    def init_options_edge(self, **kwargs):
        self.add_option('n_lanes_default', kwargs.get('n_lanes_default', 1),
                        groupnames=['options', 'edges'],
                        cml='--default.lanenumber',
                        perm='rw',
                        name='Default lanenumber',
                        info='The default number of lanes in an edge.',
                        is_enabled=lambda self: self.n_lanes_default > 0,
                        )

        self.add_option('width_lanes_default', kwargs.get('width_lanes_default', 3.25),
                        groupnames=['options', 'edges'],
                        cml='--default.lanewidth',
                        perm='rw',
                        name='Default lanewidth',
                        unit='m',
                        info='The default number of lanes in an edge.',
                        is_enabled=lambda self: self.n_lanes_default > 0,
                        )

        self.add_option('edgespeed_default', kwargs.get('edgespeed_default', 13.9),
                        groupnames=['options', 'edges'],
                        cml='--default.speed',
                        perm='rw',
                        unit='m/s',
                        name='Default edge speed',
                        info='The default speed on an edge.',
                        is_enabled=lambda self: self.edgespeed_default > 0,
                        )

        choices_priority = {}
        for i in range(11):
            choices_priority[str(i)] = i
        choices_priority['auto'] = -1

        self.add_option('priority_default', kwargs.get('priority_default', choices_priority['auto']),
                        groupnames=['options', 'edges'],
                        cml='--default.priority',
                        choices=choices_priority,
                        perm='rw',
                        name='Default priority',
                        info='The default priority of an edge. Value of-1 means automatic assignment.',
                        is_enabled=lambda self: self.priority_default > 0,
                        )

    def init_options_edgetype(self, **kwargs):
        typefilepath = kwargs.get('typefilepath', None)
        if typefilepath is None:
            typefilepath = os.path.abspath(os.path.join(os.path.dirname(
                os.path.abspath(__file__)), '..', '..', 'typemap', 'osmNetconvert.typ.xml'))
        self.add_option('typefilepath', typefilepath,
                        groupnames=['options'],
                        cml='--type-files',
                        perm='rw',
                        name='Type files',
                        wildcards='Typemap XML files (*.typ.xml)|*.typ.xml',
                        metatype='filepaths',
                        info="""Typemap XML files. In these file, 
                            OSM road types are mapped to edge and lane parameters such as width, 
                            speeds, etc. These parameters are used as defaults in absents of explicit OSM attributes.
                            Use osmNetconvert.typ.xml as a base and additional type file to meet specific needs.""",
                        )

        modesset = set(self.parent.modes.names.get_value())
        modesset_pt_rail = set(["rail_urban", "rail", "rail_electric"])
        modesset_pt_road = set(["bus", "taxi", "coach", "tram"])
        modesset_motorized = set(["private", "passenger", "emergency", "authority", "army",
                                  "vip", "hov", "motorcycle", "moped", "evehicle", "delivery", "truck"])

        roadtypes_to_disallowed_vtypes = {"roads for individual transport": ','.join(modesset.difference(["passenger", ])),
                                          "ordinary roads": ','.join(modesset.difference(modesset_pt_road | modesset_motorized)),
                                          "ordinary roads+bikeways": ','.join(modesset.difference(modesset_pt_road | modesset_motorized | set(['bicycle']))),
                                          "ordinary roads+bikeways+footpath": ','.join(modesset.difference(modesset_pt_road | modesset_motorized | set(['bicycle']) | set(['pedestrian']))),
                                          "ordinary roads+rails": ','.join(modesset.difference(modesset_pt_road | modesset_motorized | modesset_pt_rail)),
                                          "ordinary roads+rails+bikeways": ','.join(modesset.difference(modesset_pt_road | modesset_motorized | modesset_pt_rail | set(['bicycle']))),
                                          "ordinary roads+rails+bikeways+footpath": ','.join(modesset.difference(modesset_pt_road | modesset_motorized | modesset_pt_rail | set(['bicycle']) | set(['pedestrian']))),
                                          "all ways": ""
                                          }
        # print '  access_to_vtypes=',access_to_vtypes
        self.add_option('select_edges_by_access', roadtypes_to_disallowed_vtypes[kwargs.get('roadtypes', 'ordinary roads+bikeways')],
                        groupnames=['options', 'edges'],
                        cml='--remove-edges.by-vclass',
                        choices=roadtypes_to_disallowed_vtypes,
                        perm='rw',
                        name='Keep edge with acces',
                        info='Imports all edges with the given vehicle access patterns.',
                        is_enabled=lambda self: self.select_edges_by_access != "",
                        )

        self.add_option('is_remove_isolated_edges', kwargs.get('is_remove_isolated_edges', True),
                        groupnames=['options', 'edges'],
                        cml='--remove-edges.isolated',
                        perm='rw',
                        name='Remove isolated edges',
                        info='Remove isolated edges.',
                        )
        # --edges.join <BOOL> 	Merges edges whch connect the same nodes and are close to each other (recommended for VISSIM import); default: false
        # --speed.offset <FLOAT> 	Modifies all edge speeds by adding FLOAT; default: 0

        self.add_option('factor_edge_speed', kwargs.get('factor_edge_speed', 1.0),
                        groupnames=['options', 'edges'],
                        cml='--speed.factor',
                        perm='rw',
                        name='Edge speed factor',
                        info='Modifies all edge speeds by multiplying with edge speed factor.',
                        is_enabled=lambda self: self.factor_edge_speed == 1.0,
                        )
        # --keep-edges.min-speed <FLOAT> 	Only keep edges with speed in meters/second > FLOAT

    def init_options_tls(self, **kwargs):
        self.add_option('is_guess_tls', kwargs.get('is_guess_tls', False),
                        groupnames=['options', 'traffic lights'],
                        cml='--tls.guess',
                        perm='rw',
                        name='TLS-guessing',
                        info='Turns on TLS guessing.',
                        )

        self.add_option('is_guess_tls', kwargs.get('is_guess_tls', False),
                        groupnames=['options', 'traffic lights'],
                        cml='--tls.guess',
                        perm='rw',
                        name='TLS-guessing',
                        info='Turns on TLS guessing.',
                        )

        # now same as is_join_tls
        # self.add_option('is_guess_join_tls',True,
        #                groupnames = ['options','traffic lights'],#
        #                cml = '--tls-guess.joining',
        #                perm='rw',
        #                name = 'TLS-guess joining',
        #                info = 'Includes node clusters into guess.',
        #                )
        self.add_option('is_join_tls', kwargs.get('is_join_tls', False),
                        groupnames=['options', 'traffic lights'],
                        cml='--tls.join',
                        perm='rw',
                        name='TLS-joining',
                        info='Tries to cluster tls-controlled nodes.',
                        is_enabled=lambda self: self.is_guess_tls,
                        )
        self.add_option('joindist_tls', kwargs.get('joindist_tls', 20),
                        groupnames=['options', 'traffic lights'],
                        cml='--tls.join-dist',
                        perm='rw',
                        unit='m',
                        name='TLS-join dist.',
                        info='Determines the maximal distance for joining traffic lights (defaults to 20)',
                        is_enabled=lambda self: self.is_guess_tls & self.is_join_tls,
                        )
        self.add_option('is_uncontrolled_within_tls', kwargs.get('is_uncontrolled_within_tls', False),
                        groupnames=['options', 'traffic lights'],
                        cml='--tls.uncontrolled-within',
                        perm='rw',
                        name='Uncontrolled within TLS.',
                        info='Do not control edges that lie fully within a joined traffic light (use with care, may cause import problems). This may cause collisions but allows old traffic light plans to be used.',
                        is_enabled=lambda self: self.is_guess_tls,
                        )

        #tlschoices = {"static": 1,"actuated": 2,"none":0}

        self.add_option('type_tl', kwargs.get('type_tl', 'actuated'),
                        groupnames=['options', 'traffic lights'],
                        choices=["static", "actuated"],
                        cml='--tls.default-type',
                        perm='rw',
                        name='TL type',
                        info='The type of the traffic light program (fixed phase durations, phase prolongation based on time gaps between vehicles (actuated), or on accumulated time loss of queued vehicles (delay_based)',
                        #is_enabled = lambda self: self.is_guess_tls,
                        )

        self.add_option('time_all_red_tls', 2,
                        groupnames=['options', 'traffic lights'],
                        cml='--tls.allred.time',
                        perm='rw',
                        unit='s',
                        name='All red time',
                        info='Time of phase when all signals are red.',

                        )

        self.add_option('time_min_duration_actuated', 10,
                        groupnames=['options', 'traffic lights'],
                        cml='--tls.min-dur',
                        perm='rw',
                        unit='s',
                        name='Min duration',
                        info='Minimum duration of green phase in case of actuated TLS.',
                        is_enabled=lambda self: self.type_tl == "actuated",  # actuated
                        )

        self.add_option('time_max_duration_actuated', 40,
                        groupnames=['options', 'traffic lights'],
                        cml='--tls.max-dur',
                        perm='rw',
                        unit='s',
                        name='Max duration',
                        info='Maximum duration of green phase in case of actuated TLS.',
                        is_enabled=lambda self: self.type_tl == "actuated",  # actuated
                        )

        self.add_option('time_cycle_tls', 90,
                        groupnames=['options', 'traffic lights'],
                        cml='--tls.cycle.time',
                        perm='rw',
                        unit='s',
                        name='Cycle time',
                        info='Cycle time of traffic light.',
                        )

        self.add_option('time_cycle_tls', 90,
                        groupnames=['options', 'traffic lights'],
                        cml='--tls.cycle.time',
                        perm='rw',
                        unit='s',
                        name='Cycle time',
                        info='Cycle time of traffic light.',
                        #is_enabled = lambda self: self.is_guess_tls,
                        )

        # self.add_option('time_green_tls',31,
        #                groupnames = ['options','traffic lights'],#
        #                cml = '--tls.green.time',
        #                perm='rw',
        #                unit = 's',
        #                name = 'Green cycle time',
        #                info = 'reen cycle time of traffic light.',
        #                )
        self.add_option('time_leftgreen_tls', 6,
                        groupnames=['options', 'traffic lights'],
                        cml='--tls.left-green.time',
                        perm='rw',
                        unit='s',
                        name='Left green cycle time',
                        info='reen cycle time of traffic light.',
                        #is_enabled = lambda self: self.is_guess_tls ,
                        )

        self.add_option('time_yellow_tls', kwargs.get('time_yellow_tls', -1),  # default 6
                        groupnames=['options', 'traffic lights'],
                        cml='--tls.yellow.time',
                        perm='rw',
                        unit='s',
                        name='Yellow cycle time',
                        info='Fixed yellow time of traffic light. The value of -1.0 means automatic determination',
                        is_enabled=lambda self: self.is_guess_tls & (self.time_yellow_tls > 0),
                        )

        self.add_option('accel_min_yellow_tls', kwargs.get('accel_min_yellow_tls', -1.0),  # default 3.0
                        groupnames=['options'],
                        cml='--tls.yellow.min-decel',
                        perm='rw',
                        unit='m/s^2',
                        name='Min decel. at yellow',
                        info='Defines smallest vehicle deceleration at yellow. The value of -1.0 means automatic determination',
                        is_enabled=lambda self: self.is_guess_tls & (self.accel_min_yellow_tls > 0),
                        )

        # --tls.yellow.patch-small             Given yellow times are patched even ifbeing too short

    def init_options_topology(self, **kwargs):
        self.add_option('is_no_turnarounds', kwargs.get('is_no_turnarounds', False),
                        groupnames=['options', 'topology'],
                        cml='--no-turnarounds',
                        perm='rw',
                        name='no turnarounds',
                        info='Disables building turnarounds.',
                        )
        self.add_option('is_no_turnarounds_tls', kwargs.get('is_no_turnarounds_tls', False),
                        groupnames=['options', 'topology'],
                        cml='--no-turnarounds.tls',
                        perm='rw',
                        name='no TLS turnarounds',
                        info='Disables building turnarounds at traffic lights.',
                        is_enabled=lambda self: self.is_guess_tls,
                        )

        self.add_option('is_no_left_connections', kwargs.get('is_no_left_connections', False),
                        groupnames=['options', 'topology'],
                        cml='--no-left-connections',
                        perm='rw',
                        name='No left connections',
                        info='Disables building connections to left.',
                        )

        self.add_option('is_check_lane_foes', kwargs.get('is_check_lane_foes', False),
                        groupnames=['options', 'topology'],
                        cml='--check-lane-foes.all',
                        perm='rw',
                        name='Always allow entering multilane',
                        info='Always allow driving onto a multi-lane road if there are foes (arriving vehicles) on other lanes.',
                        )

        self.add_option('is_roundabouts_guess', kwargs.get('is_roundabouts_guess', True),
                        groupnames=['options', 'topology'],
                        cml='--roundabouts.guess',
                        perm='rw',
                        name='Roundabouts guess',
                        info='Enable roundabout-guessing.',
                        )
        self.add_option('is_check_lane_foes_roundabout', kwargs.get('is_check_lane_foes_roundabout', False),
                        groupnames=['options', 'topology'],
                        cml='--check-lane-foes.roundabout',
                        perm='rw',
                        name='Allow entering multilane roundabout',
                        info='Allow driving onto a multi-lane road at roundabouts if there are foes (arriving vehicles) on other lanes.',
                        is_enabled=lambda self: self.is_roundabouts_guess,
                        )

        # --lefthand <BOOL> 	Assumes left-hand traffic on the network; default: false
        self.add_option('is_no_left_connections', kwargs.get('is_no_left_connections', False),
                        groupnames=['options', 'topology'],
                        cml='--no-left-connections',
                        perm='rw',
                        name='no left connections',
                        info='Disables building connections to left.',
                        )

    def init_options_nodes(self, **kwargs):
        self.add_option('is_join_nodes', kwargs.get('is_join_nodes', True),
                        groupnames=['options', 'nodes'],
                        cml='--junctions.join',
                        perm='rw',
                        name='Join nodes',
                        info='Join nearby nodes. Specify with node join distance which nodes will be joined.',
                        )

        self.add_option('dist_join_nodes', kwargs.get('dist_join_nodes', 15.0),
                        groupnames=['options', 'nodes'],
                        cml='--junctions.join-dist',
                        perm='rw',
                        unit='m',
                        name='Node join distance',
                        info='Specify with node join distance which nodes will be joined. Join nodes option must be True.',
                        is_enabled=lambda self: self.is_join_nodes,
                        )

        self.add_option('is_keep_nodes_clear', kwargs.get('is_keep_nodes_clear', True),
                        groupnames=['options', 'nodes'],
                        cml='--default.junctions.keep-clear',
                        perm='rw',
                        name='Keep nodes clear',
                        info='Whether junctions should be kept clear by default.',
                        )

        self.add_option('is_keep_nodes_unregulated', kwargs.get('is_keep_nodes_unregulated', False),
                        groupnames=['options', 'nodes'],
                        cml='--keep-nodes-unregulated',
                        perm='rw',
                        name='Keep nodes unregulated',
                        info='Keep nodes unregulated.',
                        )
        # --default.junctions.radius <FLOAT> 	The default turning radius of intersections; default: 1.5
        # --junctions.corner-detail <INT> 	Generate INT intermediate points to smooth out intersection corners; default: 0
        # --junctions.internal-link-detail <INT> 	Generate INT intermediate points to smooth out lanes within the intersection; default: 5

    def init_options_pedestrians(self, **kwargs):
        self.add_option('is_guess_sidewalks', kwargs.get('is_guess_sidewalks', False),
                        groupnames=['options', 'pedestrians'],
                        cml='--sidewalks.guess',
                        perm='rw',
                        name='Guess sidewalks',
                        info='Guess pedestrian sidewalks based on edge speed.',
                        )

        self.add_option('edgespeed_min_sidewalks', kwargs.get('edgespeed_min_sidewalks', 5.8),
                        groupnames=['options', 'pedestrians'],
                        cml='--sidewalks.guess.min-speed',
                        perm='rw',
                        unit='m/s',
                        name='Min edge speed for sidewalk guess',
                        info='Add sidewalks for edges with a speed above the given limit.',
                        is_enabled=lambda self: self.is_guess_sidewalks | self.is_guess_sidewalks_from_permission,
                        )

        self.add_option('edgespeed_max_sidewalks', kwargs.get('edgespeed_max_sidewalks', 14.0),
                        groupnames=['options', 'pedestrians'],
                        cml='--sidewalks.guess.max-speed',
                        perm='rw',
                        unit='m/s',
                        name='Max edge speed for sidewalk guess',
                        info='Add sidewalks for edges with a speed equal or below the given limit.',
                        is_enabled=lambda self: self.is_guess_sidewalks | self.is_guess_sidewalks_from_permission,
                        )
        self.add_option('is_guess_sidewalks_from_permission', kwargs.get('is_guess_sidewalks_from_permission', False),
                        groupnames=['options', 'pedestrians'],
                        cml='--sidewalks.guess.from-permissions',
                        perm='rw',
                        name='Guess sidewalks from permission',
                        info='Add sidewalks for edges that allow pedestrians on any of their lanes regardless of speed.',
                        )

        self.add_option('width_sidewalks_default', kwargs.get('width_sidewalks_default', 1.5),
                        groupnames=['options', 'pedestrians'],
                        cml='--default.sidewalk-width',
                        perm='rw',
                        unit='m',
                        name='Default sidewalk width',
                        info='Default sidewalk width.',
                        is_enabled=lambda self: self.is_guess_sidewalks | self.is_guess_sidewalks_from_permission,
                        )

        self.add_option('is_guess_crossings', kwargs.get('is_guess_crossings', False),
                        groupnames=['options', 'pedestrians'],
                        cml='--crossings.guess',
                        perm='rw',
                        name='Guess crossings',
                        info='Guess pedestrian crossings based on the presence of sidewalks.',
                        )

        self.add_option('edgespeed_max_crossings', kwargs.get('edgespeed_max_crossings', 14.0),
                        groupnames=['options', 'pedestrians'],
                        cml='--crossings.guess.speed-threshold',
                        perm='rw',
                        unit='m/s',
                        name='Max edge speed for crossings',
                        info='At uncontrolled nodes, do not build crossings across edges with a speed above this maximum edge speed.',
                        is_enabled=lambda self: self.is_guess_crossings,
                        )

    def init_options_geometry(self, **kwargs):
        self.add_option('is_geometry_split', kwargs.get('is_geometry_split', False),
                        groupnames=['options', 'geometry'],
                        cml='--geometry.split',
                        perm='rw',
                        name='geometry split',
                        info='Splits edges across geometry nodes.',
                        )

        self.add_option('is_geometry_remove', kwargs.get('is_geometry_remove', True),
                        groupnames=['options', 'geometry'],
                        cml='--geometry.remove',
                        perm='rw',
                        name='geometry remove',
                        info='Replace nodes which only define edge geometry by geometry points (joins edges).',
                        )

        self.add_option('length_max_segment', kwargs.get('length_max_segment', -1.0),
                        groupnames=['options', 'geometry'],
                        cml='--geometry.max-segment-length',
                        perm='rw',
                        unit='m',
                        name='Max segment length',
                        info='Splits geometry to restrict segment length to maximum segment length. The value of -1.0 means no effect.',
                        is_enabled=lambda self: self.length_max_segment > 0,
                        )

        self.add_option('dist_min_geometry', kwargs.get('dist_min_geometry', -1.0),
                        groupnames=['options', 'geometry'],
                        cml='--geometry.min-dist',
                        perm='rw',
                        unit='m',
                        name='Min geom dist',
                        info='Reduces too similar geometry points. The value of -1.0 means no effect.',
                        is_enabled=lambda self: self.dist_min_geometry > 0,
                        )

        # self.add_option('angle_max_geometry',99.0,
        #                groupnames = ['options','geometry'],#
        #                cml = '--geometry.max-angle',
        #                perm='rw',
        #                unit = 'degree',
        #                name = 'Max geom angle',
        #                info = ' 	Warn about edge geometries with an angle above the maximum angle in successive segments.',
        #                )
        # --geometry.min-radius <FLOAT> 	Warn about edge geometries with a turning radius less than METERS at the start or end; default: 9
        # --geometry.junction-mismatch-threshold <FLOAT> 	Warn if the junction shape is to far away from the original node position; default: 20
        # self.add_option('projparams','!',
        #                groupnames = ['options'],#
        #                cml = '--proj',
        #                perm='rw',
        #                name = 'projection',
        #                info = 'Uses STR as proj.4 definition for projection.',
        #                )
        # --offset.disable-normalization <BOOL> 	Turn off normalizing node positions; default: false
        # self.add_option('offset_x',0.0,
        #                groupnames = ['options','geometry'],#
        #                cml = '--offset.x ',
        #                perm='rw',
        #                unit = 'm',
        #                name = 'X-Offset',
        #                info = 'Adds offset to net x-positions; default: 0.0',
        #                )
        # self.add_option('offset_y',0.0,
        #                groupnames = ['options','geometry'],#
        #                cml = '--offset.y ',
        #                perm='rw',
        #                unit = 'm',
        #                name = 'Y-Offset',
        #                info = 'Adds offset to net x-positions; default: 0.0',
        #                )
        # --flip-y-axis <BOOL> 	Flips the y-coordinate along zero; default: false

    def init_options_ramps(self, **kwargs):
        self.add_option('is_guess_ramps', kwargs.get('is_guess_ramps', True),
                        groupnames=['options', 'ramps'],
                        cml='--ramps.guess',
                        perm='rw',
                        name='Guess ramps',
                        info='Enable ramp-guessing.',
                        )

        self.add_option('rampspeed_max', kwargs.get('rampspeed_max', -1),
                        groupnames=['options', 'ramps'],
                        cml='--ramps.max-ramp-speed',
                        perm='rw',
                        unit='m/s',
                        name='Max ramp speed',
                        info='Treat edges with a higher speed as no ramps',
                        is_enabled=lambda self: self.is_guess_ramps,
                        )

        self.add_option('highwayspeed_min', kwargs.get('highwayspeed_min', 21.944),
                        groupnames=['options', 'ramps'],
                        cml='--ramps.min-highway-speed',
                        perm='rw',
                        unit='m/s',
                        name='Min highway speed',
                        info='Treat edges with lower speed as no highways (no ramps will be build there)',
                        is_enabled=lambda self: self.is_guess_ramps,
                        )

        self.add_option('ramplength', kwargs.get('ramplength', 100),
                        groupnames=['options', 'ramps'],
                        cml='--ramps.ramp-length',
                        perm='rw',
                        unit='m',
                        name='Ramp length',
                        info='Ramp length',
                        is_enabled=lambda self: self.is_guess_ramps,
                        )

        self.add_option('is_no_split_ramps', kwargs.get('is_no_split_ramps', False),
                        groupnames=['options', 'ramps'],
                        cml='--ramps.no-split',
                        perm='rw',
                        name='No ramp split',
                        info='Avoids edge splitting at ramps.',
                        is_enabled=lambda self: self.is_guess_ramps,
                        )

    def init_other(self, **kwargs):

        self.add_option('is_import_elevation_osm', kwargs.get('is_import_elevation_osm', False),
                        groupnames=['options', 'osm'],
                        cml='--osm.elevation',
                        perm='rw',
                        name='import elevation',
                        info='Imports elevation data.',
                        )

        # not command line options
        attrsman = self.get_attrsman()
        self.is_clean_nodes = attrsman.add(cm.AttrConf('is_clean_nodes', kwargs.get('is_clean_nodes', False),
                                                       groupnames=['options'],
                                                       perm='rw',
                                                       name='Clean Nodes',
                                                       info='If set, then shapes around nodes are cleaned up after importing.',
                                                       ))

    def init_all(self, **kwargs):
        # edge options
        self.init_options_edgetype(**kwargs)
        self.init_options_edge(**kwargs)

        # node options
        self.init_options_nodes(**kwargs)

        # pedestrians
        self.init_options_pedestrians(**kwargs)

        # TLS Building Options:
        self.init_options_tls(**kwargs)

        # topology
        self.init_options_topology(**kwargs)

        # geometry options
        self.init_options_geometry(**kwargs)

        # Ramps
        self.init_options_ramps(**kwargs)

        # other
        self.init_other(**kwargs)

    def update_params(self):
        """
        Make all parameters consistent.
        example: used by import OSM to calculate/update number of tiles
        from process dialog
        """
        pass

    def do(self):
        self.update_params()
        cml = self.get_cml()

        # print 'SumonetImporter.do',cml
        self.run_cml(cml)
        if self.status == 'success':
            print '  Netconvert done.'
            if os.path.isfile(self.netfilepath):
                # print '  sumo.net.xml exists, start generation of xml files'
                self.parent.import_netxml(self.netfilepath)
                # print '  import in sumopy done.'
            return True
        return False

    def get_net(self):
        return self._net


class TlSignalGenerator(NetConvertMixin):
    def __init__(self, net, netfilepath=None,
                 name='TL signal generator',
                 info='Generates or regenerates traffic light signals.',
                 logger=None, **kwargs):

        # netfilepath will be the filepath with result network
        self.init_common_netconvert('tlsignalsgenerator', net, netfilepath=netfilepath,
                                    name=name,
                                    info=info,
                                    logger=logger,
                                    cml='netconvert')

        # load only tls option
        attrsman = self.get_attrsman()
        # self.add_option('sumofilepath',net.get_filepath()+'.tmp',
        #                groupnames = ['options','_private'],#
        #                cml = '--sumo-net-file',
        #                perm='r',
        #                name = 'SUMO temp. file',
        #                wildcards = 'SUMO XML files (*.xml)|*.xml*',
        #                metatype = 'filepaths',
        #                info = 'Temporary sumo file, where cuurent network is exported befor being netconverted.',
        #                )

        self.sumofilepath = attrsman.add(cm.AttrConf('sumofilepath', net.get_filepath()+'.tmp',
                                                     groupnames=['_private'],
                                                     #cml = '--sumo-net-file',
                                                     perm='r',
                                                     name='SUMO temp. file',
                                                     wildcards='SUMO XML files (*.xml)|*.xml*',
                                                     metatype='filepaths',
                                                     info='Temporary sumo file, where current network is exported before being netconverted.',
                                                     ))

        # self.init_all(**kwargs)
        self.init_options_tls()

    def import_tlsxml(self,  is_remove_xmlfiles=False):
        print 'import_tlsxml'
        filepath = self.netfilepath
        net = self.parent
        rootname = net.get_rootfilename()
        filepath = net.get_filepath()

        dirname = os.path.dirname(filepath)

        # print '  modes.names',self.modes.names
        cml = 'netconvert'\
            + ' --sumo-net-file '+filepathlist_to_filepathstring(filepath)\
            + ' --plain-output-prefix '+filepathlist_to_filepathstring(os.path.join(dirname, rootname))

        proc = subprocess.Popen(cml, shell=True)
        print '  run_cml cml=', cml
        print '  pid = ', proc.pid
        proc.wait()
        if not proc.returncode:

            tlsfilepath = os.path.join(dirname, rootname+'.tll.xml')
            print '  import_sumotls', tlsfilepath
            net.import_sumotls_to_net(tlsfilepath, is_remove_xmlfiles=is_remove_xmlfiles)

            return True
        else:
            return False

    def do(self):
        # first export current net
        # self.parent.export_netxml(self.sumofilepath)
        net = self.parent
        filepath = self.sumofilepath
        filepath_edges, filepath_nodes, filepath_connections, filepath_tlss = net.export_painxml(
            filepath=filepath, is_export_tlss=False)

        cmlbase = 'netconvert --ignore-errors.edge-type'\
            + ' --node-files '+filepathlist_to_filepathstring(filepath_nodes)\
            + ' --edge-files '+filepathlist_to_filepathstring(filepath_edges)\
            + ' --connection-files '+filepathlist_to_filepathstring(filepath_connections)\
            #+' --output-file '+filepathlist_to_filepathstring(filepath)

        print '  cmlbase', cmlbase
        self.reset_cml(cmlbase)

        # apply netconvert and reimport
        # return NetConvertMixin.do(self)
        self.update_params()
        cml = self.get_cml()

        # print 'SumonetImporter.do',cml
        self.run_cml(cml)
        if self.status == 'success':
            print '  Netconvert done.'
            if os.path.isfile(self.netfilepath):
                # print '  sumo.net.xml exists, start generation of xml files'
                # self.parent.import_netxml(self.netfilepath)
                self.import_tlsxml()
                # print '  import in sumopy done.'
                return True
            else:
                return False
        return False


class NetConvert(NetConvertMixin):
    def __init__(self, net, netfilepath=None,
                 name='Netconvert',
                 info='Transforms/enriches current SUMO network.',
                 logger=None, **kwargs):

        self.init_common_netconvert('netconvert', net, netfilepath=netfilepath,
                                    name=name,
                                    info=info,
                                    logger=logger,
                                    cml='netconvert')

        self.add_option('sumofilepath', net.get_filepath()+'.tmp',
                        groupnames=['options', '_private'],
                        cml='--sumo-net-file',
                        perm='r',
                        name='SUMO temp. file',
                        wildcards='SUMO XML files (*.xml)|*.xml*',
                        metatype='filepaths',
                        info='Temporary sumo file, where current network is exported before being netconverted.',
                        )

        self.init_all(**kwargs)

    def do(self):
        # first export current net
        self.parent.export_netxml(self.sumofilepath)

        # apply netconvert and reimport
        return NetConvertMixin.do(self)
