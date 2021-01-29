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

# @file    prt-03-forwarding.py
# @author  Joerg Schweizer
# @date

"""
This plugin provides methods to run and analyze PRT networks.

   
"""
import os
import sys
import numpy as np
#from xml.sax import saxutils, parse, handler


from coremodules.modules_common import *
import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
import agilepy.lib_base.xmlman as xm
#from agilepy.lib_base.misc import get_inversemap
#from agilepy.lib_base.geometry import find_area
#from agilepy.lib_base.processes import Process,CmlMixin,ff,call
from coremodules.network.network import SumoIdsConf
from coremodules.network import routing
from coremodules.simulation import sumo
from coremodules.simulation.sumo import traci

BERTHSTATES = {'free': 0, 'wait': 1, 'boarding': 2, 'alighting': 3}
VEHICLESTATES = {'init': 0, 'wait': 1, 'boarding': 2, 'alighting': 3,
                 'emptytrip': 4, 'occupiedtrip': 5, 'forewarding': 6}

# def detect_entered_left(x,y):
#    """
#    returns the enter and left elemets of list x (before)
#    and list y (after)
#    """
#    if len(x) == 0:
#        if len(y) == 0:
#            return


class PrtBerths(am.ArrayObjman):

    def __init__(self, ident, prtstops, **kwargs):
        # print 'PrtVehicles vtype id_default',vtypes.ids_sumo.get_id_from_index('passenger1')
        self._init_objman(ident=ident,
                          parent=prtstops,
                          name='PRT Berths',
                          info='PRT Berths.',
                          **kwargs)

        self._init_attributes()

    def _init_attributes(self):
        #vtypes = self.get_scenario().demand.vtypes
        net = self.get_scenario().net
        self.add(cm.AttrConf('length_default', 4.0,
                             groupnames=['parameters', 'options'],
                             name='Default length',
                             info='Default berth length.',
                             unit='m',
                             ))

        self.add_col(am.IdsArrayConf('ids_prtstop', self.parent,
                                     name='PRT stop ID',
                                     info='PRT stop ID',
                                     ))

        self.add_col(am.ArrayConf('states', default=BERTHSTATES['free'],
                                  dtype=np.int32,
                                  choices=BERTHSTATES,
                                  name='state',
                                  info='State of berth.',
                                  ))

        self.add_col(am.ArrayConf('stoppositions', default=0.0,
                                  dtype=np.float32,
                                  name='Stop position',
                                  info='Position on edge where vehicle stops',
                                  ))

    def prepare_sim(self, process):
        print 'PrtBerths.prepare_sim'
        ids = self.get_ids()
        self.ids_veh = -1*np.ones(np.max(ids)+1, dtype=np.int32)

    def get_scenario(self):
        return self.parent.get_scenario()

    def get_prtvehicles(self):
        return self.parent.parent.prtvehicles

    def make(self, id_stop, position_from=None, position_to=None,
             n_berth=None,
             offset_firstberth=0.0, offset_stoppos=-0.3):
        stoplength = position_to-position_from
        print 'Berths.make', id_stop, stoplength

        # TODO: let define berth number either explicitely or through stoplength

        length_berth = self.length_default.get_value()
        positions = position_from + offset_firstberth\
            + np.arange(length_berth-offset_firstberth, stoplength+length_berth, length_berth) + offset_stoppos
        n_berth = len(positions)

        # force number of berth to be pair
        if n_berth % 2 == 1:
            positions = positions[1:]
            n_berth -= 1

        ids_berth = self.add_rows(n=n_berth,
                                  stoppositions=positions,
                                  ids_prtstop=id_stop * np.ones(n_berth, dtype=np.int32),
                                  )
        return ids_berth

    def set_prtvehicles(self, prtvehicles):
        """
        Defines attributes which are linked with prtvehicles
        """
        self.add_col(am.IdsArrayConf('ids_veh_allocated', prtvehicles,
                                     name='Alloc. veh ID',
                                     info='ID of  vehicle which have allocated this berth. -1 means no allocation.',
                                     ))


class PrtStops(am.ArrayObjman):
    def __init__(self, ident, prtservices, **kwargs):
        self._init_objman(ident=ident,
                          parent=prtservices,
                          name='Public transport stops',
                          info='Contains information on PRT stops.',
                          #xmltag = ('additional','busStop','stopnames'),
                          version=0.1,
                          **kwargs)
        self._init_attributes()

    def _init_attributes(self):
        self.add(cm.ObjConf(PrtBerths('berths', self)))

        berths = self.get_berths()
        net = self.get_scenario().net

        self.add_col(am.IdsArrayConf('ids_ptstop', net.ptstops,
                                     name='ID PT stop',
                                     info='ID of public transport stop. ',
                                     ))

        self.add_col(am.ArrayConf('are_depot', default=False,
                                  dtype='bool',
                                  perm='rw',
                                  name='Is depot?',
                                  info='If true stop is a depot.',
                                  ))

        self.add_col(am.IdlistsArrayConf('ids_berth_alight', berths,
                                         #groupnames = ['_private'],
                                         name='Alight berth IDs',
                                         info="Alight berth IDs.",
                                         ))

        self.add_col(am.IdlistsArrayConf('ids_berth_board', berths,
                                         #groupnames = ['_private'],
                                         name='Board berth IDs',
                                         info="Board berth IDs.",
                                         ))

        # self.add_col(am.ArrayConf( 'inds_berth_alight_allocated', default = 0,
        #                            #groupnames = ['_private'],
        #                            dtype = np.int32,
        #                            perm = 'rw',
        #                            name = 'Ind aberth lastalloc',
        #                            info = 'Berth index of last allocated berth in alight zone.',
        #                            ))

        # self.add_col(am.ArrayConf( 'inds_berth_board_allocated', default = 0,
        #                            #groupnames = ['_private'],
        #                            dtype = np.int32,
        #                            perm = 'rw',
        #                            name = 'Ind bberth lastalloc',
        #                            info = 'Berth index of last allocated berth in boarding zone.',
        #                            ))

    def get_berths(self):
        return self.berths.get_value()

    def get_scenario(self):
        return self.parent.get_scenario()

    def set_prtvehicles(self, prtvehicles):
        self.get_berths().set_prtvehicles(prtvehicles)

    def prepare_sim(self, process):
        print 'PrtStops.prepare_sim'
        net = self.get_scenario().net
        ptstops = net.ptstops
        berths = self.get_berths()
        lanes = net.lanes
        ids_edge_sumo = net.edges.ids_sumo
        ids = self.get_ids()
        self.ids_stop_to_ids_edge_sumo = np.zeros(np.max(ids)+1, dtype=np.object)
        self.ids_stop_to_ids_edge_sumo[ids] = ids_edge_sumo[lanes.ids_edge[ptstops.ids_lane[self.ids_ptstop[ids]]]]

        self.inds_berth_alight_allocated = -1*np.ones(np.max(ids)+1, dtype=np.int32)
        #self.inds_berth_board_allocated = -1*np.ones(np.max(ids)+1,dtype = np.int32)
        self.ids_vehs_alight_allocated = np.zeros(np.max(ids)+1, dtype=np.object)
        self.ids_vehs_board_allocated = np.zeros(np.max(ids)+1, dtype=np.object)

        self.ids_vehs_sumo_prev = np.zeros(np.max(ids)+1, dtype=np.object)

        self.ids_vehs = np.zeros(np.max(ids)+1, dtype=np.object)
        self.ids_vehs_toallocate = np.zeros(np.max(ids)+1, dtype=np.object)

        for id_stop, id_edge_sumo in zip(ids, self.ids_stop_to_ids_edge_sumo[ids]):
            # set allocation index to last possible berth
            self.inds_berth_alight_allocated[id_stop] = len(self.ids_berth_alight[id_stop])
            #self.inds_berth_board_allocated [id_stop] = len(self.ids_berth_board[id_stop])

            self.ids_vehs_alight_allocated[id_stop] = []
            self.ids_vehs_board_allocated[id_stop] = []

            self.ids_vehs_sumo_prev[id_stop] = set([])
            self.ids_vehs[id_stop] = []
            self.ids_vehs_toallocate[id_stop] = []

        #    traci.edge.subscribe(id_edge_sumo, [traci.constants.VAR_ARRIVED_VEHICLES_IDS])
        berths.prepare_sim(process)

    def process_step(self, process):
        print 'PrtStops.process_step'
        net = self.get_scenario().net
        ptstops = net.ptstops
        berths = self.get_berths()
        lanes = net.lanes
        ids_edge_sumo = net.edges.ids_sumo
        vehicles = self.parent.prtvehicles

        ids = self.get_ids()

        for id_prtstop, id_edge_sumo, ids_veh_sumo_prev in\
            zip(ids, self.ids_stop_to_ids_edge_sumo[ids],
                self.ids_vehs_sumo_prev[ids]):

            print '  process id_prtstop,id_edge_sumo', id_prtstop, id_edge_sumo

            # check for new arrivals
            ids_veh_sumo = set(traci.edge.getLastStepVehicleIDs(id_edge_sumo))
            # print '    ids_veh_sumo_prev=',ids_veh_sumo_prev
            # print '    ids_veh_sumo=',ids_veh_sumo

            if ids_veh_sumo_prev != ids_veh_sumo:
                ids_veh_entered = vehicles.get_ids_from_ids_sumo(list(ids_veh_sumo.difference(ids_veh_sumo_prev)))
                ids_veh_left = vehicles.get_ids_from_ids_sumo(list(ids_veh_sumo_prev.difference(ids_veh_sumo)))
                for id_veh in ids_veh_entered:
                    self.enter(id_prtstop, id_veh)

                for id_veh in ids_veh_left:
                    self.exit(id_prtstop, id_veh)
                self.ids_vehs_sumo_prev[id_prtstop] = ids_veh_sumo
                # print '    ids_veh_sumo_entered',ids_veh_sumo_entered
                # print '    ids_veh_sumo_left',ids_veh_sumo_left

            # check whether allocated vehicles arrived at berth
            ids_veh_remove = []
            for id_veh in self.ids_vehs_alight_allocated[id_prtstop]:
                # TODO: here we could also check vehicle position
                if traci.vehicle.isStopped(vehicles.get_id_sumo(id_veh)):
                    ids_veh_remove.append(id_veh)
                    id_berth_alight = vehicles.ids_berth[id_veh]
                    berths.ids_veh[id_berth_alight] = id_veh
                    berths.states[id_berth_alight] = BERTHSTATES['alighting']
                    vehicles.alight(id_veh)

            for id_veh in ids_veh_remove:
                self.ids_vehs_alight_allocated[id_prtstop].remove(id_veh)

            # check whether we can move vehicles from alighting to
            # boarding berths

            # TODO: here we must check if berth in boarding zone are free
            # AND if they are occupied with empty vehicles, those
            # vehicles need to be kicked out...but only in case
            # new vehicles are waiting to be allocated

            # print '  ids_berth_alight',self.ids_berth_alight[id_prtstop]
            ids_berth_alight = self.ids_berth_alight[id_prtstop][::-1]
            ids_berth_board = self.ids_berth_board[id_prtstop][::-1]
            for id_berth_alight, id_veh, id_berth_board in zip(
                    ids_berth_alight,
                    berths.ids_veh[ids_berth_alight],
                    ids_berth_board):

                # print '    check allocated veh',id_veh
                # TODO: this could go into one vehicle method?
                if id_veh >= 0:  # is there is a waiting vehicle
                    if vehicles.is_completed_alight(id_veh):
                        print '     send vehicle to id_berth_board', id_berth_board, berths.stoppositions[id_berth_board]
                        berths.ids_veh[id_berth_alight] = -1
                        berths.states[id_berth_alight] = BERTHSTATES['free']
                        vehicles.control_stop_board(id_veh, id_prtstop, id_berth_board,
                                                    id_edge_sumo=self.ids_stop_to_ids_edge_sumo[id_prtstop],
                                                    position=berths.stoppositions[id_berth_board],
                                                    )
                        self.ids_vehs_board_allocated[id_prtstop].append(id_veh)
            # if all allocated vehicles found their berth and all berths are free, then
            # reset  allocation index
            if len(self.ids_vehs_alight_allocated[id_prtstop]) == 0:
                if np.all(berths.states[ids_berth_alight] == BERTHSTATES['free']):
                    # reset index
                    self.inds_berth_alight_allocated[id_prtstop] = len(self.ids_berth_alight[id_prtstop])

                    # try to allocate unallocated vehicles
                    ids_veh_remove = []
                    for id_veh in self.ids_vehs_toallocate[id_prtstop]:
                        id_berth = self.allocate_alight(id_prtstop)
                        if id_berth < 0:
                            # allocation failed
                            # do nothing, vehicle continues to wait for allocation
                            pass
                        else:
                            # command vehicle to go to berth for alighting

                            self.parent.prtvehicles.control_stop_alight(id_veh, id_prtstop, id_berth,
                                                                        id_edge_sumo=self.ids_stop_to_ids_edge_sumo[id_prtstop],
                                                                        position=self.get_berths(
                                                                        ).stoppositions[id_berth],
                                                                        )
                            self.ids_vehs_alight_allocated[id_prtstop].append(id_veh)
                    for id_veh in ids_veh_remove:
                        self.ids_vehs_toallocate[id_prtstop].remove(id_veh)

            # read state of all vehicles in stop
            # vehicles.update_states(self.ids_vehs[id_prtstop])

        # check if vehicles need to be forwarded
        #ids_stop_forward = np.flatnonzero(self.inds_berth_alight_allocated == 0)
        # for id_prtstop in ids_stop_forward:
        #    self.forward(id_prtstop)

        # print '         arrived=',traci.edge.getSubscriptionResults()

    def enter(self, id_stop, id_veh):
        # print 'enter id_stop, id_veh',id_stop, id_veh
        self.ids_vehs[id_stop].append(id_veh)
        id_berth = self.allocate_alight(id_stop)
        if id_berth < 0:
            # allocation failed
            # command vehicle to slow down and wait for allocation
            self.ids_vehs_toallocate[id_stop].append(id_veh)
            self.parent.prtvehicles.control_slow_down(id_veh)
        else:
            # command vehicle to go to berth for alighting
            self.parent.prtvehicles.control_stop_alight(id_veh, id_stop, id_berth,
                                                        id_edge_sumo=self.ids_stop_to_ids_edge_sumo[id_stop],
                                                        position=self.get_berths().stoppositions[id_berth],
                                                        )
            self.ids_vehs_alight_allocated[id_stop].append(id_veh)

    def exit(self, id_stop, id_veh):
        self.ids_vehs[id_stop].remove(id_veh)

    def allocate_alight(self, id_stop):
        # print 'allocate_alight',id_stop, id_veh_sumo
        #self.inds_berth_alight_allocated [id_stop] = len(self.ids_berth_alight[id_stop])
        ind_berth = self.inds_berth_alight_allocated[id_stop]

        if ind_berth == 0:
            # no free berth :(
            return -1
        else:
            ind_berth -= 1
            self.inds_berth_alight_allocated[id_stop] = ind_berth
            return self.ids_berth_alight[id_stop][ind_berth]

    def make_from_net(self, mode='custom1'):
        """
        Make prt stop database from PT stops in network.
        """
        self.clear()
        net = self.get_scenario().net
        ptstops = net.ptstops
        lanes = net.lanes
        ids_ptstop = ptstops.get_ids()
        id_mode_prt = net.modes.get_id_mode(mode)
        ids_lane = ptstops.ids_lane[ids_ptstop]
        #edgelengths = net.edges.lengths

        for id_stop, modes_allow, position_from, position_to in zip(
            ids_ptstop,
            lanes.modes_allow[ids_lane],
            ptstops.positions_from[ids_ptstop],
            ptstops.positions_to[ids_ptstop],
        ):
            if id_mode_prt in modes_allow:
                self.make(id_stop,
                          position_from,
                          position_to)

    def make(self, id_ptstop, position_from, position_to):
        """
        Initialize a new prt stop and generate berth.
        """
        id_stop = self.add_row(ids_ptstop=id_ptstop)
        ids_berth = self.get_berths().make(id_stop, position_from=position_from,
                                           position_to=position_to)
        n_berth = len(ids_berth)
        n_berth_alight = int(0.5*n_berth)
        n_berth_board = n_berth-n_berth_alight
        self.ids_berth_alight[id_stop] = ids_berth[0:n_berth_alight]
        self.ids_berth_board[id_stop] = ids_berth[n_berth_alight:n_berth]
        return id_stop


class PrtVehicles(am.ArrayObjman):

    def __init__(self, ident, prtservices, **kwargs):
        # print 'PrtVehicles vtype id_default',vtypes.ids_sumo.get_id_from_index('passenger1')
        self._init_objman(ident=ident,
                          parent=prtservices,
                          name='PRT Veh.',
                          info='PRT vehicle database. These are shared vehicles.',
                          **kwargs)

        self._init_attributes()

    def _init_attributes(self):
        vtypes = self.get_scenario().demand.vtypes
        net = self.get_scenario().net

        # TODO: add/update vtypes here
        self.add_col(SumoIdsConf('Veh', xmltag='id'))

        self.add_col(am.IdsArrayConf('ids_vtype', vtypes,
                                     id_default=vtypes.ids_sumo.get_id_from_index('PRT'),
                                     groupnames=['state'],
                                     name='Veh. type',
                                     info='PRT vehicle type.',
                                     #xmltag = 'type',
                                     ))

        self.add_col(am.ArrayConf('states', default=VEHICLESTATES['init'],
                                  dtype=np.int32,
                                  choices=VEHICLESTATES,
                                  name='state',
                                  info='State of vehicle.',
                                  ))

        self.add_col(am.IdsArrayConf('ids_targetprtstop', self.parent.prtstops,
                                     groupnames=['parameters'],
                                     name='Target stop ID',
                                     info='ID of current target PRT stop.',
                                     ))

        self.add_col(am.IdsArrayConf('ids_currentedge', net.edges,
                                     groupnames=['state'],
                                     name='Current edge ID',
                                     info='Edge ID of most recent reported position.',
                                     ))

        self.add_col(am.IdsArrayConf('ids_targetedge', net.edges,
                                     groupnames=['state'],
                                     name='Target edge ID',
                                     info='Target edge ID to be reached. This can be either intermediate target edges (), such as a compressor station.',
                                     ))

    def prepare_sim(self, process):
        print 'PrtVehicles.prepare_sim'
        net = self.get_scenario().net
        ptstops = net.ptstops
        lanes = net.lanes
        ids_edge_sumo = net.edges.ids_sumo
        ids = self.get_ids()

        self.ids_berth = -1*np.ones(np.max(ids)+1, dtype=np.int32)

        # for id_veh, id_veh_sumo in zip(ids,self.ids_sumo[ids]):
        #    traci.vehicle.subscribe(id_veh_sumo,
        #                            [   traci.constants.VAR_ROAD_ID,
        #                                traci.constants.VAR_POSITION,
        #                                traci.constants.VAR_STOPSTATE,
        #                                ])

    def process_step(self, process):
        # print 'process_step',traci.vehicle.getSubscriptionResults()
        pass
        # VEHICLESTATES = {'init':0,'wait':1,'boarding':2,'alighting':3,'emptytrip':4,'occupiedtrip':5,'forewarding':6}

    def control_slow_down(self, id_veh, speed=1.0, time_slowdown=3):
        #traci.vehicle.slowDown(self.get_id_sumo(id_veh), speed, time_slowdown)
        pass

    def control_stop_alight(self, id_veh, id_stop, id_berth,
                            id_edge_sumo=None,
                            position=None,
                            ):
        print 'control_stop_alight', self.get_id_sumo(id_veh), traci.vehicle.getLanePosition(
            self.get_id_sumo(id_veh)), '->', position, id_berth
        self.ids_berth[id_veh] = id_berth
        traci.vehicle.setStop(self.get_id_sumo(id_veh),
                              id_edge_sumo,
                              pos=position,
                              flags=0,
                              laneIndex=1,
                              )

    def control_stop_board(self, id_veh, id_stop, id_berth,
                           id_edge_sumo=None,
                           position=None,
                           ):
        id_veh_sumo = self.get_id_sumo(id_veh)
        print 'control_stop_board', id_veh_sumo, traci.vehicle.getLanePosition(id_veh_sumo), '->', position, id_berth
        self.ids_berth[id_veh] = id_berth

        traci.vehicle.resume(id_veh_sumo)

        traci.vehicle.setStop(id_veh_sumo,
                              id_edge_sumo,
                              pos=position,
                              flags=0,
                              laneIndex=1,
                              )

    def alight(self, id_veh):
        # TODO: necessary to keep copy of state?
        self.states[id_veh] = VEHICLESTATES['alighting']
        # traci.vehicle.getStopState(self.get_id_sumo(id_veh))
        # VEHICLESTATES = {'init':0,'wait':1,'boarding':2,'alighting'

    def is_completed_alight(self, id_veh):
        print 'is_completed_alight', self.get_id_sumo(id_veh), self.states[id_veh], self.states[id_veh] == VEHICLESTATES['alighting'], traci.vehicle.getPersonNumber(
            self.get_id_sumo(id_veh)), type(traci.vehicle.getPersonNumber(self.get_id_sumo(id_veh)))
        if self.states[id_veh] == VEHICLESTATES['alighting']:
            if traci.vehicle.getPersonNumber(self.get_id_sumo(id_veh)) == 0:
                self.states[id_veh] = VEHICLESTATES['wait']
                return True

        else:
            return False

    def update_states(self, ids_veh):
        print 'update_states', ids_veh
        for id_veh in ids_veh:
            # getStopState
            #    returns information in regard to stopping:
            #    The returned integer is defined as 1 * stopped + 2 * parking
            #    + 4 * personTriggered + 8 * containerTriggered + 16 * isBusStop
            #    + 32 * isContainerStop

            #isStopped, isStoppedTriggered

            #
            binstate = traci.vehicle.getStopState(self.get_id_sumo(id_veh))
            print '  id_veh,binstate=', id_veh, bin(binstate)[2:]

    def make(self, n=-1, length_veh_av=4.0):
        """
        Make n PRT vehicles
        If n = -1 then fill up stops with vehicles.
        """
        print 'PrtVehicles.make', n, length_veh_av
        self.clear()
        net = self.get_scenario().net
        ptstops = net.ptstops
        prtstops = self.parent.prtstops
        lanes = net.lanes
        ids_prtstop = prtstops.get_ids()
        ids_ptstop = prtstops.ids_ptstop[ids_prtstop]
        ids_veh = []
        for id_prt, id_edge, pos_from, pos_to in zip(
            ids_prtstop,
            lanes.ids_edge[ptstops.ids_lane[ids_ptstop]],
            ptstops.positions_from[ids_ptstop],
            ptstops.positions_to[ids_ptstop],
        ):
            # TODO: here we can select depos or distribute a
            # fixed number of vehicles or put them into berth
            # print '  ',pos_to,pos_from,int((pos_to-pos_from)/length_veh_av)

            for i in range(int((pos_to-pos_from)/length_veh_av)):
                id_veh = self.add_row(ids_targetstop=id_prt,
                                      ids_currentedge=id_edge,
                                      )

                self.ids_sumo[id_veh] = self.get_id_sumo(id_veh)
                ids_veh.append(id_veh)

        return ids_veh

    # def write_veh
    #

    def get_scenario(self):
        return self.parent.get_scenario()

    def get_vtypes(self):
        """
        Returns a set with all used PRT vehicle types.
        """
        # print 'Vehicles_individual.get_vtypes',self.cols.vtype
        return set(self.ids_vtype.get_value())

    def get_id_sumo(self, id_veh):
        return 'prt.%s' % (id_veh)

    def get_id_from_id_sumo(self, id_veh_sumo):
        prefix, idstr = id_veh_sumo.split('.')
        return int(idstr)

    def get_ids_from_ids_sumo(self, ids_veh_sumo):
        n = len(ids_veh_sumo)
        ids = np.zeros(n, np.int32)
        for i in xrange(n):
            ids[i] = self.get_id_from_id_sumo(ids_veh_sumo[i])
        return ids

    def get_id_line_xml(self):
        return 'prt'


class PrtService(cm.BaseObjman):
    def __init__(self, demand=None,
                 name='PRT service', info='PRT service',
                 **kwargs):
            # print 'PrtService.__init__',name

        self._init_objman(ident='prtservices', parent=demand,
                          name=name, info=info, **kwargs)
        attrsman = self.set_attrsman(cm.Attrsman(self))

        self._init_attributes()
        self._init_constants()

    def _init_attributes(self):

        attrsman = self.get_attrsman()
        #self.virtualpop = attrsman.add( cm.ObjConf( virtualpop, is_child = False,groups = ['_private']))
        if hasattr(self, 'prtvehicles'):
            self.delete('prtvehicles')

        self.prtstops = attrsman.add(cm.ObjConf(PrtStops('prtstops', self)))
        self.prtvehicles = attrsman.add(cm.ObjConf(PrtVehicles('prtvehicles', self)))

        self.prtstops.set_prtvehicles(self.prtvehicles)

    def _init_constants(self):
        # print 'PrtService._init_constants',self,self.parent
        # this will ensure that PRT vtypes will be exported to routes
        self.parent.add_demandobject(self)

    def get_vtypes(self):

        ids_vtypes = set(self.prtvehicles.ids_vtype.get_value())
        return ids_vtypes

    def get_writexmlinfo(self, is_route=False):
        """
        Returns three array where the first array is the 
        begin time of the first vehicle and the second array is the
        write function to be called for the respectice vehicle and
        the third array contains the vehicle ids

        Method used to sort trips when exporting to route or trip xml file
        """
        print 'PRT.get_writexmlinfo'
        # time betwenn insertion of consecutive vehicles at same stop
        t_delta = 10  # s

        n_veh = len(self.prtvehicles)
        times_depart = np.zeros(n_veh, dtype=np.int32)
        writefuncs = np.zeros(n_veh, dtype=np.object)
        writefuncs[:] = self.write_prtvehicle_xml
        ids_veh = self.prtvehicles.get_ids()

        t_start = 0.0
        id_edge_prev = -1
        i = 0
        for id_edge in self.prtvehicles.ids_currentedge[ids_veh]:
            print '  id_edge, t_start, id_edge_prev', id_edge, t_start, id_edge_prev
            times_depart[i] = t_start
            t_start += t_delta
            if id_edge != id_edge_prev:
                t_start = 0.0
                id_edge_prev = 1*id_edge
            i += 1

        return times_depart, writefuncs, ids_veh

    def write_prtvehicle_xml(self,  fd, id_veh, time_begin, indent=2):
        print 'write_prtvehicle_xml', id_veh, time_begin
        # TODO: actually this should go in prtvehicles
        #time_veh_wait_after_stop = 3600
        net = self.get_scenario().net
        #lanes = net.lanes
        edges = net.edges
        #ind_ride = rides.get_inds(id_stage)
        #id_veh = rides.ids_veh[id_stage]
        prtvehicles = self.prtvehicles
        #ptstops = net.ptstops
        #prtstops = self.parent.prtstops
        #ids_prtstop = prtstops.get_ids()
        #ids_ptstop = prtstops.ids_ptstop[id_prtstop]
        # lanes.ids_edge[ptstops.ids_lane[ids_ptstop]],
        #id_lane_from = parking.ids_lane[id_parking_from]
        #laneindex_from =  lanes.indexes[id_lane_from]
        #pos_from = parking.positions[id_parking_from]

        #id_parking_to = rides.ids_parking_to[id_stage]
        #id_lane_to = parking.ids_lane[id_parking_to]
        #laneindex_to =  lanes.indexes[id_lane_to]
        #pos_to = parking.positions[id_parking_to]

        # write unique veh ID to prevent confusion with other veh declarations
        fd.write(xm.start('vehicle id="%s"' % prtvehicles.get_id_sumo(id_veh), indent+2))

        fd.write(xm.num('depart', '%d' % time_begin))
        fd.write(xm.num('type', self.parent.vtypes.ids_sumo[prtvehicles.ids_vtype[id_veh]]))
        fd.write(xm.num('line', prtvehicles.get_id_line_xml()))
        fd.write(xm.stop())

        # write route
        fd.write(xm.start('route', indent+4))
        # print '  edgeindex[ids_edge]',edgeindex[ids_edge]
        fd.write(xm.arr('edges', [edges.ids_sumo[prtvehicles.ids_currentedge[id_veh]]]))

        # does not seem to have an effect, always starts at base????
        fd.write(xm.num('departPos', 'base'))
        #fd.write(xm.num('departLane', laneindex_from ))
        fd.write(xm.stopit())

        # write depart stop
        # fd.write(xm.start('stop',indent+4))
        #fd.write(xm.num('lane', edges.ids_sumo[lanes.ids_edge[id_lane_from]]+'_%d'%laneindex_from ))
        #fd.write(xm.num('duration', time_veh_wait_after_stop))
        #fd.write(xm.num('startPos', pos_from ))
        #fd.write(xm.num('endPos', pos_from + parking.lengths[id_parking_from]))
        #fd.write(xm.num('triggered', "True"))
        # fd.write(xm.stopit())

        fd.write(xm.end('vehicle', indent+2))

    def make_stops_and_vehicles(self, n_veh=-1):
        self.prtstops.make_from_net()
        self.prtvehicles.make(n_veh)

    def prepare_sim(self, process):
        self.prtvehicles.prepare_sim(process)
        self.prtstops.prepare_sim(process)

    def process_step(self, process):
        self.prtvehicles.process_step(process)
        self.prtstops.process_step(process)

    def update(self):
        pass
        # person
        # http://www.sumo.dlr.de/daily/pydoc/traci._person.html
        # interesting
        #getPosition3D(self, personID)

        # get edge id
        # getRoadID(self, personID)

        # next edge or empty string at end of stage
        #getNextEdge(self, personID)

        # getLanePosition(self, personID)

        # getWaitingTime(self, personID)

        # vehicle
        # http://www.sumo.dlr.de/daily/pydoc/traci._vehicle.html

        # interesting:

        # The vehicle's destination edge is set to the given edge id. The route is rebuilt.
        #changeTarget(id_sumo_veh, id_sumo_edge)

        # getRoute
        # getRoute(string) -> list(string)
        # returns the ids of the edges the vehicle's route is made of.

        #rerouteTraveltime(self, vehID, currentTravelTimes=True)

        # getStopState
        #        getStopState(string) -> integer

        # or
        #'isAtBusStop', 'isAtContainerStop', 'isRouteValid', 'isStopped', 'isStoppedParking', 'isStoppedTriggered',

        # setBusStop(self, vehID, stopID, duration=2147483647, until=-1, flags=0)

        #setStop(self, vehID, edgeID, pos=1.0, laneIndex=0, duration=2147483647, flags=0, startPos=-1001.0, until=-1)

        # to make it a follower or leader
        #setType(self, vehID, typeID)

        # to reroute over compressors.....reroute after setting via!!
        # setVia(self, vehID, edgeList)

        # slow down in stops
        # slowDown(self, vehID, speed, duration)

        #traci.vehicle.setSpeed(id_veh_last, v_parking)
        #traci.vehicle.setRoute(id_veh_last, route_return)
        #traci.vehicle.setStop(id_veh_last, edgeid_return, pos= edgelength_return-20.0, laneIndex=0, duration = 15000)

        # edge
        #getLastStepPersonIDs(self, edgeID)
        # getLastStepPersonIDs(string) -> list(string)

        # Returns the ids of the persons on the given edge during the last time step.

        #getLastStepVehicleIDs(self, edgeID)
        # getLastStepVehicleIDs(string) -> list(string)

    def get_scenario(self):
        return self.parent.parent

    def get_landuse(self):
        return self.get_scenario().landuse

    def make_plans_prt(ids_person=None, mode='custom1'):
        # routing necessary?
        landuse = self.get_landuse()
        facilities = landuse.facilities

        scenario = self.parent.get_scenario()
        edges = scenario.net.edges
        lanes = scenario.net.lanes
        modes = scenario.net.modes

        virtualpop = self.parent.virtualpop
        walks = virtualpop.get_walks()
        transits = virtualpop.get_transits()

        #ptstops = virtualpop.get_ptstops()
        #ptlines = virtualpop.get_ptlines()

        #ptlinks = ptlines.ptlinks.get_value()
        #ptlinktypes = ptlinks.types.choices
        #type_enter = ptlinktypes['enter']
        #type_transit = ptlinktypes['transit']
        #type_board = ptlinktypes['board']
        #type_alight = ptlinktypes['alight']
        #type_transfer = ptlinktypes['transfer']
        #type_walk = ptlinktypes['walk']
        #type_exit = ptlinktypes['exit']

        ptfstar = ptlinks.get_fstar()
        pttimes = ptlinks.get_times()
        stops_to_enter, stops_to_exit = ptlinks.get_stops_to_enter_exit()

        ids_stoplane = ptstops.ids_lane
        ids_laneedge = scenario.net.lanes.ids_edge

        times_est_plan = self.plans.get_value().times_est
        # here we can determine edge weights for different modes
        walks.prepare_planning()
        transits.prepare_planning()

        if ids_person is None:
            # print '  ids_mode_preferred',self.ids_mode_preferred.value
            # print '  private',MODES['private']
            # print '  ',self.ids_mode_preferred == MODES['private']

            ids_person = self.select_ids(
                self.ids_mode_preferred.get_value() == modes.get_id_mode(mode),
            )

        ids_plan = self.add_plans(ids_person)

        n_plans = len(ids_plan)

        print 'make_plans_pt n_plans=', n_plans

        #ids_veh = self.get_individualvehicles().assign_to_persons(ids_person)
        inds_pers = self.get_inds(ids_person)
        # self.persons.cols.mode_preferred[inds_pers]='private'

        times_start = self.times_start.value[inds_pers]
        inds_fac_home = facilities.get_inds(self.ids_fac_home.value[inds_pers])
        inds_fac_activity = facilities.get_inds(self.ids_fac_activity.value[inds_pers])

        centroids_home = facilities.centroids.value[inds_fac_home]
        centroids_activity = facilities.centroids.value[inds_fac_activity]

        ids_edge_home = facilities.ids_roadedge_closest.value[inds_fac_home]
        poss_edge_home = facilities.positions_roadedge_closest.value[inds_fac_home]

        ids_edge_activity = facilities.ids_roadedge_closest.value[inds_fac_activity]
        poss_edge_activity = facilities.positions_roadedge_closest.value[inds_fac_activity]

        ids_stop_home = ptstops.get_closest(centroids_home)
        ids_stop_activity = ptstops.get_closest(centroids_activity)

        ids_stopedge_home = ids_laneedge[ids_stoplane[ids_stop_home]]
        ids_stopedge_activity = ids_laneedge[ids_stoplane[ids_stop_activity]]

        poss_stop_home = 0.5*(ptstops.positions_from[ids_stop_home]
                              + ptstops.positions_to[ids_stop_home])
        poss_stop_activity = 0.5*(ptstops.positions_from[ids_stop_activity]
                                  + ptstops.positions_to[ids_stop_activity])

        i = 0
        for id_person, id_plan, time_start, id_edge_home, pos_edge_home, id_edge_activity, pos_edge_activity, id_stop_home, id_stopedge_home, pos_stop_home, id_stop_activity, id_stopedge_activity, pos_stop_activity\
                in zip(ids_person, ids_plan, times_start,  ids_edge_home, poss_edge_home, ids_edge_activity, poss_edge_activity, ids_stop_home, ids_stopedge_home, poss_stop_home, ids_stop_activity, ids_stopedge_activity, poss_stop_activity):
            self.plans.value.set_row(id_plan, ids_person=id_person)

            print 79*'_'
            print '  id_plan=%d, id_person=%d, ' % (id_plan, id_person)

            id_stage_walk1, time = walks.append_stage(id_plan, time_start,
                                                      id_edge_from=id_edge_home,
                                                      position_edge_from=pos_edge_home,
                                                      id_edge_to=id_stopedge_home,
                                                      position_edge_to=pos_stop_home,  # -7.0,
                                                      )

            # print '    id_stopedge_home',id_stopedge_home
            # print '    pos_stop_home',pos_stop_home

            # print
            # print '    id_stopedge_activity',id_stopedge_activity
            # print '    pos_stop_activity',pos_stop_activity
            # print
            # print '    id_stop_home',id_stop_home
            # print '    id_stop_activity',id_stop_activity

            durations, linktypes, ids_line, ids_fromstop, ids_tostop =\
                ptlinks.route(id_stop_home, id_stop_activity,
                              fstar=ptfstar, times=pttimes,
                              stops_to_enter=stops_to_enter,
                              stops_to_exit=stops_to_exit)

            # print '  routing done. make plan..'

            if len(linktypes) > 0:
                if linktypes[-1] == type_walk:  # is last stage a walk?
                        # remove it, because will go directly to destination
                    linktypes = linktypes[:-1]
                    ids_line = ids_line[:-1]
                    durations = durations[:-1]
                    ids_fromstop = ids_fromstop[:-1]
                    ids_tostop = ids_tostop[:-1]

            # print '  ids_line    ',ids_line
            # print '  ids_fromstop',ids_fromstop
            # print '  ids_tostop  ',ids_tostop

            if len(linktypes) > 0:  # is there any public transport line to take?

                # go though PT links and generate transits and walks to trasfer
                ids_stopedge_from = ids_laneedge[ids_stoplane[ids_fromstop]]
                ids_stopedge_to = ids_laneedge[ids_stoplane[ids_tostop]]
                poss_stop_from = 0.5*(ptstops.positions_from[ids_fromstop]
                                      + ptstops.positions_to[ids_fromstop])
                poss_stop_to = 0.5*(ptstops.positions_from[ids_tostop]
                                    + ptstops.positions_to[ids_tostop])

                # this is wait time buffer to be added to the successive stage
                # as waiting is currently not modelled as an extra stage
                duration_wait = 0.0

                # create stages for PT
                for linktype, id_line, duration,\
                    id_stopedge_from, pos_fromstop,\
                    id_stopedge_to, pos_tostop in\
                        zip(linktypes,
                            ids_line,
                            durations,
                            ids_stopedge_from, poss_stop_from,
                            ids_stopedge_to, poss_stop_to,
                            ):
                    # print '    stage for linktype %2d fromedge %s toedge %s'%(linktype, edges.ids_sumo[id_stopedge_from],edges.ids_sumo[id_stopedge_to] )

                    if linktype == type_transit:  # transit!
                        id_stage_transit, time = transits.append_stage(
                            id_plan, time,
                            id_line=id_line,
                            duration=duration+duration_wait,
                            id_fromedge=id_stopedge_from,
                            id_toedge=id_stopedge_to,
                        )
                        duration_wait = 0.0

                    elif linktype == type_walk:  # walk to transfer

                        id_stage_transfer, time = walks.append_stage(
                            id_plan, time,
                            id_edge_from=id_stopedge_from,
                            position_edge_from=pos_fromstop,
                            id_edge_to=id_stopedge_to,
                            position_edge_to=pos_tostop,
                            duration=duration+duration_wait,
                        )
                        duration_wait = 0.0

                    else:  # all other link time are no modelld
                        # do not do anything , just add wait time to next stage
                        duration_wait += duration

                # walk from final stop to activity
                # print '    Stage for linktype %2d fromedge %s toedge %s'%(linktype, edges.ids_sumo[id_stopedge_to],edges.ids_sumo[id_edge_activity] )
                id_stage_walk2, time = walks.append_stage(id_plan, time,
                                                          id_edge_from=id_stopedge_to,
                                                          position_edge_from=pos_tostop,
                                                          id_edge_to=id_edge_activity,
                                                          position_edge_to=pos_edge_activity,
                                                          )

            else:
                # there is no public transport line linking these nodes.
                # Modify walk directly from home to activity
                time = walks.modify_stage(id_stage_walk1, time_start,
                                          id_edge_from=id_edge_home,
                                          position_edge_from=pos_edge_home,
                                          id_edge_to=id_edge_activity,
                                          position_edge_to=pos_edge_activity,
                                          )

            # store time estimation for this plan
            times_est_plan[id_plan] = time-time_start


class SumoPrt(sumo.SumoTraci):
    """
    SUMO simulation process with interactive control of PRT vehicles.
    """

    def _init_special(self, **kwargs):
        """
        Special initializations. To be overridden.
        """
        # prtservices = None
        self._prtservice = kwargs['prtservice']

    def run_cml(self, cml):
        cmllist = cml.split(' ')
        print 'PRT.run_cml', cmllist
        traci.start(cmllist)
        self.simtime = self.simtime_start
        self.duration = 1.0+self.simtime_end-self.simtime_start
        self.get_attrsman().status.set('running')
        print '  traci started', self.get_attrsman().status.get()

        self._prtservice.prepare_sim(self)

        return True

    def process_step(self):
        # print traci.vehicle.getSubscriptionResults(vehID)
        print 'process_step time=', self.simtime
        self._prtservice.process_step(self)
