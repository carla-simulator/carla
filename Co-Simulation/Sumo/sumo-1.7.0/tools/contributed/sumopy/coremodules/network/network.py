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

# @file    network.py
# @author  Joerg Schweizer
# @date

# size limit at 1280x1280
# http://maps.googleapis.com/maps/api/staticmap?size=500x500&path=color:0x000000|weight:10|44.35789,11.3093|44.4378,11.3935&format=GIF&maptype=satellite&scale=2

import os
import sys
import subprocess
import platform

from xml.sax import saxutils, parse, handler
if __name__ == '__main__':
    try:
        APPDIR = os.path.dirname(os.path.abspath(__file__))
    except:
        APPDIR = os.path.dirname(os.path.abspath(sys.argv[0]))
    SUMOPYDIR = os.path.join(APPDIR, '..', '..')
    sys.path.append(os.path.join(SUMOPYDIR))

import time
import numpy as np
from collections import OrderedDict
import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
import agilepy.lib_base.xmlman as xm
from agilepy.lib_base.misc import filepathlist_to_filepathstring, filepathstring_to_filepathlist

from agilepy.lib_base.processes import Process, CmlMixin, P

from agilepy.lib_base.geometry import *

import netconvert
import publictransportnet as pt


MODES = OrderedDict([
                    ("ignoring", 0),
                    ("pedestrian", 1),
                    ("bicycle", 2),
                    ("motorcycle", 3),
                    ("passenger", 4),
                    ("bus", 5),
                    ("tram", 6),
                    ("rail_urban", 7),
                    ("delivery", 8),
                    ("private", 9),
                    ("taxi", 10),
                    ("hov", 11),
                    ("evehicle", 12),
                    ("emergency", 13),
                    ("authority", 14),
                    ("army", 15),
                    ("vip", 16),
                    ("coach", 17),
                    ("truck", 18),
                    ("trailer", 19),
                    ("rail", 20),
                    ("rail_electric", 21),
                    ("moped", 22),
                    ("custom1", 23),
                    ("custom2", 24),
                    ("ship", 25),
                    ])

ID_MODE_PED = MODES['pedestrian']
ID_MODE_BIKE = MODES['bicycle']
ID_MODE_CAR = MODES['passenger']

OSMEDGETYPE_TO_MODES = {'highway.cycleway': ([MODES['bicycle']], 5.6),
                        'highway.pedestrian': ([MODES['pedestrian']], 0.8),
                        'highway.footway': ([MODES['pedestrian']], 0.8),
                        'highway.path': ([MODES['pedestrian'], MODES['bicycle']], 5.6),
                        'highway.service': ([MODES['delivery'], MODES['bicycle']], 13.8),
                        }


class SumoIdsConf(am.ArrayConf):
    """
    Sumo id array coniguration
    """
    # def __init__(self, **attrs):
    #    print 'ColConf',attrs

    def __init__(self, refname, name=None, info=None, perm='rw',  xmltag='id'):
        if name is None:
            name = 'ID '+refname
        if info is None:
            info = refname + ' ID of SUMO network'
        am.ArrayConf.__init__(self, attrname='ids_sumo', default='',
                              dtype='object',
                              perm=perm,
                              is_index=True,
                              name=name,
                              info=info,
                              xmltag=xmltag,
                              )


class Modes(am.ArrayObjman):
    # http://www.sumo.dlr.de/userdoc/Networks/Building_Networks_from_own_XML-descriptions.html#Edge_Descriptions
    def __init__(self, parent, **kwargs):
        ident = 'modes'
        self._init_objman(ident=ident, parent=parent, name='Transport Modes',
                          xmltag=('vClasses', 'vClass', 'names'),
                          version=0.1,
                          **kwargs)

        self._init_attributes()
        self.add_default()

    def _init_attributes(self):
        self.add_col(am.ArrayConf('names', '',
                                  dtype=np.object,
                                  perm='r',
                                  is_index=True,
                                  name='Name',
                                  info='Name of mode. Used as key for implementing acces restrictions on edges as well as demand modelling.',
                                  xmltag='vClass',
                                  ))

        self.add_col(am.ArrayConf('speeds_max', 50.0/3.6,
                                  dtype=np.float32,
                                  perm='rw',
                                  name='Max. Speed',
                                  unit='m/s',
                                  info='Maximum possible speed for this mode. Speed is used to estimate free flow link travel times, mainly for routig purposes. Note that speeds are usully limited by the lane speed attribute',
                                  ))
        if self.get_version() < 0.1:
            self.clear()
            self.add_default()
            self.set_version(0.1)

    def get_id_mode(self, modename):
        return self.names.get_id_from_index(modename)

    def has_modename(self, modename):
        return self.names.has_index(modename)

    def format_ids(self, ids):
        return ','.join(self.names[ids])

    def get_id_from_formatted(self, idstr):
        return self.names.get_id_from_index(idstr)

    def get_ids_from_formatted(self, idstrs):
        return self.names.get_ids_from_indices_save(idstrs.split(','))

    def add_default(self):
        """
        Sets the default maximum possible speed for certain modes.
        """
        # print 'MODES.add_default'
        self.add_rows(ids=MODES.values(), names=MODES.keys())

        # these speeds are used to estimate free flow link travel times
        # mainly for routig purposes
        # note that speed limits are usully limited by the lane speed attribute
        speeds_max_kmph = OrderedDict([
            ("ignoring", 100.0),
            ("pedestrian", 3.6),
            ("bicycle", 25.0),
            ("motorcycle", 130.0),
            ("passenger", 160.0),
            ("bus", 90.0),
            ("tram", 50.0),
            ("rail_urban", 160.0),
            ("delivery", 100),
            ("private", 160),
            ("taxi", 160),
            ("hov", 160),
            ("evehicle", 160),
            ("emergency", 160),
            ("authority", 160),
            ("army", 130),
            ("vip", 160),
            ("coach", 90),
            ("truck", 90),
            ("trailer", 90),
            ("rail", 250),
            ("rail_electric", 300),
            ("moped", 25),
            ("custom1", 100),
            ("custom2", 160),
            ("ship", 30),
        ])

        for mode, speed_kmph in speeds_max_kmph.iteritems():
            self.speeds_max[self.get_id_mode(mode)] = float(speed_kmph)/3.6

        # print '  self.speeds_max',self.speeds_max.get_value()


class TrafficLightProgram(am.ArrayObjman):
    def __init__(self, ident, parent, **kwargs):
        self._init_objman(ident, parent=parent,
                          name='TLL Program',
                          info='Signale phases of a traffic light program.',
                          xmltag=('', 'phase', ''), **kwargs)

        self.add_col(am.NumArrayConf('durations', 0,
                                     dtype=np.int32,
                                     name='Duration',
                                     unit='s',
                                     info='The duration of the phase.',
                                     xmltag='duration',
                                     ))

        self.add_col(am.NumArrayConf('durations_min', 0,
                                     dtype=np.int32,
                                     name='Min. duration',
                                     unit='s',
                                     info='The minimum duration of the phase when using type actuated. Optional, defaults to duration.',
                                     xmltag='minDur',
                                     ))

        self.add_col(am.NumArrayConf('durations_max', 0,
                                     dtype=np.int32,
                                     name='Max. duration',
                                     unit='s',
                                     info='The maximum duration of the phase when using type actuated. Optional, defaults to duration.',
                                     xmltag='maxDur',
                                     ))

        self.add_col(am.ArrayConf('states', None,
                                  dtype=np.object,
                                  perm='rw',
                                  name='State',
                                  info="The traffic light states for this phase. Values can be one of these characters: 'r'=red, 'y'=yellow, 'g'=green give priority, 'G'=Green always priority, 'o'=blinking ,'O'=TLS switched off",
                                  xmltag='state',
                                  ))

    def add_multi(self, **kwargs):
        # print 'add_multi',self.ident
        # print '  durations',kwargs.get('durations',None)
        # print '  durations_min',kwargs.get('durations_min',None)
        # print '  durations_max',kwargs.get('durations_max',None)
        # print '  states',kwargs.get('states',None)
        return self.add_rows(durations=kwargs.get('durations', None),
                             durations_min=kwargs.get('durations_min', None),
                             durations_max=kwargs.get('durations_max', None),
                             states=kwargs.get('states', None),
                             )

    # def  write_xml(self, fd, indent, **kwargs):
    #
    #    # never print begin-end tags
    #    # this could go into xml config
    #    if kwargs.has_key('is_print_begin_end'):
    #        del kwargs['is_print_begin_end']
    #    am.ArrayObjman.write_xml(self, fd, indent,is_print_begin_end = False,**kwargs)


class TrafficLightLogics(am.ArrayObjman):
    def __init__(self, ident, tlss, **kwargs):
        self._init_objman(ident, parent=tlss,
                          name='Traffic Light Logics',
                          info='Traffic light Logics (TLLs) for Trafic Light Systems (TLSs).',
                          xmltag=('tlLogics', 'tlLogic', 'ids_tls'),
                          **kwargs)

        self.add_col(am.IdsArrayConf('ids_tls', tlss,
                                     groupnames=['state'],
                                     name='ID tls',
                                     info='ID of traffic light system.  Typically the id for a traffic light is identical with the junction id. The name may be obtained by right-clicking the red/green bars in front of a controlled intersection.',
                                     xmltag='id',  # this will be ID TLS tag used as ID in xml file
                                     ))

        self.add_col(am.ArrayConf('ids_prog', '',
                                  dtype=np.object,
                                  perm='rw',
                                  name='Prog ID',
                                  info='Sumo program ID, which is unique within the same traffic light system.',
                                  xmltag='programID',
                                  ))

        self.add_col(am.ArrayConf('ptypes', 1,
                                  choices={
                                      "static": 1,
                                      "actuated": 2,
                                  },
                                  dtype=np.int32,
                                  perm='rw',
                                  name='Prog. type',
                                  info='The type of the traffic light program (fixed phase durations, phase prolongation based time gaps between vehicles).',
                                  xmltag='type',
                                  ))

        self.add_col(am.NumArrayConf('offsets', 0,
                                     dtype=np.int32,
                                     perm='rw',
                                     name='Offset',
                                     unit='s',
                                     info='The initial time offset of the program.',
                                     #is_plugin = True,
                                     xmltag='offset',
                                     ))

        self.add_col(cm.ObjsConf('programs',
                                 name='program',
                                 info='Tls program.',
                                 ))

    def make(self, id_tls, id_prog=None, ptype=None, offset=None, **kwargs_prog):
        # print 'make',id_tls,id_prog
        if id_prog is None:
            id_prog = str(len(np.flatnonzero(self.ids_tls == id_tls)))

        id_tll = self.add_row(ids_tls=id_tls,
                              ids_prog=id_prog,
                              ptypes=ptype,
                              offsets=offset,
                              )
        # init programme
        program = TrafficLightProgram(('prog', id_tll), self)
        self.programs[id_tll] = program

        # add phases
        program.add_multi(**kwargs_prog)

        return id_tll


class TrafficLightSystems(am.ArrayObjman):
    # http://www.sumo.dlr.de/userdoc/Networks/Building_Networks_from_own_XML-descriptions.html#Traffic_Light_Program_Definition

    def __init__(self, net, **kwargs):

        self._init_objman(ident='tlss', parent=net,
                          name='Traffic Light Systems',
                                # actually tlls table is exported, but ids_sumo is required for ID
                                xmltag=('tlSystems', 'tlSystem', 'ids_sumo'),
                          **kwargs)

        self.add_col(SumoIdsConf('TLS', info='SUMO ID of traffic light system.',
                                 xmltag='id'))

        self.add(cm.ObjConf(TrafficLightLogics('tlls', self)))

        self.add_col(am.IdlistsArrayConf('ids_tlls', self.tlls.value,
                                         groupnames=['state'],
                                         name='IDs TLL',
                                         info='ID list of available Traffic Light Logics (or programs) for the Traffic Light System.',
                                         ))

        self.add_col(am.IdlistsArrayConf('ids_cons', self.parent.connections,
                                         groupnames=['state'],
                                         name='IDs con.',
                                         info='ID list of controlled connections. These connections corrispond to the elements of the state vector within the program-phases.',
                                         ))

    # def clear_tlss(self):
    #    #self.tlls.get_value().clear()
    #    self.clear()

    def make(self, id_sumo, **kwargs):

        if self.ids_sumo.has_index(id_sumo):
            # recycle ID from existing
            id_tls = self.ids_sumo.get_id_from_index(id_sumo)
        else:
            # make new TLS
            id_tls = self.add_row(ids_sumo=id_sumo, ids_tlls=[])

        # make a new TL logic for this traffic light systems
        id_tll = self.tlls.get_value().make(id_tls, **kwargs)
        # append new logic to list
        self.ids_tlls[id_tls].append(id_tll)
        # print 'TLS.make',id_sumo,id_tls,self.ids_tlls[id_tls]
        return id_tls

    def clear_tlss(self):
        """
        Delete all TLS systems of the network
        """

        # this will clear the pointer from nodes to a tls
        # but the node type tls will remain
        self.parent.nodes.ids_tls.reset()
        self.parent.connections.are_uncontrolled.reset()
        self.clear()

    def set_connections(self, id_tls, ids_con):
        """
        Set connections, which represent the controlled links of TLD with id_tls
        Called after connections in ttl file have been parsed.
        """
        # print 'set_connections',id_tls,len(ids_con)

        #id_tls = self.ids_sumo.get_id_from_index(id_sumo)
        self.ids_cons[id_tls] = ids_con

        # check whether the number of connections equals to the number of
        # dignals in the states
        #
        # Attention: they are not equal because there is no connection for
        # pedestrian crossings! Thos links are under crossins,
        # but have no link index :(...so just be tolerant
        #
        #n_signals = len(ids_con)
        # print '  ids',self.get_ids(), id_tls in self.get_ids()
        #tlls = self.tlls.get_value()
        # print '  self.ids_tlls[id_tls]:',self.ids_tlls[id_tls]
        # for id_tll in self.ids_tlls[id_tls]:
        #    prog = tlls.programs[id_tll]
        #    states = prog.states.get_value()
        #    #print '  len(state0),n_signals',len(state0),n_signals
        #    if len(states[0]) != n_signals:
        #        print 'WARNING: tls %s has inconsistant program. \n  Signals =%d, states=%d'%(self.ids_sumo[id_tls],n_signals,len(states[0]))
        #        print '  ids_con=',ids_con
        #        for state in states:
        #            print '  state',state

    def change_states(self, state_current, state_new):
        """
        Change traffic light state is all traffic light programms
        from state_current to state_new
        """
        tlls = self.tlls.get_value()
        for program in tlls.programs[tlls.get_ids()]:
            ids_phases = program.get_ids()
            for id_phase, state in zip(ids_phases, program.states[ids_phases]):
                program.states[id_phase] = state.replace(state_current, state_new)

    def export_sumoxml(self, filepath=None, encoding='UTF-8'):
        """
        Export traffic light systems to SUMO xml file.
        """
        # here we export actually the traffic light logics table
        # and the controlled connections table
        tlls = self.tlls.get_value()
        connections = self.parent.connections  # self.ids_cons.get_linktab()
        lanes = self.parent.lanes
        edges = self.parent.edges

        # this is the preferred way to specify default filepath
        if filepath is None:
            filepath = self.parent.get_rootfilepath()+'.tll.xml'

        print 'export_sumoxml', filepath
        try:
            fd = open(filepath, 'w')
        except:
            print 'WARNING in export_sumoxml: could not open', filepath
            return False

        fd.write('<?xml version="1.0" encoding="%s"?>\n' % encoding)
        xmltag_ttl, xmltag_id, attrconf_id = tlls.xmltag
        fd.write(xm.begin(xmltag_ttl))
        indent = 2

        #ids_modes_used = set(self.parent.vtypes.ids_mode[self.ids_vtype.get_value()])
        ids_tlls = tlls.get_ids()
        tlls.write_xml(fd, indent=indent,
                       #xmltag_id = 'id',
                       #ids = ids_tlls,
                       #ids_xml = self.ids_sumo[tlls.ids_tls[ids_tlls]],
                       is_print_begin_end=False,
                       )
        # write controlled connections
        ids_tls = self.get_ids()
        xmltag_con = 'connection'
        for ids_con, id_sumo_tls in zip(self.ids_cons[ids_tls], self.ids_sumo[ids_tls]):
            ids_fromlane = connections.ids_fromlane[ids_con]
            ids_tolane = connections.ids_tolane[ids_con]
            inds_fromlane = lanes.indexes[ids_fromlane]
            inds_tolane = lanes.indexes[ids_tolane]
            ids_sumo_fromedge = edges.ids_sumo[lanes.ids_edge[ids_fromlane]]
            ids_sumo_toedge = edges.ids_sumo[lanes.ids_edge[ids_tolane]]

            ind_link = 0
            for id_sumo_fromedge, id_sumo_toedge, ind_fromlane, ind_tolane in \
                    zip(ids_sumo_fromedge, ids_sumo_toedge, inds_fromlane, inds_tolane):

                fd.write(xm.start(xmltag_con, indent))
                fd.write(xm.num('from', id_sumo_fromedge))
                fd.write(xm.num('to', id_sumo_toedge))
                fd.write(xm.num('fromLane', ind_fromlane))
                fd.write(xm.num('toLane', ind_tolane))
                fd.write(xm.num('tl', id_sumo_tls))
                fd.write(xm.num('linkIndex', ind_link))

                fd.write(xm.stopit())

                ind_link += 1

        fd.write(xm.end(xmltag_ttl))


class Crossings(am.ArrayObjman):
    # http://www.sumo.dlr.de/userdoc/Networks/Building_Networks_from_own_XML-descriptions.html#Edge_Descriptions
    def __init__(self, parent, **kwargs):
        ident = 'crossings'
        self._init_objman(ident=ident, parent=parent, name='Crossings',
                          xmltag=('crossings', 'crossing', ''),
                          is_plugin=True,
                          version=0.1,
                          **kwargs)

        self._init_attributes()

    def _init_attributes(self):
        net = self.parent

        self.add_col(am.IdsArrayConf('ids_node', net.nodes,
                                     groupnames=['state'],
                                     name='ID node',
                                     info='ID of node where crossings are located.',
                                     xmltag='node',
                                     ))

        self.add_col(am.IdlistsArrayConf('ids_edges', net.edges,
                                         groupnames=['state'],
                                         name='IDs Edge',
                                         info='Edge IDs at specific node, where street crossing is possible.',
                                         xmltag='edges',
                                         ))

        self.add_col(am.ArrayConf('widths', 4.0,
                                  dtype=np.float32,
                                  perm='rw',
                                  unit='m',
                                  name='Width',
                                  info='Crossing width.',
                                  xmltag='width',
                                  ))

        self.add_col(am.ArrayConf('are_priority', False,
                                  dtype=np.bool,
                                  perm='rw',
                                  name='Priority',
                                  info='Whether the pedestrians have priority over the vehicles (automatically set to true at tls-controlled intersections).',
                                  xmltag='priority',
                                  ))

        self.add_col(am.ArrayConf('are_discard', False,
                                  dtype=np.bool,
                                  perm='rw',
                                  name='Discard',
                                  info='Whether the crossing with the given edges shall be discarded.',
                                  xmltag='discard',
                                  ))

        if self.get_version() < 0.1:
            self.init_plugin(True)

    def multimake(self, ids_node=[], **kwargs):
        n = len(ids_node)
        return self.add_rows(n=n,
                             ids_node=ids_node,
                             **kwargs
                             )

    def make(self, **kwargs):
        return self.add_row(ids_node=kwargs['id_node'],
                            ids_edges=kwargs['ids_edge'],
                            widths=kwargs.get('width', None),
                            are_priority=kwargs.get('is_priority', None),
                            are_discard=kwargs.get('is_discard', None),
                            )

    def del_element(self, _id):
        # print 'del_element',id_zone
        self.del_row(_id)


class Connections(am.ArrayObjman):
    # http://www.sumo.dlr.de/userdoc/Networks/Building_Networks_from_own_XML-descriptions.html#Edge_Descriptions
    def __init__(self, parent, **kwargs):
        ident = 'connections'
        self._init_objman(ident=ident, parent=parent, name='Connections',
                          xmltag=('connections', 'connection', None),
                          **kwargs)
        self._init_attributes()

    def _init_attributes(self):

        lanes = self.parent.lanes
        self.add_col(am.IdsArrayConf('ids_fromlane', lanes,
                                     groupnames=['state'],
                                     name='ID from-lane',
                                     info='ID of lane at the beginning of the connection.',
                                     xmltag='fromLane',
                                     ))

        self.add_col(am.IdsArrayConf('ids_tolane', lanes,
                                     name='ID to-lane',
                                     info='ID of lane at the end of the connection.',
                                     xmltag='toLane',
                                     ))

        self.add_col(am.ArrayConf('are_passes', False,
                                  dtype=np.bool,
                                  perm='rw',
                                  name='Pass',
                                  info=' if set, vehicles which pass this (lane-to-lane) connection will not wait.',
                                  xmltag='pass',
                                  ))

        self.add_col(am.ArrayConf('are_keep_clear',  True,
                                  dtype=np.bool,
                                  groupnames=['state'],
                                  perm='rw',
                                  name='keep clear',
                                  info='if set to false, vehicles which pass this (lane-to-lane) connection will not worry about blocking the intersection.',
                                  xmltag='keepClear',
                                  ))

        self.add_col(am.ArrayConf('positions_cont', 0.0,
                                  dtype=np.float32,
                                  perm='rw',
                                  unit='m',
                                  name='Cont. Pos.',
                                  info='if set to 0, no internal junction will be built for this connection. If set to a positive value, an internal junction will be built at this position (in m) from the start of the internal lane for this connection. ',
                                  xmltag='contPos',
                                  ))

        self.add_col(am.ArrayConf('are_uncontrolled',  False,
                                  dtype=np.bool,
                                  perm='rw',
                                  name='uncontrolled',
                                  info='if set to true, This connection will not be TLS-controlled despite its node being controlled.',
                                  xmltag='uncontrolled',
                                  ))

    def make(self, **kwargs):
        return self.add_row(ids_fromlane=kwargs['id_fromlane'],
                            ids_tolane=kwargs['id_tolane'],
                            are_passes=kwargs.get('is_passes', None),
                            are_keep_clear=kwargs.get('is_keep_clear', None),
                            positions_cont=kwargs.get('position_cont', None),
                            are_uncontrolled=kwargs.get('is_uncontrolled', None),
                            )

    def multimake(self, ids_fromlane=[], ids_tolane=[], **kwargs):

        n = len(ids_fromlane)
        return self.add_rows(n=n,
                             ids_fromlane=ids_fromlane,
                             ids_tolane=ids_tolane,
                             **kwargs
                             )

    def get_id_from_sumoinfo(self, id_sumo_fromedge, id_sumo_toedge, ind_fromlane, ind_tolane):
        get_id_lane = self.parent.edges.get_id_lane_from_sumoinfo
        id_fromlane = get_id_lane(id_sumo_fromedge, ind_fromlane)
        id_tolane = get_id_lane(id_sumo_toedge, ind_tolane)
        ids_con = self.select_ids((self.ids_fromlane.value == id_fromlane) & (self.ids_tolane.value == id_tolane))
        if len(ids_con) == 1:
            return ids_con[0]
        else:
            return -1

    def export_sumoxml(self, filepath, encoding='UTF-8'):
        try:
            fd = open(filepath, 'w')
        except:
            print 'WARNING in export_sumoxml: could not open', filepath
            return False

        fd.write('<?xml version="1.0" encoding="%s"?>\n' % encoding)
        indent = 0
        self.write_xml(fd, indent)

        fd.close()

    def write_xml(self, fd, indent):
        # print 'Connections.write_xml'

        xmltag, xmltag_item, attrname_id = self.xmltag
        #attrsman = self.get_attrsman()
        #attrsman = self.get_attrsman()
        #config_fromlane = attrsman.get_config('ids_fromlane')
        #config_tolane = attrsman.get_config('ids_tolane')
        colconfigs = self.get_colconfigs(is_all=True)
        ids_sumoedges = self.parent.edges.ids_sumo
        ids_laneedge = self.parent.lanes.ids_edge
        # print '  header'
        fd.write(xm.start(xmltag, indent))
        # print '  ', self.parent.get_attrsman().get_config('version').attrname,self.parent.get_attrsman().get_config('version').get_value()
        #fd.write( self.parent.get_attrsman().get_config('version').write_xml(fd) )
        self.parent.get_attrsman().get_config('version').write_xml(fd)
        fd.write(xm.stop())

        for _id in self.get_ids():
            fd.write(xm.start(xmltag_item, indent+2))

            # print ' make tag and id',_id
            # fd.write(xm.num(xmltag_id,attrconfig_id[_id]))

            # print ' write columns'
            for attrconfig in colconfigs:
                # print '    colconfig',attrconfig.attrname
                if attrconfig == self.ids_fromlane:
                    fd.write(xm.num('from', ids_sumoedges[ids_laneedge[self.ids_fromlane[_id]]]))
                    attrconfig.write_xml(fd, _id)

                elif attrconfig == self.ids_tolane:
                    fd.write(xm.num('to', ids_sumoedges[ids_laneedge[self.ids_tolane[_id]]]))
                    attrconfig.write_xml(fd, _id)

                else:
                    attrconfig.write_xml(fd, _id)
            fd.write(xm.stopit())

        self.parent.crossings.write_xml(fd, indent=indent+2, is_print_begin_end=False)

        fd.write(xm.end(xmltag, indent))


class Lanes(am.ArrayObjman):
    # http://www.sumo.dlr.de/userdoc/Networks/Building_Networks_from_own_XML-descriptions.html#Edge_Descriptions
    def __init__(self, parent, **kwargs):
        ident = 'lanes'
        self._init_objman(ident=ident, parent=parent, name='Lanes',
                          is_plugin=True,
                          xmltag=('lanes', 'lane', 'indexes'),
                          version=0.1,
                          **kwargs)

        self._init_attributes()

    def _init_attributes(self):
        modes = self.parent.modes
        edges = self.parent.edges

        self.add_col(am.ArrayConf('indexes', 0,
                                  dtype=np.int32,
                                  perm='r',
                                  name='Lane index',
                                  info='The enumeration index of the lane (0 is the rightmost lane, <NUMBER_LANES>-1 is the leftmost one).',
                                  xmltag='index',
                                  ))

        self.add_col(am.ArrayConf('widths', 3.5,
                                  dtype=np.float32,
                                  perm='rw',
                                  unit='m',
                                  name='Width',
                                  info='Lane width.',
                                  is_plugin=True,
                                  xmltag='width',
                                  ))

        self.add_col(am.NumArrayConf('speeds_max', 50.0/3.6,
                                     dtype=np.float32,
                                     groupnames=['state'],
                                     perm='rw',
                                     name='Max speed',
                                     unit='m/s',
                                     info='Maximum speed on lane.',
                                     xmltag='speed',
                                     ))

        self.add_col(am.NumArrayConf('offsets_end', 0.0,
                                     dtype=np.float32,
                                     groupnames=['state'],
                                     perm='r',
                                     name='End offset',
                                     unit='m',
                                     info='Move the stop line back from the intersection by the given amount (effectively shortening the lane and locally enlarging the intersection).',
                                     xmltag='endOffset',
                                     ))

        self.add_col(am.IdlistsArrayConf('ids_modes_allow', modes,
                                         name='IDs allowed',
                                         info='Allowed mode IDs on this lane.',
                                         xmltag='allow',
                                         ))

        self.add_col(am.IdlistsArrayConf('ids_modes_disallow', modes,
                                         name='IDs disallow',
                                         info='Disallowed mode IDs on this lane.',
                                         xmltag='disallow',
                                         ))

        # if self.get_version() < 0.1:
        #    self.ids_modes_allow.set_value(self.modes_allow.get_value().copy())
        #    self.ids_modes_disallow.set_value(self.modes_disallow.get_value().copy())
        #    self.delete('modes_allow')
        #    self.delete('modes_disallow')

        self.add_col(am.IdsArrayConf('ids_mode', modes,
                                     groupnames=['state'],
                                     name='Main mode ID',
                                     info='ID of main mode of this lane.',
                                     is_plugin=True,
                                     ))

        self.add_col(am.IdsArrayConf('ids_edge', edges,
                                     groupnames=['state'],
                                     name='ID edge',
                                     info='ID of edge in which the lane is contained.',
                                     ))

        self.add_col(am.ListArrayConf('shapes',
                                      groupnames=['_private'],
                                      perm='rw',
                                      name='Shape',
                                      unit='m',
                                      info='List of 3D Shape coordinates to describe polyline.',
                                      is_plugin=True,
                                      ))
        self.set_version(0.2)
        # print 'Lanes._init_attributes ids_modes_allow',self.ids_modes_allow.get_value()

    def get_edges(self):
        return self.parent.edges

    def get_id_sumo(self, id_lane):
        return self.parent.edges.ids_sumo[self.ids_edge[id_lane]]+'_%d' % self.indexes[id_lane]

    def get_lengths(self, ids_lane):
        return self.parent.edges.lengths[self.ids_edge[ids_lane]]

    def multimake(self, indexes=[], **kwargs):

        n = len(indexes)
        # print 'Lanes.make',kwargs
        #width = kwargs.get('widths',None)
        #speed_max = kwargs.get('speed_max',-1)
        #modes_allow = kwargs.get('modes_allow',[])

        return self.add_rows(n=n,
                             indexes=indexes,
                             widths=kwargs['widths'],
                             speeds_max=kwargs['speeds_max'],
                             offsets_end=kwargs['offsets_end'],
                             ids_modes_allow=kwargs['ids_modes_allow'],
                             ids_modes_disallow=kwargs['ids_modes_disallow'],
                             ids_mode=kwargs['ids_mode'],
                             ids_edge=kwargs['ids_edge'],
                             # shapes = kwargs.get('shapes',[]), # if empty, then computation later from edge shape
                             )

    def make(self, **kwargs):
        edges = self.get_edges()
        id_edge = kwargs['id_edge']
        index = kwargs['index']
        # print 'Lanes.make',kwargs
        width = kwargs.get('width', -1)
        speed_max = kwargs.get('speed_max', -1)
        ids_modes_allow = kwargs.get('ids_modes_allow', [])

        is_sidewalk_edge = False
        is_sidewalk = False
        if len(ids_modes_allow) > 0:
            id_mode = ids_modes_allow[0]  # pick first as major mode
        else:
            id_mode = -1  # no mode specified

        if index == 0:
            width_sidewalk_edge = edges.widths_sidewalk[id_edge]
            is_sidewalk_edge = width_sidewalk_edge > 0
            is_sidewalk = (MODES['pedestrian'] in ids_modes_allow)  # test for pedestrian sidewalk

        if speed_max < 0:
            if (index == 0) & is_sidewalk:
                speed_max = 0.8  # default walk speed
            else:
                speed_max = edges.speeds_max[id_edge]

        # print ' is_sidewalk_edge ,is_sidewalk',is_sidewalk_edge ,is_sidewalk
        if width < 0:
            width = edges.widths_lanes_default[id_edge]

            if index == 0:
                if is_sidewalk_edge:  # edge wants sidewalks
                    width = width_sidewalk_edge
                elif (not is_sidewalk_edge) & is_sidewalk:  # edge does not want sidewalks, but actually there is a sidewalk
                    width = 0.9  # default sidewalk width
                    edges.widths_sidewalk[id_edge] = width

        # if sidewalk, then the edge attribute widths_sidewalk
        # should be set to actual lane width in case it is less than zero
        elif index == 0:  # width set for lane 0
            if (not is_sidewalk_edge) & is_sidewalk:  # edge does not want sidewalks, but actually there is a sidewalk
                edges.widths_sidewalk[id_edge] = width

        # if index == 0:
        #      edges.widths_sidewalk[id_edge]= width

        return self.add_row(indexes=index,
                            widths=width,
                            speeds_max=speed_max,
                            offsets_end=kwargs.get('offset_end', None),
                            ids_modes_allow=ids_modes_allow,
                            ids_modes_disallow=kwargs.get('ids_modes_disallow', []),
                            ids_mode=id_mode,
                            ids_edge=id_edge,
                            shapes=kwargs.get('shapes', []),  # if empty, then computation later from edge shape
                            )

    def reshape(self):
        for id_edge in self.parent.edges.get_ids():
            self.reshape_edgelanes(id_edge)

    def reshape_edgelanes(self, id_edge):
        """
        Recalculate shape of all lanes contained in edge id_edge
        based on the shape information of this edge.
        """
        #
        #lanes = self.get_lanes()
        edges = self.parent.edges
        ids_lane = edges.ids_lanes[id_edge]

        shape = np.array(edges.shapes[id_edge], np.float32)

        # print 'reshape: edgeshape id_edge,ids_lane=',id_edge,ids_lane
        # print '  shape =',shape
        n_lanes = len(ids_lane)
        n_vert = len(shape)

        angles_perb = get_angles_perpendicular(shape)

        dxn = np.cos(angles_perb)
        dyn = np.sin(angles_perb)

        #laneshapes = np.zeros((n_lanes,n_vert,3), np.float32)

        id_lane = ids_lane[0]
        # np.ones(n_lanes,np.float32)#lanes.widths[ids_lane]
        widths = self.widths[ids_lane]
        widths_tot = np.sum(widths)
        if edges.types_spread[id_edge] == 1:  # center lane spread
            widths2 = np.concatenate(([0.0], widths[:-1]))
            # print '  widths',widths_tot,widths
            # print '  widths2',widths2
            displacement = np.cumsum(widths2)
            displacement = 0.5*(widths_tot)-displacement-0.5*widths
            # print '  displacement',displacement
        else:
            widths2 = np.concatenate(([0.0], widths[:-1]))
            displacement = np.cumsum(widths2)
            displacement = displacement[-1]-displacement-0.5*widths+widths[-1]

        for i in range(n_lanes):
            id_lane = ids_lane[i]
            # print ' displacement[i] ',displacement[i]#,

            # if 1:#len(self.shapes[id_lane])==0: # make only if not existant
            laneshape = np.zeros(shape.shape, np.float32)
            # print ' dx \n',dxn*displacement[i]
            # print ' dy \n',dyn*displacement[i]
            laneshape[:, 0] = dxn*displacement[i] + shape[:, 0]
            laneshape[:, 1] = dyn*displacement[i] + shape[:, 1]
            laneshape[:, 2] = shape[:, 2]
            self.shapes[id_lane] = laneshape

        self.shapes.set_modified(True)

    def get_laneindex_allowed(self, ids_lane, id_mode):
        # print 'get_laneindex_allowed',ids_lane, id_mode
        # check ignoring mode and give it the first non pedestrian only lane
        if id_mode == MODES["ignoring"]:
            ind = 0
            while is_cont & (ind < len(ids_lane)):
                id_lane = ids_lane[ind]
                if len(self.ids_modes_allow[id_lane]) > 0:
                    if MODES["pedestrian"] not in self.ids_modes_allow[id_lane]:
                        return ind
                    else:
                        ind += 1

                else:
                    return ind
            print 'WARNING: ignoring mode has no access on footpath'
            return -1

            # return len(ids_lane)-1

        is_cont = True
        ind = 0
        while is_cont & (ind < len(ids_lane)):
            # print '  ',ind,len(self.ids_modes_allow[id_lane]),len(self.ids_modes_disallow[id_lane]),id_mode in self.ids_modes_allow[id_lane],id_mode in self.ids_modes_disallow[id_lane]
            id_lane = ids_lane[ind]
            if len(self.ids_modes_allow[id_lane]) > 0:
                if id_mode in self.ids_modes_allow[id_lane]:
                    return ind
                else:
                    ind += 1

            elif len(self.ids_modes_disallow[id_lane]) > 0:
                if id_mode in self.ids_modes_disallow[id_lane]:
                    ind += 1
                else:
                    return ind
            else:
                # no restrictions
                return ind

        # no unrestricted lane found
        return -1  # not allowed on either lane

    def add_access(self, id_lane, id_mode):
        if len(self.ids_modes_allow[id_lane]) > 0:
            # there are restrictions
            self.ids_modes_allow[id_lane].append(id_mode)

        # make sure id_mode is not disallowed
        if id_mode in self.ids_modes_disallow[id_lane]:
            self.ids_modes_disallow[id_lane].remove(id_lane)

    def get_laneindex_allowed_old(self, ids_lane, id_mode):
        is_cont = True
        ind = 0
        n_lane = len(ids_lane)
        while is_cont & (ind < n_lane):
            id_lane = ids_lane[ind]
            if len(self.ids_modes_allow[id_lane]) > 0:
                if id_mode in self.ids_modes_allow[id_lane]:
                    return ind
                else:
                    ind += 1

            elif len(self.ids_modes_disallow[id_lane]) > 0:
                if id_mode in self.ids_modes_disallow[id_lane]:
                    ind += 1
                else:
                    return ind
            else:
                # no restrictions
                return ind

        # no unrestricted lane found
        return -1  # not allowed on either lane

    def get_accesslevel(self, ids_lane, id_mode):
        """
        Returns access level of mode on lanes ids_lane:
            -1 = No access
            0 = all modes can access
            1 = mode can access with a restricted number of other modes
            2 = exclusive access for id_mode
        """
        # print 'get_accesslevel',ids_lane
        is_mode_only = False
        is_mode_mixed = False
        is_modes_all = False
        #is_blocked = False
        # print '  ids_modes_allow',self.ids_modes_allow.get_value()
        for id_lane, ids_modes_allow, ids_modes_disallow in zip(ids_lane, self.ids_modes_allow[ids_lane], self.ids_modes_disallow[ids_lane]):
            # print '  ids_modes_allow',ids_modes_allow,'ids_modes_disallow',ids_modes_disallow,'id_lane',id_lane
            if ids_modes_allow is None:
                ids_modes_allow = []
            if ids_modes_disallow is None:
                ids_modes_disallow = []

            n_allow = len(ids_modes_allow)
            #n_disallow = len(ids_modes_disallow)

            if n_allow == 0:
                is_mode_only = False
                is_modes_all = True
                is_blocked = id_mode in ids_modes_disallow

            elif n_allow == 1:
                # if id_mode == ids_modes_allow[0]:
                #    is_mode_only = True
                # else:
                #    is_blocked = True
                # break
                is_mode_only = id_mode == ids_modes_allow[0]
                is_blocked = (id_mode in ids_modes_disallow) | (not(is_mode_only))

            else:
                is_mode_only = False
                #is_blocked &=  id_mode in ids_modes_disallow
                is_blocked = (id_mode not in ids_modes_allow) | (id_mode in ids_modes_disallow)

            # print '  is_blocked',is_blocked,'is_modes_all',is_modes_all,'is_mode_only',is_mode_only
            if not is_blocked:
                # means access has been found
                break

        # print '  Final: is_blocked',is_blocked,'is_modes_all',is_modes_all,'is_mode_only',is_mode_only
        if is_blocked:  # & (not is_mode_only)&(not is_modes_all):
            a = -1
            # print '  accesslevel=',a
            return a
            # return -1

        if is_mode_only:
            a = 2
            # return 2

        elif is_modes_all:
            a = 0
            # return 0

        else:
            a = 1
            # return 1

        # print '  accesslevel=',a
        return a

    def get_coord_from_pos(self, id_lane, pos):
        return get_coord_on_polyline_from_pos(self.shapes[id_lane], pos)

    def get_sumoinfo_from_id_lane(self, id_lane):
        id_sumo_edge = self.parent.edges.ids_sumo[self.ids_edge[id_lane]]
        return id_sumo_edge+'_'+str(self.indexes[id_lane])


class Roundabouts(am.ArrayObjman):
    # http://www.sumo.dlr.de/userdoc/Networks/Building_Networks_from_own_XML-descriptions.html#Edge_Descriptions
    def __init__(self, parent, edges, nodes, **kwargs):
        ident = 'roundabouts'
        self._init_objman(ident=ident, parent=parent,
                          name='Roundabouts',
                          xmltag=('roundabouts', 'roundabout', ''),
                          **kwargs)

        self.add_col(am.IdlistsArrayConf('ids_edges', edges,
                                         groupnames=['state'],
                                         name='IDs edges',
                                         info='List with edges IDs.',
                                         xmltag='edges',
                                         ))

        self.add_col(am.IdlistsArrayConf('ids_nodes', nodes,
                                         groupnames=['state'],
                                         name='IDs Nodes',
                                         info='List with node IDs.',
                                         xmltag='nodes',
                                         ))

    def multimake(self, ids_nodes=[], **kwargs):
        n = len(ids_nodes)
        return self.add_rows(n=n,
                             ids_nodes=ids_nodes, **kwargs
                             )

    def make(self, **kwargs):
        return self.add_row(ids_nodes=kwargs['ids_node'],
                            ids_edges=kwargs['ids_edge'],
                            )


class Edges(am.ArrayObjman):
    # http://www.sumo.dlr.de/userdoc/Networks/Building_Networks_from_own_XML-descriptions.html#Edge_Descriptions
    def __init__(self, parent, **kwargs):
        ident = 'edges'
        self._init_objman(ident=ident, parent=parent,
                          name='Edges',
                          xmltag=('edges', 'edge', 'ids_sumo'),
                          version=0.2,
                          **kwargs)

        self._init_attributes()
        self._init_constants()

    def _init_attributes(self):

        self.add_col(SumoIdsConf('Edge'))

        self.add_col(am.ArrayConf('types', '',
                                  dtype=np.object,
                                  perm='rw',
                                  name='Type',
                                  info='Edge reference OSM type.',
                                  xmltag='type',  # should not be exported?
                                  ))

        self.add_col(am.ArrayConf('nums_lanes', 1,
                                  dtype=np.int32,
                                  perm='r',
                                  name='# of lanes',
                                  info='Number of lanes.',
                                  xmltag='numLanes',
                                  ))

        self.add_col(am.NumArrayConf('speeds_max', 50.0/3.6,
                                     dtype=np.float32,
                                     groupnames=['state'],
                                     perm='rw',
                                     name='Max speed',
                                     unit='m/s',
                                     info='Maximum speed on edge.',
                                     xmltag='speed',
                                     ))

        self.add_col(am.ArrayConf('priorities', 1,
                                  dtype=np.int32,
                                  perm='rw',
                                  name='Priority',
                                  info='Road priority, 1 is lowest (local access road or footpath), 13 is highest (national motorway).',
                                  xmltag='priority',
                                  ))

        self.add_col(am.NumArrayConf('lengths', 0.0,
                                     dtype=np.float32,
                                     groupnames=['state'],
                                     perm='r',
                                     name='Length',
                                     unit='m',
                                     info='Edge length.',
                                     xmltag='length ',
                                     ))
        if self.get_version() < 0.3:
            self.lengths.set_xmltag('length')
            # self.lengths.set_xmltag(None)

        self.add_col(am.NumArrayConf('widths', 0.0,
                                     dtype=np.float32,
                                     groupnames=['state'],
                                     perm='r',
                                     name='Width',
                                     unit='m',
                                     info='Edge width.',
                                     is_plugin=True,
                                     #xmltag = 'width',
                                     ))

        self.add_col(am.ListArrayConf('shapes',
                                      groupnames=['_private'],
                                      perm='rw',
                                      name='Shape',
                                      unit='m',
                                      info='Edge shape as list of 3D shape coordinates representing a polyline.',
                                      is_plugin=True,
                                      xmltag='shape',
                                      ))

        self.add_col(am.ArrayConf('types_spread', 0,
                                  choices={
                                      "right": 0,
                                      "center": 1,
                                  },
                                  dtype=np.int32,
                                  perm='rw',
                                  name='Spread type',
                                  info='Determines how the lanes are spread with respect to main link coordinates.',
                                  xmltag='spreadType',
                                  ))

        self.add_col(am.ArrayConf('names', '',
                                  dtype=np.object,
                                  perm='rw',
                                  name='Name',
                                  info='Road name, for visualization only.',
                                  xmltag='name',
                                  ))

        self.add_col(am.NumArrayConf('offsets_end', 0.0,
                                     dtype=np.float32,
                                     groupnames=['state'],
                                     perm='r',
                                     name='End offset',
                                     unit='m',
                                     info='Move the stop line back from the intersection by the given amount (effectively shortening the edge and locally enlarging the intersection).',
                                     xmltag='endOffset',
                                     ))

        self.add_col(am.NumArrayConf('widths_lanes_default', 3.5,
                                     dtype=np.float32,
                                     groupnames=['state'],
                                     perm='rw',
                                     name='Default lane width',
                                     unit='m',
                                     info='Default lane width for all lanes of this edge in meters (used for visualization).',
                                     #xmltag = '',
                                     ))

        # move this to lane in future versions
        self.add_col(am.NumArrayConf('widths_sidewalk', -1.0,
                                     dtype=np.float32,
                                     groupnames=['state'],
                                     perm='rw',
                                     name='Sidewalk width',
                                     unit='m',
                                     info='Adds a sidewalk with the given width (defaults to -1 which adds nothing).',
                                     #xmltag = 'sidewalkWidth',
                                     ))

        self.set_version(0.2)

    def _init_constants(self):
        self._segvertices = None
        self._edgeinds = None
        self._seginds = None
        self._segvertexinds = None
        self.do_not_save_attrs(['_segvertices', '_edgeinds', '_seginds', '_segvertexinds'])

    def set_nodes(self, nodes):
        # set ref to nodes table, once initialized
        self.add_col(am.IdsArrayConf('ids_fromnode', nodes,
                                     groupnames=['state'],
                                     name='ID from-node',
                                     info='ID of node at the beginning of the edge.',
                                     xmltag='from',
                                     ))

        self.add_col(am.IdsArrayConf('ids_tonode', nodes,
                                     groupnames=['state'],
                                     name='ID to-node',
                                     info='ID of node at the end of the edge.',
                                     xmltag='to',
                                     ))

    def set_lanes(self, lanes):
        self.add_col(am.IdlistsArrayConf('ids_lanes', lanes,
                                         groupnames=['state'],
                                         name='IDs Lanes',
                                         info='List with IDs of lanes.',
                                         xmltag='lanes',
                                         is_xml_include_tab=True,
                                         ))

    def get_outgoing(self, id_edge):
        # print 'get_outgoing',id_edge,self.ids_tonode[id_edge],self.parent.nodes.ids_outgoing[self.ids_tonode[id_edge]]
        ids_edges = self.parent.nodes.ids_outgoing[self.ids_tonode[id_edge]]
        if ids_edges is None:  # dead end
            return []
        else:
            return ids_edges

    def get_incoming(self, id_edge):
        # TODO: would be good to have [] as default instead of None!!
        ids_edges = self.parent.nodes.ids_incoming[self.ids_fromnode[id_edge]]
        if ids_edges is None:  # dead end
            return []
        else:
            return ids_edges

    def get_lanes(self):
        return self.parent.lanes

    def get_id_lane_from_sumoinfo_check(self, id_sumo_edge,  ind_lane):
        if self.ids_sumo.has_index(id_sumo_edge):
            id_edge = self.ids_sumo.get_id_from_index(id_sumo_edge)
            if ind_lane < self.nums_lanes[id_edge]:
                return self.ids_lanes[id_edge][ind_lane]
        return -1

    def get_id_lane_from_sumoinfo(self, id_sumo_edge,  ind_lane):
        id_edge = self.ids_sumo.get_id_from_index(id_sumo_edge)
        return self.ids_lanes[id_edge][ind_lane]

    def get_sumoinfo_from_id_lane(self, id_lane):
        return self.parent.lanes.get_sumoinfo_from_id_lane(id_lane)

    def is_oneway(self, id_edge):
        ids_incoming = self.parent.nodes.ids_incoming[self.ids_fromnode[id_edge]]
        ids_outgoing = self.parent.nodes.ids_outgoing[self.ids_tonode[id_edge]]
        if (ids_incoming is None) | (ids_outgoing is None):
            return True
        else:
            return set(ids_incoming).isdisjoint(ids_outgoing)

    def has_sidewalk(self, id_edge):
        return MODES["pedestrian"] in self.parent.lanes.ids_modes_allow[self.ids_lanes[id_edge][0]]

    def get_fstar(self, is_return_lists=False, is_return_arrays=False,
                  is_ignor_connections=False):
        """
        Returns the forward star graph of the network as dictionary:
            fstar[id_fromedge] = set([id_toedge1, id_toedge2,...])

            if is_return_lists = True then a list of edges is the value
            of fstar

            if is_return_arrays = True then a numpy array of edges is the value
            of fstar

            if is_ignor_connections = True then all possible successive edges
            are considered, disregarding the actual connections

        """
        #ids_edge = self.get_ids()
        #fstar = np.array(np.zeros(np.max(ids_edge)+1, np.obj))
        fstar = {}
        if is_ignor_connections:
            # here we ignore connections and look at the
            # outgoing edges of node database
            ids_outgoing = self.parent.nodes.ids_outgoing
            ids = self.get_ids()
            for id_edge, id_tonode in zip(ids, self.ids_tonode[ids]):
                ids_edge_outgoing = ids_outgoing[id_tonode]
                if ids_edge_outgoing is not None:
                    fstar[id_edge] = set(ids_edge_outgoing)
                else:
                    fstar[id_edge] = set()
        else:
            # here we check actual connections
            # this is important if correct turns are desired
            # for exampole in car routing
            connections = self.parent.connections
            lanes = self.parent.lanes
            inds_con = connections.get_inds()
            ids_fromedge = lanes.ids_edge[connections.ids_fromlane.get_value()[inds_con]]
            ids_toedge = lanes.ids_edge[connections.ids_tolane.get_value()[inds_con]]

            for id_edge in self.get_ids():
                fstar[id_edge] = set()

            for id_fromedge, id_toedge in zip(ids_fromedge, ids_toedge):
                fstar[id_fromedge].add(id_toedge)

        if is_return_lists | is_return_arrays:
            for id_edge in self.get_ids():
                ids_toedges = list(fstar[id_edge])
                if is_return_arrays:
                    fstar[id_edge] = np.array(ids_toedges, dtype=np.int32)
                else:
                    fstar[id_edge] = ids_toedges
        return fstar

    def get_bstar(self, is_return_lists=False, is_return_arrays=False,
                  is_ignor_connections=False):
        """
        Returns the backward star graph of the network as dictionary:
            fstar[id_fromedge] = set([id_fromedge1, id_fromedge2,...])

            if is_return_lists = True then a list of edges is the value
            of bstar

            if is_return_arrays = True then a numpy array of edges is the value
            of bstar

            if is_ignor_connections = True then all possible preceding edges
            are considered, disregarding the actual connections

        """
        #ids_edge = self.get_ids()
        #fstar = np.array(np.zeros(np.max(ids_edge)+1, np.obj))
        bstar = {}
        if is_ignor_connections:
            # here we ignore connections and look at the
            # outgoing edges of node database
            ids_incoming = self.parent.nodes.ids_incoming
            ids = self.get_ids()
            for id_edge, id_fromnode in zip(ids, self.ids_fromnode[ids]):
                ids_edge_incoming = ids_incoming[id_fromnode]
                if ids_edge_incoming is not None:
                    bstar[id_edge] = set(ids_edge_incoming)
                else:
                    bstar[id_edge] = set()
        else:
            # here we check actual connections
            # this is important if correct turns are desired
            # for exampole in car routing
            connections = self.parent.connections
            lanes = self.parent.lanes
            inds_con = connections.get_inds()
            ids_fromedge = lanes.ids_edge[connections.ids_fromlane.get_value()[inds_con]]
            ids_toedge = lanes.ids_edge[connections.ids_tolane.get_value()[inds_con]]

            for id_edge in self.get_ids():
                bstar[id_edge] = set()

            for id_fromedge, id_toedge in zip(ids_fromedge, ids_toedge):
                bstar[id_toedge].add(id_fromedge)

        if is_return_lists | is_return_arrays:
            for id_edge in self.get_ids():
                ids_fromedges = list(bstar[id_edge])
                if is_return_arrays:
                    bstar[id_edge] = np.array(ids_fromedges, dtype=np.int32)
                else:
                    bstar[id_edge] = ids_fromedges
        return bstar

    def get_accesslevel(self, id_edge, id_mode):
        """
        Returns access level of mode on edge id_edge:
            -1 = No access
            0 = all modes can access
            1 = mode can access with a restricted number of other modes
            2 = exclusive access for id_mode
        """
        # print 'get_accesslevel',id_edge,self.ids_sumo[id_edge]
        return self.parent.lanes.get_accesslevel(self.ids_lanes[id_edge], id_mode)

    def get_accesslevels(self, id_mode):
        """
        The returned array represents the access levels that corresponds to
        edge IDs.

        Access levels of mode on edge id_edge:
            -1 = No access
            0 = all modes can access
            1 = mode can access with a restricted number of other modes
            2 = exclusive access for id_mode
        """
        get_accesslevel = self.parent.lanes.get_accesslevel
        ids_edge = self.get_ids()
        accesslevels = np.zeros(np.max(ids_edge)+1, np.int8)
        for id_edge, ids_lane in zip(ids_edge, self.ids_lanes[ids_edge]):
            accesslevels[id_edge] = get_accesslevel(ids_lane, id_mode)
        return accesslevels

    def get_distances(self, id_mode=0, is_check_lanes=False):
        """
        Returns distances for all edges.
        The returned array represents the distance that corresponds to
        edge IDs.

        If is_check_lanes is True, then the lane speeds are considered where
        the respective mode is allowed.

        If not allowed on a particular edge,
        then the respective edge distance is negative.
        """
        # print 'get_distances id_mode,is_check_lanes,speed_max',id_mode,is_check_lanes,speed_max
        ids_edge = self.get_ids()
        dists = np.zeros(np.max(ids_edge)+1, np.float32)
        #speeds = self.speeds_max[ids_edge]

        # if speed_max is not None:
        #    speeds = np.clip(speeds, 0.0, speed_max)
        #
        # elif id_mode is not None:
        #    # limit allowed speeds with max speeds of mode
        #    speeds = np.clip(speeds, 0.0, self.parent.modes.speeds_max[id_mode])

        radii = self.ids_fromnode.get_linktab().radii
        dists[ids_edge] = radii[self.ids_fromnode[ids_edge]] + self.lengths[ids_edge] + radii[self.ids_tonode[ids_edge]]
        ids_lanes = self.ids_lanes
        if is_check_lanes & (id_mode > 0):  # mode 0 can pass everywhere
            get_laneindex_allowed = self.parent.lanes.get_laneindex_allowed
            has_noaccess = -1
            for id_edge in ids_edge:
                if get_laneindex_allowed(ids_lanes[id_edge], id_mode) == has_noaccess:
                    dists[id_edge] = has_noaccess

                #ind = get_laneindex_allowed(ids_lanes[id_edge], id_mode)
                # print '  check id_edge, ind',id_edge, ind
                # if ind<0:
                #    dists[id_edge] = ind # =-1

        return dists

    def get_times(self, id_mode=0, is_check_lanes=False, speed_max=None,
                  modeconst_excl=0.0, modeconst_mix=0.0, ):
        """
        Returns freeflow travel times for all edges..radii
        The returned array represents the travel time that corresponds to
        edge IDs.

        If is_check_lanes is True, then the lane speeds are considered where
        the respective mode is allowed.

        If not allowed on a particular edge,
        then the respective edge travel time is negative.

        modeconst_excl and modeconst_mix are constants added to the
        time if the respective edge provides exclusive or reserver mixed
        access for the specifird mode

        """
        # print 'get_times id_mode,is_check_lanes,speed_max',id_mode,is_check_lanes,speed_max
        ids_edge = self.get_ids()
        times = np.zeros(np.max(ids_edge)+1, np.float32)
        speeds = self.speeds_max[ids_edge]

        if speed_max is not None:
            speeds = np.clip(speeds, 0.0, speed_max)

        elif id_mode is not None:
            # limit allowed speeds with max speeds of mode
            speeds = np.clip(speeds, 0.0, self.parent.modes.speeds_max[id_mode])

        times[ids_edge] = self.lengths[ids_edge]/speeds
        ids_lanes = self.ids_lanes
        if is_check_lanes & (id_mode > 0):  # mode 0 can pass everywhere
            #get_laneindex_allowed = self.parent.lanes.get_laneindex_allowed
            get_accesslevel = self.parent.lanes.get_accesslevel
            invalid = -1
            #ids_edge = self.get_ids()
            #accesslevels = np.zeros(np.max(ids_edge)+1, np.int8)
            for id_edge, ids_lane in zip(ids_edge, self.ids_lanes[ids_edge]):
                #accesslevels[id_edge] = get_accesslevel(ids_lane, id_mode)
                accesslevel = get_accesslevel(ids_lane, id_mode)
                if accesslevel == invalid:
                    times[id_edge] = invalid
                elif accesslevel == 2:
                    times[id_edge] = max(times[id_edge] + modeconst_excl, 0)
                elif accesslevel == 1:
                    times[id_edge] = max(times[id_edge] + modeconst_mix, 0)
                    # here we could multiply with a factor
                #ind = get_laneindex_allowed(ids_lanes[id_edge], id_mode)
                # print '  check id_edge, ind',id_edge, ind
                # if ind<0:
                #    # making times negative will prevent the router to use
                #    # this edge
                #    times[id_edge] = ind # =-1

        return times

    def select_accessible_mode(self, id_mode):
        """
        Returns an array with all allowed edge ids for this mode
        and an array with the corrisponding lane index
        """
        get_laneindex_allowed = self.parent.lanes.get_laneindex_allowed
        ids_lanes = self.ids_lanes
        ids_edges = self.get_ids()
        are_allowed = np.zeros(len(ids_edges), dtype=np.bool)
        inds_lane = np.zeros(len(ids_edges), dtype=np.int32)
        for i, id_edge in zip(xrange(len(ids_edges)), ids_edges):
            ind_lane = get_laneindex_allowed(ids_lanes[id_edge], id_mode)
            are_allowed[i] = ind_lane >= 0
            inds_lane[i] = ind_lane
        return ids_edges[are_allowed], inds_lane[are_allowed]

    def get_laneindex_allowed(self, id_edge, id_mode):
        """
        Returns first lane index of edge id_edge on which id_mode
        is allowed.
        -1 means not allowed on edge
        """
        return self.parent.lanes.get_laneindex_allowed(self.ids_lanes[id_edge], id_mode)

    def get_laneid_allowed(self, id_edge, id_mode):
        """
        Returns first lane ID of edge id_edge on which id_mode
        is allowed.
        -1 means not allowed on edge
        """
        ids_lane = self.ids_lanes[id_edge]
        laneind = self.parent.lanes.get_laneindex_allowed(ids_lane, id_mode)
        if laneind == -1:
            return -1
        else:
            return ids_lane[laneind]

    def multimake(self, ids_sumo=[], **kwargs):
        # fixing of insufficient shape data in edge reader
        return self.add_rows(n=len(ids_sumo), ids_sumo=ids_sumo,  **kwargs)

    def add_reverse(self, id_edge):
        self.types_spread[id_edge] = self.types_spread.choices["right"]

        id_edge_reverse = self.add_row(ids_sumo='-'+self.ids_sumo[id_edge],
                                       ids_fromnode=self.ids_tonode[id_edge],
                                       ids_tonode=self.ids_fromnode[id_edge],
                                       types=self.types[id_edge],
                                       nums_lanes=self.num_lanes[id_edge],
                                       speeds_max=self.speed_max[id_edge],
                                       priorities=self.priority[id_edge],
                                       #lengths = length,
                                       shapes=self.shapes[id_edge][::-1],
                                       types_spread=self.types_spread[id_edge],
                                       names=self.names[id_edge],
                                       offsets_end=self.offsets_end[id_edge],
                                       widths_lanes_default=self.widths_lanes_default[id_edge],
                                       widths_sidewalk=self.widths_sidewalk[id_edge],
                                       )
        # TODO: add lanes and connections
        return id_edge_reverse

    def make(self, id_fromnode=0,
             id_tonode=0,
             id_sumo='',
             type_edge='',
             num_lanes=1,
             speed_max=50.0/3.6,
             priority=1,
             #length = 0.0,
             shape=[],
             type_spread='right',
             name='',
             offset_end=0.0,
             width_lanes_default=None,
             width_sidewalk=-1,
             ):

        if len(shape) < 2:  # insufficient shape data
            #shape = np.array([ nodes.coords[id_fromnode], nodes.coords[id_tonode] ], np.float32)
            # shape should be a list of np array coords
            # ATTENTIOn: we need to copy here, otherwise the reference
            # to node coordinates will be kept!!
            coords = self.ids_tonode.get_linktab().coords
            shape = [1.0*coords[id_fromnode], 1.0*coords[id_tonode]]

        # print 'Edges.make'
        # print '  shape',shape,type(shape)

        return self.add_row(ids_sumo=id_sumo,
                            ids_fromnode=id_fromnode,
                            ids_tonode=id_tonode,
                            types=type_edge,
                            nums_lanes=num_lanes,
                            speeds_max=speed_max,
                            priorities=priority,
                            #lengths = length,
                            shapes=shape,
                            types_spread=self.types_spread.choices[type_spread],
                            names=name,
                            offsets_end=offset_end,
                            widths_lanes_default=width_lanes_default,
                            widths_sidewalk=width_sidewalk,
                            )

    def make_segment_edge_map(self, ids=None, is_laneshapes=True):
        """
        Generates a vertex matrix with line segments of all edges
        and a map that maps each line segment to edge index.
        """
        # TODO: _seginds not correctly constructed for given ids

        # here we can make some selection on edge inds
        if ids is None:
            inds = self.get_inds()
        else:
            inds = self.get_inds(ids)
        print 'make_linevertices', len(inds)

        linevertices = np.zeros((0, 2, 3), np.float32)
        vertexinds = np.zeros((0, 2), np.int32)
        polyinds = []

        lineinds = []
        #linecolors = []
        #linecolors_highl = []
        linebeginstyles = []
        lineendstyles = []

        i = 0
        ind_line = 0

        if is_laneshapes:
            ids_lanes = self.ids_lanes.get_value()
            laneshapes = self.parent.lanes.shapes
        else:
            polylines = self.shapes.get_value()  # [inds]

        #polylines = self.shapes[inds]
        # print '  len(polylines)',len(polylines)
        for ind in inds:

            if is_laneshapes:
                polyline = laneshapes[ids_lanes[ind][0]]
            else:
                polyline = polylines[ind]

            n_seg = len(polyline)
            # print '  =======',n_seg#,polyline

            if n_seg > 1:
                polyvinds = range(n_seg)
                # print '  polyvinds\n',polyvinds
                vi = np.zeros((2*n_seg-2), np.int32)
                vi[0] = polyvinds[0]
                vi[-1] = polyvinds[-1]

                # Important type conversion!!
                v = np.zeros((2*n_seg-2, 3), np.float32)
                v[0] = polyline[0]
                v[-1] = polyline[-1]
                if len(v) > 2:

                    # print 'v[1:-1]',v[1:-1]
                    # print 'v=\n',v
                    #m = np.repeat(polyline[1:-1],2,0)
                    # print 'm\n',m,m.shape,m.dtype
                    #v[1:-1] = m
                    v[1:-1] = np.repeat(polyline[1:-1], 2, 0)
                    vi[1:-1] = np.repeat(polyvinds[1:-1], 2)
                #vadd = v.reshape((-1,2,3))
                # print '  v\n',v
                # print '  vi\n',vi

                n_lines = len(v)/2
                # print '  v\n',v
                polyinds += n_lines*[ind]
                lineinds.append(np.arange(ind_line, ind_line+n_lines))
                ind_line += n_lines
                # print '  polyinds\n',polyinds,n_lines
                #linecolors += n_lines*[colors[ind]]
                #linecolors_highl += n_lines*[colors_highl[ind]]

                # print '  linebeginstyle',linebeginstyle,beginstyles[ind]

            else:
                # empty polygon treatment
                v = np.zeros((0, 3), np.float32)
                vi = np.zeros((0), np.int32)

            linevertices = np.concatenate((linevertices, v.reshape((-1, 2, 3))))
            vertexinds = np.concatenate((vertexinds, vi.reshape((-1, 2))))
            # print '  linevertex\n',linevertices
            i += 1
        self._segvertices = linevertices

        self._edgeinds = np.array(polyinds, np.int32)
        self._seginds = lineinds
        self._segvertexinds = np.array(vertexinds, np.int32)

    def get_dist_point_to_edge(self, p, id_edge,
                               is_detect_initial=False,
                               is_detect_final=False,
                               is_return_segment=False):
        """
        Returns eucledian distance from a point p to a given edge.
        As a second argument it returns the coordinates of the
        line segment (x1,y1,x2,y2) which is closest to the point.
        """
        inds_seg = self.get_inds_seg_from_id_edge(id_edge)
        vertices = self._segvertices
        x1 = vertices[inds_seg, 0, 0]
        y1 = vertices[inds_seg, 0, 1]

        x2 = vertices[inds_seg, 1, 0]
        y2 = vertices[inds_seg, 1, 1]

        dists2 = get_dist_point_to_segs(p[0:2], x1, y1, x2, y2,
                                        is_ending=True,
                                        is_detect_initial=is_detect_initial,
                                        is_detect_final=is_detect_final
                                        )
        if is_detect_final | is_detect_initial:
            are_finals = np.isnan(dists2)
            # print '  dists2',dists2
            # print '  are_finals',are_finals
            if np.all(are_finals):  # point outside all segments of edge
                if is_return_segment:
                    return np.nan, [np.nan, np.nan, np.nan, np.nan]
                else:
                    return np.nan
            else:
                dists2[are_finals] = np.inf

        ind_min = np.argmin(dists2)
        if is_return_segment:
            return np.sqrt(dists2[ind_min]), (x1[ind_min], y1[ind_min], x2[ind_min], y2[ind_min])
        else:
            return np.sqrt(dists2[ind_min])

    def get_closest_edge(self, p, is_get2=False):
        """
        Returns edge id which is closest to point p.
        Requires execution of make_segment_edge_map
        """
        # print 'get_closest_edge',p
        if len(self) == 0:
            return np.array([], np.int)

        if self._segvertices is None:
            self.make_segment_edge_map()

        vertices = self._segvertices
        x1 = vertices[:, 0, 0]
        y1 = vertices[:, 0, 1]

        x2 = vertices[:, 1, 0]
        y2 = vertices[:, 1, 1]

        # print '  x1', x1
        # print '  x2', x2
        #halfwidths = 0.5*self.get_widths_array()[self._polyinds]
        d2 = get_dist_point_to_segs(p[0:2], x1, y1, x2, y2, is_ending=True)
        # print '  min(d2)=',np.min(d2),'argmin=',np.argmin(d2),self.get_ids(self._edgeinds[np.argmin(d2)])
        if not is_get2:
            return self.get_ids(self._edgeinds[np.argmin(d2)])
        else:
            # return 2 best matches
            ind1 = np.argmin(d2)
            id_edge1 = self.get_ids(self._edgeinds[ind1])
            d2[ind1] = np.inf
            id_edge2 = self.get_ids(self._edgeinds[np.argmin(d2)])
            return [id_edge1, id_edge2]

    def get_ids_edge_from_inds_seg(self, inds_seg):
        return self.get_ids(self._edgeinds[inds_seg])

    def get_inds_seg_from_id_edge(self, id_edge):
        # print 'get_inds_seg_from_id_edge id_edge, ind_edge',id_edge,self.get_ind(id_edge)
        return self._seginds[self.get_ind(id_edge)]

    def get_segvertices_xy(self):
        if self._segvertices is None:
            self.make_segment_edge_map()

        vertices = self._segvertices
        x1 = vertices[:, 0, 0]
        y1 = vertices[:, 0, 1]

        x2 = vertices[:, 1, 0]
        y2 = vertices[:, 1, 1]
        return x1, y1, x2, y2

    def get_closest_edge_fast(self, p, x1, y1, x2, y2):
        """
        Returns edge id which is closest to point p.
        Requires execution of make_segment_edge_map
        and predetermined 2d segment coordinates with  get_segvertices_xy
        """
        d2 = get_dist_point_to_segs(p[0:2], x1, y1, x2, y2, is_ending=True)
        # print '  min(d2)=',np.min(d2),'argmin=',np.argmin(d2),self.get_ids(self._edgeinds[np.argmin(d2)])
        return self.get_ids(self._edgeinds[np.argmin(d2)])

    def export_sumoxml(self, filepath, encoding='UTF-8'):
        try:
            fd = open(filepath, 'w')
        except:
            print 'WARNING in export_sumoxml: could not open', filepath
            return False
        fd.write('<?xml version="1.0" encoding="%s"?>\n' % encoding)

        fd.write(xm.begin('edges'))
        indent = 2
        self.write_xml(fd, indent=indent, is_print_begin_end=False)
        self.parent.roundabouts.write_xml(fd, indent=indent, is_print_begin_end=False)
        fd.write(xm.end('edges'))
        fd.close()

    def update(self, ids=None, is_update_lanes=False):
        print 'Edges.update'

        if ids is None:
            self.widths.value = self.nums_lanes.value * self.widths_lanes_default.value \
                + (self.widths_sidewalk.value >= 0) * (self.widths_sidewalk.value-self.widths_lanes_default.value)

            # print '  self.widths.values =  \n',self.widths.value
            #polylines = polypoints_to_polylines(self.shapes.value)
            # print '  polylines[0:4]=\n',polylines[0:4]
            # print '  polylines[3].shape',polylines[3].shape
            #self.lengths.value = get_length_polylines(polypoints_to_polylines(self.shapes.value))
            # if len(self)>10:
            #    print '  shapes',self.shapes[1:10]
            #    print '  shapes.get_value',self.shapes.get_value()[0:9]
            #    print '  shapes.value',self.shapes.value[0:9]
            # else:
            #    print '  only len %d'%len(self)
            self.lengths.value = get_length_polypoints(self.shapes.value)
            ids = self.get_ids()
        else:
            self.widths[ids] = self.nums_lanes[ids] * self.widths_lanes_default[ids] \
                + (self.widths_sidewalk[ids] >= 0) * (self.widths_sidewalk[ids]-self.widths_lanes_default[ids])
            # print '  self.shapes[ids]',self.shapes[ids],type(self.shapes[ids])
            self.lengths[ids] = get_length_polypoints(self.shapes[ids])

        self.widths.set_modified(True)
        self.lengths.set_modified(True)

        if is_update_lanes:
            # print 'recalc laneshapes',ids
            lanes = self.get_lanes()
            for id_edge in ids:
                lanes.reshape_edgelanes(id_edge)
            self.make_segment_edge_map()

    def set_shapes(self, ids, vertices, is_update_lanes=True):
        # print 'set_shapes',ids,vertices

        self.shapes[ids] = vertices
        if not hasattr(ids, '__iter__'):
            ids = [ids]
        self.update(ids, is_update_lanes=is_update_lanes)

    def get_coord_from_pos(self, id_edge, pos):
        """
        Returns network coordinate on edge id_edge at position pos.
        """
        return get_coord_on_polyline_from_pos(self.shapes[id_edge], pos)

    def get_pos_from_coord(self, id_edge, coord):
        """
        Returns position on edge id_edge with coord
        perpendicularly projected on edge.
        """

        return get_pos_on_polyline_from_coord(self.shapes[id_edge], coord)

    def update_lanes(self, id_edge, ids_lane):
        # print 'update_lanes',id_edge,self.ids_sumo[id_edge] ,ids_lanes,self.nums_lanes[id_edge]
        # if self._is_laneshape:
        #    laneshapes = edges.get_laneshapes(self._id_edge, )
        #    lanes.shapes[self._ids_lanes[0]]
        if len(ids_lane) == 0:
            # no lanes given...make some with default values
            ids_lane = []
            lanes = self.get_lanes()
            for i in xrange(self.nums_lanes[id_edge]):
                id_lane = lanes.make(index=i, id_edge=id_edge)
                ids_lane.append(id_lane)

        self.ids_lanes[id_edge] = ids_lane

    def correct_endpoint(self):
        """
        Corrects end-point for older versione.
        """
        ids_sumo = self.ids_sumo.get_value()
        types_spread = self.types_spread.get_value()
        shapes = self.shapes.get_value()
        ids_fromnode = self.ids_fromnode.get_value()
        ids_tonode = self.ids_tonode.get_value()
        coords = self.parent.nodes.coords
        ind = 0
        is_corrected = False
        eps = 50.0
        for id_sumo, type_spread, shape, id_fromnode, id_tonode in zip(ids_sumo, types_spread, shapes, ids_fromnode, ids_tonode):

            inds_oppo = np.flatnonzero((ids_tonode == id_fromnode) & (ids_fromnode == id_tonode))
            if len(inds_oppo) >= 1:
                ind_oppo = inds_oppo[0]
                # print '  correct',id_sumo,ids_sumo[ind_oppo]

                ind_oppo = inds_oppo[0]
                shape_oppo = list(shapes[ind_oppo])
                shape_oppo.reverse()
                # print '  shape',shape
                # print '  shape',shape_oppo
                # print '  id_fromnode',id_fromnode,ids_tonode[ind_oppo]
                # print '  id_tomnode',id_tonode,ids_fromnode[ind_oppo]
                # print '  coords',coords[id_fromnode], coords[id_tonode]
                if len(shape_oppo) == len(shape):

                    shapes[ind][0] = coords[id_fromnode]
                    shapes[ind_oppo][-1] = coords[id_fromnode]
                    #types_spread[inds_oppo[0]] = 0
                    #types_spread[ind] = 0
                    is_corrected = True
            ind += 1

        if is_corrected:
            self.update(is_update_lanes=True)

    def correct_spread(self):
        """
        Corrects spread type for older versione.
        """
        ids_sumo = self.ids_sumo.get_value()
        types_spread = self.types_spread.get_value()
        shapes = self.shapes.get_value()
        ind = 0
        is_corrected = False
        eps = 50.0
        for id_sumo, type_spread, shape in zip(ids_sumo, types_spread, shapes):
            if type_spread == 1:
                if id_sumo[0] == '-':
                    inds_oppo = np.flatnonzero(ids_sumo == id_sumo[1:])
                    if len(inds_oppo) == 1:
                        ind_oppo = inds_oppo[0]
                        shape_oppo = np.array(shapes[ind_oppo], np.float32)
                        if len(shape_oppo) == len(shape):
                            shape_oppo = list(shapes[ind_oppo])
                            shape_oppo.reverse()
                            shape_oppo = np.array(shape_oppo, np.float32)
                            dist = np.sum(np.abs(shape_oppo - np.array(shape, np.float32)))/float(len(shape))
                            # print '   id_sumo,dist',id_sumo,dist,eps
                            if dist < eps:
                                types_spread[inds_oppo[0]] = 0
                                types_spread[ind] = 0
                                is_corrected = True
            ind += 1

        if is_corrected:
            self.update(is_update_lanes=True)


class Nodes(am.ArrayObjman):
    # http://www.sumo.dlr.de/userdoc/Networks/Building_Networks_from_own_XML-descriptions.html#Node_Descriptions
    def __init__(self, parent,
                 **kwargs):
        ident = 'nodes'
        self._init_objman(ident=ident, parent=parent, name='Nodes',
                          xmltag=('nodes', 'node', 'ids_sumo'),
                          version=0.1,
                          **kwargs)
        self._init_attributes()

    def _init_attributes(self):

        self.add_col(SumoIdsConf('Node'))

        self.add_col(am.ArrayConf('coords',  np.zeros(3, dtype=np.float32),
                                  dtype=np.float32,
                                  groupnames=['state'],
                                  perm='r',
                                  name='Coords',
                                  unit='m',
                                  info='Node center coordinates.',
                                  ))

        self.add_col(am.ArrayConf('radii',  5.0,
                                  dtype=np.float32,
                                  groupnames=['state'],
                                  perm='rw',
                                  name='Radius',
                                  info='Node radius',
                                  ))

        self.add_col(am.ListArrayConf('shapes',
                                      groupnames=['_private'],
                                      perm='rw',
                                      name='Shape',
                                      unit='m',
                                      info='Node shape as list of 3D shape coordinates representing a polyline.',
                                      is_plugin=True,
                                      xmltag='shape',
                                      ))

        self.add_col(am.ArrayConf('are_costum_shape',  False,
                                  dtype=np.bool,
                                  groupnames=['state'],
                                  perm='rw',
                                  name='costum shape',
                                  info='Node has a custom shape.',
                                  xmltag='customShape',
                                  ))

        self.add(cm.AttrConf('radius_default', 3.0,
                             groupnames=['options'],
                             perm='rw',
                             unit='m',
                             name='Default radius',
                             info='Default node radius.',
                             ))

        self.add_col(am.ArrayConf('types', 0,
                                  choices={
                                      "priority": 0,
                                      "traffic_light": 1,
                                      "right_before_left": 2,
                                      "unregulated": 3,
                                      "priority_stop": 4,
                                      "traffic_light_unregulated": 5,
                                      "allway_stop": 6,
                                      "rail_signal": 7,
                                      "zipper": 8,
                                      "traffic_light_right_on_red": 9,
                                      "rail_crossing": 10,
                                      "dead_end": 11,
                                  },
                                  dtype=np.int32,
                                  perm='rw',
                                  name='Type',
                                  info='Node type.',
                                  xmltag='type',
                                  ))

        # this is actually a property defined in the TLS logic
        self.add_col(am.ArrayConf('types_tl', 0,
                                  dtype=np.int32,
                                  choices={
                                      "none": 0,
                                      "static": 1,
                                      "actuated": 2,
                                  },
                                  perm='rw',
                                  name='TL type',
                                  info='Traffic light type.',
                                  xmltag='tlType',
                                  ))

        self.add_col(am.ArrayConf('turnradii',  1.5,
                                  dtype=np.float32,
                                  groupnames=['state'],
                                  perm='rw',
                                  name='Turn rad',
                                  unit='m',
                                  info='optional turning radius (for all corners) for that node.',
                                  xmltag='radius',
                                  ))

        self.add_col(am.ArrayConf('are_keep_clear',  True,
                                  dtype=np.bool,
                                  groupnames=['state'],
                                  perm='rw',
                                  name='keep clear',
                                  info='Whether the junction-blocking-heuristic should be activated at this node.',
                                  xmltag='keepClear',
                                  ))

    def set_edges(self, edges):

        self.add_col(am.IdlistsArrayConf('ids_incoming', edges,
                                         groupnames=['state'],
                                         name='ID incoming',
                                         info='ID list of incoming edges.',
                                         ))

        self.add_col(am.IdlistsArrayConf('ids_outgoing', edges,
                                         groupnames=['state'],
                                         name='ID outgoing',
                                         info='ID list of outgoing edges.',
                                         ))

        self.add_col(am.IdlistsArrayConf('ids_controlled', edges,
                                         groupnames=['state'],
                                         name='IDs controlled',
                                         info='ID list of controlled edges. Edges which shall be controlled by a joined TLS despite being incoming as well as outgoing to the jointly controlled nodes.',
                                         xmltag='controlledInner',
                                         ))
        if self.get_version() < 0.1:
            self.delete('ids_tl_prog')
            self.turnradii.xmltag = 'radius'
            self.are_keep_clear.xmltag = 'keepClear'
            self.types_tl.xmltag = 'tlType'
            self.add_col(am.IdlistsArrayConf('ids_controlled', edges,
                                             groupnames=['state'],
                                             name='IDs controlled',
                                             info='ID list of controlled edges. Edges which shall be controlled by a joined TLS despite being incoming as well as outgoing to the jointly controlled nodes.',
                                             ))

    def set_tlss(self, tlss):

        self.add_col(am.IdsArrayConf('ids_tls',  tlss,
                                     groupnames=['state'],
                                     name='ID Tls',
                                     info='ID of traffic light system (TLS). Nodes with the same tls-value will be joined into a single traffic light system.',
                                     xmltag='tl',
                                     ))

    def multimake(self, ids_sumo=[], **kwargs):
        return self.add_rows(n=len(ids_sumo), ids_sumo=ids_sumo,  **kwargs)

    def configure_tls(self, id_node, id_tls, typecode=None, tlstype=None, nodetype="traffic_light"):
        if typecode is None:
            self.types_tl[id_node] = self.types_tl.choices[tlstype]
        else:
            self.types_tl[id_node] = typecode

        self.types[id_node] = self.types.choices[nodetype]
        self.ids_tls[id_node] = id_tls

    def remove_tls(self, id_node=None, id_tls=None, nodetype="priority"):
        if id_tls is not None:
            ids_node = self.select_ids(self.ids_tls.get_value() == id_tls)

            self.types_tl[ids_node] = self.types_tl.choices["none"]
            self.types[ids_node] = self.types.choices[nodetype]
            self.ids_tls[ids_node] = -1
        else:
            self.types_tl[id_node] = self.types_tl.choices["none"]
            self.types[id_node] = self.types.choices[nodetype]
            self.ids_tls[id_node] = -1

    def make(self, id_sumo='', nodetype='priority', coord=[],
             type_tl='static', id_tl_prog=0,
             shape=[],
             is_costum_shape=False,
             turnradius=1.5,
             is_keep_clear=True):

        return self.add_row(ids_sumo=id_sumo,
                            types=self.types.choices[nodetype],
                            coords=coord,
                            are_costum_shape=is_costum_shape,
                            shape=shape,
                            types_tl=self.types_tl.choices[type_tl],
                            ids_tl_prog=id_tl_prog,
                            turnradii=turnradius,
                            are_keep_clear=is_keep_clear,
                            )

    def add_outgoing(self, id_node, id_edge):
        if self.ids_outgoing[id_node] is not None:
            if id_edge not in self.ids_outgoing[id_node]:
                self.ids_outgoing[id_node].append(id_edge)
        else:
            self.ids_outgoing[id_node] = [id_edge]

    def add_incoming(self, id_node, id_edge):
        # print 'add_incoming id_node,id_edge',id_node,id_edge
        # print '  ids_incoming',self.ids_incoming[id_node],type(self.ids_incoming[id_node])
        if self.ids_incoming[id_node] is not None:
            if id_edge not in self.ids_incoming[id_node]:
                self.ids_incoming[id_node].append(id_edge)
        else:
            self.ids_incoming[id_node] = [id_edge]

    def export_sumoxml(self, filepath, encoding='UTF-8'):
        try:
            fd = open(filepath, 'w')
        except:
            print 'WARNING in export_sumoxml: could not open', filepath
            return False
        fd.write('<?xml version="1.0" encoding="%s"?>\n' % encoding)
        indent = 0
        self.write_xml(fd, indent)
        fd.close()

    def write_xml(self, fd, indent):
        # print 'Nodes.write_xml'
        xmltag, xmltag_item, attrname_id = self.xmltag
        attrsman = self.get_attrsman()
        attrconfig_id = attrsman.get_config(attrname_id)  # getattr(self.get_attrsman(), attrname_id)
        xmltag_id = attrconfig_id.xmltag
        #attrsman = self.get_attrsman()
        coordsconfig = attrsman.get_config('coords')
        shapesconfig = attrsman.get_config('shapes')
        colconfigs = attrsman.get_colconfigs(is_all=True)

        # print '  header'
        fd.write(xm.start(xmltag, indent))
        # print '  ', self.parent.get_attrsman().get_config('version').attrname,self.parent.get_attrsman().get_config('version').get_value()
        #fd.write( self.parent.get_attrsman().get_config('version').write_xml(fd) )
        self.parent.get_attrsman().get_config('version').write_xml(fd)
        fd.write(xm.stop())

        fd.write(xm.start('location', indent+2))
        # print '  groups:',self.parent.get_attrsman().get_groups()

        for attrconfig in self.parent.get_attrsman().get_group('location'):
            # print '    locationconfig',attrconfig.attrname
            attrconfig.write_xml(fd)
        fd.write(xm.stopit())

        ids = self.get_ids()
        for _id, is_costum_shape in zip(ids, self.are_costum_shape[ids]):
            fd.write(xm.start(xmltag_item, indent+2))

            # print ' make tag and id',_id
            fd.write(xm.num(xmltag_id, attrconfig_id[_id]))

            # print ' write columns'
            for attrconfig in colconfigs:
                # print '    colconfig',attrconfig.attrname
                if attrconfig == coordsconfig:
                    x, y, z = attrconfig[_id]
                    fd.write(xm.num('x', x))
                    fd.write(xm.num('y', y))
                    fd.write(xm.num('z', z))

                elif attrconfig == shapesconfig:
                    # write only if customshaped
                    if is_costum_shape:
                        attrconfig.write_xml(fd, _id)

                elif attrconfig != attrconfig_id:
                    attrconfig.write_xml(fd, _id)

            fd.write(xm.stopit())

        fd.write(xm.end(xmltag, indent))

    # def clean_node(self, id_node):

    def clean(self, is_reshape_edgelanes=False, nodestretchfactor=1.2, n_min_nodeedges=2):
        # is_reshape_edgelanes = False, nodestretchfactor = 2.8
        print 'Nodes.clean', len(self), 'is_reshape_edgelanes', is_reshape_edgelanes

        edges = self.parent.edges
        lanes = self.parent.lanes
        rad_min = self.radius_default.value

        # print '  id(edges.shapes),id(edges.shapes.value)', id(edges.shapes),id(edges.shapes.value)#,edges.shapes.value
        # print '  id(self.coords),id(self.coords.value)', id(self.coords),id(self.coords.value)#,self.coords.value
        # print '  self.coords.value.shape',self.coords.value.shape
        # print '  len(self.coords),self.coords.shape',len(self.coords.value),self.coords.value
        for id_node in self.get_ids():
            ind_node = self.get_inds(id_node)
            # if id_node in TESTNODES:
            #    print 79*'_'
            #    print '  node',id_node
            #    print '   coords',self.coords[id_node]
            #    print '   coords',TESTNODES[0],self.coords[TESTNODES[0]]
            #    print '   coords',TESTNODES[1],self.coords[TESTNODES[1]]
            #    print '   radii',self.radii[id_node]

            # attention, this is s safe method in case
            # that ids_outgoing and ids_incoming are not yet defined
            ids_edge_out = edges.select_ids(edges.ids_fromnode.value == id_node)
            ids_edge_in = edges.select_ids(edges.ids_tonode.value == id_node)

            if (len(ids_edge_out) > 0) | (len(ids_edge_in) > 0):
                # distanza ad altri nodi
                #d = np.sum(np.abs(self.coords[id_node]-self.coords.value),1)
                #d = np.linalg.norm(self.coords[id_node]-self.coords.value,1)
                coords = self.coords[id_node]
                d = get_norm_2d(coords-self.coords.value)
                d[ind_node] = np.inf
                d_min = np.min(d)
                # print '  d_min',d_min

                # estimate circumference of junction and determine node radius
                n_edges = len(ids_edge_in) + len(ids_edge_out)
                width_av = np.mean(np.concatenate((edges.widths[ids_edge_in], edges.widths[ids_edge_out])))

                # here we assume a node with 6 entrance sides and a and 2 average width edges per side
                #circum = 2.0*max(6,n_edges)*width_av
                circum = nodestretchfactor*max(2, n_edges)*width_av

                # print '  n_edges,width_av,radius',n_edges,width_av,max(6,n_edges)*width_av/(2*np.pi)
                radius = min(max(circum/(n_min_nodeedges*np.pi), rad_min), 0.4*d_min)
                self.radii[id_node] = radius

                # if id_node in TESTNODES:
                #    print '  AFTER change radius:'#OK
                #    print '   coords',TESTNODES[0],self.coords[TESTNODES[0]]
                #    print '   coords',TESTNODES[1],self.coords[TESTNODES[1]]

                for id_edge in ids_edge_in:
                    # print '    in edge',id_edge
                    shape = edges.shapes[id_edge]
                    n_shape = len(shape)
                    # edges.shapes[id_edge][::-1]:
                    for i in xrange(n_shape-1, -1, -1):
                        d = get_norm_2d(np.array([shape[i]-coords]))[0]
                        # print '      i,d,r',i , d, radius,d>radius
                        if d > radius:
                            # print '        **',i,d, radius
                            break
                    x, y = shape[i][:2]
                    # print 'shape',shape,
                    #dx,dy = shape[i+1][:2] - shape[i][:2]
                    dx, dy = coords[:2] - shape[i][:2]
                    dn = np.sqrt(dx*dx + dy*dy)
                    x1 = x + (d-radius)*dx/dn
                    y1 = y + (d-radius)*dy/dn

                    if i == n_shape-1:

                        shape[-1][:2] = [x1, y1]
                        edges.shapes[id_edge] = shape

                    else:  # elif i>0:
                        shape[i+1][:2] = [x1, y1]
                        edges.shapes[id_edge] = shape[:i+2]

                    # print '    x,y',x,y
                    # print '    x1,y1',x1,y1
                    # print '  shape[:i+2]',shape[:i+2]
                    # print '  shapes[id_edge]',edges.shapes[id_edge]
                    if is_reshape_edgelanes:
                        lanes.reshape_edgelanes(id_edge)

                for id_edge in ids_edge_out:
                    # print '    out edge',id_edge
                    shape = edges.shapes[id_edge]
                    n_shape = len(shape)
                    # edges.shapes[id_edge][::-1]:
                    for i in xrange(n_shape):
                        d = get_norm_2d(np.array([shape[i]-coords]))[0]
                        # print '      i,d,r',i , d, radius,d>radius
                        if d > radius:
                            # print '        **',i,d, radius
                            break
                    x, y = coords[:2]  # shape[i-1][:2]
                    # print 'shape',shape,
                    #dx,dy = shape[i][:2]- shape[i-1][:2]
                    dx, dy = shape[i][:2] - coords[:2]
                    dn = np.sqrt(dx*dx + dy*dy)
                    x1 = x + (radius)*dx/dn
                    y1 = y + (radius)*dy/dn
                    if i == 0:
                        shape[0][:2] = [x1, y1]
                        edges.shapes[id_edge] = shape

                    elif i < n_shape:

                        shape[i-1][:2] = [x1, y1]
                        edges.shapes[id_edge] = shape[i-1:]
                    # print '    x,y',x,y
                    # print '    x1,y1',x1,y1
                    # print '  shape[:i+2]',shape[:i+2]
                    # print '  shapes[id_edge]',edges.shapes[id_edge]
                    if is_reshape_edgelanes:
                        lanes.reshape_edgelanes(id_edge)

        self.radii.set_modified(True)
        edges.shapes.set_modified(True)


class Network(cm.BaseObjman):
    def __init__(self, parent=None, name='Network', **kwargs):
        print 'Network.__init__', parent, name
        self._init_objman(ident='net', parent=parent, name=name,
                          # xmltag = 'net',# no, done by netconvert
                          version=0.1,
                          **kwargs)
        attrsman = self.set_attrsman(cm.Attrsman(self))
        # print '  Network.parent',self.parent
        self._init_attributes()

    def _init_attributes(self):
        attrsman = self.get_attrsman()
        self.version = attrsman.add(cm.AttrConf('version', '0.25',
                                                groupnames=['aux'],
                                                perm='r',
                                                name='Network version',
                                                info='Sumo network version',
                                                xmltag='version'
                                                ))

        self.modes = attrsman.add(cm.ObjConf(Modes(self)))
        self.modes.clear()
        self.modes.add_default()

        # print 'Network.__init__'
        # print '  MODES.values()',MODES.values()
        # print '  MODES.keys()',MODES.keys()

        # self.modes.print_attrs()
        ##

        ##
        self.nodes = attrsman.add(cm.ObjConf(Nodes(self)))
        self.edges = attrsman.add(cm.ObjConf(Edges(self)))
        self.lanes = attrsman.add(cm.ObjConf(Lanes(self)))

        self.edges.set_nodes(self.nodes)
        self.edges.set_lanes(self.lanes)
        self.nodes.set_edges(self.edges)
        self.roundabouts = attrsman.add(cm.ObjConf(Roundabouts(self, self.edges, self.nodes)))
        self.connections = attrsman.add(cm.ObjConf(Connections(self)))
        self.crossings = attrsman.add(cm.ObjConf(Crossings(self)))

        self.tlss = attrsman.add(cm.ObjConf(TrafficLightSystems(self)))
        self.nodes.set_tlss(self.tlss)

        self.ptstops = attrsman.add(cm.ObjConf(pt.PtStops(self)))

        self._offset = attrsman.add(cm.AttrConf('_offset', np.array([0.0, 0.0], dtype=np.float32),
                                                groupnames=['location', ],
                                                perm='r',
                                                name='Offset',
                                                info='Network offset in WEP coordinates',
                                                xmltag='netOffset',
                                                xmlsep=',',
                                                ))

        self._projparams = attrsman.add(cm.AttrConf('_projparams', "!",
                                                    groupnames=['location', ],
                                                    perm='r',
                                                    name='Projection',
                                                    info='Projection parameters',
                                                    xmltag='projParameter',
                                                    ))

        self._boundaries = attrsman.add(cm.AttrConf('_boundaries', np.array([0.0, 0.0, 0.0, 0.0], dtype=np.float32),
                                                    groupnames=['location', ],
                                                    perm='r',
                                                    name='Boundaries',
                                                    unit='m',
                                                    info='Network boundaries',
                                                    xmltag='convBoundary',
                                                    xmlsep=',',
                                                    ))

        self._boundaries_orig = attrsman.add(cm.AttrConf('_boundaries_orig', np.array([0.0, 0.0, 0.0, 0.0]),
                                                         groupnames=['location', ],
                                                         perm='r',
                                                         name='Orig. boundaries',
                                                         info='Original network boundaries',
                                                         xmltag='origBoundary',
                                                         xmlsep=',',
                                                         ))

    def _init_constants(self):
        pass
        #self._oldoffset = self._offset.copy()
        # print 'net._init_constants',self._offset,self._oldoffset
    # def set_oldoffset(self, offset):
    #    """
    #    Set explicitely an old net offset, if existing.
    #    This allows to update coordinates and shapes outside the network.
    #
    #    """
    #    self._oldoffset = offset

    def set_version(self, version):
        self.version = version

    def get_version(self):
        return self.version

    def is_empty(self):
        return (len(self.nodes) == 0) & (len(self.edges) == 0)

    def set_offset(self, offset):
        # if (offset is not self._offset) :
        #        self._oldoffset = self._offset.copy()
        self._offset = offset

    def get_offset(self):
        return self._offset

    # def is_offset_change(self):
    #    """
    #    Returns true if offset changed be approx 1 mm after last net import
    #    """
    #    return np.sum(abs(self._oldoffset - self._offset))>0.002

    # def get_deltaoffset(self):
    #    return self._offset - self._oldoffset

    # def remove_oldoffset(self):
    #    self._oldoffset = None

    def set_boundaries(self, convBoundary, origBoundary=None):
        """
        Format of Boundary box
         [MinX, MinY ,MaxX, MaxY ]

        """
        self._boundaries = convBoundary
        if origBoundary is None:
            self._boundaries_orig = self._boundaries
        else:
            self._boundaries_orig = origBoundary

    def get_boundaries(self, is_netboundaries=False):
        if is_netboundaries:
            return self._boundaries
        else:
            return self._boundaries, self._boundaries_orig

    def merge_boundaries(self, convBoundary, origBoundary=None):
        """
        Format of Boundary box
         [MinX, MinY ,MaxX, MaxY ]

        """
        # print 'mergeBoundaries'
        self._boundaries = self.get_boundary_union(convBoundary, self._boundaries)
        if origBoundary is None:
            self._boundaries_orig = self._boundaries
        else:
            self._boundaries_orig = self.get_boundary_union(origBoundary, self._boundaries_orig)
        # print '  self._boundaries_orig =',self._boundaries_orig
        # print '  self._boundaries =',self._boundaries

    def get_boundary_union(self, BB1, BB2):
        return [min(BB1[0], BB2[0]), min(BB1[1], BB2[1]), max(BB1[2], BB2[2]), max(BB1[3], BB2[3])]

    def intersects_boundaries(self, BB):
        """
        Tests if the given Bounding Box or line intersects with
        the network boundaries.

        Returns True if it is partially inside, or touching the
        border.
        Format of Boundary box
         [MinX, MinY ,MaxX, MaxY ]
            0     1     2     3

        Returns False otherwise
        """
        # print 'intersects_boundaries'
        # print '  self',self._boundaries
        # print '  BB',BB
        # print ' return',( (self._boundaries[2] >= BB[0]) & (self._boundaries[0] <= BB[2]) &
        #     (self._boundaries[3] >= BB[1]) & (self._boundaries[1] <= BB[3]) )

        return ((self._boundaries[2] >= BB[0]) & (self._boundaries[0] <= BB[2]) &
                (self._boundaries[3] >= BB[1]) & (self._boundaries[1] <= BB[3]))
        # if ( (self._boundaries[2] >= BB[0]) & (self._boundaries[0] <= BB[2]) &
        #     (self._boundaries[3] >= BB[1]) & (self._boundaries[1] <= BB[3]) ):
        #    return True
        # else:
        #    return False

    def get_projparams(self):
        return self._projparams

    def set_projparams(self, projparams="!"):
        # print 'setprojparams',projparams
        self._projparams = projparams

    def get_rootfilename(self):
        if self.parent is not None:  # scenario exists
            return self.parent.get_rootfilename()
        else:
            return self.get_ident()

    def get_rootfilepath(self):
        if self.parent is not None:
            return self.parent.get_rootfilepath()
        else:
            return os.path.join(os.getcwd(), self.get_rootfilename())

    def get_filepath(self):
        """
        Default network filepath.
        """
        return self.get_rootfilepath()+'.net.xml'

    def get_addfilepath(self):
        """
        Default filepath for additional files.
        """
        return self.get_rootfilepath()+'.add.xml'

    def clear_net(self):
        """
        Remove all netelements.
        """
        self.clear()
        self.modes.add_default()
        # do other cleanup jobs

    def call_netedit(self, filepath=None, is_maps=False, is_poly=True):

        #filepath = self.export_netxml(filepath)
        if filepath is None:
            filepath = self.get_filepath()

        # remove old netfile, is exists
        if os.path.isfile(filepath):
            os.remove(filepath)

        filepath_edges, filepath_nodes, filepath_connections, filepath_tlss = self.export_painxml(filepath=filepath)
        if filepath_edges != "":
            # print '  netconvert: success'
            names = os.path.basename(filepath).split('.')
            dirname = os.path.dirname(filepath)
            if len(names) >= 3:
                rootname = '.'.join(names[:-2])
            elif len(names) <= 2:
                rootname = names[0]

            addfilepath = self.export_addxml(is_ptstops=True, is_poly=False)
            if addfilepath is not False:
                option_addfiles_in = '  --sumo-additionals-file '+filepathlist_to_filepathstring(addfilepath)
            else:
                option_addfiles_in = ''

            option_addfiles_out = ' --additionals-output ' + filepathlist_to_filepathstring(self.get_addfilepath())

            configfilepath = self.write_guiconfig(rootname, dirname, is_maps)

            #+ ' --sumo-net-file ' + filepathlist_to_filepathstring(filepath)

            cml = 'netedit --ignore-errors.edge-type'\
                + ' --node-files '+filepathlist_to_filepathstring(filepath_nodes)\
                + ' --edge-files '+filepathlist_to_filepathstring(filepath_edges)\
                + ' --connection-files '+filepathlist_to_filepathstring(filepath_connections)\
                + ' --output-file '+filepathlist_to_filepathstring(filepath)\
                + ' --gui-settings-file ' + filepathlist_to_filepathstring(configfilepath)\
                + option_addfiles_in + option_addfiles_out
            #+ ' --output-prefix '+ filepathlist_to_filepathstring(os.path.join(dirname,rootname))

            if len(self.tlss) > 0:
                cml += ' --tllogic-files '+filepathlist_to_filepathstring(filepath_tlss)

            proc = subprocess.Popen(cml, shell=True)
            print '  run_cml cml=', cml
            # print '  pid = ',proc.pid
            proc.wait()
            if proc.returncode == 0:
                print '  netedit:success'

                return self.import_netxml()
                # return self.import_xml() # use if netedit exports to plain xml files
            else:
                print '  netedit:error'
                return False
        else:
            print '  netconvert:error'
            return False

    def call_sumogui(self, filepath=None, is_maps=True, is_poly=True):

        if filepath is None:
            filepath = self.get_filepath()
            dirname = os.path.dirname(filepath)
        names = os.path.basename(filepath).split('.')
        dirname = os.path.dirname(filepath)
        if len(names) >= 3:
            rootname = '.'.join(names[:-2])
        elif len(names) <= 2:
            rootname = names[0]

        configfilepath = self.write_guiconfig(rootname, dirname, is_maps)

        #addfilepath = self.export_addxml(is_ptstops = True, is_poly = True)

        stopfilepath = self.ptstops.export_sumoxml()
        if is_poly:
            polyfilepath = self.parent.landuse.export_polyxml()
        else:
            polyfilepath = None

        # print '  is_polystopfilepath,polyfilepath',is_poly,stopfilepath,polyfilepath
        addfilepathlist = []
        if (stopfilepath is not None):
            addfilepathlist.append(stopfilepath)

        if (polyfilepath is not None):
            addfilepathlist.append(polyfilepath)

        if len(addfilepathlist) > 0:
            option_addfiles = '  --additional-files '+filepathlist_to_filepathstring(addfilepathlist)
        else:
            option_addfiles = ''

        # if addfilepath is not False:
        #    option_addfiles = '  --additional-files '+filepathlist_to_filepathstring(addfilepath)
        # else:
        #    option_addfiles = ''

        cml = 'sumo-gui '\
            + ' --net-file '+filepathlist_to_filepathstring(filepath)\
            + ' --gui-settings-file '+filepathlist_to_filepathstring(configfilepath)\
            + option_addfiles

        proc = subprocess.Popen(cml, shell=True)
        print '  run_cml cml=', cml
        print '  pid = ', proc.pid
        proc.wait()
        return proc.returncode

    def write_guiconfig(self, rootname=None, dirname=None, is_maps=False):

        # check if there are maps
        maps = None
        if is_maps:
            if self.parent is not None:
                maps = self.parent.landuse.maps

        # write netedit configfile
        templatedirpath = os.path.dirname(os.path.abspath(__file__))
        fd_template = open(os.path.join(templatedirpath, 'netedit_config.xml'), 'r')

        if (rootname is not None) & (dirname is not None):
            configfilepath = os.path.join(dirname, rootname+'.netedit.xml')
        else:
            configfilepath = self.get_rootfilepath()+'.netedit.xml'

        print 'write_guiconfig', configfilepath, is_maps & (maps is not None), maps
        fd_config = open(configfilepath, 'w')
        for line in fd_template.readlines():
            if line.count('<decals>') == 1:
                fd_config.write(line)
                if is_maps & (maps is not None):
                    maps.write_decals(fd_config, indent=12)
            else:
                fd_config.write(line)

        fd_template.close()
        fd_config.close()

        return configfilepath

    def import_netxml(self, filepath=None, rootname=None, is_clean_nodes=False, is_remove_xmlfiles=False):
        print 'import_netxml', filepath

        if rootname is None:
            rootname = self.get_rootfilename()

        if filepath is None:
            filepath = self.get_filepath()

        dirname = os.path.dirname(filepath)

        if os.path.isfile(filepath):
            # print '  modes.names',self.modes.names
            cml = 'netconvert'\
                + ' --sumo-net-file '+filepathlist_to_filepathstring(filepath)\
                + ' --plain-output-prefix '+filepathlist_to_filepathstring(os.path.join(dirname, rootname))
            proc = subprocess.Popen(cml, shell=True)
            print '  run_cml cml=', cml
            print '  pid = ', proc.pid
            proc.wait()
            if not proc.returncode:
                print '  modes.names', self.modes.names
                return self.import_xml(rootname, dirname)
            else:
                return False
        else:
            return False

    def export_addxml(self, filepath=None,
                      is_ptstops=True, is_poly=False,
                      encoding='UTF-8'):
        """
        Export additional file
        """
        if filepath is None:
            filepath = self.get_addfilepath()

        try:
            fd = open(filepath, 'w')

        except:
            print 'WARNING in write_obj_to_xml: could not open', filepath
            return False

        #xmltag, xmltag_item, attrname_id = self.xmltag
        fd.write('<?xml version="1.0" encoding="%s"?>\n' % encoding)
        indent = 0
        #fd.write(xm.begin('routes xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="https://sumo.sf.net/xsd/routes_file.xsd"',indent))

        fd.write(xm.begin('additional', indent))

        if is_ptstops:
            if len(self.ptstops) > 0:
                self.ptstops.write_xml(fd, indent=indent+2)

        if is_poly:
            if self.parent is not None:
                facilities = self.parent.landuse.facilities
                if len(facilities) > 0:
                    fd.write(xm.start('location', indent+2))
                    # print '  groups:',self.parent.net.get_attrsman().get_groups()
                    for attrconfig in self.get_attrsman().get_group('location'):
                            # print '    locationconfig',attrconfig.attrname
                        attrconfig.write_xml(fd)
                    fd.write(xm.stopit())

                    facilities.write_xml(fd, indent=indent+2, is_print_begin_end=False)

        fd.write(xm.end('additional', indent))
        fd.close()
        return filepath

    def export_painxml(self, filepath=None, is_export_tlss=True):
        # now create rootfilepath in order to export first
        # the various xml file , then call netconvert

        if filepath is None:
            filepath = self.get_filepath()

        names = os.path.basename(filepath).split('.')
        dirname = os.path.dirname(filepath)
        if len(names) >= 3:
            rootname = '.'.join(names[:-2])
        elif len(names) <= 2:
            rootname = names[0]

        filepath_edges = os.path.join(dirname, rootname+'.edg.xml')
        filepath_nodes = os.path.join(dirname, rootname+'.nod.xml')
        filepath_connections = os.path.join(dirname, rootname+'.con.xml')
        filepath_tlss = os.path.join(dirname, rootname+'.tll.xml')

        self.edges.export_sumoxml(filepath_edges)
        self.nodes.export_sumoxml(filepath_nodes)
        self.connections.export_sumoxml(filepath_connections)

        if (len(self.tlss) > 0) & is_export_tlss:
            self.tlss.export_sumoxml(filepath_tlss)

        return filepath_edges, filepath_nodes, filepath_connections, filepath_tlss

    def export_netxml(self, filepath=None, is_export_tlss=True, is_netconvert=True):

        # now create rootfilepath in order to export first
        # the various xml file , then call netconvert
        if filepath is None:
            filepath = self.get_filepath()

        print 'Net.export_netxml', filepath
        filepath_edges, filepath_nodes, filepath_connections, filepath_tlss = self.export_painxml(
            filepath=filepath, is_export_tlss=is_export_tlss)

        #cml = 'netconvert --verbose --ignore-errors.edge-type'
        cml = 'netconvert --ignore-errors.edge-type'\
            + ' --node-files '+filepathlist_to_filepathstring(filepath_nodes)\
            + ' --edge-files '+filepathlist_to_filepathstring(filepath_edges)\
            + ' --connection-files '+filepathlist_to_filepathstring(filepath_connections)\
            + ' --output-file '+filepathlist_to_filepathstring(filepath)

        if (len(self.tlss) > 0) & (is_export_tlss):
            cml += ' --tllogic-files '+filepathlist_to_filepathstring(filepath_tlss)

        if is_netconvert:
            proc = subprocess.Popen(cml, shell=True)
            print 'run_cml cml=', cml
            print '  pid = ', proc.pid
            proc.wait()
            if proc.returncode == 0:
                print '  success'
                return filepath
            else:
                print '  success'
                return ''
        else:
            return ''

    def import_xml(self, rootname=None, dirname=None, is_clean_nodes=False, is_remove_xmlfiles=False):

        if not self.is_empty():
            oldoffset = self.get_offset()
        else:
            oldoffset = None
        print 'Network.import_xml oldoffset', oldoffset
        # remove current network
        # print '  remove current network'
        self.clear_net()
        # reload default SUMO MODES (maybe should not be here)
        # self.modes.add_rows(ids =  MODES.values(), names = MODES.keys())

        if rootname is None:
            rootname = self.get_rootfilename()

        if dirname is None:
            dirname = os.path.dirname(self.get_rootfilepath())

        # print 'import_xml',dirname,rootname
        nodefilepath = os.path.join(dirname, rootname+'.nod.xml')
        edgefilepath = os.path.join(dirname, rootname+'.edg.xml')
        confilepath = os.path.join(dirname, rootname+'.con.xml')
        tlsfilepath = os.path.join(dirname, rootname+'.tll.xml')

        if os.path.isfile(edgefilepath) & os.path.isfile(nodefilepath) & os.path.isfile(confilepath):
            nodereader = self.import_sumonodes(nodefilepath, is_remove_xmlfiles)
            edgereader = self.import_sumoedges(edgefilepath, is_remove_xmlfiles)
            if is_clean_nodes:
                # make edges and lanes end at the node boundaries
                # also recalculate lane shapes from edge shapes...if lane shapes are missing
                #self.lanes.reshape() #
                self.nodes.clean(is_reshape_edgelanes=True)
            else:
                # just recalculate lane shapes from edge shapes...if lane shapes are missing
                self.lanes.reshape()
            #    #pass

            self.import_sumoconnections(confilepath, is_remove_xmlfiles)

            if os.path.isfile(tlsfilepath):
                self.import_sumotls(tlsfilepath, is_remove_xmlfiles)

            # print '  check additionals',self.ptstops.get_stopfilepath(),os.path.isfile(self.ptstops.get_stopfilepath())
            if os.path.isfile(self.ptstops.get_stopfilepath()):
                self.ptstops.import_sumostops(is_remove_xmlfiles=is_remove_xmlfiles)

            # this fixes some references to edges and tls
            nodereader.write_to_net_post()

            if oldoffset is not None:
                # check if offset changed
                # if self.is_offset_change():
                deltaoffset = self.get_offset()-oldoffset
                # print '  check update_netoffset',deltaoffset,oldoffset,self.get_offset(),np.sum(abs(deltaoffset))>0.002
                if np.sum(abs(deltaoffset)) > 0.002:
                    # communicate to scenario
                    if self.parent is not None:
                        self.parent.update_netoffset(deltaoffset)

            # clean up ...should be done in each importer??
            # if is_remove_xmlfiles:
            #    os.remove(nodefilepath)
            #    os.remove(edgefilepath)
            #    os.remove(confilepath)

            #    if os.path.isfile(tlsfilepath):
            #        os.remove(tlsfilepath)
            return True
        else:
            self.get_logger().w('import_sumonodes: files not found', key='message')
            return False

    def import_sumonodes(self, filename, is_remove_xmlfiles=False, logger=None, **others):
        print 'import_sumonodes', filename
        # print '  parent',self.parent
        self.get_logger().w('import_sumonodes', key='message')

        # timeit
        exectime_start = time.clock()

        counter = SumoNodeCounter()

        #reader = SumoEdgeReader(self, **others)
        # try:

        parse(filename, counter)
        # print '  after: n_edge', counter.n_edge
        fastreader = SumoNodeReader(self, counter)
        parse(filename, fastreader)

        fastreader.write_to_net()

        # timeit
        print '  exec time=', time.clock() - exectime_start
        return fastreader

    def import_sumoedges(self, filename, is_remove_xmlfiles=False, logger=None, **others):
        print 'import_sumoedges', filename
        logger = self.get_logger()
        logger.w('import_sumoedges', key='message')
        # timeit
        exectime_start = time.clock()

        counter = SumoEdgeCounter()

        #reader = SumoEdgeReader(self, **others)
        # try:

        parse(filename, counter)
        # print '  after: n_edge', counter.n_edge
        fastreader = SumoEdgeReader(self, counter)
        parse(filename, fastreader)

        fastreader.write_to_net()
        self.edges.update()

        if is_remove_xmlfiles:
            os.remove(filename)
        # timeit
        print '  exec time=', time.clock() - exectime_start

        # except KeyError:
        #    print >> sys.stderr, "Please mind that the network format has changed in 0.16.0, you may need to update your network!"
        #    raise
        return fastreader

    def import_sumoconnections(self, filename, is_remove_xmlfiles=False, logger=None, **others):
        print 'import_sumoedges', filename
        logger = self.get_logger()
        logger.w('import_sumoconnections', key='message')

        # timeit
        exectime_start = time.clock()

        counter = SumoConnectionCounter()

        parse(filename, counter)
        fastreader = SumoConnectionReader(self, counter)
        parse(filename, fastreader)

        fastreader.write_to_net()

        # timeit
        exectime_end = time.clock()
        print '  exec time=', exectime_end - exectime_start
        return fastreader

    def import_sumotls(self, filename, is_remove_xmlfiles=False, logger=None, **others):
        """
        Import traffic ligh signals from tll.xml file
        as part of a complete import net process.
        """
        print 'import_sumotls', filename

        if logger is None:
            logger = self.get_logger()
        logger.w('import_sumotls', key='message')

        # timeit
        exectime_start = time.clock()

        reader = SumoTllReader(self)
        parse(filename, reader)

        # timeit
        exectime_end = time.clock()
        print '  exec time=', exectime_end - exectime_start
        return reader

    def import_sumotls_to_net(self, filename, is_remove_xmlfiles=False, logger=None, **others):
        """
        Import traffic ligh signals from tll.xml file into an existing network.
        """
        print 'import_sumotls_to_net', filename

        # associate nodes with sumo tls ID
        #map_id_node_to_id_tlss_sumo = {}

        ids_node = self.nodes.select_ids(self.nodes.ids_tls.get_value() > -1)
        ids_tlss_sumo = self.tlss.ids_sumo[self.nodes.ids_tls[ids_node]].copy()
        # for id_node, id_tls_sumo in zip(ids_node, self.tlss.ids_sumo[self.nodes.ids_tls[ids_node]]):
        #    map_id_node_to_id_tlss_sumo[id_node] = id_tls_sumo

        # clear all TLSs
        self.tlss.clear()

        if logger is None:
            logger = self.get_logger()
        logger.w('import_sumotls_to_net', key='message')

        # timeit
        exectime_start = time.clock()

        reader = SumoTllReader(self)
        parse(filename, reader)

        # timeit
        exectime_end = time.clock()
        print '  exec time=', exectime_end - exectime_start

        # reestablish TLS IDs of nodes

        # for id_node, id_tls_sumo in
        # zip(ids_node, self.nodes.ids_tls[ids_node]):
        #    map_id_node_to_id_tlss_sumo[id_node] = id_tls_sumo
        self.nodes.ids_tls[ids_node] = self.tlss.ids_sumo.get_ids_from_indices(ids_tlss_sumo)

    def get_id_mode(self, modename):
        return self.modes.get_id_mode(modename)

    def add_node(self, **kwargs):
        return self.nodes.make(**kwargs)

    def add_nodes(self,  **kwargs):
        # print 'add_nodes'
        return self.nodes.multimake(**kwargs)

    def clean_nodes(self, **kwargs):
        self.nodes.clean(**kwargs)
        self.edges.update()

    def add_edge(self,  **kwargs):
        # print 'add_edge'
        return self.edges.make(**kwargs)

    def add_edges(self,  **kwargs):
        # print 'add_edges'
        return self.edges.multimake(**kwargs)

    def add_roundabout(self, **kwargs):
        return self.roundabouts.make(**kwargs)

    def add_roundabouts(self, **kwargs):
        return self.roundabouts.multimake(**kwargs)

    def add_lane(self, **kwargs):
        # print 'add_lane\n',
        # for key, value in kwargs.iteritems():
        #    print '  ',key,type(value),value
        return self.lanes.make(**kwargs)

    def add_lanes(self, **kwargs):
        # print 'add_lanes\n',
        # for key, value in kwargs.iteritems():
        #    print '  ',key,type(value),value
        return self.lanes.multimake(**kwargs)

    def complete_connections(self):
        """
        Extend connections also to pedestrian edges.
        """
        edges = self.edges
        nodes = self.nodes
        for id_node in self.nodes.get_ids():
            ids_outgoing = nodes.ids_outgoing[id_node]
            ids_incoming = nodes.ids_incoming[id_node]
            if ids_outgoing is None:
                ids_outgoing = set()
            else:
                ids_outgoing = set(ids_outgoing)

            if ids_incoming is None:
                ids_incoming = set()
            else:
                ids_incoming = set(ids_incoming)

            ids_outgoing_all = set(edges.select_ids(edges.ids_fromnode.get_value() == id_node))
            ids_incoming_all = set(edges.select_ids(edges.ids_tonode.get_value() == id_node))

            for id_edge in ids_outgoing_all.difference(ids_outgoing):
                self.connect_edge_incoming(id_edge, ids_incoming_all)

            for id_edge in ids_incoming_all.difference(ids_incoming):
                self.connect_edge_outgoing(id_edge, ids_outgoing_all)

    def connect_edge_incoming(self, id_edge_to, ids_edge_from):
        """
        Connect id_edge_to with edges in list ids_edge_from.
        Attention, only lane index 0 of edge id_edge_to is connected.
        """
        # print 'connect_edge', ids_edge_from, id_edge_to
        for id_edge_from in ids_edge_from:
            ids_lane_from = self.edges.ids_lanes[id_edge_from]
            ids_lane_to = self.edges.ids_lanes[id_edge_to]
            if len(ids_lane_from) == 1:
                self.add_connection(id_fromlane=ids_lane_from[0], id_tolane=ids_lane_to[0])

            # TODO: make connections also to multi lane edge
            else:
                for id_lane_from in ids_lane_from:
                    if not MODES["pedestrian"] in self.lanes.ids_modes_allow[id_lane_from]:
                        self.add_connection(id_fromlane=id_lane_from, id_tolane=ids_lane_to[0])
                        break

    def connect_edge_outgoing(self, id_edge, ids_edge_to):
        """
        Connect id_edge with edges in list ids_edge_to.
        Attention, only lane index 0 of edge id_edge_from is connected.
        """
        # print 'connect_edge', ids_edge_from, id_edge_to
        for id_edge_to in ids_edge_to:
            ids_lane_to = self.edges.ids_lanes[id_edge_to]
            ids_lane_from = self.edges.ids_lanes[id_edge]
            if len(ids_lane_to) == 1:
                self.add_connection(id_fromlane=ids_lane_from[0], id_tolane=ids_lane_to[0])

            # TODO: make connections also to multi lane edge
            else:
                for id_lane_to in ids_lane_to:
                    if not MODES["pedestrian"] in self.lanes.ids_modes_allow[id_lane_to]:
                        self.add_connection(id_fromlane=ids_lane_from[0], id_tolane=id_lane_to)
                        break

    def add_connection(self, id_fromlane=-1, id_tolane=-1, **kwargs):
        # print 'add_connections id_fromlane , id_tolane ',id_fromlane , id_tolane

        # for key, value in kwargs.iteritems():
        #    print '  ',key,type(value),value

        id_fromedge = self.lanes.ids_edge[id_fromlane]
        id_toedge = self.lanes.ids_edge[id_tolane]
        id_node = self.edges.ids_tonode[id_fromedge]
        self.nodes.add_incoming(id_node, id_fromedge)
        self.nodes.add_outgoing(id_node, id_toedge)

        return self.connections.make(id_fromlane=id_fromlane, id_tolane=id_tolane, **kwargs)

    def add_connections(self, ids_fromlane=[], ids_tolane=[], **kwargs):

        # for key, value in kwargs.iteritems():
        #    print '  ',key,type(value),value

        ids_fromedge = self.lanes.ids_edge[ids_fromlane]
        ids_toedge = self.lanes.ids_edge[ids_tolane]
        ids_node = self.edges.ids_tonode[ids_fromedge]
        add_incoming = self.nodes.add_incoming
        add_outgoing = self.nodes.add_outgoing
        for id_node, id_fromedge, id_toedge in zip(ids_node, ids_fromedge, ids_toedge):
            add_incoming(id_node, id_fromedge)
            add_outgoing(id_node, id_toedge)

        return self.connections.multimake(ids_fromlane=ids_fromlane, ids_tolane=ids_tolane, **kwargs)

    def add_crossing(self, **kwargs):
        # print 'add_crossing\n',
        return self.crossings.make(**kwargs)

    def add_crossings(self, **kwargs):
        # print 'add_crossings\n',
        return self.crossings.multimake(**kwargs)


class SumoConnectionCounter(handler.ContentHandler):
    """Parses a SUMO edge XML file and counts edges and lanes."""

    def __init__(self):
        self.n_con = 0
        self.n_cross = 0

    def startElement(self, name, attrs):
        if name == 'connection':
            if attrs.has_key('to'):
                self.n_con += 1

        if name == 'crossing':
            self.n_cross += 1


class SumoConnectionReader(handler.ContentHandler):
    """Parses a SUMO connection XML file"""

    def __init__(self, net, counter):
        self._net = net

        # print 'SumoConnectionReader:n_con,n_cross',counter.n_con,counter.n_cross

        # connections
        self._ind_con = -1
        self.ids_fromlane = np.zeros(counter.n_con, np.int32)
        self.ids_tolane = np.zeros(counter.n_con, np.int32)
        self.are_passes = np.zeros(counter.n_con, np.bool)
        self.are_keep_clear = np.zeros(counter.n_con, np.bool)
        self.positions_cont = np.zeros(counter.n_con, np.float32)
        self.are_uncontrolled = np.zeros(counter.n_con, np.bool)

        # crossings
        self._ind_cross = -1
        self.ids_node = np.zeros(counter.n_cross, np.int32)
        self.ids_edges = np.zeros(counter.n_cross, np.object)
        self.widths = np.zeros(counter.n_cross, np.float32)
        self.are_priority = np.zeros(counter.n_cross, np.bool)
        self.are_discard = np.zeros(counter.n_cross, np.bool)

        self._ids_node_sumo = self._net.nodes.ids_sumo
        self._ids_edge_sumo = self._net.edges.ids_sumo
        self._ids_edgelanes = self._net.edges.ids_lanes

    def startElement(self, name, attrs):
        # print 'startElement',name

        if name == 'connection':
            # <connection from="153009994" to="153009966#1" fromLane="0" toLane="0" pass="1"/>

            if attrs.has_key('to'):
                self._ind_con += 1
                i = self._ind_con
                # print 'startElement',name,i
                id_fromedge = self._ids_edge_sumo.get_id_from_index(attrs['from'])
                id_toedge = self._ids_edge_sumo.get_id_from_index(attrs['to'])

                #id_fromlane = self._ids_edgelanes[id_fromedge][int(attrs.get('fromLane',0))]
                #id_tolane = self._ids_edgelanes[id_toedge][int(attrs.get('toLane',0))]

                # print '  id_sumo fromedge', attrs['from'],len(self._ids_edgelanes[id_fromedge]) ,  int(attrs['fromLane'])
                self.ids_fromlane[i] = self._ids_edgelanes[id_fromedge][int(attrs['fromLane'])]
                self.ids_tolane[i] = self._ids_edgelanes[id_toedge][int(attrs['toLane'])]
                self.are_passes[i] = int(attrs.get('pass', 0))
                self.are_keep_clear[i] = int(attrs.get('keepClear ', 1))
                self.positions_cont[i] = float(attrs.get('contPos ', 0.0))
                self.are_uncontrolled[i] = int(attrs.get('uncontrolled', 0))
            else:
                id_fromedge = self._ids_edge_sumo.get_id_from_index(attrs['from'])

        if name == 'crossing':
            self._ind_cross += 1
            i = self._ind_cross
            # print 'startElement',name

            self.ids_node[i] = self._ids_node_sumo.get_id_from_index(attrs['node'])
            self.ids_edges[i] = self._ids_edge_sumo.get_ids_from_indices(attrs['edges'].split(' '))
            self.widths[i] = float(attrs.get('width ', 4.0))
            self.are_priority[i] = int(attrs.get('priority ', 0))
            self.are_discard[i] = int(attrs.get('discard', 0))

    def write_to_net(self):

        # print 'write_to_net'
        ids_con = self._net.add_connections(
            ids_fromlane=self.ids_fromlane,
            ids_tolane=self.ids_tolane,
            sare_passes=self.are_passes,
            are_keep_clear=self.are_keep_clear,
            positions_cont=self.positions_cont,
            are_uncontrolled=self.are_uncontrolled,
        )

        ids_cross = self._net.add_crossings(
            ids_node=self.ids_node,
            ids_edges=self.ids_edges,
            widths=self.widths,
            are_priority=self.are_priority,
            are_discard=self.are_discard,
        )


class SumoNodeCounter(handler.ContentHandler):
    """Parses a SUMO edge XML file and counts edges and lanes."""

    def __init__(self):
        self.n_node = 0

    def startElement(self, name, attrs):
        # print 'startElement',name,self.n_edge,self.n_lane,self.n_roundabout
        if name == 'node':
            self.n_node += 1


class SumoNodeReader(handler.ContentHandler):
    """Parses a SUMO node XML file"""

    def __init__(self, net, counter):
        self._net = net

        # print 'SumoEdgeFastreader'

        #self._ids_node_sumo = net.nodes.ids_sumo
        #self._nodecoords = net.nodes.coords

        self._nodetypemap = self._net.nodes.types.choices
        self._tltypemap = self._net.nodes.types_tl.choices
        self.radius_default = self._net.nodes.radius_default.get_value()
        # node attrs
        self.ids_sumo = np.zeros(counter.n_node, np.object)
        self.types = np.zeros(counter.n_node, np.int32)
        self.coords = np.zeros((counter.n_node, 3), np.float32)
        self.types_tl = np.zeros(counter.n_node, np.int32)
        self.ids_sumo_tls = np.zeros(counter.n_node, np.object)
        self.turnradii = np.zeros(counter.n_node, np.float32)
        self.are_costum_shape = np.zeros(counter.n_node, np.bool)
        self.shapes = np.zeros(counter.n_node, np.object)
        self.are_keep_clear = np.zeros(counter.n_node, np.bool)
        self._ind_node = -1
        self.ids_sumo_controlled = np.zeros(counter.n_node, np.object)
        self.ids_sumo_controlled[:] = None
        self._offset_delta = np.array([0.0, 0.0])
        self._isNew = len(self._net.nodes) == 0

    def write_to_net(self):

        # print 'write_to_net'
        self.ids_node = self._net.add_nodes(
            ids_sumo=self.ids_sumo,
            types=self.types,
            coords=self.coords,
            shapes=self.shapes,
            are_costum_shape=self.are_costum_shape,
            types_tl=self.types_tl,
            turnradii=self.turnradii,
            are_keep_clear=self.are_keep_clear,
        )
        # attention:
        # attributes ids_sumo_tls and ids_sumo_controlled will be added later
        # when tls and edges are read
        # see write_to_net_post

    def write_to_net_post(self):
        """
        To be called after edges and tls are read.
        """
        # print 'write_to_net_post'
        get_ids_edge = self._net.edges.ids_sumo.get_ids_from_indices
        ids_controlled = self._net.nodes.ids_controlled
        for id_node, ids_sumo_edge in zip(self.ids_node, self.ids_sumo_controlled):
            if ids_sumo_edge is not None:
                if len(ids_sumo_edge) == 0:
                    ids_controlled[id_node] = []
                else:
                    ids_controlled[id_node] = get_ids_edge(ids_sumo_edge)

        # convert sumo ids into internal ids and set to nodes
        # print '  self.ids_sumo_tls',self.ids_sumo_tls
        # print '  self._net.tlss.ids_sumo',self._net.tlss.ids_sumo.value
        self._net.nodes.ids_tls[self.ids_node] = self._net.tlss.ids_sumo.get_ids_from_indices_save(self.ids_sumo_tls)

    def startElement(self, name, attrs):
        # print 'startElement',name
        # if attrs.has_key('id'): print attrs['id']
        # elif (attrs.has_key('from')&attrs.has_key('to')): print 'from',attrs['from'],'to',attrs['to']
        # elif (attrs.has_key('from')&attrs.has_key('to')): print 'from',attrs['from'],'to',attrs['to']
        # else: print '.'

        if name == 'nodes':
            version = self._net.get_version()
            if self._isNew | (version == attrs['version']):
                self._net.set_version(attrs['version'])
            else:
                print 'WARNING: merge with incompatible net versions %s versus %s.' % (version, attrs['version'])

        elif name == 'location':  # j.s
            # print 'startElement',name,self._isNew
            netOffsetStrings = attrs['netOffset'].strip().split(",")
            offset = np.array([float(netOffsetStrings[0]), float(netOffsetStrings[1])])
            offset_prev = self._net.get_offset()
            if self._isNew:
                self._net.set_offset(offset)
                # print '  offset_prev,offset',offset_prev,offset,type(offset)
            else:

                self._offset_delta = offset-offset_prev
                self._net.set_offset(offset)
                # print '  offset_prev,offset,self._offset_delta',offset_prev,offset,type(offset),self._offset_delta

            convBoundaryStr = attrs['convBoundary'].strip().split(",")
            origBoundaryStr = attrs['origBoundary'].strip().split(",")
            # print '  convBoundaryStr',convBoundaryStr
            # print '  origBoundary',origBoundaryStr

            if self._isNew:
                self._net.set_boundaries([float(convBoundaryStr[0]),
                                          float(convBoundaryStr[1]),
                                          float(convBoundaryStr[2]),
                                          float(convBoundaryStr[3])],
                                         [float(origBoundaryStr[0]),
                                          float(origBoundaryStr[1]),
                                          float(origBoundaryStr[2]),
                                          float(origBoundaryStr[3])]
                                         )
            else:
                self._net.merge_boundaries([float(convBoundaryStr[0]),
                                            float(convBoundaryStr[1]),
                                            float(convBoundaryStr[2]),
                                            float(convBoundaryStr[3])],
                                           [float(origBoundaryStr[0]),
                                            float(origBoundaryStr[1]),
                                            float(origBoundaryStr[2]),
                                            float(origBoundaryStr[3])]
                                           )
            if self._isNew:
                if attrs.has_key('projParameter'):
                    self._net.set_projparams(attrs['projParameter'])
            else:
                if attrs.has_key('projParameter'):
                    if self._net.get_projparams() != attrs['projParameter']:
                        print 'WARNING: merge with incompatible projections %s versus %s.' % (
                            self._net.getprojparams(), attrs['projparams'])

        elif name == 'node':
            if attrs['id'][0] != ':':  # no internal node
                self._ind_node += 1
                i = self._ind_node
                x0, y0 = self._offset_delta

                self.ids_sumo[i] = attrs['id']
                sumotypes_node = str(attrs.get('type', 'priority'))

                self.types[i] = self._nodetypemap[sumotypes_node]
                x, y, z = float(attrs['x'])-x0, float(attrs['y'])-y0, float(attrs.get('z', 0.0))
                self.coords[i] = [x, y, z]

                sumotype_tl = attrs.get('tlType', 'none')
                if sumotypes_node == 'traffic_light':
                    if sumotype_tl == 'none':
                        sumotype_tl = 'static'

                self.types_tl[i] = self._tltypemap[sumotype_tl]
                self.ids_sumo_tls[i] = attrs.get('tl', None)
                self.turnradii[i] = attrs.get('radius', 1.5)

                # not in nodes

                shape = np.array(xm.process_shape(attrs.get('shape', ''), offset=self._offset_delta))

                if len(shape) < 3:  # no or insufficient shape info
                    # shape should be a list of np array coords

                    # generate some default shape
                    r0 = self.radius_default
                    shape = [np.array([x-r0, y-r0, z], dtype=np.float32),
                             np.array([x+r0, y-r0, z], dtype=np.float32),
                             np.array([x+r0, y+r0, z], dtype=np.float32),
                             np.array([x-r0, y+r0, z], dtype=np.float32),
                             ]
                    self.are_costum_shape[i] = False

                else:
                    self.are_costum_shape[i] = True

                self.shapes[i] = list(shape)

                self.are_keep_clear[i] = bool(attrs.get('keepClear', True))

                # 'controlledInner'
                # Edges which shall be controlled by a joined TLS
                # despite being incoming as well as outgoing to
                # the jointly controlled nodes
                # problem: we do not know yet the edge IDs
                #
                if attrs.has_key('controlledInner'):
                    self.ids_sumo_controlled[i] = attrs['controlledInner'].strip().split(' ')
                else:
                    self.ids_sumo_controlled[i] = []


# class SumoTllCounter(handler.ContentHandler):
#    """Parses a SUMO tll XML file and counts edges and lanes."""
#
#    def __init__(self):
#        self.n_tls = 0
#
#
#    def startElement(self, name, attrs):
#        #print 'startElement',name,self.n_tls
#        if name == 'tlLogic':
#            self.n_tls += 1

class SumoTllReader(handler.ContentHandler):
    """Parses a SUMO tll XML file and reads it into net."""

    def __init__(self, net):
        self.net = net
        self.connections = net.connections
        self.tlss = net.tlss
        # print 'SumoEdgeFastreader'

        self.get_id_tls = net.nodes.ids_sumo.get_id_from_index

        #n_tls = counter.n_tls
        self.ptypes_choices = self.tlss.tlls.value.ptypes.choices
        self.ids_sumo_tls = self.tlss.ids_sumo

        self.reset_prog()

        self.tlsconnections = {}

    def reset_prog(self):
        self.id_sumo_tls = None
        self.durations = []
        self.durations_min = []
        self.durations_max = []
        self.states = []

    def startElement(self, name, attrs):

        if name == 'tlLogic':
            # print '\n startElement',name,attrs['id']
            self.id_sumo_tls = attrs['id']
            self.ptype = self.ptypes_choices.get(attrs.get('type', None), 1)
            self.id_prog = attrs.get('programID', None)
            self.offset = attrs.get('offset', None)

        elif name == 'phase':
            # print 'startElement',name,self.id_sumo_tls
            if self.id_sumo_tls is not None:
                # print '  append',attrs.get('duration',None),attrs.get('state',None),len(attrs.get('state',''))
                duration = int(attrs.get('duration', 0))
                self.durations.append(duration)
                self.durations_min.append(int(attrs.get('minDur', duration)))
                self.durations_max.append(int(attrs.get('maxDur', duration)))
                self.states.append(attrs.get('state', None))

        # elif name == 'tlLogics':
        #    pass

        elif name == 'connection':
            id_sumo_tls = attrs['tl']
            # print 'startElement',name,id_sumo_tls,int(attrs['linkIndex'])
            # print '  self.tlsconnections',self.tlsconnections

            if not self.tlsconnections.has_key(id_sumo_tls):
                self.tlsconnections[id_sumo_tls] = {}

            id_con = self.connections.get_id_from_sumoinfo(attrs['from'],
                                                           attrs['to'], int(attrs['fromLane']), int(attrs['toLane']))
            if id_con >= 0:
                self.tlsconnections[id_sumo_tls][int(attrs['linkIndex'])] = id_con

    def endElement(self, name):
        #edges = self._net.edges
        #lanes = self._net.lanes
        if name == 'tlLogic':
            # print 'endElement',name,self.id_sumo_tls
            # print '  ptype',self.ptype
            # print '  durations',self.durations
            # print '  durations_min',self.durations_min
            # print '  durations_max',self.durations_max
            # print '  states',self.states
            # print '  self.id_prog='+self.id_prog+'='
            self.tlss.make(self.id_sumo_tls,
                           id_prog=self.id_prog,
                           ptype=self.ptype,
                           offset=self.offset,
                           durations=self.durations,
                           durations_min=self.durations_min,
                           durations_max=self.durations_max,
                           states=self.states,
                           )

            self.reset_prog()

        elif name == 'tlLogics':
            # print 'endElement',name,len(self.tlss)
            # end of scanning. Write controlled connections to tlss
            # print '  tlsconnections',self.tlsconnections

            for id_sumo_tls, conmap in self.tlsconnections.iteritems():

                inds_con = np.array(conmap.keys(), dtype=np.int32)

                ids_con = np.zeros(np.max(inds_con)+1, np.int32)
                # print '  cons for',id_sumo_tls,conmap
                # print '  inds',inds_con,len(ids_con)
                # print '  values',conmap.values(),len(ids_con)
                ids_con[inds_con] = conmap.values()  # <<<<<<<<<<<

                id_tls = self.tlss.ids_sumo.get_id_from_index(id_sumo_tls)
                self.tlss.set_connections(id_tls, ids_con)
                #self.tlss.set_connections(self.get_id_tls(id_sumo_tls), ids_con)


class SumoEdgeCounter(handler.ContentHandler):
    """Parses a SUMO edge XML file and counts edges and lanes."""

    def __init__(self):
        self.n_edge = 0
        self.n_lane = 0
        self.n_roundabout = 0
        self._n_edgelane = 0
        #self._net = net
        #self._ids_edge_sumo = net.edges.ids_sumo
        #self._ids_node_sumo = net.nodes.ids_sumo

    def startElement(self, name, attrs):
        # print 'startElement',name,self.n_edge,self.n_lane,self.n_roundabout
        if name == 'edge':
            self.n_edge += 1
            self.n_lane += int(attrs['numLanes'])

        elif name == 'roundabout':
            self.n_roundabout += 1


class SumoEdgeReader(handler.ContentHandler):
    """Parses a SUMO edge XML file and reads it into net."""

    def __init__(self, net, counter, offset_delta=np.array([0.0, 0.0])):
        self._net = net

        # print 'SumoEdgeFastreader'

        self._ids_node_sumo = net.nodes.ids_sumo
        self._nodecoords = net.nodes.coords
        self._modenames = net.modes.names
        self._offset_delta = offset_delta
        #self._isNew = len(self._net.nodes)==0

        # edge attrs
        self._ind_edge = -1
        # print '  n_edge',counter.n_edge
        self.ids_edge_sumo = np.zeros(counter.n_edge, np.object)  # net.edges.ids_sumo
        self.ids_edge_sumo[:] = None  # ??needed

        self.ids_fromnode = np.zeros(counter.n_edge, np.int32)
        self.ids_tonode = np.zeros(counter.n_edge, np.int32)
        self.types_edge = np.zeros(counter.n_edge, np.object)
        self.widths = np.zeros(counter.n_edge, np.float32)  # used only for lane width if no lane data is given
        self.nums_lanes = np.zeros(counter.n_edge, np.int32)
        self.speeds_max = np.zeros(counter.n_edge, np.float32)
        self.priorities = np.zeros(counter.n_edge, np.int32)
        #length = 0.0,
        self.shapes = np.zeros(counter.n_edge, np.object)
        self.types_spread = np.zeros(counter.n_edge, np.int32)
        self.spread_choices = net.edges.types_spread.choices
        # "right": 0,
        # "center": 1,
        self.names = np.zeros(counter.n_edge, np.object)
        self.offsets_end = np.zeros(counter.n_edge, np.float32)
        self.widths_lanes_default = np.zeros(counter.n_edge, np.float32)
        self.widths_sidewalk = -1*np.ones(counter.n_edge, np.float32)
        self.inds_lanes_edges = np.zeros(counter.n_edge, np.object)
        #self.inds_lanes_edges[:] = None
        self._ind_lanes_edges = []

        #self.ids_sumoedge_to_ind = {}

        # lane attrs
        # print '  n_lane',counter.n_lane
        self._ind_lane = -1
        self.index_lanes = np.zeros(counter.n_lane, np.int32)
        self.width_lanes = np.zeros(counter.n_lane, np.float32)
        self.speed_max_lanes = np.zeros(counter.n_lane, np.float32)
        self.offset_end_lanes = np.zeros(counter.n_lane, np.float32)
        self.ids_modes_allow = np.zeros(counter.n_lane, np.object)
        self.ids_modes_disallow = np.zeros(counter.n_lane, np.object)
        self.ids_mode_lanes = np.zeros(counter.n_lane, np.int32)
        self.inds_edge_lanes = np.zeros(counter.n_lane, np.int32)
        #self.shapes_lanes = np.zeros(counter.n_lane,np.object)

        # roundabout attrs
        # print '  n_roundabout',counter.n_roundabout
        self._ind_ra = -1
        self.ids_sumoedges_ra = np.zeros(counter.n_roundabout, np.object)
        self.ids_nodes_ra = np.zeros(counter.n_roundabout, np.object)

        ############################

    def startElement(self, name, attrs):
        # print 'startElement',name
        # if attrs.has_key('id'): print attrs['id']
        # elif (attrs.has_key('from')&attrs.has_key('to')): print 'from',attrs['from'],'to',attrs['to']
        # elif (attrs.has_key('from')&attrs.has_key('to')): print 'from',attrs['from'],'to',attrs['to']
        # else: print '.'

        if name == 'edge':
            # if not attrs.has_key('function') or attrs['function'] != 'internal':
            #id_fromnode = nodes.ids_sumo.get_id_from_index(id_fromnode_sumo)
            #id_tonode = nodes.ids_sumo.get_id_from_index(id_tonode_sumo)
            self._ind_edge += 1
            ind = self._ind_edge
            # print 'startElement edge',ind,attrs['id']
            self.ids_edge_sumo[ind] = attrs['id']

            id_fromnode = self._ids_node_sumo.get_id_from_index(str(attrs['from']))
            id_tonode = self._ids_node_sumo.get_id_from_index(str(attrs['to']))
            self.ids_fromnode[ind] = id_fromnode
            self.ids_tonode[ind] = id_tonode

            self.types_edge[ind] = str(attrs.get('type', ''))
            self.nums_lanes[ind] = int(attrs.get('numLanes', 1))

            # attention sumo width attribute is actually lane width!!
            # here we multiply with number of lanes
            # however, will be updated later as a sum of lanewidth
            self.widths[ind] = float(attrs.get('width', 3.5)) * self.nums_lanes[ind]

            self.types_spread[ind] = self.spread_choices[str(attrs.get('spreadType', 'right'))]  # usually center
            # print '  ',self.types_spread[ind]

            #length = 0.0,
            shape = np.array(xm.process_shape(attrs.get('shape', ''), offset=self._offset_delta))

            if len(shape) < 2:  # insufficient shape data
                # shape should be a list of np array coords
                # ATTENTIOn: we need to copy here, otherwise the reference
                # to node coordinates will be kept!!
                shape = np.array([1.0*self._nodecoords[id_fromnode], 1.0*self._nodecoords[id_tonode]])

                if self.types_spread[ind] == 1:  # center
                    angles_perb = get_angles_perpendicular(shape)
                    halfwidth = self.widths[ind]
                    shape[:, 0] += np.cos(angles_perb) * halfwidth
                    shape[:, 1] += np.sin(angles_perb) * halfwidth

            self.shapes[ind] = shape

            self.speeds_max[ind] = float(attrs.get('speed', 13.888))
            self.priorities[ind] = int(attrs.get('priority', 9))
            self.names[ind] = unicode(attrs.get('name', ''))
            self.offsets_end[ind] = float(attrs.get('endOffset', 0.0))

            # this lanewidth will be used as default if no lane width attribute
            # is given
            self.widths_lanes_default[ind] = float(attrs.get('width', 3.0))
            #self.widths_sidewalk[ind] = float(attrs.get('sidewalkWidth',-1.0))

            # check for some attributes that are actually lane attributes

            self._allow_egdeattr = attrs.get('allow', None)
            self._disallow_egdeattr = attrs.get('disallow', None)
            #self._is_laneshape = True
            # print '  self._id_edge',self._id_edge

        elif name == 'lane':
            self._ind_lane += 1
            ind = self._ind_lane
            ind_edge = self._ind_edge
            speed_max_default = -1

            if attrs.has_key('allow'):
                ids_modes_allow = list(self._modenames.get_ids_from_indices(attrs['allow'].split(' ')))

            # done in end element
            # elif self._allow_egdeattr is not None:
            #    ids_modes_allow = list(self._modenames.get_ids_from_indices(self._allow_egdeattr.split(' ')))

            else:
                edgetype = self.types_edge[self._ind_edge]

                if OSMEDGETYPE_TO_MODES.has_key(edgetype):
                    ids_modes_allow, speed_max_default = OSMEDGETYPE_TO_MODES[edgetype]
                else:
                    ids_modes_allow = []

            if attrs.has_key('disallow'):
                ids_modes_disallow = list(self._modenames.get_ids_from_indices(attrs['disallow'].split(' ')))

            # done in end element
            # elif self._disallow_egdeattr is not None:
            #    ids_modes_allow = list(self._modenames.get_ids_from_indices(self._disallow_egdeattr.split(' ')))
            #
            else:
                ids_modes_disallow = []

            index = int(attrs.get('index', -1))
            # use defaults from edge
            width = float(attrs.get('width', -1))
            speed_max = float(attrs.get('speed', -1))
            offset_end_lane = float(attrs.get('endOffset', -1))

            self.set_lane(ind, ind_edge, index, width, speed_max,
                          ids_modes_allow, ids_modes_disallow, offset_end_lane)

        elif name == 'roundabout':
            self._ind_ra += 1
            self.ids_sumoedges_ra[self._ind_ra] = attrs.get('edges', '').split(' ')
            self.ids_nodes_ra[self._ind_ra] = self._ids_node_sumo.get_ids_from_indices(
                attrs.get('nodes', '').split(' '))

    # def characters(self, content):
    #    if self._currentLane is not None:
    #        self._currentShape = self._currentShape + content

    def endElement(self, name):
        #edges = self._net.edges
        #lanes = self._net.lanes
        if name == 'edge':
            # this is the number of lanes declared within the edge tag
            n_lane = self.nums_lanes[self._ind_edge]

            # print 'SumoEdgeReader.endElement',self._ind_lane,n_lane

            # this loop counts from the current number of pased lanes
            # (which may be zero) to the declared number of lanes.
            # This is necessary because it can happen that no lane
            # specifications are provided
            index = len(self._ind_lanes_edges)
            ind_edge = self._ind_edge
            while index < n_lane:
                # if len(self._ind_lanes_edges) ==0:
                # edge description provided no specific lane information
                # create n_lanes and us some properties from current edge
                self._ind_lane += 1
                ind = self._ind_lane

                edgetype = self.types_edge[ind_edge]

                if self._allow_egdeattr is not None:
                    ids_modes_allow = list(self._modenames.get_ids_from_indices(self._allow_egdeattr.split(' ')))
                else:
                    if OSMEDGETYPE_TO_MODES.has_key(edgetype):
                        ids_modes_allow, speed_max_default = OSMEDGETYPE_TO_MODES[edgetype]
                    else:
                        ids_modes_allow = []

                if self._disallow_egdeattr is not None:
                    ids_modes_disallow = list(self._modenames.get_ids_from_indices(self._disallow_egdeattr.split(' ')))
                else:
                    ids_modes_disallow = []

                # figure main mode
                if len(ids_modes_allow) == 1:
                    id_mode_main = ids_modes_allow[0]  # pick  as major mode
                else:
                    id_mode_main = -1  # no major mode specified

                self.index_lanes[ind] = index
                # derive lane attributes from edge attributes
                self.width_lanes[ind] = self.widths_lanes_default[ind_edge]  # copy from edge attr
                self.speed_max_lanes[ind] = self.speeds_max[ind_edge]  # copy from edge attr
                self.offset_end_lanes[ind] = self.offset_end_lanes[ind_edge]  # copy from edge attr

                self.ids_modes_allow[ind] = ids_modes_allow
                self.ids_modes_disallow[ind] = ids_modes_disallow
                self.inds_edge_lanes[ind] = ind_edge
                #self.shapes_lanes[ind]  = self.getShape(attrs.get('shape',''), offset = self._offset_delta)
                self.ids_mode_lanes[ind] = id_mode_main
                self._ind_lanes_edges.append(ind)

                index += 1

            self.inds_lanes_edges[self._ind_edge] = self._ind_lanes_edges
            self._ind_lanes_edges = []

    def set_lane(self, ind, ind_edge, index, width, speed_max, ids_modes_allow, ids_modes_disallow, offset_end_lane):

        if len(ids_modes_allow) == 1:
            id_mode_main = ids_modes_allow[0]  # pick  as major mode
            # elif len(ids_modes_allow) == 1:

        else:
            id_mode_main = -1  # no major mode specified

        is_sidewalk = False
        if index == 0:
            is_sidewalk = (MODES['pedestrian'] in ids_modes_allow)  # test for pedestrian sidewalk

        if speed_max < 0:
            if (index == 0) & is_sidewalk:
                speed_max = 0.8  # default walk speed
            else:
                speed_max = self.speeds_max[ind_edge]  # copy from edge

        if width < 0:
            if is_sidewalk:
                width = 1.0
            else:
                width = self.widths_lanes_default[ind_edge]  # copy from edge

        if is_sidewalk:
            self.widths_sidewalk[ind_edge] = width

        if offset_end_lane < 0:
            offset_end_lane = self.offsets_end[ind_edge]

        self.index_lanes[ind] = index
        self.width_lanes[ind] = width
        self.speed_max_lanes[ind] = speed_max
        self.offset_end_lanes[ind] = offset_end_lane
        self.ids_modes_allow[ind] = ids_modes_allow
        self.ids_modes_disallow[ind] = ids_modes_disallow
        self.ids_mode_lanes[ind] = id_mode_main
        self.inds_edge_lanes[ind] = ind_edge
        #self.shapes_lanes[ind]  = self.getShape(attrs.get('shape',''), offset = self._offset_delta)

        self._ind_lanes_edges.append(ind)
        # self._ids_lane.append(id_lane)

    def write_to_net(self):

        # print 'write_to_net'
        ids_edge = self._net.add_edges(
            ids_sumo=self.ids_edge_sumo,
            ids_fromnode=self.ids_fromnode,
            ids_tonode=self.ids_tonode,
            types=self.types_edge,
            nums_lanes=self.nums_lanes,
            speeds_max=self.speeds_max,
            priorities=self.priorities,
            #lengths = length,
            shapes=self.shapes,
            types_spread=self.types_spread,
            names=self.names,
            offsets_end=self.offsets_end,
            widths_lanes_default=self.widths_lanes_default,
            widths_sidewalk=self.widths_sidewalk,
        )

        # print '  self.inds_edge_lanes',self.inds_edge_lanes
        ids_lanes = self._net.add_lanes(
            indexes=self.index_lanes,
            widths=self.width_lanes,
            speeds_max=self.speed_max_lanes,
            offsets_end=self.offset_end_lanes,
            ids_modes_allow=self.ids_modes_allow,
            ids_modes_disallow=self.ids_modes_disallow,
            ids_mode=self.ids_mode_lanes,  # main mode will be determined from other attributes
            ids_edge=ids_edge[self.inds_edge_lanes],
            # shapes = self.shapes_lanes, # lane shapes are not given -> must be derived from edge shape
        )
        #edges.update_lanes(self._id_edge, self._ids_lane)
        ids_edgelanes = self._net.edges.ids_lanes
        ind = 0
        for inds_lane in self.inds_lanes_edges:
            ids_edgelanes[ids_edge[ind]] = ids_lanes[inds_lane]
            # print '  id_edge,ids_lanes[inds_lane]',ids_edge[ind],ids_lanes[inds_lane]
            ind += 1

        # roundaboutS
        ids_edge_sumo = self._net.edges.ids_sumo
        ids_roundabout = self._net.add_roundabouts(
            ids_nodes=self.ids_nodes_ra,
        )
        ids_edges_ra = self._net.roundabouts.ids_edges
        i = 0
        for id_roundabout in ids_roundabout:
            ids_edges_ra[id_roundabout] = ids_edge_sumo.get_ids_from_indices(self.ids_sumoedges_ra[i])
            i += 1


class SumonetImporter(CmlMixin, Process):
    def __init__(self, net, rootname=None, rootdirpath=None, netfilepath=None,
                 is_clean_nodes=False, logger=None, **kwargs):

        self._init_common('sumonetimporter', name='SUMO net import',
                          logger=logger,
                          info='Converts a SUMO .net.xml file to nod.xml, edg.xml and con.xml file and reads into scenario.',
                          )
        self._net = net

        self.init_cml('netconvert')

        if rootname is None:
            rootname = net.parent.get_rootfilename()

        if rootdirpath is None:
            if net.parent is not None:
                rootdirpath = net.parent.get_workdirpath()
            else:
                rootdirpath = os.getcwd()

        if netfilepath is None:
            netfilepath = os.path.join(rootdirpath, rootname+'.net.xml')

        attrsman = self.get_attrsman()
        self.add_option('netfilepath', netfilepath,
                        groupnames=['options'],  # this will make it show up in the dialog
                        cml='--sumo-net-file',
                        perm='rw',
                        name='Net file',
                        wildcards='Net XML files (*.net.xml)|*.net.xml',
                        metatype='filepath',
                        info='SUMO Net file in XML format.',
                        )

        self.workdirpath = attrsman.add(cm.AttrConf('workdirpath', rootdirpath,
                                                    groupnames=['_private'],  # ['options'],#['_private'],
                                                    perm='r',
                                                    name='Workdir',
                                                    metatype='dirpath',
                                                    info='Working directory for this scenario.',
                                                    ))

        self.rootname = attrsman.add(cm.AttrConf('rootname', rootname,
                                                 groupnames=['_private'],
                                                 perm='r',
                                                 name='Scenario shortname',
                                                 info='Scenario shortname is also rootname of converted files.',
                                                 ))

        self.is_clean_nodes = attrsman.add(cm.AttrConf('is_clean_nodes', is_clean_nodes,
                                                       groupnames=['options'],
                                                       perm='rw',
                                                       name='Clean Nodes',
                                                       info='If set, then shapes around nodes are cleaned up.',
                                                       ))

    def update_params(self):
        """
        Make all parameters consistent.
        example: used by import OSM to calculate/update number of tiles
        from process dialog
        """
        pass
        #self.workdirpath = os.path.dirname(self.netfilepath)
        #bn =  os.path.basename(self.netfilepath).split('.')
        # if len(bn)>0:
        #    self.rootname = bn[0]

    def do(self):
        self.update_params()
        cml = self.get_cml()+' --plain-output-prefix '+filepathlist_to_filepathstring(os.path.join(self.workdirpath, self.rootname))
        # print 'SumonetImporter.do',cml
        #import_xml(self, rootname, dirname, is_clean_nodes = True)
        self.run_cml(cml)
        if self.status == 'success':
            self._net.import_xml(self.rootname, self.workdirpath, is_clean_nodes=self.is_clean_nodes)

        # print 'do',self.newident
        # self._scenario = Scenario(  self.newident,
        #                                parent = None,
        #                                workdirpath = self.workdirpath,
        #                                logger = self.get_logger(),
        #                                )
        return True

    def get_net(self):
        return self._net


class OsmImporter(netconvert.NetConvertMixin):
    def __init__(self, net=None,
                 osmfilepaths=None,
                 netfilepath=None,
                 logger=None,
                 **kwargs):

        # All parameters and Default Values in NetconvertMixin

        if net is None:
            self._net = Network()
        else:
            self._net = net

        self.init_common_netconvert('osmimporter', net,  name='OSM import',
                                    logger=logger,
                                    info='Converts a OSM  file to SUMO nod.xml, edg.xml and con.xml file and reads into scenario.',
                                    )

        # osm specific options
        if osmfilepaths is None:
            if net.parent is not None:
                rootname = net.parent.get_rootfilename()
                rootdirpath = net.parent.get_workdirpath()
            else:
                rootname = net.get_ident()
                rootdirpath = os.getcwd()

            osmfilepaths = os.path.join(rootdirpath, rootname+'.osm.xml')

        self.add_option('osmfilepaths', osmfilepaths,
                        groupnames=['options'],
                        cml='--osm-files',
                        perm='rw',
                        name='OSM files',
                        wildcards='OSM XML files (*.osm)|*.osm*',
                        metatype='filepaths',
                        info='Openstreetmap files to be imported.',
                        )

        self.init_all(**kwargs)
        self.add_option('is_guess_signals_tls', kwargs.get('is_guess_signals_tls', False),
                        groupnames=['options', 'traffic lights'],
                        cml='--tls.guess-signals',
                        perm='rw',
                        name='Guess signals.',
                        info='Interprets tls nodes surrounding an intersection as signal positions for a  larger TLS. This is typical pattern for OSM-derived networks',
                        is_enabled=lambda self: self.is_guess_tls,
                        )
        self.add_option('dist_guess_signal_tls', kwargs.get('dist_guess_signal_tls', 20.0),
                        groupnames=['options', 'traffic lights'],
                        cml='--tls.guess-signals.dist',
                        perm='rw',
                        unit='m',
                        name='Signal guess dist.',
                        info='Distance for interpreting nodes as signal locations',
                        is_enabled=lambda self: self.is_guess_tls & self.is_guess_signals_tls,
                        )


if __name__ == '__main__':
    ###############################################################################
    # print 'sys.path',sys.path
    from agilepy.lib_wx.objpanel import objbrowser
    from agilepy.lib_base.logger import Logger
    #net = Network(logger = Logger())
    net = Network(logger=Logger())
    net.import_xml('facsp2', 'testnet')

    objbrowser(net)
