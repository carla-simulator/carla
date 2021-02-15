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

# @file    publictransportnet.py
# @author  Joerg Schweizer
# @date

import os
import sys
import string
from xml.sax import saxutils, parse, handler


import numpy as np
from collections import OrderedDict
from coremodules.modules_common import *
import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
import agilepy.lib_base.xmlman as xm
from agilepy.lib_base.geometry import *
from agilepy.lib_base.processes import Process, CmlMixin
#from coremodules.network.network import SumoIdsConf, MODES


# example
# https://github.com/planetsumo/sumo/blob/master/tests/sumo/extended/busses/three_busses/input_additional.add.xml

class StopAccessProvider(Process):
    def __init__(self,  net, logger=None, **kwargs):
        print 'StopAccessProvider.__init__'
        self._init_common('stopaccessprovider',
                          parent=net,
                          name='Stop access provider',
                          logger=logger,
                          info='Provides pedestrian (or bike) access to public transport stops.',
                          )

        attrsman = self.set_attrsman(cm.Attrsman(self))

        self.is_bikeaccess = attrsman.add(cm.AttrConf('is_bikeaccess', kwargs.get('is_bikeaccess', False),
                                                      groupnames=['options'],
                                                      perm='rw',
                                                      name='Provide bike access',
                                                      info='If True, provides also bike access to stops.',
                                                      ))

        #self.ids_modes_tocomplete = set([MODES["pedestrian"], MODES["delivery"], MODES["bus"]])

    def do(self):
        print 'StopAccessProvider.do'
        edges = self.parent.edges
        nodes = self.parent.nodes
        lanes = self.parent.lanes
        ptstops = self.parent.ptstops
        ids_stop = ptstops.get_ids()
        ids_lane = ptstops.ids_lane[ids_stop]
        #connections = self.parent.connections
        #ids_edge = edges.get_ids()
        #allow_cycloped = [self.id_mode_bike,self.id_mode_ped]
        #ids_edge_update = []

        id_mode_bike = self.parent.modes.get_id_mode("bicycle")
        id_mode_ped = self.parent.modes.get_id_mode("pedestrian")
        #self.id_mode_ped = MODES["delivery"]

        n_add_access = 0
        for id_stop, id_edge, ids_lane, laneindex in zip(
            ids_stop,
            lanes.ids_edge[ids_lane],
            edges.ids_lanes[lanes.ids_edge[ids_lane]],
            lanes.indexes[ids_lane],
        ):
            if laneindex > 0:
                id_lane = ids_lane[laneindex-1]
                # print ' Check access at stop %s lane %d. al='%(id_stop,id_lane)
                if lanes.get_accesslevel([id_lane], id_mode_ped) == -1:
                    # no pedaccess
                    print '    add ped access at stop %s lane %d, ID edge SUMO "%s".' % (
                        id_stop, id_lane, edges.ids_sumo[id_edge])
                    lanes.add_access(id_lane, id_mode_ped)
                    n_add_access += 1

                if self.is_bikeaccess:
                    if lanes.get_accesslevel([id_lane], id_mode_bike) == -1:
                        print '    add bike access at stop %s lane %d, ID edge SUMO "%s".' % (
                            id_stop, id_lane, edges.ids_sumo[id_edge])
                        lanes.add_access(id_lane, id_mode_bike)
                        n_add_access += 1

            else:
                print 'WARNING: stop %s at edge %d, SUMO ID %s with without access lane.' % (
                    id_stop, id_edge, edges.ids_sumo[id_edge])
                # return False

        print '  Added access to %d stops' % n_add_access
        return True


class PtStops(am.ArrayObjman):
    def __init__(self, net, **kwargs):
        self._init_objman(ident='ptstops',
                          parent=net,
                          name='Public transport stops',
                          info='Contains information on public transport stops and stations.',
                          xmltag=('additional', 'busStop', 'stopnames'),
                          version=0.1,
                          **kwargs)
        self._init_attributes()

    def _init_attributes(self):
        net = self.parent
        self.add_col(am.ArrayConf('stopnames', default='',
                                  dtype='object',
                                  perm='rw',
                                  name='Short name',
                                  symbol='Short N.',
                                  is_index=True,
                                  info='This is a unique name or number of the stop or station.',
                                  xmltag='id',
                                  ))

        self.add_col(am.ArrayConf('stopnames_human', default='',
                                  dtype=np.object,
                                  perm='rw',
                                  name='Long name',
                                  symbol='Long N.',
                                  info='This is a human readable name of the stop or station.',
                                  xmltag='name',
                                  ))

        self.add_col(am.IdsArrayConf('ids_lane', net.lanes,
                                     name='ID Lane',
                                     info='ID of lane for this parking position. ',
                                     xmltag='lane',
                                     ))

        self.add_col(am.ArrayConf('positions_from', 0.0,
                                  perm='r',
                                  name='Start pos',
                                  unit='m',
                                  xmltag='startPos',
                                  info='Position on lane where stop starts.',
                                  ))

        self.add_col(am.ArrayConf('positions_to', 0.0,
                                  perm='r',
                                  name='End pos',
                                  unit='m',
                                  xmltag='endPos',
                                  info='Position on lane where stop ends.',
                                  ))

        self.add_col(am.ArrayConf('is_rightside', True,
                                  dtype=np.bool,
                                  perm='rw',
                                  name='Right side?',
                                  info='If true, stop is on the right side of the lane.',
                                  ))

        self.add_col(am.ArrayConf('widths', 1.5,
                                  perm='rw',
                                  name='Width',
                                  unit='m',
                                  info='Width of stop.',
                                  ))

        self.add_col(am.ArrayConf('centroids', np.array([0.0, 0.0, 0.0], dtype=np.float32),
                                  dtype=np.float32,
                                  perm='r',
                                  name='Centroid',
                                  unit='m',
                                  info='Central coordinates of this stop.',
                                  ))

        self.add_col(am.ArrayConf('are_friendly_pos', False,
                                  dtype=np.bool,
                                  perm='rw',
                                  name='Is friendly pos.',
                                  symbol='friendly pos.',
                                  xmltag='friendlyPos',
                                  info='If True, invalid stop positions will be corrected automatically.',
                                  ))

        # if self.get_version()<0.1:
        #    self.set_xmltag(('additional','busStop','stopnames'))
        #    self.ids_lane.set_xmltag('lane')

    # necessary?
    # def set_ptlines(self, ptlines):
    #    self.add_col(am.IdlistsArrayConf( 'ids_lines', ptlines,
    #                                groupnames = ['state'],
    #                                name = 'PT line IDs',
    #                                info = 'List of Public Transport lines  stopping at this stop, for visualization only.',
    #                                ))

    def clear_stops(self):
        self.clear()

    def adjust(self, pos_from_min=5.0, length_stop_default=20.0):
        lanes = self.ids_lane.get_linktab()
        edges = lanes.parent.edges
        ids = self.get_ids()
        ids_eliminate = []
        for id_stop, id_lane in zip(ids, self.ids_lane):
            pos_from, pos_to = (self.positions_from[id_stop], self.positions_to[id_stop])
            edgelength = edges.lengths[lanes.ids_edge[id_lane]]
            stoplength = pos_to-pos_from
            if stoplength < length_stop_default:
                stoplength = length_stop_default

            if pos_from < pos_from_min:
                self.positions_from[id_stop] = pos_from_min
                pos_to_new = pos_from_min+stoplength
                if pos_to_new > edgelength-pos_from_min:
                    self.positions_to[id_stop] = edgelength-pos_from_min
                else:
                    self.positions_to[id_stop] = pos_to_new

    def format_ids(self, ids):
        return ','.join(self.stopnames[ids])

    def get_id_from_formatted(self, idstr):
        return self.stopnames.get_id_from_index(idstr)

    def get_ids_from_formatted(self, idstrs):
        return self.stopnames.get_ids_from_indices_save(idstrs.split(','))

    def make(self, stopname, **kwargs):
        return self.add_row(stopnames=stopname,
                            ids_lane=kwargs.get('id_lane', None),
                            positions_from=kwargs.get('position_from', None),
                            positions_to=kwargs.get('position_to', None),
                            widths=kwargs.get('width', None),
                            ids_lines=kwargs.get('ids_line', None),
                            stopnames_human=kwargs.get('stopname_human', None),
                            are_friendly_pos=kwargs.get('is_friendly_pos', None),
                            )

    def get_stopfilepath(self):
        return self.parent.get_rootfilepath()+'.add.xml'

    def get_closest(self, coords):
        """
        Returns the closest stop for each coord in coords vector.
        """

        n = len(coords)
        # print 'get_closest',n

        ids_stop = self.get_ids()
        coords_stop = self.centroids[ids_stop]
        ids_stop_closest = np.zeros(n, dtype=np.int32)
        i = 0
        for coord in coords:
            ind_closest = np.argmin(np.sum((coord-coords_stop)**2, 1))
            ids_stop_closest[i] = ids_stop[ind_closest]
            i += 1

        return ids_stop_closest

    def get_stops_proximity(self, dist_walk_los=200):
        """
        Returns a dictionary with stop ID as key
        and a list of stop IDs of close stations and a list of the resoective
        distances as value.
        Close stops are considered stops with a walking distance less than
        dist_walk_los
        """
        ids_stop = self.get_ids()
        coords = self.centroids[ids_stop]
        stop_proximity = {}
        ind = 0
        for id_stop, coord in zip(ids_stop, coords):
            # print '   id_stop ',id_stop,coord
            # print '    ',np.sqrt(np.sum(coords-coord,1)**2)
            dists = np.sqrt(np.sum((coords-coord)**2, 1))
            inds_prox = dists <= dist_walk_los
            # inds_prox[ind] = False # avoid putting stop itself
            stop_proximity[id_stop] = (dists[inds_prox], ids_stop[inds_prox])
            ind += 1
        return stop_proximity

    def write_xml(self, fd=None, indent=0):
        #  <busStop id="busstop4" lane="1/0to2/0_0" startPos="20" endPos="40" lines="100 101"/>

        ids_stop = self.get_ids()
        get_sumoinfo_from_id_lane = self.parent.lanes.get_sumoinfo_from_id_lane
        for id_stop, id_lane, pos_from, pos_to, name_human, is_friendly_pos in zip(
            self.stopnames[ids_stop],
            self.ids_lane[ids_stop],
            self.positions_from[ids_stop],
            self.positions_to[ids_stop],
            self.stopnames_human[ids_stop],
            self.are_friendly_pos[ids_stop],
        ):

            fd.write(xm.start('busStop', indent=indent))
            fd.write(xm.num('id', id_stop))
            fd.write(xm.num('lane', get_sumoinfo_from_id_lane(id_lane)))
            fd.write(xm.num('startPos', pos_from))
            fd.write(xm.num('endPos', pos_to))
            # fd.write(xm.arr('lines',lines,indent+2))
            if len(name_human) > 0:
                fd.write(xm.num('name', name_human))
            if is_friendly_pos:
                fd.write(xm.num('friendlyPos', '1'))
            else:
                fd.write(xm.num('friendlyPos', '0'))
            fd.write(xm.stopit())  # ends stop

    def export_sumoxml(self, filepath=None, encoding='UTF-8'):
        """
        Export stops to SUMO stop xml file.
        """
        print 'export_sumoxml', filepath, len(self)
        if len(self) == 0:
            return None

        if filepath is None:
            filepath = self.get_stopfilepath()

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

        self.write_xml(fd, indent+2)

        fd.write(xm.end('additional', indent))
        fd.close()
        return filepath

    def import_sumostops(self, filepath=None, logger=None, is_remove_xmlfiles=False):
        if filepath is None:
            filepath = self.get_stopfilepath()

        print 'import_sumostops %s ' % (filepath,)

        fd = open(filepath, 'r')

        reader = StopReader(self)
        try:
            parse(filepath, reader)
            self.update_centroids()

            if is_remove_xmlfiles:
                os.remove(filepath)

        except KeyError:
            print >> sys.stderr, "Error: Problems with reading file %s" % filepath
            raise

    def update_centroids(self):

        inds = self.get_inds()
        # print 'update_centroids',inds
        poss_center = 0.5*(self.positions_from.get_value()[inds]
                           + self.positions_to.get_value()[inds])
        centroids = self.centroids.get_value()
        get_coord_from_pos = self.parent.lanes.get_coord_from_pos
        #ids_stops = self.get_keys()
        #inds = self.get_inds_from_keys(ids_stops)

        for ind, id_lane, pos_center in zip(inds, self.ids_lane.get_value()[inds], poss_center):
            # print '  centroids[ind]',centroids[ind]
            # print '  id_lane, pos_center',id_lane, pos_center
            # print '  get_coord_from_pos(id_lane, pos_center)',get_coord_from_pos(id_lane, pos_center)
            centroids[ind] = get_coord_from_pos(id_lane, pos_center)


class StopReader(handler.ContentHandler):
    """Reads pt stops from xml file into pt stop structure"""

    def __init__(self, stops):
        self._stops = stops
        self._edges = stops.parent.edges
        self._get_id_lane_from_sumoinfo = self._edges.get_id_lane_from_sumoinfo_check
        #self._get_id_lane_from_sumoinfo = net.edges.ids_sumo.get_ids_from_indices

    def startElement(self, name, attrs):

        if name == 'busStop':
            # print 'startElement',name,attrs['id']
            # for key in attrs.keys():
            #    print '  ',key, attrs[key],type(attrs[key]),attrs[key]=='1',attrs[key]==1

            id_stop = attrs['id']
            id_edge_sumo, ind_lane_str = attrs['lane'].split('_')

            id_lane = self._get_id_lane_from_sumoinfo(id_edge_sumo, int(ind_lane_str.strip()))
            if id_lane != -1:

                if attrs.has_key('startPos'):
                    pos_from = float(attrs['startPos'])
                else:
                    pos_from = None

                if attrs.has_key('endPos'):
                    pos_to = float(attrs['endPos'])
                else:
                    pos_to = None

                #is_friendly_pos = attrs.get('friendlyPos','') == '1'
                # print '  friendly_pos=',attrs['friendlyPos'],attrs.get('friendlyPos',''),is_friendly_pos

                # if attrs.has_key('name'):
                #    print '  name is',attrs['name'],'--',attrs.get('name',''),'--'
                #    stopname = attrs['name']
                # else:
                #    stopname = ''
                # print '  => stopname =',stopname
                id_stop = id_stop.replace('/', '-')
                id_stop = id_stop.replace(':', '-')
                #is_friendly_pos = attrs.get('friendly_pos','') == 'true'
                self._stops.make(id_stop,
                                 id_lane=id_lane,
                                 position_from=pos_from,
                                 position_to=pos_to,
                                 stopname_human=attrs.get('name', ''),
                                 is_friendly_pos=attrs.get('friendlyPos', '') == '1',
                                 )

            else:
                print 'WARNING in StopReader: stop %s has non-existant lane %s' % (id_stop, attrs['lane'])
