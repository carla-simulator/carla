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

# @file    origin_to_destination.py
# @author  Joerg Schweizer
# @date


import numpy as np
from numpy import random
import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
import agilepy.lib_base.xmlman as xm
#from coremodules.modules_common import *
from coremodules.network.network import SumoIdsConf, MODES
from coremodules.network import routing
from agilepy.lib_base.processes import Process, CmlMixin
#import coremodules.demand.demand as dm
import demand as dm
import demandbase as db
# print 'dir(dm)',dir(dm)
#from demand import OPTIONMAP_POS_DEPARTURE
# OPTIONMAP_POS_ARRIVAL
# OPTIONMAP_SPEED_DEPARTURE
# OPTIONMAP_SPEED_ARRIVAL
# OPTIONMAP_LANE_DEPART
# OPTIONMAP_LANE_ARRIVAL


class OdTripgenerator(Process):
    def __init__(self, odintervals, trips, logger=None, **kwargs):
        """
        CURRENTLY NOT IN USE!!
        """
        self._init_common('odtripgenerator', name='OD tripgenerator',
                          logger=logger,
                          info='Generates trips from OD demand .',
                          )
        self._odintervals = odintervals

        attrsman = self.get_attrsman()
        self.add_option('netfilepath', netfilepath,
                        # this will make it show up in the dialog
                        groupnames=['options'],
                        cml='--sumo-net-file',
                        perm='rw',
                        name='Net file',
                        wildcards='Net XML files (*.net.xml)|*.net.xml',
                        metatype='filepath',
                        info='SUMO Net file in XML format.',
                        )

        self.workdirpath = attrsman.add(cm.AttrConf('workdirpath', rootdirpath,
                                                    # ['options'],#['_private'],
                                                    groupnames=['_private'],
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
        cml = self.get_cml()+' --plain-output-prefix ' + \
            filepathlist_to_filepathstring(
                os.path.join(self.workdirpath, self.rootname))
        # print 'SumonetImporter.do',cml
        #import_xml(self, rootname, dirname, is_clean_nodes = True)
        self.run_cml(cml)
        if self.status == 'success':
            self._net.import_xml(
                self.rootname, self.workdirpath, is_clean_nodes=self.is_clean_nodes)
            return True
        else:
            return False
        # print 'do',self.newident
        # self._scenario = Scenario(  self.newident,
        #                                parent = None,
        #                                workdirpath = self.workdirpath,
        #                                logger = self.get_logger(),
        #                                )

    def get_net(self):
        return self._net


class OdFlowTable(am.ArrayObjman):
    def __init__(self, parent, modes, zones, activitytypes=None, **kwargs):
        self._init_objman(ident='odflowtab', parent=parent,
                          name='OD flows',
                          info='Table with intervals, modes, OD and respective number of trips.',
                          #xmltag = ('odtrips','odtrip',None),
                          **kwargs)

        self.add_col(am.ArrayConf('times_start', 0,
                                  groupnames=['parameters'],
                                  perm='r',
                                  name='Start time',
                                  unit='s',
                                  info='Start time of interval in seconds (no fractional seconds).',
                                  xmltag='t_start',
                                  ))

        self.add_col(am.ArrayConf('times_end', 3600,
                                  groupnames=['parameters'],
                                  perm='r',
                                  name='End time',
                                  unit='s',
                                  info='End time of interval in seconds (no fractional seconds).',
                                  xmltag='t_end',
                                  ))

        self.add_col(am.IdsArrayConf('ids_mode', modes,
                                     groupnames=['parameters'],
                                     perm='r',
                                     #choices = MODES,
                                     name='ID mode',
                                     xmltag='vClass',
                                     info='ID of transport mode.',
                                     ))

        self.add_col(am.IdsArrayConf('ids_orig', zones,
                                     groupnames=['parameters'],
                                     name='Orig.',
                                     perm='r',
                                     #choices =  zones.ids_sumo.get_indexmap(),
                                     info='traffic assignment zone of origin of trip.',
                                     xmltag='id_orig',
                                     ))

        self.add_col(am.IdsArrayConf('ids_dest', zones,
                                     groupnames=['parameters'],
                                     name='Dest.',
                                     perm='r',
                                     #choices =  zones.ids_sumo.get_indexmap(),
                                     info='ID of traffic assignment zone of destination of trip.',
                                     xmltag='id_dest',
                                     ))

        self.add_col(am.ArrayConf('tripnumbers', 0,
                                  groupnames=['state'],
                                  perm='rw',
                                  name='Trips',
                                  info='Number of trips from zone with ID Orig to zone with ID Dest.',
                                  xmltag='tripnumber',
                                  ))

        if activitytypes is not None:
            self.add_col(am.IdsArrayConf('ids_activitytype_orig', activitytypes,
                                         groupnames=['parameters'],
                                         perm='rw',
                                         #choices = activitytypes.names.get_indexmap(),
                                         name='Activity type at orig.',
                                         symbol='Act. orig.',
                                         info='Type of activity performed at origin, before the trip.',
                                         ))

            self.add_col(am.IdsArrayConf('ids_activitytype_dest', activitytypes,
                                         groupnames=['parameters'],
                                         perm='rw',
                                         #choices = activitytypes.names.get_indexmap(),
                                         name='Activity type at dest.',
                                         symbol='Act. dest.',
                                         info='Type of activity performed at destination, after the trip.',
                                         ))
        #self.add( cm.ObjConf( zones, is_child = False,groups = ['_private']))

    def add_flows(self,  time_start,
                  time_end,
                  id_mode,
                  ids_orig,
                  ids_dest,
                  tripnumbers,
                  id_activitytype_orig=1,
                  id_activitytype_dest=1,
                  ):
        n = len(tripnumbers)
        self.add_rows(n=n,
                      times_start=time_start*np.ones(n),
                      times_end=time_end*np.ones(n),
                      ids_mode=id_mode*np.ones(n),
                      ids_orig=ids_orig,
                      ids_dest=ids_dest,
                      tripnumbers=tripnumbers,
                      ids_activitytype_orig=id_activitytype_orig*np.ones(n),
                      ids_activitytype_dest=id_activitytype_dest*np.ones(n),
                      )


class OdTrips(am.ArrayObjman):
    def __init__(self, ident, parent, zones, **kwargs):
        self._init_objman(ident, parent=parent,
                          name='OD trips',
                          info='Contains the number of trips between an origin and a destination zone.',
                          version=0.2,
                          xmltag=('odtrips', 'odtrip', None), **kwargs)

        self._init_attributes(zones)

    def _init_attributes(self, zones=None):
        # print '_init_attributes',self.ident
        if not self.has_attrname('zones'):
            self.add(cm.ObjConf(
                zones, is_child=False, groups=['_private']))
        else:
            # zones is already an attribute
            zones = self.zones.get_value()

        if self.get_version() < 0.1:
            # update attrs from previous
            # IdsArrayConf not yet modifiable interactively, despite perm = 'rw',!!!
            self.ids_orig.set_perm('rw')
            self.ids_dest.set_perm('rw')

        if hasattr(self, 'func_delete_row'):
            self.func_make_row._is_returnval = False
            self.func_delete_row._is_returnval = False

        self.add_col(am.IdsArrayConf('ids_orig', zones,
                                     groupnames=['state'],
                                     perm='rw',
                                     name='Orig.',
                                     #choices =  zones.ids_sumo.get_indexmap(),
                                     info='traffic assignment zone of origin of trip.',
                                     xmltag='id_orig',
                                     ))

        self.add_col(am.IdsArrayConf('ids_dest', zones,
                                     groupnames=['state'],
                                     perm='rw',
                                     name='Dest.',
                                     #choices =  zones.ids_sumo.get_indexmap(),
                                     info='ID of traffic assignment zone of destination of trip.',
                                     xmltag='id_dest',
                                     ))

        self.add_col(am.ArrayConf('tripnumbers', 0,
                                  groupnames=['state'],
                                  perm='rw',
                                  name='Trips',
                                  info='Number of trips from zone with ID Orig to zone with ID Dest.',
                                  xmltag='tripnumber',
                                  ))

        # print '  pre add func_make_row'
        self.add(cm.FuncConf('func_make_row', 'on_add_row', None,
                             groupnames=['rowfunctions', '_private'],
                             name='New OD flow.',
                             info='Add a new OD flow.',
                             is_returnval=False,
                             ))
        # print '  post add func_make_row'
        self.add(cm.FuncConf('func_delete_row', 'on_del_row', None,
                             groupnames=['rowfunctions', '_private'],
                             name='Del OD flow',
                             info='Delete OD flow.',
                             is_returnval=False,
                             ))

        # print '  _init_attributes done',self.ident

    def _init_constants(self):
        #self.edgeweights_orig = None
        #self.edgeweights_dest = None
        pass

    def on_del_row(self, id_row=None):
        if id_row is not None:
            # print 'on_del_row', id_row
            self.del_row(id_row)

    def on_add_row(self, id_row=None):
        print 'on_add_row'
        if len(self) > 0:

            # copy previous
            od_last = self.get_row(self.get_ids()[-1])
            #id_orig = self.odtab.ids_orig.get(id_last)
            #id_dest = self.odtab.ids_dest.get(id_last)
            #id = self.suggest_id()
            self.add_row(**od_last)
        else:
            self.add_row(self.suggest_id())

    def generate_odflows(self, odflowtab,  time_start, time_end, id_mode, **kwargs):
        """
        Insert all od flows in odflowtab.
        """
        # for id_od in self.get_ids():
        odflowtab.add_flows(time_start,
                            time_end,
                            id_mode,
                            self.ids_orig.get_value(),
                            self.ids_dest.get_value(),
                            self.tripnumbers.get_value(),
                            **kwargs
                            )

    def generate_trips(self, demand, time_start, time_end, id_mode,
                       pos_depart_default=db.OPTIONMAP_POS_DEPARTURE['random_free'],
                       #pos_arrival_default = db.OPTIONMAP_POS_ARRIVAL['max'],
                       pos_arrival_default=db.OPTIONMAP_POS_ARRIVAL['random'],
                       speed_depart_default=0.0,
                       speed_arrival_default=0.0,
                       # pedestrians always depart on lane 0
                       ind_lane_depart_default=db.OPTIONMAP_LANE_DEPART['allowed'],
                       # pedestrians always arrive on lane 0
                       ind_lane_arrival_default=db.OPTIONMAP_LANE_ARRIVAL['current'],
                       n_trials_connect=5,
                       is_make_route=True,
                       ):
        """
        Generates trips in demand.trip table.
        """
        print 'generate_trips', time_start, time_end, id_mode
        id_mode_ped = MODES['pedestrian']
        #OPTIONMAP_POS_DEPARTURE = { -1:"random",-2:"free",-3:"random_free",-4:"base"}
        #OPTIONMAP_POS_ARRIVAL = { -1:"random",-2:"max"}
        #OPTIONMAP_SPEED_DEPARTURE = { -1:"random",-2:"max"}
        #OPTIONMAP_SPEED_ARRIVAL = { -1:"current"}
        #OPTIONMAP_LANE_DEPART = {-1:"random",-2:"free",-3:"departlane"}
        #OPTIONMAP_LANE_ARRIVAL = { -1:"current"}

        trips = demand.trips
        #ids_vtype_mode = demand.vtypes.select_by_mode(id_mode)
        ids_vtype_mode, prob_vtype_mode = demand.vtypes.select_by_mode(
            id_mode, is_share=True)
        # print '  ids_vtype_mode', ids_vtype_mode
        n_vtypes = len(ids_vtype_mode)
        zones = self.zones.get_value()
        edges = zones.ids_edges_orig.get_linktab()
        edgelengths = edges.lengths

        if n_trials_connect > 0:
            # initialize routing to verify connection
            fstar = edges.get_fstar(is_ignor_connections=False)
            times = edges.get_times(id_mode=id_mode, is_check_lanes=True)

        n_trips_generated = 0
        n_trips_failed = 0

        is_nocon = False
        route = []
        for id_od in self.get_ids():
            id_orig = self.ids_orig[id_od]
            id_dest = self.ids_dest[id_od]
            tripnumber = self.tripnumbers[id_od]

            ids_edges_orig_raw = zones.ids_edges_orig[id_orig]
            ids_edges_dest_raw = zones.ids_edges_dest[id_dest]

            prob_edges_orig_raw = zones.probs_edges_orig[id_orig]
            prob_edges_dest_raw = zones.probs_edges_dest[id_dest]

            # check accessibility of origin edges
            ids_edges_orig = []
            prob_edges_orig = []
            inds_lane_orig = []
            for i in xrange(len(ids_edges_orig_raw)):
                id_edge = ids_edges_orig_raw[i]
                # if check accessibility...
                ind_lane_depart = edges.get_laneindex_allowed(id_edge, id_mode)
                # print '  get_laneindex_allowed',id_mode,id_edge,edges.ids_sumo[id_edge],ind_lane_depart
                if ind_lane_depart >= 0:
                    ids_edges_orig.append(id_edge)
                    prob_edges_orig.append(prob_edges_orig_raw[i])
                    inds_lane_orig.append(ind_lane_depart)

            # check accessibility of destination edges
            ids_edges_dest = []
            prob_edges_dest = []
            inds_lane_dest = []
            for i in xrange(len(ids_edges_dest_raw)):
                id_edge = ids_edges_dest_raw[i]
                # if check accessibility...
                ind_lane_arrival = edges.get_laneindex_allowed(
                    id_edge, id_mode)
                if ind_lane_arrival >= 0:
                    ids_edges_dest.append(id_edge)
                    prob_edges_dest.append(prob_edges_dest_raw[i])
                    inds_lane_dest.append(ind_lane_arrival)

            n_edges_orig = len(ids_edges_orig)
            n_edges_dest = len(ids_edges_dest)

            if (n_edges_orig > 0) & (n_edges_dest > 0) & (tripnumber > 0):
                # renormalize weights
                prob_edges_orig = np.array(prob_edges_orig, np.float)
                prob_edges_orig = prob_edges_orig/np.sum(prob_edges_orig)
                prob_edges_dest = np.array(prob_edges_dest, np.float)
                prob_edges_dest = prob_edges_dest/np.sum(prob_edges_dest)

                for d in xrange(int(tripnumber+0.5)):
                    time_depart = random.uniform(time_start, time_end)

                    if (n_trials_connect > 0) & (id_mode != id_mode_ped):
                        # check if origin and destination edges are connected
                        n = n_trials_connect
                        is_nocon = True
                        while (n > 0) & is_nocon:
                            # this algorithm can be improved by calculating
                            # the minimum cost tree and chacking all destinations
                            i_orig = np.argmax(random.rand(
                                n_edges_orig)*prob_edges_orig)
                            id_edge_orig = ids_edges_orig[i_orig]
                            i_dest = np.argmax(random.rand(
                                n_edges_dest)*prob_edges_dest)
                            id_edge_dest = ids_edges_dest[i_dest]

                            cost, route = routing.get_mincostroute_edge2edge(id_edge_orig,
                                                                             id_edge_dest,
                                                                             weights=times,
                                                                             fstar=fstar
                                                                             )
                            is_nocon = len(route) == 0
                            n -= 1
                        # print '  found route',len(route),n_trials_connect-n
                        if not is_make_route:
                            route = []
                    else:
                        # no check if origin and destination edges are connected
                        i_orig = np.argmax(random.rand(
                            n_edges_orig)*prob_edges_orig)
                        id_edge_orig = ids_edges_orig[i_orig]

                        i_dest = np.argmax(random.rand(
                            n_edges_dest)*prob_edges_dest)
                        id_edge_dest = ids_edges_dest[i_dest]

                    if not is_nocon:
                        ind_lane_orig = inds_lane_orig[i_orig]
                        ind_lane_dest = inds_lane_dest[i_dest]

                        pos_depart = pos_depart_default
                        pos_arrival = pos_arrival_default
                        # print '  bef:pos_depart,pos_arrival,id_mode,id_mode_ped',  pos_depart,pos_arrival,id_mode,id_mode_ped
                        if id_mode_ped == id_mode:
                            # persons do not understand "random", "max" etc
                            # so produce a random number here

                            #{ -1:"random",-2:"free",-3:"random_free",-4:"base"}
                            edgelength = edgelengths[id_edge_orig]
                            if pos_depart in (-1, -2, -3):
                                pos_depart = random.uniform(
                                    0.1*edgelength, 0.9*edgelength, 1)[0]
                            else:
                                pos_depart = 0.1*edgelength

                            # { -1:"random",-2:"max"}
                            edgelength = edgelengths[id_edge_dest]
                            if pos_arrival == -1:
                                pos_arrival = random.uniform(
                                    0.1*edgelength, 0.9*edgelength, 1)[0]
                            else:
                                pos_arrival = 0.9*edgelength
                        # print '  af:pos_depart,pos_arrival,id_mode,id_mode_ped',  pos_depart,pos_arrival,id_mode,id_mode_ped
                        # print '  n_vtypes',n_vtypes
                        # print '  random.randint(n_vtypes)',random.randint(n_vtypes)
                        #id_vtype = ids_vtype_mode[random.randint(n_vtypes)]
                        id_vtype = ids_vtype_mode[np.argmax(
                            random.rand(n_vtypes)*prob_vtype_mode)]
                        id_trip = trips.make_trip(id_vtype=id_vtype,
                                                  time_depart=time_depart,
                                                  id_edge_depart=id_edge_orig,
                                                  id_edge_arrival=id_edge_dest,
                                                  ind_lane_depart=ind_lane_orig,
                                                  ind_lane_arrival=ind_lane_dest,
                                                  position_depart=pos_depart,
                                                  position_arrival=pos_arrival,
                                                  speed_depart=speed_depart_default,
                                                  speed_arrival=speed_arrival_default,
                                                  route=route,
                                                  )
                        # print '  ',id_trip,id_edge_orig,edges.ids_sumo[id_edge_orig],ind_lane_depart
                        # print '  ',id_trip,self.position_depart[id_trip],
                        n_trips_generated += 1
                    else:
                        n_trips_failed += tripnumber
            else:
                n_trips_failed += tripnumber

        print '  n_trips_generated', n_trips_generated
        print '  n_trips_failed', n_trips_failed

    def add_od_trips(self, scale, names_orig, names_dest, tripnumbers):
        print 'OdTrips.add_od_trips'
        # print '  scale, names_orig, names_dest, tripnumbers',scale, names_orig, names_dest, tripnumbers,len(tripnumbers)
        zones = self.get_zones()

        for name_orig, name_dest, tripnumber in zip(names_orig, names_dest, tripnumbers):
            # print '  check',name_orig, name_dest, tripnumbers,zones.ids_sumo.has_index(name_orig),zones.ids_sumo.has_index(name_dest)
            if (zones.ids_sumo.has_index(name_orig)) & (zones.ids_sumo.has_index(name_dest)):
                print '  add', zones.ids_sumo.get_id_from_index(
                    name_orig), zones.ids_sumo.get_id_from_index(name_dest)
                self.add_row(ids_orig=zones.ids_sumo.get_id_from_index(name_orig),
                             ids_dest=zones.ids_sumo.get_id_from_index(
                                 name_dest),
                             tripnumbers=scale * tripnumber)
            else:
                print '  WARNING: zone named %s or %s not known' % (
                    name_orig, names_dest)
                print '  zones indexmap', zones.get_indexmap()
                print '  ids_sumo', zones.ids_sumo.get_value()
                print '  ids_sumo._index_to_id', zones.ids_sumo._index_to_id

    def get_zones(self):
        return self.ids_dest.get_linktab()


class OdModes(am.ArrayObjman):
    def __init__(self, ident, parent, modes, zones, **kwargs):
        self._init_objman(ident, parent=parent,
                          name='Mode OD tables',
                          info='Contains for each transport mode an OD trip table.',
                          xmltag=('modesods', 'modeods', 'ids_mode'), **kwargs)

        self.add_col(am.IdsArrayConf('ids_mode', modes,
                                     groupnames=['state'],
                                     choices=MODES,
                                     name='ID mode',
                                     xmltag='vClass',
                                     info='ID of transport mode.',
                                     ))

        self.add_col(cm.ObjsConf('odtrips',
                                 groupnames=['state'],
                                 is_save=True,
                                 name='OD matrix',
                                 info='Matrix with trips from origin to destintion for a specific mode.',
                                 ))

        self.add(cm.ObjConf(zones, is_child=False, groups=['_private']))

    def generate_trips(self, demand, time_start, time_end, **kwargs):
        for id_od_mode in self.get_ids():
            self.odtrips[id_od_mode].generate_trips(
                demand, time_start, time_end, self.ids_mode[id_od_mode], **kwargs)

    def generate_odflows(self, odflowtab, time_start, time_end, **kwargs):
        for id_od_mode in self.get_ids():
            self.odtrips[id_od_mode].generate_odflows(
                odflowtab, time_start, time_end, self.ids_mode[id_od_mode], **kwargs)

    def add_od_trips(self, id_mode, scale, names_orig, names_dest, tripnumbers):
        # print 'OdModes.add_od_trips',id_mode, scale, names_orig, names_dest, tripnumbers
        ids_mode = self.select_ids(self.ids_mode.get_value() == id_mode)
        if len(ids_mode) == 0:
            id_od_modes = self.add_row(ids_mode=id_mode)
            # print '  create',id_od_modes
            odtrips = OdTrips((self.odtrips.attrname, id_od_modes),
                              self, self.zones.get_value())
            self.odtrips[id_od_modes] = odtrips
            odtrips.add_od_trips(scale, names_orig, names_dest, tripnumbers)
            return odtrips
        else:
            id_od_modes = ids_mode[0]  # modes are unique
            # print '  use',id_od_modes
            self.odtrips[id_od_modes].add_od_trips(
                scale, names_orig, names_dest, tripnumbers)
            return self.odtrips[id_od_modes]


class OdIntervals(am.ArrayObjman):
    def __init__(self, ident='odintervals',  parent=None, net=None, zones=None, **kwargs):
        self._init_objman(ident, parent=parent,  # = demand
                          name='OD Demand',
                          info='Contains origin-to-destination zone transport demand for different time intervals.',
                          xmltag=('odintervals', 'odinteval', None), **kwargs)

        self.add_col(am.ArrayConf('times_start', 0,
                                  groupnames=['state'],
                                  perm='rw',
                                  name='Start time',
                                  unit='s',
                                  info='Start time of interval in seconds (no fractional seconds).',
                                  xmltag='t_start',
                                  ))

        self.add_col(am.ArrayConf('times_end', 3600,
                                  groupnames=['state'],
                                  perm='rw',
                                  name='End time',
                                  unit='s',
                                  info='End time of interval in seconds (no fractional seconds).',
                                  xmltag='t_end',
                                  ))

        activitytypes = self.parent.activitytypes
        self.add_col(am.IdsArrayConf('ids_activitytype_orig', activitytypes,
                                     groupnames=['parameters'],
                                     perm='rw',
                                     #choices = activitytypes.names.get_indexmap(),
                                     name='Activity type at orig.',
                                     symbol='Act. orig.',
                                     info='Type of activity performed at origin, before the trip.',
                                     #xmltag = 'actType',
                                     #xmlmap = get_inversemap( activitytypes.names.get_indexmap()),
                                     ))

        self.add_col(am.IdsArrayConf('ids_activitytype_dest', activitytypes,
                                     groupnames=['parameters'],
                                     perm='rw',
                                     #choices = activitytypes.names.get_indexmap(),
                                     name='Activity type at dest.',
                                     symbol='Act. dest.',
                                     info='Type of activity performed at destination, after the trip.',
                                     #xmltag = 'actType',
                                     #xmlmap = get_inversemap( activitytypes.names.get_indexmap()),
                                     ))

        self.add_col(cm.ObjsConf('odmodes',
                                 groupnames=['state'],
                                 is_save=True,
                                 name='OD modes',
                                 info='OD transport demand for all transport modes within the respective time interval.',
                                 ))
        self.add(cm.ObjConf(net, is_child=False, groups=['_private']))
        self.add(cm.ObjConf(zones, is_child=False, groups=['_private']))
        # print 'OdIntervals.__init__',self,dir(self)

    def generate_trips(self, **kwargs):
        """
        Generates trips in trip table.
        """
        # make sure zone edges are up to date
        self.get_zones().refresh_zoneedges()
        demand = self.parent
        for id_inter in self.get_ids():
            self.odmodes[id_inter].generate_trips(demand,   self.times_start[id_inter],
                                                  self.times_end[id_inter],
                                                  **kwargs)

    def generate_odflows(self, **kwargs):
        """
        Generates a flat table with all OD flows.
        """
        odflowtab = OdFlowTable(self, self.get_modes(),
                                self.get_zones(), self.get_activitytypes())
        for id_inter in self.get_ids():
            self.odmodes[id_inter].generate_odflows(odflowtab,
                                                    self.times_start[id_inter],
                                                    self.times_end[id_inter],
                                                    id_activitytype_orig=self.ids_activitytype_orig[id_inter],
                                                    id_activitytype_dest=self.ids_activitytype_dest[id_inter],
                                                    **kwargs)
        return odflowtab

    def clear_od_trips(self):
        self.clear()

    def add_od_flows(self, t_start, t_end, id_mode,
                     id_activitytype_orig, id_activitytype_dest,
                     scale, names_orig, names_dest, tripnumbers):

        # print 'OdIntervals.add_od_flows',t_start, t_end, id_mode, scale
        ids_inter = self.select_ids(
            (self.times_start.get_value() == t_start)
            & (self.times_end.get_value() == t_end)
            & (self.ids_activitytype_orig.get_value() == id_activitytype_orig)
            & (self.ids_activitytype_dest.get_value() == id_activitytype_dest)
        )

        if len(ids_inter) == 0:
            # no existing interval found. Create a new one
            id_inter = self.add_row(times_start=t_start, times_end=t_end,
                                    ids_activitytype_orig=id_activitytype_orig,
                                    ids_activitytype_dest=id_activitytype_dest,
                                    )
            # print '  create new',id_inter
            #odintervals.add_rows(2, t_start=[0,3600], t_end=[3600, 7200])
            odmodes = OdModes((self.odmodes.attrname, id_inter), parent=self,
                              modes=self.get_net().modes, zones=self.get_zones())
            # NO!! odmodes = OdModes( ('ODMs for modes', id_inter), parent = self, modes = self.get_net().modes, zones = self.get_zones())
            self.odmodes[id_inter] = odmodes

            odtrips = odmodes.add_od_trips(
                id_mode, scale, names_orig, names_dest, tripnumbers)
            return odtrips
        else:

            # there should be only one demand table found for a certain interval
            id_inter = ids_inter[0]
            # print '  use',id_inter
            odtrips = self.odmodes[id_inter].add_od_trips(
                id_mode, scale, names_orig, names_dest, tripnumbers)
            return odtrips

    def add_od_flow(self, t_start, t_end, id_mode,
                    id_activitytype_orig, id_activitytype_dest,
                    scale,
                    name_orig, name_dest, tripnumber):

        # print 'OdIntervals.add_od_flow',t_start, t_end, id_mode, scale, name_orig,name_dest,tripnumber
        odtrips = self.add_od_flows(t_start, t_end, id_mode,
                                    id_activitytype_orig, id_activitytype_dest,
                                    scale,
                                    [name_orig], [name_dest], [tripnumber])

        return odtrips

    def get_net(self):
        return self.net.get_value()

    def get_zones(self):
        return self.zones.get_value()

    def get_modes(self):
        return self.net.get_value().modes

    def get_activitytypes(self):
        return self.parent.activitytypes
