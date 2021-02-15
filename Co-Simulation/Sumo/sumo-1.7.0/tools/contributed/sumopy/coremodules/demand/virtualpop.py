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

# @file    virtualpop.py
# @author  Joerg Schweizer
# @date

import numpy as np
from numpy import random
import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
import agilepy.lib_base.xmlman as xm
from agilepy.lib_base.misc import random_choice, get_inversemap
from agilepy.lib_base.processes import Process

# from coremodules.modules_common import *
from coremodules.network.network import SumoIdsConf, MODES
from coremodules.network import routing
from coremodules.simulation import results as res
from coremodules.demand.demandbase import *
import virtualpop_results as res


GENDERS = {'male': 0, 'female': 1, 'unknown': -1}

OCCUPATIONS = {'unknown': -1,
               'worker': 1,
               'student': 2,
               'employee': 3,
               'public employee': 4,
               'selfemployed': 5,
               'pensioneer': 6,
               'other': 7
               }


class Activities(am.ArrayObjman):
    # http://www.sumo.dlr.de/userdoc/Networks/Building_Networks_from_own_XML-descriptions.html#Edge_Descriptions
    def __init__(self, ident, virtualpop, **kwargs):

        self._init_objman(ident=ident, parent=virtualpop, name='Activities',
                          info='Activity database of persons contains type, time, duration and location of activities.',
                          version=0.0,
                          **kwargs)

        self._init_attributes()

    def _init_attributes(self):

        # activy types now in demand
        activitytypes = self.parent.get_demand().activitytypes
        self.add_col(am.IdsArrayConf('ids_activitytype', activitytypes,
                                     groupnames=['parameters'],
                                     choices=activitytypes.names.get_indexmap(),
                                     name='Type',
                                     info='Type of activity performed during the stop.',
                                     #xmltag = 'actType',
                                     #xmlmap = get_inversemap( activitytypes.names.get_indexmap()),
                                     ))

        # attention, this may cause trouble durung init if
        # facilities are not yet initialized
        self.add_col(am.IdsArrayConf('ids_facility', self.parent.get_scenario().landuse.facilities,
                                     groupnames=['parameters'],
                                     name='ID fac.',
                                     info='Facility ID where activity takes place.',
                                     #activitytype = 'home',
                                     ))

        # self.add_col(am.ArrayConf( 'descriptions', '',
        #                            dtype = np.object,
        #                            perm='rw',
        #                            is_index = True,
        #                            name = 'Description',
        #                            info = 'Description of activity.',
        #                            ))

        # self.add_col(am.IdlistsArrayConf( 'ids_landusetypes', self.parent.get_landuse().landusetypes,
        #                                name = 'Landuse types',
        #                                info = "Landuse type IDs, eher this activity type can take place.",
        #                                ))

        self.add_col(am.ArrayConf('hours_begin_earliest', 0.0,
                                  dtype=np.float32,
                                  groupnames=['parameters'],
                                  perm='rw',
                                  name='Earliest hour begin',
                                  unit='h',
                                  info='Earliest hour when this activity can begin.',
                                  ))

        self.add_col(am.ArrayConf('hours_begin_latest', 1.0,
                                  dtype=np.float32,
                                  groupnames=['parameters'],
                                  perm='rw',
                                  name='Latest begin hour',
                                  unit='h',
                                  info='Latest hour when this activity can begin.',
                                  ))

        self.add_col(am.ArrayConf('durations_min', 6.0,
                                  dtype=np.float32,
                                  groupnames=['parameters'],
                                  perm='rw',
                                  name='Min. Duration',
                                  unit='h',
                                  info='Minimum activity duration for a person within a day.',
                                  ))

        self.add_col(am.ArrayConf('durations_max', 8.0,
                                  dtype=np.float32,
                                  groupnames=['parameters'],
                                  perm='rw',
                                  name='Max. Duration',
                                  unit='h',
                                  info='Maximum activity duration for a person within a day.',
                                  ))

    def get_hours_end_earliest(self, ids):
        return self.hours_begin_earliest[ids]+self.durations_min[ids]

    def get_hours_end_latest(self, ids):
        return self.hours_begin_latest[ids]+self.durations_max[ids]

    def get_durations(self, ids, pdf='unit'):
        durations = np.zeros(len(ids), dtype=np.float32)
        i = 0
        for time_start, time_end in zip(
                np.array(self.durations_min[ids]*3600, dtype=np.int32),
                np.array(self.durations_max[ids]*3600, dtype=np.int32)):

            durations[i] = np.random.randint(time_start, time_end, 1)
            i += 1
        return durations

    def get_times_end(self, ids, pdf='unit'):
        """
        Returns an array with activity ending time for the
        given activity IDs. 
        The ending time is calculated by drawing random samples
        from the departure interval.
        The random samples are drawn according to the given probability
        density function, pdf.

        Input arguments:
            ids: integer array with activity IDs
            pdf: probability density function 'unit'|'normal'

        Returned arguments:
            times_end: integer array with departure times
        """
        times_end = np.zeros(len(ids), dtype=np.float32)
        i = 0
        for time_start, time_end in zip(
                np.array(self.get_hours_end_earliest(ids)*3600, dtype=np.int32),
                np.array(self.get_hours_end_latest(ids)*3600, dtype=np.int32)):

            times_end[i] = np.random.randint(time_start, time_end, 1)
            i += 1

        return times_end

    def get_times_begin(self, ids, pdf='unit'):
        """
        Returns an array with beginning time for the
        given activity IDs. 
        The begin time is calculated by drawing random samples
        from the departure interval.
        The random samples are drawn according to the given probability
        density function, pdf.

        Input arguments:
            ids: integer array with activity IDs
            pdf: probability density function 'unit'|'normal'

        Returned arguments:
            times_begin: integer array with departure times
        """
        times_begin = np.zeros(len(ids), dtype=np.float32)
        i = 0
        for time_start, time_end in zip(
                np.array(self.get_hours_begin_earliest(ids)*3600, dtype=np.int32),
                np.array(self.get_hours_begin_latest(ids)*3600, dtype=np.int32)):

            times_begin[i] = np.random.randint(time_start, time_end, 1)
            i += 1

        return times_begin


class IndividualAutos(am.ArrayObjman):

    def __init__(self, ident, population, **kwargs):
        # print 'individualvehicle vtype id_default',vtypes.ids_sumo.get_id_from_index('passenger1')
        self._init_objman(ident=ident,
                          parent=population,
                          name='Indiv. Autos',
                          info='Individual auto database. These are privately owned autos.',
                          **kwargs)

        self._init_attributes()
        self._init_constants()

    def _init_constants(self):

        self.do_not_save_attrs(['mode', 'mode_prefix',
                                '_edges', '_lanes', '_individualvehicle', '_ids_vtype_sumo', '_ids_edge_sumo',
                                '_id_mode', '_get_laneid_allowed', '_get_sumoinfo_from_id_lane',
                                '_space_access', '_parking', '_time_after_unboarding',
                                ])

    def def_mode(self):
        self.mode = 'passenger'
        self.mode_prefix = 'iauto'

    def _init_attributes(self):
        vtypes = self.parent.get_demand().vtypes
        self.def_mode()

        ids_vtype = vtypes.select_by_mode(mode=self.mode)

        self.add(cm.AttrConf('space_access', 0.5,
                             groupnames=['options'],
                             perm='rw',
                             name='Space access',
                             unit='m',
                             info='Space to access vehicles at parkings. This is typically less than the vehicle length.',
                             ))

        self.add(cm.AttrConf('time_after_unboarding', 5,
                             groupnames=['options'],
                             perm='rw',
                             name='time after unboarding',
                             unit='s',
                             info='Time the vehicle waits before disappearing after unboarding.',
                             ))

        self.add_col(am.IdsArrayConf('ids_vtype', vtypes,
                                     id_default=ids_vtype[0],
                                     groupnames=['state'],
                                     name='Veh. type',
                                     info='Vehicle type.',
                                     #xmltag = 'type',
                                     ))

        self.add_col(am.IdsArrayConf('ids_person', self.parent,
                                     groupnames=['state'],
                                     name='ID person',
                                     info='ID of person who ownes the vehicle.',
                                     ))

        self.add_col(am.ArrayConf('times_exec', 0.0,
                                  name='Exec time',
                                  info='Total route execution time from simulation run of last plan.',
                                  unit='s',
                                  ))

    def get_virtualpop(self):
        return self.parent

    def get_ids_veh_pop(self):
        """
        To be overridden by other individual vehicle types.
        """
        return self.get_virtualpop().ids_iauto

    def get_share(self, is_abs=False):
        n_veh = len(self)
        if is_abs:
            return n_veh
        else:
            return float(n_veh)/float(len(self.get_virtualpop()))

    def get_stagetable(self):
        return self.parent.get_plans().get_stagetable('autorides')

    def get_demand(self):
        return self.parent.parent

    def clear_vehicles(self):
        self.get_ids_veh_pop()[self.ids_person.get_value()] = -1
        self.clear()

    def assign_to_persons(self, ids_person):
        # print 'assign_to_persons',len(ids_person),self.mode
        # self.clear_vehicles()
        #ids_person_noveh = set(ids_person).difference(set(self.ids_person))
        n_new = len(ids_person)
        #
        # this call is selecting a veh id aof the specific mode
        # according to its share within this mode
        ids_vtype = self.get_demand().vtypes.generate_vtypes_for_mode(n_new, mode=self.mode)

        # print '  ids_vtype',ids_vtype
        ids_veh = self.add_rows(n=n_new,
                                ids_person=ids_person,
                                ids_vtype=ids_vtype,
                                )
        self.get_ids_veh_pop()[ids_person] = ids_veh
        return ids_veh

    def get_vtypes(self):
        """
        Returns a set with all used vehicle types.
        """
        # print 'Vehicles_individual.get_vtypes',self.cols.vtype
        return set(self.ids_vtype.get_value())

    def get_id_veh_xml(self, id_veh, id_stage):
        return self.mode_prefix + '.%s.%s' % (id_veh, id_stage)

    def get_id_line_xml(self, id_veh):
        return self.mode_prefix + '.%s' % (id_veh)

    def get_id_from_id_sumo(self, id_veh_sumo):
        # print 'get_id_from_id_sumo',id_veh_sumo,id_veh_sumo.split('.'),self.mode_prefix
        if len(id_veh_sumo.split('.')) == 3:
            prefix, id_veh, id_stage = id_veh_sumo.split('.')
            if prefix == self.mode_prefix:
                return int(id_veh)
            else:
                return -1
        return -1

    # def append_ride(self, id_veh, id_ride):
    #    ids_ride = self.ids_rides[id_veh]
    #    if ids_ride  is None:
    #        self.ids_rides[id_veh] = [id_ride]
    #    else:
    #        ids_ride.append(id_ride)
    def prepare_write_xml(self):
        """
        Prepare xml export. Must return export function.
        """
        virtualpop = self.get_virtualpop()
        scenario = virtualpop.get_scenario()
        #plans = virtualpop.get_plans()

        self._rides = self.get_stagetable()
        self._edges = scenario.net.edges
        self._lanes = scenario.net.lanes
        #self._individualvehicle = virtualpop.get_ibikes()
        self._ids_vtype_sumo = scenario.demand.vtypes.ids_sumo
        self._ids_edge_sumo = self._edges.ids_sumo
        self._id_mode = scenario.net.modes.get_id_mode(self.mode)
        self._get_laneid_allowed = self._edges.get_laneid_allowed
        self._get_sumoinfo_from_id_lane = scenario.net.lanes.get_sumoinfo_from_id_lane
        self._space_access = self.space_access.get_value()
        #self._time_veh_wait_after_stop = 3600
        self._parking = virtualpop.get_landuse().parking
        self._time_after_unboarding = self.time_after_unboarding.get_value()
        return self.write_xml

    def get_id_veh(self, id_stage):
        return self._rides.ids_iauto[id_stage]

    def write_xml(self,  fd, id_stage, time_begin, indent=2):

        # TODO: actually this should go in individualvehicle
        #time_veh_wait_after_stop = 3600
        #plans = self.get_plans()
        #walkstages = plans.get_stagetable('walks')
        #rides = plans.get_stagetable('autorides')
        #activitystages = plans.get_stagetable('activities')

        rides = self._rides
        #lanes = self._lanes
        parking = self._parking
        #net = self.get_net()
        #lanes = net.lanes
        #edges = net.edges
        #ind_ride = rides.get_inds(id_stage)
        id_veh = self.get_id_veh(id_stage)
        #individualvehicle = self._iveh
        id_vtype = self.ids_vtype[id_veh]

        # id_veh_ride,
        #                            ids_vtypes_iveh[id_veh],
        #                            ids_edges_rides_arr[ind_ride],
        #                            ids_parking_from_rides_arr[ind_ride],
        #                            ids_parking_to_rides_arr[ind_ride],

        id_parking_from = rides.ids_parking_from[id_stage]
        id_lane_from = parking.ids_lane[id_parking_from]
        #laneindex_from =  self._lanes.indexes[id_lane_from]
        pos_from = parking.positions[id_parking_from]

        id_parking_to = rides.ids_parking_to[id_stage]
        id_lane_to = parking.ids_lane[id_parking_to]
        #laneindex_to =  self._lanes.indexes[id_lane_to]
        pos_to = parking.positions[id_parking_to]

        # write unique veh ID to prevent confusion with other veh declarations
        fd.write(xm.start('vehicle id="%s"' % self.get_id_veh_xml(id_veh, id_stage), indent+2))

        # get start time of first stage of the plan
        #id_plan = rides.ids_plan[id_stage]
        #stages0, id_stage0 = self.get_plans().stagelists[id_plan][0]

        # this is the time when the vehicle appers in the scenario
        fd.write(xm.num('depart', '%.d' % rides.times_init[id_stage]))
        #fd.write(xm.num('depart', '%.d'%stages0.times_start[id_stage0]))

        fd.write(xm.num('type', self._ids_vtype_sumo[id_vtype]))
        fd.write(xm.num('line', self.get_id_line_xml(id_veh)))
        fd.write(xm.num('departPos', pos_from))
        fd.write(xm.num('departLane', self._lanes.indexes[id_lane_from]))

        fd.write(xm.stop())

        # write route
        fd.write(xm.start('route', indent+4))
        # print '  edgeindex[ids_edge]',edgeindex[ids_edge]
        fd.write(xm.arr('edges', self._ids_edge_sumo[rides.ids_edges[id_stage]]))

        # does not seem to have an effect, always starts at base????
        #fd.write(xm.num('departPos', pos_from))
        #fd.write(xm.num('departLane', laneindex_from ))
        fd.write(xm.stopit())

        # write depart stop
        fd.write(xm.start('stop', indent+4))
        #id_lane = self._lanes.ids_edge[id_lane_from]
        fd.write(xm.num('lane', self._get_sumoinfo_from_id_lane(id_lane_from)))
        # in 0.31 the vehicle will wait until after this duration
        # so it will be removed unless it will get a timeout function
        #fd.write(xm.num('duration', time_veh_wait_after_stop))
        fd.write(xm.num('startPos', pos_from - parking.lengths[id_parking_from]))
        fd.write(xm.num('endPos', pos_from))
        fd.write(xm.num('triggered', "True"))

        # chrashes with parking=True in 0.30!
        # however if not parked the vhcle is blocking the traffic
        # while waiting: workaround: delay departure to be shure that person already arrived

        fd.write(xm.num('parking', "True"))  # in windows 0.30 parked vehicles do not depart!!
        #fd.write(xm.num('parking', "False"))
        fd.write(xm.stopit())

        # write arrival stop
        fd.write(xm.start('stop', indent+4))
        fd.write(xm.num('lane', self._get_sumoinfo_from_id_lane(id_lane_to)))
        fd.write(xm.num('duration', self._time_after_unboarding))  # for unboarding only
        fd.write(xm.num('startPos', pos_to - parking.lengths[id_parking_to]))
        fd.write(xm.num('endPos', pos_to))
        #fd.write(xm.num('triggered', "True"))
        fd.write(xm.stopit())

        fd.write(xm.end('vehicle', indent+2))


class IndividualBikes(IndividualAutos):

    def __init__(self, ident, population, **kwargs):
        # print 'individualvehicle vtype id_default',vtypes.ids_sumo.get_id_from_index('passenger1')
        self._init_objman(ident=ident,
                          parent=population,
                          name='Indiv. Bikes',
                          info='Individual bike database. These are privately owned bikes.',
                          **kwargs)
        self._init_attributes()
        self._init_constants()

    def _init_attributes(self):

        IndividualAutos._init_attributes(self)

    def _init_constants(self):

        self.do_not_save_attrs(['mode', 'mode_prefix',
                                '_edges', '_ids_vtype_sumo', '_ids_edge_sumo',
                                '_id_mode', '_get_laneid_allowed', '_get_sumoinfo_from_id_lane',
                                '_space_access',
                                ])

    def def_mode(self):
        self.mode = 'bicycle'
        self.mode_prefix = 'ibike'

    def get_ids_veh_pop(self):
        """
        To be overridden by other individual vehicle types.
        """
        return self.parent.ids_ibike

    def get_stagetable(self):
        return self.parent.get_plans().get_stagetable('bikerides')

    def get_id_veh(self, id_stage):
        return self._rides.ids_ibike[id_stage]

    def prepare_write_xml(self):
        """
        Prepare xml export. Must return export function.
        """
        virtualpop = self.get_virtualpop()
        scenario = virtualpop.get_scenario()
        #plans = virtualpop.get_plans()
        self._rides = self.get_stagetable()
        self._edges = scenario.net.edges
        #self._individualvehicle = virtualpop.get_ibikes()
        self._ids_vtype_sumo = scenario.demand.vtypes.ids_sumo
        self._ids_edge_sumo = self._edges.ids_sumo
        self._id_mode = scenario.net.modes.get_id_mode(self.mode)
        self._get_laneid_allowed = self._edges.get_laneid_allowed
        self._get_sumoinfo_from_id_lane = scenario.net.lanes.get_sumoinfo_from_id_lane
        self._space_access = self.space_access.get_value()
        return self.write_xml

    # def _limit_pos(self,pos,id_edge):

    def write_xml(self,  fd, id_stage, time_begin, indent=2):
        rides = self._rides
        id_veh = self.get_id_veh(id_stage)
        # print 'write_xml',id_stage, time_begin,self.get_id_veh_xml(id_veh, id_stage)
        # print '  ids_edge_from,ids_edge_to',rides.ids_edge_from[id_stage],rides.ids_edge_to[id_stage],self._get_laneid_allowed( rides.ids_edge_from[id_stage], self._id_mode),self._get_laneid_allowed( rides.ids_edge_to[id_stage], self._id_mode)

        # TODO: actually this should go in individualvehicle
        #time_veh_wait_after_stop = 3600
        #plans = self.get_plans()
        #walkstages = plans.get_stagetable('walks')
        #rides = plans.get_stagetable('bikerides')
        #activitystages = plans.get_stagetable('activities')

        # for debug only:
        #virtualpop = self.get_virtualpop()
        #ids_edge_sumo = virtualpop.get_net().edges.ids_sumo

        #parking = self.get_landuse().parking
        #net = self.get_net()
        #lanes = net.lanes
        #edges = net.edges

        #ind_ride = rides.get_inds(id_stage)

        #individualvehicle = self.get_ibikes()
        id_vtype = self.ids_vtype[id_veh]

        # id_veh_ride,
        #                            ids_vtypes_iveh[id_veh],
        #                            ids_edges_rides_arr[ind_ride],
        #                            ids_parking_from_rides_arr[ind_ride],
        #                            ids_parking_to_rides_arr[ind_ride],

        #id_parking_from = rides.ids_parking_from[id_stage]
        #id_lane_from = parking.ids_lane[id_parking_from]
        #laneindex_from =  lanes.indexes[id_lane_from]
        #pos_from = parking.positions[id_parking_from]

        #id_parking_to = rides.ids_parking_to[id_stage]
        #id_lane_to = parking.ids_lane[id_parking_to]
        #laneindex_to =  lanes.indexes[id_lane_to]
        #pos_to = parking.positions[id_parking_to]

        # write unique veh ID to prevent confusion with other veh declarations
        fd.write(xm.start('vehicle id="%s"' % self.get_id_veh_xml(id_veh, id_stage), indent+2))

        # get start time of first stage of the plan
        #id_plan = rides.ids_plan[id_stage]
        #stages0, id_stage0 = self.get_plans().stagelists[id_plan][0]

        # this is the time when the vehicle appers in the scenario
        #fd.write(xm.num('depart', '%.d'%rides.times_init[id_stage]))
        fd.write(xm.num('depart', '%.d' % time_begin))
        fd.write(xm.num('type', self._ids_vtype_sumo[id_vtype]))
        fd.write(xm.num('line', self.get_id_line_xml(id_veh)))
        #fd.write(xm.num('departPos', pos_from))
        #fd.write(xm.num('departLane', laneindex_from ))
        fd.write(xm.num('from', self._ids_edge_sumo[rides.ids_edge_from[id_stage]]))
        pos_from = rides.positions_from[id_stage]
        pos_to = rides.positions_to[id_stage]
        fd.write(xm.num('departPos', pos_from))
        fd.write(xm.num('arrivalPos', pos_to))
        fd.write(xm.num('departLane', 'best'))

        fd.write(xm.stop())

        # write route
        fd.write(xm.start('route', indent+4))
        # print '  ids_edges',rides.ids_edges[id_stage]
        # print '  ids_sumo',self._ids_edge_sumo[rides.ids_edges[id_stage]]
        fd.write(xm.arr('edges', self._ids_edge_sumo[rides.ids_edges[id_stage]]))
        # fd.write(xm.arr('edges',edges.ids_sumo[rides.ids_edges[id_stage]]))

        # does not seem to have an effect, always starts at base????

        #id_edge = rides.ids_edge_from[id_stage]
        # print '  id_lane',id_lane,self._get_sumoinfo_from_id_lane(id_lane),'id_edge',id_edge,ids_edge_sumo[id_edge]

        fd.write(xm.stopit())

        # write depart stop
        fd.write(xm.start('stop', indent+4))
        id_lane = self._get_laneid_allowed(rides.ids_edge_from[id_stage], self._id_mode)
        fd.write(xm.num('lane', self._get_sumoinfo_from_id_lane(id_lane)))
        # in 0.31 the vehicle will wait until after this duration
        # so it will be removed unless it will get a timeout function
        #fd.write(xm.num('duration', time_veh_wait_after_stop))
        if pos_from > self._space_access:
            fd.write(xm.num('startPos', pos_from - self._space_access))
            fd.write(xm.num('endPos', pos_from+self._space_access))
        else:
            fd.write(xm.num('startPos', 0.1*pos_from))
            fd.write(xm.num('endPos', pos_from+self._space_access))

        fd.write(xm.num('triggered', "True"))

        # chrashes with parking=True in 0.30!
        # however if not parked the vhcle is blocking the traffic
        # while waiting: workaround: delay departure to be shure that person already arrived

        fd.write(xm.num('parking', 'True'))  # in windows 0.30 parked vehicles do not depart!!
        #fd.write(xm.num('parking', "False"))
        fd.write(xm.stopit())

        # write arrival stop
        fd.write(xm.start('stop', indent+4))

        id_lane = self._get_laneid_allowed(rides.ids_edge_to[id_stage], self._id_mode)
        #id_edge = rides.ids_edge_to[id_stage]
        # print '  id_lane',id_lane,self._get_sumoinfo_from_id_lane(id_lane),'id_edge',id_edge,ids_edge_sumo[id_edge]

        fd.write(xm.num('lane', self._get_sumoinfo_from_id_lane(id_lane)))
        fd.write(xm.num('duration', 5))  # for unboarding only
        if pos_to > self._space_access:
            fd.write(xm.num('startPos', pos_to - self._space_access))
            fd.write(xm.num('endPos', pos_to))
        else:
            fd.write(xm.num('startPos', 0.1*pos_to))
            fd.write(xm.num('endPos', pos_to))
        #fd.write(xm.num('triggered', "True"))
        fd.write(xm.stopit())

        fd.write(xm.end('vehicle', indent+2))


class IndividualMotorcycles(IndividualBikes):

    def __init__(self, ident, population, **kwargs):
        # print 'individualvehicle vtype id_default',vtypes.ids_sumo.get_id_from_index('passenger1')
        self._init_objman(ident=ident,
                          parent=population,
                          name='Indiv. Moto',
                          info='Individual Motorcycle/moped database. These are privately owned motorcycles.',
                          **kwargs)
        IndividualBikes._init_attributes(self)
        IndividualBikes._init_constants(self)

    def def_mode(self):
        self.mode = 'moped'
        self.mode_prefix = 'imoto'

    def get_ids_veh_pop(self):
        """
        To be overridden by other individual vehicle types.
        """
        return self.parent.ids_imoto

    def get_stagetable(self):
        return self.parent.get_plans().get_stagetable('motorides')

    def get_id_veh(self, id_stage):
        return self._rides.ids_imoto[id_stage]


class StrategyMixin(cm.BaseObjman):
    def __init__(self, ident, parent=None,
                 name='Strategy mixin', info='Info on strategy.',
                 **kwargs):
        """
        To be overridden.
        """
        # attention parent is the Strategies table
        self._init_objman(ident, parent, **kwargs)
        attrsman = self.set_attrsman(cm.Attrsman(self))

    def _init_attributes(self, **kwargs):
        # print 'StrategyMixin._init_attributes'
        attrsman = self.get_attrsman()

    def get_id_strategy(self):
        return self.parent.names.get_id_from_index(self.get_ident())

    def get_scenario(self):
        return self.parent.parent.get_scenario()

    def get_activities(self):
        return self.parent.parent.get_activities()

    def get_virtualpop(self):
        return self.parent.parent

    def get_plans(self):
        return self.parent.parent.plans

    def clip_positions(self, positions, ids_edge):
        lengths = self.get_scenario().net.edges.lengths[ids_edge]
        # print 'clip_positions',positions.shape,ids_edge.shape,lengths.shape
        positions_clip = np.clip(positions, self.dist_node_min*np.ones(len(positions),
                                                                       dtype=np.float32), lengths-self.dist_node_min)
        inds = lengths < 2*self.dist_node_min
        # print '  inds.shape',inds.shape,positions_clip.shape
        positions_clip[inds] = 0.5*lengths[inds]
        return positions_clip

    def _init_attributes_strategy(self, **kwargs):
        attrsman = self.get_attrsman()
        self.dist_node_min = attrsman.add(cm.AttrConf('dist_node_min', kwargs.get('dist_node_min', 40.0),
                                                      groupnames=['options'],
                                                      perm='rw',
                                                      name='Min. dist to nodes',
                                                      unit='m',
                                                      info='Minimum distance between starting position and node center.',
                                                      ))
    # def _init_constants_strategy(self):
    #    #print '_init_constants_strategy'
    #    modes = self.get_virtualpop().get_scenario().net.modes
    #    self._id_mode_bike = modes.get_id_mode('bicycle')
    #    self._id_mode_auto = modes.get_id_mode('passenger')
    #    self._id_mode_moto = modes.get_id_mode('motorcycle')
    #   self.get_attrsman().do_not_save_attrs([
    #                '_id_mode_bike','_id_mode_auto','_id_mode_moto',
    #                        ])
    # print '  _id_mode_auto',self._id_mode_auto

    # def are_feasible(self, ids_person):
    #    """
    #    Returns a bool vector, with True values for
    #    persons where this strategy can be applied.
    #    """
    #    return []

    # def is_feasible(self, id_person):
    #    """
    #    Returns True if this strategy is feasible for this person.
    #    Overriden by specific strategy.
    #    """
    #    return False

    def preevaluate(self, ids_person):
        """
        Preevaluation strategies for person IDs in vector ids_person.

        Returns a preevaluation vector with a preevaluation value 
        for each person ID. The values of the preevaluation vector are as follows:
            -1 : Strategy cannot be applied
            0  : Stategy can be applied, but the preferred mode is not used
            1  : Stategy can be applied, and preferred mode is part of the strategy
            2  : Strategy uses predomunantly preferred mode

        """
        return zeros(len(ids_person), dtype=np.int32)

    def plan(self, ids_person, logger=None):
        """
        Generates a plan for these person according to this strategie.
        Overriden by specific strategy.
        """
        pass


class NoneStrategy(StrategyMixin):
    def __init__(self, ident, parent=None,
                 name='None strategy',
                 info='With this strategy, no mobility plan is generated.',
                 **kwargs):

        self._init_objman(ident, parent, name=name, info=info, **kwargs)
        attrsman = self.set_attrsman(cm.Attrsman(self))


class TransitStrategy(StrategyMixin):
    def __init__(self, ident, parent=None,
                 name='Public Transport Strategy',
                 info='With this strategy, the person uses his private auto as main transport mode. He may accept passengers or public transport with P&R',
                 **kwargs):

        self._init_objman(ident, parent, name=name, info=info, **kwargs)
        attrsman = self.set_attrsman(cm.Attrsman(self))
        # specific init
        self._init_attributes()
        self._init_constants()

    def _init_attributes(self):
        # print 'StrategyMixin._init_attributes'
        pass

    def _init_constants(self):
        #virtualpop = self.get_virtualpop()
        #stagetables = virtualpop.get_stagetables()

        #self._walkstages = stagetables.get_stagetable('walks')
        #self._ridestages = stagetables.get_stagetable('rides')
        #self._activitystages = stagetables.get_stagetable('activities')

        #self._plans = virtualpop.get_plans()
        #
        # print 'AutoStrategy._init_constants'
        # print dir(self)
        # self.get_attrsman().do_not_save_attrs(['_activitystages','_ridestages','_walkstages','_plans'])

        modes = self.get_virtualpop().get_scenario().net.modes
        self._id_mode_bike = modes.get_id_mode('bicycle')
        self._id_mode_auto = modes.get_id_mode('passenger')
        self._id_mode_moto = modes.get_id_mode('motorcycle')
        self._id_mode_bus = modes.get_id_mode('bus')
        self.get_attrsman().do_not_save_attrs([
            '_id_mode_bike', '_id_mode_auto', '_id_mode_moto', '_id_mode_bus'
        ])

    def preevaluate(self, ids_person):
        """
        Preevaluation strategies for person IDs in vector ids_person.

        Returns a preevaluation vector with a preevaluation value 
        for each person ID. The values of the preevaluation vector are as follows:
            -1 : Strategy cannot be applied
            0  : Stategy can be applied, but the preferred mode is not used
            1  : Stategy can be applied, and preferred mode is part of the strategy
            2  : Strategy uses predomunantly preferred mode

        """
        n_pers = len(ids_person)
        persons = self.get_virtualpop()
        preeval = np.zeros(n_pers, dtype=np.int32)

        # TODO: here we could exclude by age or distance facilities-stops

        # put 0 for persons whose preference is not public transport
        preeval[persons.ids_mode_preferred[ids_person] != self._id_mode_bus] = 0

        # put 2 for persons with car access and who prefer cars
        preeval[persons.ids_mode_preferred[ids_person] == self._id_mode_bus] = 2

        print '  TransitStrategy.preevaluate', len(np.flatnonzero(preeval))
        return preeval

    def plan(self, ids_person, logger=None):
        """
        Generates a plan for these person according to this strategie.
        Overriden by specific strategy.
        """
        print 'TransitStrategy.pan', len(ids_person)
        #make_plans_private(self, ids_person = None, mode = 'passenger')
        # routing necessary?
        virtualpop = self.get_virtualpop()
        plans = virtualpop.get_plans()  # self._plans
        demand = virtualpop.get_demand()
        ptlines = demand.ptlines

        walkstages = plans.get_stagetable('walks')
        transitstages = plans.get_stagetable('transits')
        activitystages = plans.get_stagetable('activities')

        activities = virtualpop.get_activities()
        activitytypes = demand.activitytypes
        landuse = virtualpop.get_landuse()
        facilities = landuse.facilities
        parking = landuse.parking

        scenario = virtualpop.get_scenario()
        net = scenario.net
        edges = net.edges
        lanes = net.lanes
        modes = net.modes

        ptstops = net.ptstops

        # print '   demand',demand
        # print '   demand.ptlines',demand.ptlines,dir(demand.ptlines)
        # print '   demand.ptlines.get_ptlinks()',demand.ptlines.get_ptlinks()
        # print '   demand.virtualpop',demand.virtualpop,dir(demand.virtualpop)
        # print '   demand.trips',demand.trips,dir(demand.trips)
        if len(ptlines) == 0:
            print 'WARNING in TrasitStrategy.plan: no transit services available.'
            return False

        ptlinks = ptlines.get_ptlinks()
        ptlinktypes = ptlinks.types.choices
        type_enter = ptlinktypes['enter']
        type_transit = ptlinktypes['transit']
        type_board = ptlinktypes['board']
        type_alight = ptlinktypes['alight']
        type_transfer = ptlinktypes['transfer']
        type_walk = ptlinktypes['walk']
        type_exit = ptlinktypes['exit']

        ptfstar = ptlinks.get_fstar()
        pttimes = ptlinks.get_times()
        stops_to_enter, stops_to_exit = ptlinks.get_stops_to_enter_exit()

        ids_stoplane = ptstops.ids_lane
        ids_laneedge = net.lanes.ids_edge

        times_est_plan = plans.times_est
        # here we can determine edge weights for different modes

        # this could be centralized to avoid redundance
        plans.prepare_stagetables(['walks', 'transits', 'activities'])

        ids_person_act, ids_act_from, ids_act_to\
            = virtualpop.get_activities_from_pattern(0, ids_person=ids_person)

        if len(ids_person_act) == 0:
            print 'WARNING in TrasitStrategy.plan: no eligible persons found.'
            return False

        # temporary maps from ids_person to other parameters
        nm = np.max(ids_person_act)+1
        map_ids_plan = np.zeros(nm, dtype=np.int32)
        #ids_plan_act = virtualpop.add_plans(ids_person_act, id_strategy = self.get_id_strategy())
        map_ids_plan[ids_person_act] = virtualpop.add_plans(ids_person_act, id_strategy=self.get_id_strategy())

        map_times = np.zeros(nm, dtype=np.int32)
        map_times[ids_person_act] = activities.get_times_end(ids_act_from, pdf='unit')

        # set start time to plans (important!)
        plans.times_begin[map_ids_plan[ids_person_act]] = map_times[ids_person_act]

        map_ids_fac_from = np.zeros(nm, dtype=np.int32)
        map_ids_fac_from[ids_person_act] = activities.ids_facility[ids_act_from]

        n_plans = len(ids_person_act)
        print 'TrasitStrategy.plan n_plans=', n_plans

        # make initial activity stage
        ids_edge_from = facilities.ids_roadedge_closest[map_ids_fac_from[ids_person_act]]
        poss_edge_from = facilities.positions_roadedge_closest[map_ids_fac_from[ids_person_act]]
        # this is the time when first activity starts
        # first activity is normally not simulated

        names_acttype_from = activitytypes.names[activities.ids_activitytype[ids_act_from]]
        durations_act_from = activities.get_durations(ids_act_from)
        times_from = map_times[ids_person_act]-durations_act_from
        #times_from = activities.get_times_end(ids_act_from, pdf = 'unit')

        for id_plan,\
            time,\
            id_act_from,\
            name_acttype_from,\
            duration_act_from,\
            id_edge_from,\
            pos_edge_from  \
            in zip(map_ids_plan[ids_person_act],
                   times_from,
                   ids_act_from,
                   names_acttype_from,
                   durations_act_from,
                   ids_edge_from,
                   poss_edge_from):

            id_stage_act, time = activitystages.append_stage(
                id_plan, time,
                ids_activity=id_act_from,
                names_activitytype=name_acttype_from,
                durations=duration_act_from,
                ids_lane=edges.ids_lanes[id_edge_from][0],
                positions=pos_edge_from,
            )

        ##

        ind_act = 0

        # main loop while there are persons performing
        # an activity at index ind_act
        while len(ids_person_act) > 0:
            ids_plan = map_ids_plan[ids_person_act]

            times_from = map_times[ids_person_act]

            names_acttype_to = activitytypes.names[activities.ids_activitytype[ids_act_to]]
            durations_act_to = activities.get_durations(ids_act_to)

            ids_fac_from = map_ids_fac_from[ids_person_act]
            ids_fac_to = activities.ids_facility[ids_act_to]

            centroids_from = facilities.centroids[ids_fac_from]
            centroids_to = facilities.centroids[ids_fac_to]

            # origin edge and position
            ids_edge_from = facilities.ids_roadedge_closest[ids_fac_from]
            poss_edge_from = facilities.positions_roadedge_closest[ids_fac_from]

            # destination edge and position
            ids_edge_to = facilities.ids_roadedge_closest[ids_fac_to]
            poss_edge_to = facilities.positions_roadedge_closest[ids_fac_to]

            ids_stop_from = ptstops.get_closest(centroids_from)
            ids_stop_to = ptstops.get_closest(centroids_to)

            ids_stopedge_from = ids_laneedge[ids_stoplane[ids_stop_from]]
            ids_stopedge_to = ids_laneedge[ids_stoplane[ids_stop_to]]

            # do random pos here
            poss_stop_from = 0.5*(ptstops.positions_from[ids_stop_from]
                                  + ptstops.positions_to[ids_stop_from])

            poss_stop_to = 0.5*(ptstops.positions_from[ids_stop_to]
                                + ptstops.positions_to[ids_stop_to])

            i = 0.0
            for id_person, id_plan, time_from, id_act_from, id_act_to, name_acttype_to, duration_act_to, id_edge_from, pos_edge_from, id_edge_to, pos_edge_to, id_stop_from, id_stopedge_from, pos_stop_from, id_stop_to, id_stopedge_to, pos_stop_to\
                    in zip(ids_person_act, ids_plan, times_from, ids_act_from, ids_act_to, names_acttype_to, durations_act_to,  ids_edge_from, poss_edge_from, ids_edge_to, poss_edge_to, ids_stop_from, ids_stopedge_from, poss_stop_from, ids_stop_to, ids_stopedge_to, poss_stop_to):
                n_pers = len(ids_person_act)
                if logger:
                    logger.progress(i/n_pers*100)
                i += 1.0
                print 79*'_'
                print '  id_plan=%d, id_person=%d, ' % (id_plan, id_person)

                id_stage_walk1, time = walkstages.append_stage(id_plan, time_from,
                                                               id_edge_from=id_edge_from,
                                                               position_edge_from=pos_edge_from,
                                                               id_edge_to=id_stopedge_from,
                                                               position_edge_to=pos_stop_from,  # -7.0,
                                                               )

                # print '    id_stopedge_from',id_stopedge_from
                # print '    pos_stop_from',pos_stop_from

                # print
                # print '    id_stopedge_to',id_stopedge_to
                # print '    pos_stop_to',pos_stop_to
                # print
                # print '    id_stop_from',id_stop_from
                # print '    id_stop_to',id_stop_to

                durations, linktypes, ids_line, ids_fromstop, ids_tostop =\
                    ptlinks.route(id_stop_from, id_stop_to,
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
                        print '    stage for linktype %2d fromedge %s toedge %s' % (
                            linktype, edges.ids_sumo[id_stopedge_from], edges.ids_sumo[id_stopedge_to])

                        print '    id_stopedge_from,id_stopedge_to', id_stopedge_from, id_stopedge_to
                        if linktype == type_transit:  # transit!
                            print '    add transit'
                            id_stage_transit, time = transitstages.append_stage(
                                id_plan, time,
                                id_line=id_line,
                                duration=duration+duration_wait,
                                id_fromedge=id_stopedge_from,
                                id_toedge=id_stopedge_to,
                            )
                            duration_wait = 0.0

                        elif linktype == type_walk:  # walk to transfer
                            print '    add transfer'
                            id_stage_transfer, time = walkstages.append_stage(
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
                            print '    add duration', duration
                            duration_wait += duration

                    # walk from final stop to activity
                    # print '    Stage for linktype %2d fromedge %s toedge %s'%(linktype, edges.ids_sumo[id_stopedge_to],edges.ids_sumo[id_edge_to] )
                    id_stage_walk2, time = walkstages.append_stage(id_plan, time,
                                                                   id_edge_from=id_stopedge_to,
                                                                   position_edge_from=pos_tostop,
                                                                   id_edge_to=id_edge_to,
                                                                   position_edge_to=pos_edge_to,
                                                                   )

                else:
                    # there is no public transport line linking these nodes.
                    # Modify walk directly from home to activity
                    time = walkstages.modify_stage(id_stage_walk1, time_from,
                                                   id_edge_from=id_edge_from,
                                                   position_edge_from=pos_edge_from,
                                                   id_edge_to=id_edge_to,
                                                   position_edge_to=pos_edge_to,
                                                   )

                # update time for trips estimation for this plan
                plans.times_est[id_plan] += time-time_from

                # define current end time without last activity duration
                plans.times_end[id_plan] = time

                id_stage_act, time = activitystages.append_stage(
                    id_plan, time,
                    ids_activity=id_act_to,
                    names_activitytype=name_acttype_to,
                    durations=duration_act_to,
                    ids_lane=edges.ids_lanes[id_edge_to][0],
                    positions=pos_edge_to,
                )

                # store time for next iteration in case other activities are
                # following
                map_times[id_person] = time

            # select persons and activities for next setp
            ind_act += 1
            ids_person_act, ids_act_from, ids_act_to\
                = virtualpop.get_activities_from_pattern(ind_act, ids_person=ids_person_act)


class WalkStrategy(StrategyMixin):
    def __init__(self, ident, parent=None,
                 name='Walk Strategy',
                 info='With this strategy, the person walks to all destinations.',
                 **kwargs):

        self._init_objman(ident, parent, name=name, info=info, **kwargs)
        attrsman = self.set_attrsman(cm.Attrsman(self))
        # specific init
        self._init_attributes()
        self._init_constants()

    def _init_attributes(self):
        # print 'StrategyMixin._init_attributes'
        pass

    def _init_constants(self):
        #virtualpop = self.get_virtualpop()
        #stagetables = virtualpop.get_stagetables()

        #self._walkstages = stagetables.get_stagetable('walks')
        #self._ridestages = stagetables.get_stagetable('rides')
        #self._activitystages = stagetables.get_stagetable('activities')

        #self._plans = virtualpop.get_plans()
        #
        # print 'AutoStrategy._init_constants'
        # print dir(self)
        # self.get_attrsman().do_not_save_attrs(['_activitystages','_ridestages','_walkstages','_plans'])

        modes = self.get_virtualpop().get_scenario().net.modes
        self._id_mode_bike = modes.get_id_mode('bicycle')
        self._id_mode_auto = modes.get_id_mode('passenger')
        self._id_mode_moto = modes.get_id_mode('motorcycle')
        self._id_mode_bus = modes.get_id_mode('bus')
        self._id_mode_ped = modes.get_id_mode('pedestrian')
        self.get_attrsman().do_not_save_attrs([
            '_id_mode_bike', '_id_mode_auto', '_id_mode_moto',
            '_id_mode_bus', '_id_mode_ped',
        ])

    def preevaluate(self, ids_person):
        """
        Preevaluation strategies for person IDs in vector ids_person.

        Returns a preevaluation vector with a preevaluation value 
        for each person ID. The values of the preevaluation vector are as follows:
            -1 : Strategy cannot be applied
            0  : Stategy can be applied, but the preferred mode is not used
            1  : Stategy can be applied, and preferred mode is part of the strategy
            2  : Strategy uses predomunantly preferred mode

        """
        n_pers = len(ids_person)
        persons = self.get_virtualpop()
        preeval = np.zeros(n_pers, dtype=np.int32)

        # TODO: here we could exclude by age or distance facilities-stops

        # put 0 for persons whose preference is not public transport
        preeval[persons.ids_mode_preferred[ids_person] != self._id_mode_ped] = 0

        # put 2 for persons with car access and who prefer cars
        preeval[persons.ids_mode_preferred[ids_person] == self._id_mode_ped] = 2

        print '  WalkStrategy.preevaluate', len(np.flatnonzero(preeval))
        return preeval

    def plan(self, ids_person, logger=None):
        """
        Generates a plan for these person according to this strategie.
        Overriden by specific strategy.
        """
        print 'WalkStrategy.pan', len(ids_person)
        #make_plans_private(self, ids_person = None, mode = 'passenger')
        # routing necessary?
        virtualpop = self.get_virtualpop()
        plans = virtualpop.get_plans()  # self._plans
        demand = virtualpop.get_demand()
        #ptlines = demand.ptlines

        walkstages = plans.get_stagetable('walks')
        #transitstages = plans.get_stagetable('transits')
        activitystages = plans.get_stagetable('activities')

        activities = virtualpop.get_activities()
        activitytypes = demand.activitytypes
        landuse = virtualpop.get_landuse()
        facilities = landuse.facilities
        #parking = landuse.parking

        scenario = virtualpop.get_scenario()
        net = scenario.net
        edges = net.edges
        lanes = net.lanes
        modes = net.modes

        #ptstops = net.ptstops

        ids_laneedge = net.lanes.ids_edge

        times_est_plan = plans.times_est
        # here we can determine edge weights for different modes

        # this could be centralized to avoid redundance
        plans.prepare_stagetables(['walks', 'activities'])

        ids_person_act, ids_act_from, ids_act_to\
            = virtualpop.get_activities_from_pattern(0, ids_person=ids_person)

        if len(ids_person_act) == 0:
            print 'WARNING in WalkStrategy.plan: no eligible persons found.'
            return False

        # temporary maps from ids_person to other parameters
        nm = np.max(ids_person_act)+1
        map_ids_plan = np.zeros(nm, dtype=np.int32)
        #ids_plan_act = virtualpop.add_plans(ids_person_act, id_strategy = self.get_id_strategy())
        map_ids_plan[ids_person_act] = virtualpop.add_plans(ids_person_act, id_strategy=self.get_id_strategy())

        map_times = np.zeros(nm, dtype=np.int32)
        map_times[ids_person_act] = activities.get_times_end(ids_act_from, pdf='unit')

        # set start time to plans (important!)
        plans.times_begin[map_ids_plan[ids_person_act]] = map_times[ids_person_act]

        map_ids_fac_from = np.zeros(nm, dtype=np.int32)
        map_ids_fac_from[ids_person_act] = activities.ids_facility[ids_act_from]

        n_plans = len(ids_person_act)
        print 'TrasitStrategy.plan n_plans=', n_plans

        # make initial activity stage
        ids_edge_from = facilities.ids_roadedge_closest[map_ids_fac_from[ids_person_act]]
        poss_edge_from = facilities.positions_roadedge_closest[map_ids_fac_from[ids_person_act]]
        # this is the time when first activity starts
        # first activity is normally not simulated

        names_acttype_from = activitytypes.names[activities.ids_activitytype[ids_act_from]]
        durations_act_from = activities.get_durations(ids_act_from)
        times_from = map_times[ids_person_act]-durations_act_from
        #times_from = activities.get_times_end(ids_act_from, pdf = 'unit')

        for id_plan,\
            time,\
            id_act_from,\
            name_acttype_from,\
            duration_act_from,\
            id_edge_from,\
            pos_edge_from  \
            in zip(map_ids_plan[ids_person_act],
                   times_from,
                   ids_act_from,
                   names_acttype_from,
                   durations_act_from,
                   ids_edge_from,
                   poss_edge_from):

            id_stage_act, time = activitystages.append_stage(
                id_plan, time,
                ids_activity=id_act_from,
                names_activitytype=name_acttype_from,
                durations=duration_act_from,
                ids_lane=edges.ids_lanes[id_edge_from][0],
                positions=pos_edge_from,
            )

        ##

        ind_act = 0

        # main loop while there are persons performing
        # an activity at index ind_act
        while len(ids_person_act) > 0:
            ids_plan = map_ids_plan[ids_person_act]

            times_from = map_times[ids_person_act]

            names_acttype_to = activitytypes.names[activities.ids_activitytype[ids_act_to]]
            durations_act_to = activities.get_durations(ids_act_to)

            ids_fac_from = map_ids_fac_from[ids_person_act]
            ids_fac_to = activities.ids_facility[ids_act_to]

            centroids_from = facilities.centroids[ids_fac_from]
            centroids_to = facilities.centroids[ids_fac_to]

            # origin edge and position
            ids_edge_from = facilities.ids_roadedge_closest[ids_fac_from]
            poss_edge_from = facilities.positions_roadedge_closest[ids_fac_from]

            # destination edge and position
            ids_edge_to = facilities.ids_roadedge_closest[ids_fac_to]
            poss_edge_to = facilities.positions_roadedge_closest[ids_fac_to]

            #ids_stop_from = ptstops.get_closest(centroids_from)
            #ids_stop_to = ptstops.get_closest(centroids_to)

            #ids_stopedge_from = ids_laneedge[ids_stoplane[ids_stop_from]]
            #ids_stopedge_to = ids_laneedge[ids_stoplane[ids_stop_to]]

            # do random pos here
            # poss_stop_from = 0.5*(  ptstops.positions_from[ids_stop_from]\
            #                        +ptstops.positions_to[ids_stop_from])

            # poss_stop_to = 0.5*(  ptstops.positions_from[ids_stop_to]\
            #                            +ptstops.positions_to[ids_stop_to])

            i = 0.0
            for id_person, id_plan, time_from, id_act_from, id_act_to, name_acttype_to, duration_act_to, id_edge_from, pos_edge_from, id_edge_to, pos_edge_to, \
                    in zip(ids_person_act, ids_plan, times_from, ids_act_from, ids_act_to, names_acttype_to, durations_act_to,  ids_edge_from, poss_edge_from, ids_edge_to, poss_edge_to):
                n_pers = len(ids_person_act)
                if logger:
                    logger.progress(i/n_pers*100)
                i += 1.0
                print 79*'_'
                print '  id_plan=%d, id_person=%d, ' % (id_plan, id_person)

                id_stage_walk1, time = walkstages.append_stage(id_plan, time_from,
                                                               id_edge_from=id_edge_from,
                                                               position_edge_from=pos_edge_from,
                                                               id_edge_to=id_edge_to,
                                                               position_edge_to=pos_edge_to,  # -7.0,
                                                               )

                # update time for trips estimation for this plan
                plans.times_est[id_plan] += time-time_from

                # define current end time without last activity duration
                plans.times_end[id_plan] = time

                id_stage_act, time = activitystages.append_stage(
                    id_plan, time,
                    ids_activity=id_act_to,
                    names_activitytype=name_acttype_to,
                    durations=duration_act_to,
                    ids_lane=edges.ids_lanes[id_edge_to][0],
                    positions=pos_edge_to,
                )

                # store time for next iteration in case other activities are
                # following
                map_times[id_person] = time

            # select persons and activities for next setp
            ind_act += 1
            ids_person_act, ids_act_from, ids_act_to\
                = virtualpop.get_activities_from_pattern(ind_act, ids_person=ids_person_act)


class AutoStrategy(StrategyMixin):
    def __init__(self, ident, parent=None,
                 name='Auto strategy',
                 info='With this strategy, the person uses his private auto as main transport mode.',
                 **kwargs):

        self._init_objman(ident, parent, name=name, info=info, **kwargs)
        attrsman = self.set_attrsman(cm.Attrsman(self))
        # specific init
        self._init_attributes()
        self._init_constants()

    def _init_attributes(self):
        # print 'StrategyMixin._init_attributes'
        pass

    def _init_constants(self):
        #virtualpop = self.get_virtualpop()
        #stagetables = virtualpop.get_stagetables()

        #self._walkstages = stagetables.get_stagetable('walks')
        #self._ridestages = stagetables.get_stagetable('rides')
        #self._activitystages = stagetables.get_stagetable('activities')

        #self._plans = virtualpop.get_plans()
        #
        # print 'AutoStrategy._init_constants'
        # print dir(self)
        # self.get_attrsman().do_not_save_attrs(['_activitystages','_ridestages','_walkstages','_plans'])

        modes = self.get_virtualpop().get_scenario().net.modes
        self._id_mode_bike = modes.get_id_mode('bicycle')
        self._id_mode_auto = modes.get_id_mode('passenger')
        self._id_mode_moto = modes.get_id_mode('motorcycle')
        self.get_attrsman().do_not_save_attrs([
            '_id_mode_bike', '_id_mode_auto', '_id_mode_moto',
        ])

    def preevaluate(self, ids_person):
        """
        Preevaluation strategies for person IDs in vector ids_person.

        Returns a preevaluation vector with a preevaluation value 
        for each person ID. The values of the preevaluation vector are as follows:
            -1 : Strategy cannot be applied
            0  : Stategy can be applied, but the preferred mode is not used
            1  : Stategy can be applied, and preferred mode is part of the strategy
            2  : Strategy uses predomunantly preferred mode

        """
        n_pers = len(ids_person)
        print 'Autostrategy.preevaluate', n_pers, 'persons'
        persons = self.get_virtualpop()
        preeval = np.zeros(n_pers, dtype=np.int32)

        # put -1 for persons without car access
        preeval[persons.ids_iauto[ids_person] == -1] = -1
        print '  persons having no auto', len(np.flatnonzero(persons.ids_iauto[ids_person] == -1))

        # put 0 for persons with car but with a different preferred mode
        preeval[(persons.ids_iauto[ids_person] > -1)
                & (persons.ids_mode_preferred[ids_person] != self._id_mode_auto)] = 0

        print '  persons with car but with a different preferred mode', len(np.flatnonzero(
            (persons.ids_iauto[ids_person] > -1) & (persons.ids_mode_preferred[ids_person] != self._id_mode_auto)))

        # put 2 for persons with car access and who prefer the car
        preeval[(persons.ids_iauto[ids_person] > -1)
                & (persons.ids_mode_preferred[ids_person] == self._id_mode_auto)] = 2
        print '  persons  with car access and who prefer the car', len(np.flatnonzero(
            (persons.ids_iauto[ids_person] > -1) & (persons.ids_mode_preferred[ids_person] == self._id_mode_auto)))

        return preeval

    # def are_feasible(self, ids_person):
    #    """
    #    Returns a bool vector, with True values for
    #    persons where this strategy can be applied.
    #    """
    #    persons = self.get_virtualpop()
    #
    #    # check if person has a car
    #    # one may also check if there is parking available
    #    # at all desinations
    #    return persons.ids_iautos[ids_person] >= 0

    def plan(self, ids_person, logger=None):
        """
        Generates a plan for these person according to this strategie.
        Overriden by specific strategy.
        """
        #make_plans_private(self, ids_person = None, mode = 'passenger')
        # routing necessary?
        virtualpop = self.get_virtualpop()
        plans = virtualpop.get_plans()  # self._plans
        walkstages = plans.get_stagetable('walks')
        ridestages = plans.get_stagetable('autorides')
        activitystages = plans.get_stagetable('activities')

        activities = virtualpop.get_activities()
        activitytypes = virtualpop.get_demand().activitytypes
        landuse = virtualpop.get_landuse()
        facilities = landuse.facilities
        parking = landuse.parking

        scenario = virtualpop.get_scenario()
        edges = scenario.net.edges
        lanes = scenario.net.lanes
        modes = scenario.net.modes

        #times_est_plan = plans.times_est

        # here we can determine edge weights for different modes
        plans.prepare_stagetables(['walks', 'autorides', 'activities'])

        # get initial travel times for persons.
        # initial travel times depend on the initial activity

        landuse.parking.clear_booking()

        ids_person_act, ids_act_from, ids_act_to\
            = virtualpop.get_activities_from_pattern(0, ids_person=ids_person)

        if len(ids_person_act) == 0:
            print 'WARNING in Autostrategy.plan: no eligible persons found.'
            return False

        # ok

        # temporary maps from ids_person to other parameters
        nm = np.max(ids_person_act)+1
        map_ids_plan = np.zeros(nm, dtype=np.int32)
        #ids_plan_act = virtualpop.add_plans(ids_person_act, id_strategy = self.get_id_strategy())
        map_ids_plan[ids_person_act] = virtualpop.add_plans(ids_person_act, id_strategy=self.get_id_strategy())

        # err
        map_times = np.zeros(nm, dtype=np.int32)
        map_times[ids_person_act] = activities.get_times_end(ids_act_from, pdf='unit')

        # set start time to plans (important!)
        plans.times_begin[map_ids_plan[ids_person_act]] = map_times[ids_person_act]

        map_ids_fac_from = np.zeros(nm, dtype=np.int32)
        map_ids_fac_from[ids_person_act] = activities.ids_facility[ids_act_from]

        # err
        map_ids_parking_from = np.zeros(nm, dtype=np.int32)
        ids_parking_from, inds_vehparking = parking.get_closest_parkings(virtualpop.ids_iauto[ids_person_act],
                                                                         facilities.centroids[activities.ids_facility[ids_act_from]])
        if len(ids_parking_from) == 0:
            return False

        # err

        map_ids_parking_from[ids_person_act] = ids_parking_from

        n_plans = len(ids_person_act)
        print 'AutoStrategy.plan n_plans=', n_plans
        # print '  map_ids_parking_from[ids_person_act].shape',map_ids_parking_from[ids_person_act].shape
        # set initial activity
        # this is because the following steps start with travel
        # and set the next activity
        #names_acttype_from = activitytypes.names[activities.ids_activitytype[ids_act_from]]
        # for id_plan

        ind_act = 0

        # make initial activity stage
        ids_edge_from = facilities.ids_roadedge_closest[map_ids_fac_from[ids_person_act]]
        poss_edge_from = facilities.positions_roadedge_closest[map_ids_fac_from[ids_person_act]]
        # this is the time when first activity starts
        # first activity is normally not simulated

        names_acttype_from = activitytypes.names[activities.ids_activitytype[ids_act_from]]
        durations_act_from = activities.get_durations(ids_act_from)
        times_from = map_times[ids_person_act]-durations_act_from
        #times_from = activities.get_times_end(ids_act_from, pdf = 'unit')

        for id_plan,\
            time,\
            id_act_from,\
            name_acttype_from,\
            duration_act_from,\
            id_edge_from,\
            pos_edge_from  \
            in zip(map_ids_plan[ids_person_act],
                   times_from,
                   ids_act_from,
                   names_acttype_from,
                   durations_act_from,
                   ids_edge_from,
                   poss_edge_from):

            id_stage_act, time = activitystages.append_stage(
                id_plan, time,
                ids_activity=id_act_from,
                names_activitytype=name_acttype_from,
                durations=duration_act_from,
                ids_lane=edges.ids_lanes[id_edge_from][0],
                positions=pos_edge_from,
            )

        # main loop while there are persons performing
        # an activity at index ind_act
        while len(ids_person_act) > 0:
            ids_plan = map_ids_plan[ids_person_act]
            ids_veh = virtualpop.ids_iauto[ids_person_act]
            #inds_pers = virtualpop.get_inds(ids_person)
            # self.persons.cols.mode_preferred[inds_pers]='private'

            times_from = map_times[ids_person_act]

            names_acttype_to = activitytypes.names[activities.ids_activitytype[ids_act_to]]
            durations_act_to = activities.get_durations(ids_act_to)

            ids_fac_from = map_ids_fac_from[ids_person_act]
            ids_fac_to = activities.ids_facility[ids_act_to]

            centroids_to = facilities.centroids[ids_fac_to]

            # origin edge and position
            ids_edge_from = facilities.ids_roadedge_closest[ids_fac_from]
            poss_edge_from = facilities.positions_roadedge_closest[ids_fac_from]

            # this method will find and occupy parking space
            ids_parking_from = map_ids_parking_from[ids_person_act]

            # print '  ids_veh.shape',ids_veh.shape
            # print '  centroids_to.shape',centroids_to.shape
            ids_parking_to, inds_vehparking = parking.get_closest_parkings(ids_veh, centroids_to)

            ids_lane_parking_from = parking.ids_lane[ids_parking_from]
            ids_edge_parking_from = lanes.ids_edge[ids_lane_parking_from]
            poss_edge_parking_from = parking.positions[ids_parking_from]

            # print '  ids_parking_to.shape',ids_parking_to.shape
            # print '  np.max(parking.get_ids()), np.max(ids_parking_to)',np.max(parking.get_ids()), np.max(ids_parking_to)
            ids_lane_parking_to = parking.ids_lane[ids_parking_to]
            ids_edge_parking_to = lanes.ids_edge[ids_lane_parking_to]
            poss_edge_parking_to = parking.positions[ids_parking_to]

            # destination edge and position
            ids_edge_to = facilities.ids_roadedge_closest[ids_fac_to]
            poss_edge_to = facilities.positions_roadedge_closest[ids_fac_to]

            i = 0.0
            n_pers = len(ids_person_act)
            for id_person, id_plan, time_from, id_act_from, id_act_to, name_acttype_to, duration_act_to, id_veh, id_edge_from, pos_edge_from, id_edge_parking_from, pos_edge_parking_from, id_parking_from, id_parking_to, id_edge_parking_to, pos_edge_parking_to, id_edge_to, pos_edge_to\
                    in zip(ids_person_act, ids_plan, times_from, ids_act_from, ids_act_to, names_acttype_to, durations_act_to, ids_veh, ids_edge_from, poss_edge_from, ids_edge_parking_from, poss_edge_parking_from, ids_parking_from, ids_parking_to, ids_edge_parking_to, poss_edge_parking_to, ids_edge_to, poss_edge_to):
                if logger:
                    logger.progress(i/n_pers*100)
                i += 1.0
                #plans.set_row(id_plan, ids_person = id_person, ids_strategy = self.get_id_strategy())

                #times_est_plan[id_plan] = time-time_start
                # map_times[id_person] = self.plan_activity(\
                #                id_person, id_plan, time_from,
                #                id_act_from, id_act_to,
                #                name_acttype_to, duration_act_to,
                #                id_veh,
                #                id_edge_from, pos_edge_from,
                #                id_parking_from, id_edge_parking_from, pos_edge_parking_from,
                #                id_parking_to, id_edge_parking_to, pos_edge_parking_to,
                #                id_edge_to, pos_edge_to, edges.ids_lanes[id_edge_to][0])

                # start creating stages for activity
                id_stage_walk1, time = walkstages.append_stage(
                    id_plan, time_from,
                    id_edge_from=id_edge_from,
                    position_edge_from=pos_edge_from,
                    id_edge_to=id_edge_parking_from,
                    position_edge_to=pos_edge_parking_from-1.5,  # wait 1.5 m before nose of parked car
                )

                # ride from  car parking to road edge near activity
                id_stage_car, time = ridestages.append_stage(
                    id_plan, time,
                    id_veh=id_veh,
                    # delay to be sure that person arrived!(workaround in combination with parking=False)
                    time_init=time+30,  # time_from,
                    id_parking_from=id_parking_from,
                    id_parking_to=id_parking_to,
                    # TODO: here we could use id_edge_to as via edge to emulate search for parking
                )
                if id_stage_car >= 0:
                    # print '  car ride successful'
                    id_stage_walk2, time = walkstages.append_stage(
                        id_plan, time,
                        id_edge_from=id_edge_parking_to,
                        position_edge_from=pos_edge_parking_to-1.5,  # ecessary?
                        id_edge_to=id_edge_to,
                        position_edge_to=pos_edge_to,
                    )
                else:
                    # print '  parking not connected or distance too short, modify first walk and go directly to activity'
                    # print '  id_stage_walk1',id_stage_walk1,type(id_stage_walk1)
                    # print '  id_edge_from',id_edge_from
                    # print '  position_edge_from',position_edge_from
                    # print '  id_edge_to',id_edge_to
                    # print '  position_edge_to',position_edge_to

                    time = walkstages.modify_stage(
                        id_stage_walk1, time_from,
                        id_edge_from=id_edge_from,
                        position_edge_from=pos_edge_from,
                        id_edge_to=id_edge_to,
                        position_edge_to=pos_edge_to,
                    )

                # store time estimation for this plan
                # note that these are the travel times, no activity time
                plans.times_est[id_plan] += time-time_from

                # define current end time without last activity duration
                plans.times_end[id_plan] = time

                # finally add activity and respective duration

                id_stage_act, time = activitystages.append_stage(
                    id_plan, time,
                    ids_activity=id_act_to,
                    names_activitytype=name_acttype_to,
                    durations=duration_act_to,
                    ids_lane=edges.ids_lanes[id_edge_to][0],
                    positions=pos_edge_to,
                )

                map_times[id_person] = time
                # return time
                ##

            # select persons and activities for next setp
            ind_act += 1
            ids_person_act, ids_act_from, ids_act_to\
                = virtualpop.get_activities_from_pattern(ind_act, ids_person=ids_person_act)
            # update timing with (random) activity duration!!

        return True

    def plan_activity(self, id_person, id_plan, time_start,
                      id_act_from, id_act_to,
                      name_acttype_to, duration_act_to,
                      id_veh,
                      id_edge_from, pos_edge_from,
                      id_parking_from, id_edge_parking_from, pos_edge_parking_from,
                      id_parking_to, id_edge_parking_to, pos_edge_parking_to,
                      id_edge_to, pos_edge_to, id_lane_to):
        print 79*'_'
        print '  id_plan=%d, id_person=%d, ids_veh=%d' % (id_plan, id_person,  id_veh)

        plans = self.get_virtualpop().get_plans()
        #stagetables = virtualpop.get_stagetables()
        walkstages = plans.get_stagetable('walks')
        ridestages = plans.get_stagetable('autorides')
        activitystages = plans.get_stagetable('activities')

        id_stage_walk1, time = walkstages.append_stage(
            id_plan, time_start,
            id_edge_from=id_edge_from,
            position_edge_from=pos_edge_from,
            id_edge_to=id_edge_parking_from,
            position_edge_to=pos_edge_parking_from-1.5,  # wait 1.5 m before nose of parked car
        )

        # ride from  car parking to road edge near activity
        id_stage_car, time = ridestages.append_stage(
            id_plan, time,
            id_veh=id_veh,
            # delay to be sure that person arrived!(workaround in combination with parking=False)
            time_init=time+30,  # time_start,
            id_parking_from=id_parking_from,
            id_parking_to=id_parking_to,
            # TODO: here we could use id_edge_to as via edge to emulate search for parking
        )
        if id_stage_car >= 0:
                # print '  car ride successful'
            id_stage_walk2, time = walkstages.append_stage(
                id_plan, time,
                id_edge_from=id_edge_parking_to,
                position_edge_from=pos_edge_parking_to-1.5,  # ecessary?
                id_edge_to=id_edge_to,
                position_edge_to=pos_edge_to,
            )
        else:
            # print '  parking not connected or distance too short, modify first walk and go directly to activity'
            # print '  id_stage_walk1',id_stage_walk1,type(id_stage_walk1)
            # print '  id_edge_from',id_edge_from
            # print '  position_edge_from',position_edge_from
            # print '  id_edge_to',id_edge_to
            # print '  position_edge_to',position_edge_to

            time = walkstages.modify_stage(
                id_stage_walk1, time_start,
                id_edge_from=id_edge_from,
                position_edge_from=pos_edge_from,
                id_edge_to=id_edge_to,
                position_edge_to=pos_edge_to,
            )

        # store time estimation for this plan
        # note that these are the travel times, no activity time
        plans.times_est[id_plan] += time-time_start

        # define current end time without last activity duration
        plans.times_end[id_plan] = time

        # finally add activity and respective duration
        id_stage_act, time = activitystages.append_stage(
            id_plan, time,
            ids_activity=id_act_to,
            names_activitytype=name_acttype_to,
            durations=duration_act_to,
            ids_lane=id_lane_to,
            positions=pos_edge_to,
        )

        return time


class BikeStrategy(StrategyMixin):
    def __init__(self, ident, parent=None,
                 name='Bike strategy',
                 info='With this strategy, the person uses his private bike as main transport mode.',
                 **kwargs):

        self._init_objman(ident, parent, name=name, info=info, **kwargs)
        attrsman = self.set_attrsman(cm.Attrsman(self))
        # specific init
        self._init_attributes(**kwargs)
        self._init_constants()

    def _init_attributes(self, **kwargs):
        # print 'StrategyMixin._init_attributes'
        attrsman = self.get_attrsman()

        self._init_attributes_strategy(**kwargs)
        self.n_iter_bikeacces_max = attrsman.add(cm.AttrConf('n_iter_bikeacces_max', kwargs.get('n_iter_bikeacces_max', 5),
                                                             groupnames=['options'],
                                                             perm='rw',
                                                             name='Max. bike access search iterations',
                                                             info='Max. number of iterations while searching an edge with bike access.',
                                                             ))

        self.length_edge_min = attrsman.add(cm.AttrConf('length_edge_min', kwargs.get('length_edge_min', 20.0),
                                                        groupnames=['options'],
                                                        perm='rw',
                                                        name='Min. edge length search',
                                                        unit='m',
                                                        info='Min. edge length when searching an edge with bike access.',
                                                        ))

    def _init_constants(self):
        #virtualpop = self.get_virtualpop()
        #stagetables = virtualpop.get_stagetables()

        #self._walkstages = stagetables.get_stagetable('walks')
        #self._ridestages = stagetables.get_stagetable('rides')
        #self._activitystages = stagetables.get_stagetable('activities')

        #self._plans = virtualpop.get_plans()
        #
        # print 'AutoStrategy._init_constants'
        # print dir(self)
        # self.get_attrsman().do_not_save_attrs(['_activitystages','_ridestages','_walkstages','_plans'])

        modes = self.get_virtualpop().get_scenario().net.modes
        self._id_mode_ped = modes.get_id_mode('pedestrian')
        self._id_mode_bike = modes.get_id_mode('bicycle')
        self._id_mode_auto = modes.get_id_mode('passenger')
        self._id_mode_moto = modes.get_id_mode('motorcycle')
        self._edges = self.get_virtualpop().get_scenario().net.edges
        self.get_attrsman().do_not_save_attrs([
            '_id_mode_bike', '_id_mode_auto', '_id_mode_moto',
            '_id_mode_ped',
            '_edges'])

    def preevaluate(self, ids_person):
        """
        Preevaluation strategies for person IDs in vector ids_person.

        Returns a preevaluation vector with a preevaluation value 
        for each person ID. The values of the preevaluation vector are as follows:
            -1 : Strategy cannot be applied
            0  : Stategy can be applied, but the preferred mode is not used
            1  : Stategy can be applied, and preferred mode is part of the strategy
            2  : Strategy uses predomunantly preferred mode

        """
        n_pers = len(ids_person)
        print 'BikeStrategy.preevaluate', n_pers, 'persons'
        persons = self.get_virtualpop()
        preeval = np.zeros(n_pers, dtype=np.int32)

        # put -1 for persons without car access
        preeval[persons.ids_ibike[ids_person] == -1] = -1
        print '  persons having no bike', len(np.flatnonzero(persons.ids_ibike[ids_person] == -1))

        # put 0 for persons with bike but with a different preferred mode
        preeval[(persons.ids_ibike[ids_person] > -1)
                & (persons.ids_mode_preferred[ids_person] != self._id_mode_bike)] = 0

        print '  persons with bike but with a different preferred mode', len(np.flatnonzero(
            (persons.ids_ibike[ids_person] > -1) & (persons.ids_mode_preferred[ids_person] != self._id_mode_bike)))

        # put 2 for persons with bike access and who prefer the bikr
        preeval[(persons.ids_ibike[ids_person] > -1)
                & (persons.ids_mode_preferred[ids_person] == self._id_mode_auto)] = 2
        print '  persons  with car access and who prefer the car', len(np.flatnonzero(
            (persons.ids_ibike[ids_person] > -1) & (persons.ids_mode_preferred[ids_person] == self._id_mode_bike)))

        return preeval

    def get_edge_bikeaccess(self, id_edge, is_search_backward=False):

        # print 'get_edge_bikeaccess',id_edge, is_search_backward,'id_sumo',self._edges.ids_sumo[id_edge]
        id_mode = self._id_mode_bike
        id_mode_ped = self._id_mode_ped
        get_accesslevel = self._edges.get_accesslevel

        if is_search_backward:
            get_next = self._edges.get_incoming
        else:
            get_next = self._edges.get_outgoing

        edgelengths = self._edges.lengths
        #ids_tried = set()
        ids_current = [id_edge]
        id_bikeedge = -1
        pos = 0.0
        n = 0
        while (id_bikeedge < 0) & (n < self.n_iter_bikeacces_max):
            n += 1
            ids_new = []
            for id_edge_test, is_long_enough in zip(ids_current, edgelengths[ids_current] > self.length_edge_min):
                # print '    check id',id_edge_test, is_long_enough,get_accesslevel(id_edge_test, id_mode)
                if is_long_enough & (get_accesslevel(id_edge_test, id_mode) >= 0) & (get_accesslevel(id_edge_test, id_mode_ped) >= 0):
                    id_bikeedge = id_edge_test
                    # print '    found',id_bikeedge,self._edges.ids_sumo[id_bikeedge]
                    break
                else:
                    ids_new += get_next(id_edge_test)

            ids_current = ids_new

        if id_bikeedge > -1:
            if is_search_backward:
                pos = edgelengths[id_bikeedge]-0.5*self.length_edge_min
            else:
                pos = 0.5*self.length_edge_min

        if id_bikeedge == -1:
            print 'WARNING in get_edge_bikeaccess no access for', id_edge, self._edges.ids_sumo[id_edge]
        return id_bikeedge, pos

    def plan_bikeride(self, id_plan, time_from, id_veh,
                      id_edge_from, pos_edge_from,
                      id_edge_to, pos_edge_to,
                      dist_from_to, dist_walk_max,
                      walkstages, ridestages):

        # start creating stages
        id_stage_walk1 = -1
        id_stage_bike = -1

        id_edge_from_bike, pos_from_bike = self.get_edge_bikeaccess(id_edge_from)
        id_edge_to_bike, pos_to_bike = self.get_edge_bikeaccess(id_edge_to, is_search_backward=True)

        if (dist_from_to < dist_walk_max) | (id_edge_from_bike == -1) | (id_edge_to_bike == -1):
            # print '    go by foot because distance is too short or no bike access',dist_from_to,id_edge_from_bike,id_edge_to_bike
            id_stage_walk1, time = walkstages.append_stage(
                id_plan, time_from,
                id_edge_from=id_edge_from,
                position_edge_from=pos_edge_from,
                id_edge_to=id_edge_to,
                position_edge_to=pos_edge_to,
            )

        else:
            # print '    try to take the bike',id_veh
            # print '    id_edge_from_bike',edges.ids_sumo[id_edge_from_bike],pos_from_bike
            # print '    id_edge_to_bike',edges.ids_sumo[id_edge_to_bike],pos_to_bike

            if id_edge_from_bike != id_edge_from:
                # print '    must walk from origin to bikerack',time_from

                id_stage_walk1, time = walkstages.append_stage(
                    id_plan, time_from,
                    id_edge_from=id_edge_from,
                    position_edge_from=pos_edge_from,
                    id_edge_to=id_edge_from_bike,
                    position_edge_to=pos_from_bike,
                )

                if id_edge_to_bike != id_edge_to:
                    # print '    must cycle from bikerack to dest bike rack',time
                    id_stage_bike, time = ridestages.append_stage(
                        id_plan, time,
                        id_veh=id_veh,
                        # delay to be sure that person arrived!(workaround in combination with parking=False)
                        time_init=time-10,  # time_from,
                        id_edge_from=id_edge_from_bike,
                        position_edge_from=pos_from_bike,
                        id_edge_to=id_edge_to_bike,
                        position_edge_to=pos_to_bike,
                    )
                    if id_stage_bike > -1:
                        # print '    must walk from dest bikerack to dest',time
                        id_stage_walk2, time = walkstages.append_stage(
                            id_plan, time,
                            id_edge_from=id_edge_to_bike,
                            position_edge_from=pos_to_bike,
                            id_edge_to=id_edge_to,
                            position_edge_to=pos_edge_to,
                        )

                else:
                    # print '    cycle from bikerack to destination',time
                    id_stage_bike, time = ridestages.append_stage(
                        id_plan, time,
                        id_veh=id_veh,
                        # delay to be sure that person arrived!(workaround in combination with parking=False)
                        time_init=time-10,  # time_from,
                        id_edge_from=id_edge_from_bike,
                        position_edge_from=pos_from_bike,
                        id_edge_to=id_edge_to,
                        position_edge_to=pos_edge_to,
                    )
            else:
                # print '    cycle directly from orign edge',time_from
                if id_edge_to_bike != id_edge_to:
                    # print '    must cycle from origin to bikerack',time_from

                    #pos_to_bike = 0.1*edges.lengths[id_edge_to_bike]
                    id_stage_bike, time = ridestages.append_stage(
                        id_plan, time_from,
                        id_veh=id_veh,
                        # delay to be sure that person arrived!(workaround in combination with parking=False)
                        time_init=time_from-10,  # time_from,
                        id_edge_from=id_edge_from,
                        position_edge_from=pos_edge_from,
                        id_edge_to=id_edge_to_bike,
                        position_edge_to=pos_to_bike,
                    )
                    if id_stage_bike > -1:
                        id_stage_walk2, time = walkstages.append_stage(
                            id_plan, time,
                            id_edge_from=id_edge_to_bike,
                            position_edge_from=pos_to_bike,
                            id_edge_to=id_edge_to,
                            position_edge_to=pos_edge_to,
                        )
                else:
                    # print '    must cycle from origin to destination',time_from
                    id_stage_bike, time = ridestages.append_stage(
                        id_plan, time_from,
                        id_veh=id_veh,
                        # delay to be sure that person arrived!(workaround in combination with parking=False)
                        time_init=time_from-10,  # time_from,
                        id_edge_from=id_edge_from,
                        position_edge_from=pos_edge_from,
                        id_edge_to=id_edge_to,
                        position_edge_to=pos_edge_to,
                    )

            # here we should have created a bike ride
            # if not, for ehatever reason,
            # we walk from origin to destination
            if id_stage_bike == -1:
                # print '    walk because no ride stage has been planned',time_from
                if id_stage_walk1 == -1:
                    # no walk stage has been planned
                    id_stage_walk1, time = walkstages.append_stage(
                        id_plan, time_from,
                        id_edge_from=id_edge_from,
                        position_edge_from=pos_edge_from,
                        id_edge_to=id_edge_to,
                        position_edge_to=pos_edge_to,
                    )

                elif time_from == time:
                    # walking to bike has already been schedules,
                    # but cycling failed. So walk the whole way
                    time = walkstages.modify_stage(
                        id_stage_walk1, time_from,
                        id_edge_from=id_edge_from,
                        position_edge_from=pos_edge_from,
                        id_edge_to=id_edge_to,
                        position_edge_to=pos_edge_to,
                    )
        return time

    def plan(self, ids_person, logger=None):
        """
        Generates a plan for these person according to this strategie.
        Overriden by specific strategy.
        """
        #make_plans_private(self, ids_person = None, mode = 'passenger')
        # routing necessary?
        virtualpop = self.get_virtualpop()
        plans = virtualpop.get_plans()  # self._plans
        walkstages = plans.get_stagetable('walks')
        ridestages = plans.get_stagetable('bikerides')
        activitystages = plans.get_stagetable('activities')

        activities = virtualpop.get_activities()
        activitytypes = virtualpop.get_demand().activitytypes
        landuse = virtualpop.get_landuse()
        facilities = landuse.facilities
        #parking = landuse.parking

        scenario = virtualpop.get_scenario()
        edges = scenario.net.edges
        lanes = scenario.net.lanes
        modes = scenario.net.modes

        #times_est_plan = plans.times_est

        # here we can determine edge weights for different modes
        plans.prepare_stagetables(['walks', 'bikerides', 'activities'])

        # get initial travel times for persons.
        # initial travel times depend on the initial activity

        # landuse.parking.clear_booking()

        ids_person_act, ids_act_from, ids_act_to\
            = virtualpop.get_activities_from_pattern(0, ids_person=ids_person)

        if len(ids_person_act) == 0:
            print 'WARNING in BikeStrategy.plan: no eligible persons found.'
            return False

        # ok

        # temporary maps from ids_person to other parameters
        nm = np.max(ids_person_act)+1
        map_ids_plan = np.zeros(nm, dtype=np.int32)
        map_ids_plan[ids_person_act] = virtualpop.add_plans(ids_person_act, id_strategy=self.get_id_strategy())

        map_times = np.zeros(nm, dtype=np.int32)
        map_times[ids_person_act] = activities.get_times_end(ids_act_from, pdf='unit')

        # set start time to plans (important!)
        plans.times_begin[map_ids_plan[ids_person_act]] = map_times[ids_person_act]

        map_ids_fac_from = np.zeros(nm, dtype=np.int32)
        map_ids_fac_from[ids_person_act] = activities.ids_facility[ids_act_from]

        #map_ids_parking_from = np.zeros(nm, dtype = np.int32)
        # ids_parking_from, inds_vehparking = parking.get_closest_parkings( virtualpop.ids_iauto[ids_person_act],
        # facilities.centroids[activities.ids_facility[ids_act_from]])
        # if len(ids_parking_from)==0:
        #    return False

        # err

        #map_ids_parking_from[ids_person_act] = ids_parking_from

        n_plans = len(ids_person_act)
        print 'BikeStrategy.plan n_plans=', n_plans
        # print '  map_ids_parking_from[ids_person_act].shape',map_ids_parking_from[ids_person_act].shape
        # set initial activity
        # this is because the following steps start with travel
        # and set the next activity
        #names_acttype_from = activitytypes.names[activities.ids_activitytype[ids_act_from]]
        # for id_plan

        ind_act = 0

        # make initial activity stage
        ids_edge_from = facilities.ids_roadedge_closest[map_ids_fac_from[ids_person_act]]

        poss_edge_from = facilities.positions_roadedge_closest[map_ids_fac_from[ids_person_act]]
        poss_edge_from = self.clip_positions(poss_edge_from, ids_edge_from)

        # this is the time when first activity starts
        # first activity is normally not simulated

        names_acttype_from = activitytypes.names[activities.ids_activitytype[ids_act_from]]
        durations_act_from = activities.get_durations(ids_act_from)
        times_from = map_times[ids_person_act]-durations_act_from
        #times_from = activities.get_times_end(ids_act_from, pdf = 'unit')

        # do initial stage
        # could be common to all strategies
        for id_plan,\
            time,\
            id_act_from,\
            name_acttype_from,\
            duration_act_from,\
            id_edge_from,\
            pos_edge_from  \
            in zip(map_ids_plan[ids_person_act],
                   times_from,
                   ids_act_from,
                   names_acttype_from,
                   durations_act_from,
                   ids_edge_from,
                   poss_edge_from):

            id_stage_act, time = activitystages.append_stage(
                id_plan, time,
                ids_activity=id_act_from,
                names_activitytype=name_acttype_from,
                durations=duration_act_from,
                ids_lane=edges.ids_lanes[id_edge_from][0],
                positions=pos_edge_from,
            )

        # main loop while there are persons performing
        # an activity at index ind_act
        while len(ids_person_act) > 0:
            ids_plan = map_ids_plan[ids_person_act]
            ids_veh = virtualpop.ids_ibike[ids_person_act]
            dists_walk_max = virtualpop.dists_walk_max[ids_person_act]
            times_from = map_times[ids_person_act]

            names_acttype_to = activitytypes.names[activities.ids_activitytype[ids_act_to]]
            durations_act_to = activities.get_durations(ids_act_to)

            ids_fac_from = map_ids_fac_from[ids_person_act]
            ids_fac_to = activities.ids_facility[ids_act_to]

            # origin edge and position
            ids_edge_from = facilities.ids_roadedge_closest[ids_fac_from]
            poss_edge_from = facilities.positions_roadedge_closest[ids_fac_from]
            poss_edge_from = self.clip_positions(poss_edge_from, ids_edge_from)

            centroids_from = facilities.centroids[ids_fac_from]

            # this method will find and occupy parking space
            #ids_parking_from = map_ids_parking_from[ids_person_act]

            # print '  ids_veh.shape',ids_veh.shape
            # print '  centroids_to.shape',centroids_to.shape
            #ids_parking_to, inds_vehparking = parking.get_closest_parkings(ids_veh, centroids_to)

            #ids_lane_parking_from = parking.ids_lane[ids_parking_from]
            #ids_edge_parking_from = lanes.ids_edge[ids_lane_parking_from]
            #poss_edge_parking_from = parking.positions[ids_parking_from]

            # print '  ids_parking_to.shape',ids_parking_to.shape
            # print '  np.max(parking.get_ids()), np.max(ids_parking_to)',np.max(parking.get_ids()), np.max(ids_parking_to)
            #ids_lane_parking_to = parking.ids_lane[ids_parking_to]
            #ids_edge_parking_to = lanes.ids_edge[ids_lane_parking_to]
            #poss_edge_parking_to = parking.positions[ids_parking_to]

            # destination edge and position
            ids_edge_to = facilities.ids_roadedge_closest[ids_fac_to]

            poss_edge_to1 = facilities.positions_roadedge_closest[ids_fac_to]
            poss_edge_to = self.clip_positions(poss_edge_to1, ids_edge_to)
            centroids_to = facilities.centroids[ids_fac_to]

            # debug poscorrection..OK
            # for id_edge, id_edge_sumo, length, pos_to1, pos in zip(ids_edge_to, edges.ids_sumo[ids_edge_to],edges.lengths[ids_edge_to],poss_edge_to1, poss_edge_to):
            #    print '  ',id_edge, 'IDe%s'%id_edge_sumo, 'L=%.2fm'%length, '%.2fm'%pos_to1, '%.2fm'%pos

            dists_from_to = np.sqrt(np.sum((centroids_to - centroids_from)**2, 1))

            i = 0.0
            n_pers = len(ids_person_act)
            for id_person, id_plan, time_from, id_act_from, id_act_to, name_acttype_to, duration_act_to, id_veh, id_edge_from, pos_edge_from, id_edge_to, pos_edge_to, dist_from_to, dist_walk_max\
                    in zip(ids_person_act, ids_plan, times_from, ids_act_from, ids_act_to, names_acttype_to, durations_act_to, ids_veh, ids_edge_from, poss_edge_from, ids_edge_to, poss_edge_to, dists_from_to, dists_walk_max):
                if logger:
                    logger.progress(i/n_pers*100)
                i += 1.0
                print 79*'*'
                print '  plan id_plan', id_plan, 'time_from', time_from, 'from', id_edge_from, pos_edge_from, 'to', id_edge_to, pos_edge_to
                print '  id_edge_from', edges.ids_sumo[id_edge_from], 'id_edge_to', edges.ids_sumo[id_edge_to]

                time = self.plan_bikeride(id_plan, time_from, id_veh,
                                          id_edge_from, pos_edge_from,
                                          id_edge_to, pos_edge_to,
                                          dist_from_to, dist_walk_max,
                                          walkstages, ridestages)

                ################

                # store time estimation for this plan
                # note that these are the travel times, no activity time
                plans.times_est[id_plan] += time-time_from

                # define current end time without last activity duration
                plans.times_end[id_plan] = time

                # finally add activity and respective duration

                id_stage_act, time = activitystages.append_stage(
                    id_plan, time,
                    ids_activity=id_act_to,
                    names_activitytype=name_acttype_to,
                    durations=duration_act_to,
                    ids_lane=edges.ids_lanes[id_edge_to][0],
                    positions=pos_edge_to,
                )

                map_times[id_person] = time
                # return time
                ##

            # select persons and activities for next setp
            ind_act += 1
            ids_person_act, ids_act_from, ids_act_to\
                = virtualpop.get_activities_from_pattern(ind_act, ids_person=ids_person_act)
            # update timing with (random) activity duration!!

        return True


class TransitStrategy(StrategyMixin):
    def __init__(self, ident, parent=None,
                 name='Public Transport Strategy',
                 info='With this strategy, the person uses public transport.',
                 **kwargs):

        self._init_objman(ident, parent, name=name, info=info, **kwargs)
        attrsman = self.set_attrsman(cm.Attrsman(self))
        # specific init
        self._init_attributes()
        self._init_constants()

    def _init_attributes(self):
        # print 'StrategyMixin._init_attributes'
        pass

    def _init_constants(self):
        #virtualpop = self.get_virtualpop()
        #stagetables = virtualpop.get_stagetables()

        #self._walkstages = stagetables.get_stagetable('walks')
        #self._ridestages = stagetables.get_stagetable('rides')
        #self._activitystages = stagetables.get_stagetable('activities')

        #self._plans = virtualpop.get_plans()
        #
        # print 'AutoStrategy._init_constants'
        # print dir(self)
        # self.get_attrsman().do_not_save_attrs(['_activitystages','_ridestages','_walkstages','_plans'])

        modes = self.get_virtualpop().get_scenario().net.modes
        self._id_mode_bike = modes.get_id_mode('bicycle')
        self._id_mode_auto = modes.get_id_mode('passenger')
        self._id_mode_moto = modes.get_id_mode('motorcycle')
        self._id_mode_bus = modes.get_id_mode('bus')
        self.get_attrsman().do_not_save_attrs([
            '_id_mode_bike', '_id_mode_auto', '_id_mode_moto', '_id_mode_bus'
        ])

    def preevaluate(self, ids_person):
        """
        Preevaluation strategies for person IDs in vector ids_person.

        Returns a preevaluation vector with a preevaluation value 
        for each person ID. The values of the preevaluation vector are as follows:
            -1 : Strategy cannot be applied
            0  : Stategy can be applied, but the preferred mode is not used
            1  : Stategy can be applied, and preferred mode is part of the strategy
            2  : Strategy uses predomunantly preferred mode

        """
        n_pers = len(ids_person)
        persons = self.get_virtualpop()
        preeval = np.zeros(n_pers, dtype=np.int32)

        # TODO: here we could exclude by age or distance facilities-stops

        # put 0 for persons whose preference is not public transport
        preeval[persons.ids_mode_preferred[ids_person] != self._id_mode_bus] = 0

        # put 2 for persons with car access and who prefer cars
        preeval[persons.ids_mode_preferred[ids_person] == self._id_mode_bus] = 2

        print '  TransitStrategy.preevaluate', len(np.flatnonzero(preeval))
        return preeval

    def plan_transit(self, id_plan, time_from,
                     id_edge_from, pos_edge_from,
                     id_edge_to, pos_edge_to,
                     id_stop_from, id_stopedge_from, pos_stop_from,
                     id_stop_to, id_stopedge_to, pos_stop_to,
                     #dist_from_to, dist_walk_max,
                     walkstages, transitstages,
                     ptlines, ptfstar, pttimes,
                     stops_to_enter, stops_to_exit,
                     ids_laneedge, ids_stoplane, ptstops):

        ptlinks = ptlines.get_ptlinks()
        ptlinktypes = ptlinks.types.choices
        type_enter = ptlinktypes['enter']
        type_transit = ptlinktypes['transit']
        type_board = ptlinktypes['board']
        type_alight = ptlinktypes['alight']
        type_transfer = ptlinktypes['transfer']
        type_walk = ptlinktypes['walk']
        type_exit = ptlinktypes['exit']

        if (id_edge_from == id_stopedge_from) & (abs(pos_edge_from-pos_stop_from) < 1.0):
            time = time_from
            id_stage_walk1 = None
        else:
            id_stage_walk1, time = walkstages.append_stage(id_plan, time_from,
                                                           id_edge_from=id_edge_from,
                                                           position_edge_from=pos_edge_from,
                                                           id_edge_to=id_stopedge_from,
                                                           position_edge_to=pos_stop_from,  # -7.0,
                                                           )

        # print '    id_stopedge_from',id_stopedge_from
        # print '    pos_stop_from',pos_stop_from

        # print
        # print '    id_stopedge_to',id_stopedge_to
        # print '    pos_stop_to',pos_stop_to
        # print
        # print '    id_stop_from',id_stop_from
        # print '    id_stop_to',id_stop_to

        durations, linktypes, ids_line, ids_fromstop, ids_tostop =\
            ptlinks.route(id_stop_from, id_stop_to,
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

                print '    id_stopedge_from,id_stopedge_to', id_stopedge_from, id_stopedge_to
                if linktype == type_transit:  # transit!
                    print '    add transit'
                    id_stage_transit, time = transitstages.append_stage(
                        id_plan, time,
                        id_line=id_line,
                        duration=duration+duration_wait,
                        id_fromedge=id_stopedge_from,
                        id_toedge=id_stopedge_to,
                    )
                    duration_wait = 0.0

                elif linktype == type_walk:  # walk to transfer
                    print '    add transfer'
                    id_stage_transfer, time = walkstages.append_stage(
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
                    print '    add duration', duration
                    duration_wait += duration

            # walk from final stop to activity
            # print '    Stage for linktype %2d fromedge %s toedge %s'%(linktype, edges.ids_sumo[id_stopedge_to],edges.ids_sumo[id_edge_to] )
            # if (id_edge_to == id_stopedge_to)&(abs(pos_edge_to-pos_tostop)<1.0):
            #    print '  already at right edge and position'
            #    pass
            # else:
            id_stage_walk2, time = walkstages.append_stage(id_plan, time,
                                                           id_edge_from=id_stopedge_to,
                                                           position_edge_from=pos_tostop,
                                                           id_edge_to=id_edge_to,
                                                           position_edge_to=pos_edge_to,
                                                           )

        else:
            # there is no public transport line linking these nodes.

            if id_stage_walk1 is None:
                # Create first walk directly from home to activity
                id_stage_walk1, time = walkstages.append_stage(id_plan,
                                                               time_from,
                                                               id_edge_from=id_edge_from,
                                                               position_edge_from=pos_edge_from,
                                                               id_edge_to=id_edge_to,
                                                               position_edge_to=pos_edge_to,
                                                               )
            else:
                # Modify first walk directly from home to activity
                time = walkstages.modify_stage(id_stage_walk1, time_from,
                                               id_edge_from=id_edge_from,
                                               position_edge_from=pos_edge_from,
                                               id_edge_to=id_edge_to,
                                               position_edge_to=pos_edge_to,
                                               )

        return time

    def plan(self, ids_person, logger=None):
        """
        Generates a plan for these person according to this strategie.
        Overriden by specific strategy.
        """
        print 'TransitStrategy.plan', len(ids_person)
        #make_plans_private(self, ids_person = None, mode = 'passenger')
        # routing necessary?
        virtualpop = self.get_virtualpop()
        plans = virtualpop.get_plans()  # self._plans
        demand = virtualpop.get_demand()
        ptlines = demand.ptlines

        walkstages = plans.get_stagetable('walks')
        transitstages = plans.get_stagetable('transits')
        activitystages = plans.get_stagetable('activities')

        activities = virtualpop.get_activities()
        activitytypes = demand.activitytypes
        landuse = virtualpop.get_landuse()
        facilities = landuse.facilities
        parking = landuse.parking

        scenario = virtualpop.get_scenario()
        net = scenario.net
        edges = net.edges
        lanes = net.lanes
        modes = net.modes

        ptstops = net.ptstops

        # print '   demand',demand
        # print '   demand.ptlines',demand.ptlines,dir(demand.ptlines)
        # print '   demand.ptlines.get_ptlinks()',demand.ptlines.get_ptlinks()
        # print '   demand.virtualpop',demand.virtualpop,dir(demand.virtualpop)
        # print '   demand.trips',demand.trips,dir(demand.trips)
        if len(ptlines) == 0:
            print 'WARNING in TrasitStrategy.plan: no transit services available. Create public trasit services by connecting stops.'
            return False

        ptlinks = ptlines.get_ptlinks()
        if len(ptlinks) == 0:
            print 'WARNING in TrasitStrategy.plan: no public transport links. Run methods: "create routes" and "build links" from public trasport services.'
            return False

        ptlinktypes = ptlinks.types.choices

        ptfstar = ptlinks.get_fstar()
        pttimes = ptlinks.get_times()
        stops_to_enter, stops_to_exit = ptlinks.get_stops_to_enter_exit()

        ids_stoplane = ptstops.ids_lane
        ids_laneedge = net.lanes.ids_edge

        times_est_plan = plans.times_est
        # here we can determine edge weights for different modes

        # this could be centralized to avoid redundance
        plans.prepare_stagetables(['walks', 'transits', 'activities'])

        ids_person_act, ids_act_from, ids_act_to\
            = virtualpop.get_activities_from_pattern(0, ids_person=ids_person)

        if len(ids_person_act) == 0:
            print 'WARNING in TrasitStrategy.plan: no eligible persons found.'
            return False

        # temporary maps from ids_person to other parameters
        nm = np.max(ids_person_act)+1
        map_ids_plan = np.zeros(nm, dtype=np.int32)
        #ids_plan_act = virtualpop.add_plans(ids_person_act, id_strategy = self.get_id_strategy())
        map_ids_plan[ids_person_act] = virtualpop.add_plans(ids_person_act, id_strategy=self.get_id_strategy())

        map_times = np.zeros(nm, dtype=np.int32)
        map_times[ids_person_act] = activities.get_times_end(ids_act_from, pdf='unit')

        # set start time to plans (important!)
        plans.times_begin[map_ids_plan[ids_person_act]] = map_times[ids_person_act]

        map_ids_fac_from = np.zeros(nm, dtype=np.int32)
        map_ids_fac_from[ids_person_act] = activities.ids_facility[ids_act_from]

        n_plans = len(ids_person_act)
        print 'TrasitStrategy.plan n_plans=', n_plans

        # make initial activity stage
        ids_edge_from = facilities.ids_roadedge_closest[map_ids_fac_from[ids_person_act]]
        poss_edge_from = facilities.positions_roadedge_closest[map_ids_fac_from[ids_person_act]]
        # this is the time when first activity starts
        # first activity is normally not simulated

        names_acttype_from = activitytypes.names[activities.ids_activitytype[ids_act_from]]
        durations_act_from = activities.get_durations(ids_act_from)
        times_from = map_times[ids_person_act]-durations_act_from
        #times_from = activities.get_times_end(ids_act_from, pdf = 'unit')

        for id_plan,\
            time,\
            id_act_from,\
            name_acttype_from,\
            duration_act_from,\
            id_edge_from,\
            pos_edge_from  \
            in zip(map_ids_plan[ids_person_act],
                   times_from,
                   ids_act_from,
                   names_acttype_from,
                   durations_act_from,
                   ids_edge_from,
                   poss_edge_from):

            id_stage_act, time = activitystages.append_stage(
                id_plan, time,
                ids_activity=id_act_from,
                names_activitytype=name_acttype_from,
                durations=duration_act_from,
                ids_lane=edges.ids_lanes[id_edge_from][0],
                positions=pos_edge_from,
            )

        ##

        ind_act = 0

        # main loop while there are persons performing
        # an activity at index ind_act
        while len(ids_person_act) > 0:
            ids_plan = map_ids_plan[ids_person_act]

            times_from = map_times[ids_person_act]

            names_acttype_to = activitytypes.names[activities.ids_activitytype[ids_act_to]]
            durations_act_to = activities.get_durations(ids_act_to)

            ids_fac_from = map_ids_fac_from[ids_person_act]
            ids_fac_to = activities.ids_facility[ids_act_to]

            centroids_from = facilities.centroids[ids_fac_from]
            centroids_to = facilities.centroids[ids_fac_to]

            # origin edge and position
            ids_edge_from = facilities.ids_roadedge_closest[ids_fac_from]
            poss_edge_from = facilities.positions_roadedge_closest[ids_fac_from]

            # destination edge and position
            ids_edge_to = facilities.ids_roadedge_closest[ids_fac_to]
            poss_edge_to = facilities.positions_roadedge_closest[ids_fac_to]

            ids_stop_from = ptstops.get_closest(centroids_from)
            ids_stop_to = ptstops.get_closest(centroids_to)

            ids_stopedge_from = ids_laneedge[ids_stoplane[ids_stop_from]]
            ids_stopedge_to = ids_laneedge[ids_stoplane[ids_stop_to]]

            # do random pos here
            poss_stop_from = 0.5*(ptstops.positions_from[ids_stop_from]
                                  + ptstops.positions_to[ids_stop_from])

            poss_stop_to = 0.5*(ptstops.positions_from[ids_stop_to]
                                + ptstops.positions_to[ids_stop_to])

            i = 0.0
            for id_person, id_plan, time_from, id_act_from, id_act_to, name_acttype_to, duration_act_to, id_edge_from, pos_edge_from, id_edge_to, pos_edge_to, id_stop_from, id_stopedge_from, pos_stop_from, id_stop_to, id_stopedge_to, pos_stop_to\
                    in zip(ids_person_act, ids_plan, times_from, ids_act_from, ids_act_to, names_acttype_to, durations_act_to,  ids_edge_from, poss_edge_from, ids_edge_to, poss_edge_to, ids_stop_from, ids_stopedge_from, poss_stop_from, ids_stop_to, ids_stopedge_to, poss_stop_to):
                n_pers = len(ids_person_act)
                if logger:
                    logger.progress(i/n_pers*100)
                i += 1.0
                print 79*'_'
                print '  id_plan=%d, id_person=%d, ' % (id_plan, id_person)

                time = self.plan_transit(id_plan, time_from,
                                         id_edge_from, pos_edge_from,
                                         id_edge_to, pos_edge_to,
                                         id_stop_from, id_stopedge_from, pos_stop_from,
                                         id_stop_to, id_stopedge_to, pos_stop_to,
                                         #dist_from_to, dist_walk_max,
                                         walkstages, transitstages,
                                         ptlines, ptfstar, pttimes,
                                         stops_to_enter, stops_to_exit,
                                         ids_laneedge, ids_stoplane, ptstops)

                # update time for trips estimation for this plan
                plans.times_est[id_plan] += time-time_from

                # define current end time without last activity duration
                plans.times_end[id_plan] = time

                id_stage_act, time = activitystages.append_stage(
                    id_plan, time,
                    ids_activity=id_act_to,
                    names_activitytype=name_acttype_to,
                    durations=duration_act_to,
                    ids_lane=edges.ids_lanes[id_edge_to][0],
                    positions=pos_edge_to,
                )

                # store time for next iteration in case other activities are
                # following
                map_times[id_person] = time

            # select persons and activities for next setp
            ind_act += 1
            ids_person_act, ids_act_from, ids_act_to\
                = virtualpop.get_activities_from_pattern(ind_act, ids_person=ids_person_act)


class BikeTransitStrategy(BikeStrategy, TransitStrategy):
    def __init__(self, ident, parent=None,
                 name='Bike+Public Transport Strategy',
                 info='With this strategy, the person combines bike and public transport.',
                 **kwargs):

        self._init_objman(ident, parent, name=name, info=info, **kwargs)
        attrsman = self.set_attrsman(cm.Attrsman(self))
        # specific init
        self._init_attributes()
        self._init_constants()

    def _init_attributes(self):
        # print 'StrategyMixin._init_attributes'
        BikeStrategy._init_attributes(self)
        TransitStrategy._init_attributes(self)

    def _init_constants(self):

        BikeStrategy._init_constants(self)
        TransitStrategy._init_constants(self)

    def preevaluate(self, ids_person):
        """
        Preevaluation strategies for person IDs in vector ids_person.

        Returns a preevaluation vector with a preevaluation value 
        for each person ID. The values of the preevaluation vector are as follows:
            -1 : Strategy cannot be applied
            0  : Stategy can be applied, but the preferred mode is not used
            1  : Stategy can be applied, and preferred mode is part of the strategy
            2  : Strategy uses predomunantly preferred mode

        """
        n_pers = len(ids_person)
        persons = self.get_virtualpop()
        preeval = np.zeros(n_pers, dtype=np.int32)

        inds_prefer = (persons.ids_mode_preferred[ids_person] == self._id_mode_bus)\
            | (persons.ids_mode_preferred[ids_person] == self._id_mode_bike)\

        inds_avail = persons.ids_ibike[ids_person] > -1
        preeval[np.logical_not(inds_avail)] = -1
        # TODO: here we could exclude by age or distance facilities-stops

        # put 0 for persons whose preference is not public transport
        preeval[inds_avail & np.logical_not(inds_prefer)] = 0

        # put 2 for persons with bike access and who prefer bike or bus
        preeval[inds_avail & inds_prefer] = 1

        print '  BikeTransitStrategy.preevaluate', len(np.flatnonzero(preeval))
        return preeval

    def plan(self, ids_person, logger=None):
        """
        Generates a plan for these person according to this strategie.
        Overriden by specific strategy.
        """
        print 'TransitStrategy.plan', len(ids_person)
        #make_plans_private(self, ids_person = None, mode = 'passenger')
        # routing necessary?
        virtualpop = self.get_virtualpop()
        plans = virtualpop.get_plans()  # self._plans
        demand = virtualpop.get_demand()
        ptlines = demand.ptlines

        walkstages = plans.get_stagetable('walks')
        transitstages = plans.get_stagetable('transits')
        ridestages = plans.get_stagetable('bikerides')
        activitystages = plans.get_stagetable('activities')

        activities = virtualpop.get_activities()
        activitytypes = demand.activitytypes
        landuse = virtualpop.get_landuse()
        facilities = landuse.facilities
        #parking = landuse.parking

        scenario = virtualpop.get_scenario()
        net = scenario.net
        edges = net.edges
        lanes = net.lanes
        modes = net.modes

        ptstops = net.ptstops

        # print '   demand',demand
        # print '   demand.ptlines',demand.ptlines,dir(demand.ptlines)
        # print '   demand.ptlines.get_ptlinks()',demand.ptlines.get_ptlinks()
        # print '   demand.virtualpop',demand.virtualpop,dir(demand.virtualpop)
        # print '   demand.trips',demand.trips,dir(demand.trips)
        if len(ptlines) == 0:
            print 'WARNING in TrasitStrategy.plan: no transit services available.'
            return False

        ptlinks = ptlines.get_ptlinks()
        ptlinktypes = ptlinks.types.choices

        ptfstar = ptlinks.get_fstar()
        pttimes = ptlinks.get_times()
        stops_to_enter, stops_to_exit = ptlinks.get_stops_to_enter_exit()

        ids_stoplane = ptstops.ids_lane
        ids_laneedge = net.lanes.ids_edge

        times_est_plan = plans.times_est
        # here we can determine edge weights for different modes

        # this could be centralized to avoid redundance
        plans.prepare_stagetables(['walks', 'bikerides', 'transits', 'activities'])

        ids_person_act, ids_act_from, ids_act_to\
            = virtualpop.get_activities_from_pattern(0, ids_person=ids_person)

        if len(ids_person_act) == 0:
            print 'WARNING in TrasitStrategy.plan: no eligible persons found.'
            return False

        # temporary maps from ids_person to other parameters
        nm = np.max(ids_person_act)+1
        map_ids_plan = np.zeros(nm, dtype=np.int32)
        #ids_plan_act = virtualpop.add_plans(ids_person_act, id_strategy = self.get_id_strategy())
        map_ids_plan[ids_person_act] = virtualpop.add_plans(ids_person_act, id_strategy=self.get_id_strategy())

        map_times = np.zeros(nm, dtype=np.int32)
        map_times[ids_person_act] = activities.get_times_end(ids_act_from, pdf='unit')

        # set start time to plans (important!)
        plans.times_begin[map_ids_plan[ids_person_act]] = map_times[ids_person_act]

        map_ids_fac_from = np.zeros(nm, dtype=np.int32)
        map_ids_fac_from[ids_person_act] = activities.ids_facility[ids_act_from]

        n_plans = len(ids_person_act)
        print 'TrasitStrategy.plan n_plans=', n_plans

        # make initial activity stage
        ids_edge_from = facilities.ids_roadedge_closest[map_ids_fac_from[ids_person_act]]
        poss_edge_from = facilities.positions_roadedge_closest[map_ids_fac_from[ids_person_act]]
        # this is the time when first activity starts
        # first activity is normally not simulated

        names_acttype_from = activitytypes.names[activities.ids_activitytype[ids_act_from]]
        durations_act_from = activities.get_durations(ids_act_from)
        times_from = map_times[ids_person_act]-durations_act_from
        #times_from = activities.get_times_end(ids_act_from, pdf = 'unit')

        for id_plan,\
            time,\
            id_act_from,\
            name_acttype_from,\
            duration_act_from,\
            id_edge_from,\
            pos_edge_from  \
            in zip(map_ids_plan[ids_person_act],
                   times_from,
                   ids_act_from,
                   names_acttype_from,
                   durations_act_from,
                   ids_edge_from,
                   poss_edge_from):

            id_stage_act, time = activitystages.append_stage(
                id_plan, time,
                ids_activity=id_act_from,
                names_activitytype=name_acttype_from,
                durations=duration_act_from,
                ids_lane=edges.ids_lanes[id_edge_from][0],
                positions=pos_edge_from,
            )

        ##

        ind_act = 0

        # main loop while there are persons performing
        # an activity at index ind_act
        while len(ids_person_act) > 0:
            ids_plan = map_ids_plan[ids_person_act]

            times_from = map_times[ids_person_act]

            ids_veh = virtualpop.ids_ibike[ids_person_act]
            dists_walk_max = virtualpop.dists_walk_max[ids_person_act]
            names_acttype_to = activitytypes.names[activities.ids_activitytype[ids_act_to]]
            durations_act_to = activities.get_durations(ids_act_to)

            ids_fac_from = map_ids_fac_from[ids_person_act]
            ids_fac_to = activities.ids_facility[ids_act_to]

            centroids_from = facilities.centroids[ids_fac_from]
            centroids_to = facilities.centroids[ids_fac_to]

            # origin edge and position
            ids_edge_from = facilities.ids_roadedge_closest[ids_fac_from]
            poss_edge_from = facilities.positions_roadedge_closest[ids_fac_from]

            # destination edge and position
            ids_edge_to = facilities.ids_roadedge_closest[ids_fac_to]
            poss_edge_to = facilities.positions_roadedge_closest[ids_fac_to]

            ids_stop_from = ptstops.get_closest(centroids_from)
            ids_stop_to = ptstops.get_closest(centroids_to)

            ids_stopedge_from = ids_laneedge[ids_stoplane[ids_stop_from]]
            ids_stopedge_to = ids_laneedge[ids_stoplane[ids_stop_to]]

            centroids_stop_from = ptstops.centroids[ids_stop_from]
            centroids_stop_to = ptstops.centroids[ids_stop_to]
            # do random pos here
            poss_stop_from = 0.5*(ptstops.positions_from[ids_stop_from]
                                  + ptstops.positions_to[ids_stop_from])

            poss_stop_to = 0.5*(ptstops.positions_from[ids_stop_to]
                                + ptstops.positions_to[ids_stop_to])

            dists_from_to = np.sqrt(np.sum((centroids_to - centroids_from)**2, 1))
            dists_from_stop_from = np.sqrt(np.sum((centroids_stop_from - centroids_from)**2, 1))
            dists_stop_to_to = np.sqrt(np.sum((centroids_to - centroids_stop_to)**2, 1))

            i = 0.0
            for id_person, id_plan, time_from, id_act_from, id_act_to, name_acttype_to, duration_act_to, id_veh, id_edge_from, pos_edge_from, id_edge_to, pos_edge_to, id_stop_from, id_stopedge_from, pos_stop_from, id_stop_to, id_stopedge_to, pos_stop_to, dists_from_to, dist_from_stop_from, dist_stop_to_to, dist_walk_max\
                    in zip(ids_person_act, ids_plan, times_from, ids_act_from, ids_act_to, names_acttype_to, durations_act_to, ids_veh, ids_edge_from, poss_edge_from, ids_edge_to, poss_edge_to, ids_stop_from, ids_stopedge_from, poss_stop_from, ids_stop_to, ids_stopedge_to, poss_stop_to, dists_from_to, dists_from_stop_from, dists_stop_to_to, dists_walk_max):
                n_pers = len(ids_person_act)
                if logger:
                    logger.progress(i/n_pers*100)
                i += 1.0
                print 79*'_'
                print '  id_plan=%d, id_person=%d, ' % (id_plan, id_person)
                #dist_from_stop_from, dist_stop_to_to

                time = self.plan_bikeride(id_plan, time_from, id_veh,
                                          id_edge_from, pos_edge_from,
                                          id_stopedge_from, pos_stop_from,
                                          dist_from_stop_from, dist_walk_max,
                                          walkstages, ridestages)

                time = self.plan_transit(id_plan, time,
                                         id_stopedge_from, pos_stop_from,
                                         id_stopedge_to, pos_stop_to,
                                         id_stop_from, id_stopedge_from, pos_stop_from,
                                         id_stop_to, id_stopedge_to, pos_stop_to,
                                         #dist_from_to, dist_walk_max,
                                         walkstages, transitstages,
                                         ptlines, ptfstar, pttimes,
                                         stops_to_enter, stops_to_exit,
                                         ids_laneedge, ids_stoplane, ptstops)

                time = self.plan_bikeride(id_plan, time, id_veh,
                                          id_stopedge_to, pos_stop_to,
                                          id_edge_to, pos_edge_to,
                                          dist_stop_to_to, dist_walk_max,
                                          walkstages, ridestages)

                # update time for trips estimation for this plan
                plans.times_est[id_plan] += time-time_from

                # define current end time without last activity duration
                plans.times_end[id_plan] = time

                id_stage_act, time = activitystages.append_stage(
                    id_plan, time,
                    ids_activity=id_act_to,
                    names_activitytype=name_acttype_to,
                    durations=duration_act_to,
                    ids_lane=edges.ids_lanes[id_edge_to][0],
                    positions=pos_edge_to,
                )

                # store time for next iteration in case other activities are
                # following
                map_times[id_person] = time

            # select persons and activities for next setp
            ind_act += 1
            ids_person_act, ids_act_from, ids_act_to\
                = virtualpop.get_activities_from_pattern(ind_act, ids_person=ids_person_act)


class Strategies(am.ArrayObjman):
    def __init__(self, ident, virtualpop, **kwargs):
        self._init_objman(ident,
                          parent=virtualpop,
                          name='Mobility Strategies',
                          info="""Contains different mobility strategies. 
                            A strategy has methods to identify whether a strategy is applicaple to a person
                            and to make a plan fo a person.
                            """,
                          version=0.2,
                          **kwargs)

        self.add_col(am.ArrayConf('names',
                                  default='',
                                  dtype='object',
                                  perm='r',
                                  is_index=True,
                                  name='Short name',
                                  info='Strategy name. Must be unique, used as index.',
                                  ))
        self._init_attributes()
        self._init_constants()

    def _init_attributes(self):

        self.add_col(cm.ObjsConf('strategies',
                                 #groupnames = ['state'],
                                 name='Strategy',
                                 info='Strategy object.',
                                 ))

        self.add_col(am.ArrayConf('colors', np.ones(4, np.float32),
                                  dtype=np.float32,
                                  metatype='color',
                                  perm='rw',
                                  name='Color',
                                  info="Route color. Color as RGBA tuple with values from 0.0 to 1.0",
                                  xmltag='color',
                                  ))
        if self.get_version() < 0.2:
            self._set_colors_default()
            self.set_version(0.2)

    def format_ids(self, ids):
        return ', '.join(self.names[ids])

    def get_id_from_formatted(self, idstr):
        return self.names.get_id_from_index(idstr)

    def get_ids_from_formatted(self, idstrs):
        return self.names.get_ids_from_indices_save(idstrs.split(','))

    def add_default(self):

        self.add_strategy('walk', WalkStrategy)
        self.add_strategy('auto', AutoStrategy)
        self.add_strategy('bike', BikeStrategy)
        self.add_strategy('transit', TransitStrategy)
        self.add_strategy('biketransit', BikeTransitStrategy)
        self._set_colors_default()

    def _set_colors_default(self):
        colors_default = {'walk': np.array([160, 72, 0, 220], np.float32)/255,
                          'auto': np.array([250, 213, 0, 220], np.float32)/255,
                          'bike': np.array([8, 191, 73, 210], np.float32)/255,
                          'transit': np.array([8, 77, 191, 220], np.float32)/255,
                          'biketransit': np.array([8, 201, 223, 220], np.float32)/255,
                          }
        ids = self.names.get_ids_from_indices_save(colors_default.keys())
        self.colors[ids] = colors_default.values()  # np.array(colors_default.values(), np.float32).reshape(-1,4)

        #self.colors.indexset(colors_default.keys(), colors_default.values())

    def add_strategy(self, ident, Strategyclass, color=(0.0, 0.0, 0.0, 1.0)):
        # print 'add_strategy', ident
        if not self.names.has_index(ident):
            strategy = Strategyclass(ident, self)
            self.add_row(names=ident,
                         strategies=strategy,
                         colours=color,
                         )
            return strategy
        else:
            # print 'WARNING in add_strategy: strategy %s already initialized'%ident
            return self.strategies[self.names.get_id_from_index(ident)]

    def preevaluate(self, ids_person):
        """
        Preevaluation of strategies for person IDs in vector ids_person.

        Returns a vector with strategy IDs and a preevaluation matrix.
        The rows of the matrix corrispond to each person ID.
        The columns corrsopond to reck strategy ID.
        The values of the preevaluation matrix are as follows:
            -1 : Strategy cannot be applied
            0  : Stategy can be applied, but the preferred mode is not used
            1  : Stategy can be applied, and preferred mode is part of the strategy
            2  : Strategy uses predomunantly preferred mode

        """
        print 'preevaluate strategies'
        ids_strat = self.get_ids()
        n_pers = len(ids_person)
        n_strat = len(ids_strat)

        preeval = np.zeros((n_pers, n_strat), dtype=np.int32)
        for i, strategy in zip(range(n_strat), self.strategies[ids_strat]):
            print '  preevaluate strategiy', strategy.ident
            preeval[i, :] = strategy.preevaluate(ids_person)

        return ids_strat, preeval


class StageTypeMixin(am.ArrayObjman):
    def init_stagetable(self, ident, stages,  name='', info="Stage of Plan"):

        self._init_objman(ident=ident, parent=stages, name=name,
                          info=info,
                          version=0.2,
                          )

        self.add_col(am.IdsArrayConf('ids_plan', self.get_plans(),
                                     #groupnames = ['state'],
                                     name='ID plan',
                                     info='ID of plan.',
                                     xmltag='type',
                                     ))

        self.add_col(am.ArrayConf('times_start', -1.0,
                                  groupnames=['parameters'],    # new
                                  name='Start time',
                                  unit='s',
                                  info='Planned or estimated time when this stage starts. Value -1 means unknown.',
                                  ))

        self.add_col(am.ArrayConf('durations', -1.0,
                                  name='Duration',
                                  groupnames=['parameters'],  # new
                                  unit='s',
                                  info='Planned or estimated duration for this stage starts. Value -1 means unknown.',
                                  xmltag='type',
                                  ))

    def get_plans(self):
        return self.parent

    def get_ids_from_ids_plan(self, ids_plan):
        """
        Returns only stage IDs which are part of the given plans IDs. 
        """
        # print 'get_ids_from_ids_plan for',type(ids_plan),ids_plan

        ids = self.get_ids()
        #ids_plan_part = self.ids_plan[ids]
        are_selected = np.zeros(len(ids), dtype=np.bool)
        for ind, id_plan_part in zip(xrange(len(ids)), self.ids_plan[ids]):
            are_selected[ind] = id_plan_part in ids_plan
        return ids[are_selected]
        # print '  ids_plan_part',type(ids_plan_part),ids_plan_part
        # print '  ids_selected',type(ids_plan_part.intersection(ids_plan)),ids_plan_part.intersection(ids_plan)
        # return np.array(list(ids_plan_part.intersection(ids_plan)), dtype = np.int32)

    def get_virtualpop(self):
        # print 'get_virtualpop  '
        return self.parent.parent

    def prepare_planning(self):
        pass

    def append_stage(self, id_plan, time_start,  **kwargs):

        # try to fix timing
        # if time_start<0:
        #    time_start_prev, duration_prev = self.parent.plans.get_timing_laststage(id_plan)
        #    if (duration_prev>=0)&(time_start_prev>=0):
        #        time_start = time_start_prev+duration_prev

        id_stage = self.add_row(ids_plan=id_plan,  times_start=time_start, **kwargs)
        # print 'STAGE.appended stage %s id_plan=%d, id_stage=%d, t=%d'%(self.get_name(),id_plan,id_stage,time_start)
        # for key in kwargs.keys():
        #    print '    %s=%s'%(key,kwargs[key])
        # print '    --id_plan, self, id_stage',id_plan, self, id_stage#,self.ids_plan.get_linktab()

        self.parent.append_stage(id_plan, self, id_stage)
        # print '  plan appended',id_plan, self, id_stage

        return id_stage, time_start+self.durations[id_stage]

    def modify_stage(self, id_stage, time_start,  **kwargs):
        self.set_row(id_stage, **kwargs)
        return time_start+self.durations[id_stage]

    def get_timing(self, id_stage):
        #ind = self.get_ind(id_stage)
        return self.times_start[id_stage], self.durations[id_stage]

    def to_xml(self, id_stage, fd, indent=0):
        """
        To be overridden by specific stage class.
        """
        pass


class TransitStages(StageTypeMixin):
    def __init__(self, ident, stages, name='Transit rides', info='Ride on a single public transport line (no transfers).'):
        self.init_stagetable(ident,
                             stages, name=name,
                             info=info,
                             )
        self._init_attributes()

    def _init_attributes(self):
        #ptstops =  self.parent.get_ptstops()
        edges = self.get_virtualpop().get_net().edges
        if hasattr(self, 'ids_fromstop'):
            self.delete('ids_fromstop')
            self.delete('ids_tostop')

        self.add_col(am.IdsArrayConf('ids_line', self.get_virtualpop().get_ptlines(),
                                     groupnames=['parameters'],
                                     name='ID line',
                                     info='ID of public transport line.',
                                     ))

        self.add_col(am.IdsArrayConf('ids_fromedge', edges,
                                     groupnames=['parameters'],
                                     name='Edge ID from',
                                     info='Edge ID of departure bus stop or station.',
                                     ))

        self.add_col(am.IdsArrayConf('ids_toedge', edges,
                                     groupnames=['parameters'],
                                     name='Edge ID to',
                                     info='Edge ID of destination bus stop or station.',
                                     ))

    def _init_constants(self):
        self.get_attrsman().do_not_save_attrs(['_costs', '_fstar', ])

    def prepare_planning(self):
        ptlinks = self.ids_line.get_linktab().ptlinks.get_value()
        if len(ptlinks) == 0:
            # no links built...built them
            ptlinks.build()

        self._costs = ptlinks.get_times()
        self._fstar = ptlinks.get_fstar()

    def append_stage(self, id_plan, time_start=-1.0,
                     id_line=-1, duration=0.0,
                     id_fromedge=-1, id_toedge=-1, **kwargs):
        """
        Appends a transit stage to plan id_plan.

        """
        # print 'TransitStages.append_stage',id_stage

        id_stage, time_end = StageTypeMixin.append_stage(self,
                                                         id_plan,
                                                         time_start,
                                                         durations=duration,
                                                         ids_line=id_line,
                                                         ids_fromedge=id_fromedge,
                                                         ids_toedge=id_toedge,
                                                         )

        # add this stage to the vehicle database
        # ind_ride gives the index of this ride (within the same plan??)
        #ind_ride = self.parent.get_iautos().append_ride(id_veh, id_stage)
        return id_stage, time_end

    def to_xml(self, id_stage, fd, indent=0):
        # <ride from="1/3to0/3" to="0/4to1/4" lines="train0"/>
        net = self.get_virtualpop().get_net()
        #ids_stoplane = net.ptstops.ids_lane
        #ids_laneedge = net.lanes.ids_edge
        ids_sumoedge = net.edges.ids_sumo

        #ind = self.get_ind(id_stage)
        fd.write(xm.start('ride', indent=indent))
        fd.write(xm.num('from', ids_sumoedge[self.ids_fromedge[id_stage]]))
        fd.write(xm.num('to', ids_sumoedge[self.ids_toedge[id_stage]]))
        fd.write(xm.num('lines', self.ids_line.get_linktab().linenames[self.ids_line[id_stage]]))
        # if self.cols.pos_edge_from[ind]>0:
        #    fd.write(xm.num('departPos', self.cols.pos_edge_from[ind]))
        # if self.cols.pos_edge_to[ind]>0:
        #    fd.write(xm.num('arrivalPos', self.cols.pos_edge_to[ind]))

        fd.write(xm.stopit())  # ends stage


class AutorideStages(StageTypeMixin):

    def __init__(self, ident, population,
                 name='Auto rides',
                 info='Rides with privately owned auto.',
                 version=1.0,
                 ):

        self.init_stagetable(ident, population, name=name, info=info)
        # print 'Rides.__init__',self.get_name()
        self._init_attributes()

    def _init_attributes(self):
        # TODO: this structure needs review: the private vehicle is part of a person, not a stage
        # street parking at home and work could be in stage. Private garage is part of person...
        # print '_init_attributes',self.parent.get_iautos(), self.ident,self.parent.get_landuse().parking

        self.add_col(am.IdsArrayConf('ids_iauto', self.get_virtualpop().get_iautos(),
                                     groupnames=['state'],
                                     name='ID vehicle',
                                     info='ID of private vehicle.',
                                     ))

        self.add_col(am.ArrayConf('times_init', -1.0,
                                  name='Init. time',
                                  unit='s',
                                  info='Initialization time, which is the time when the vehicle appears in the scenario. Value -1 means unknown.',
                                  ))

        self.add_col(am.IdsArrayConf('ids_parking_from', self.get_virtualpop().get_landuse().parking,
                                     groupnames=['state'],
                                     name='ID dep. parking',
                                     info='Parking ID at the departure of the ride starts.',
                                     ))

        self.add_col(am.IdsArrayConf('ids_parking_to', self.get_virtualpop().get_landuse().parking,
                                     groupnames=['state'],
                                     name='ID arr. parking',
                                     info='Parking ID  at the arrival of the ride.',
                                     ))

        self.add_col(am.IdlistsArrayConf('ids_edges', self.get_virtualpop().get_net().edges,
                                         groupnames=['_private'],
                                         name='Route',
                                         info="The vehicle's route as a sequence of edge IDs.",
                                         ))

        self.add(cm.AttrConf('dist_ride_min', 400.0,
                             dtype='object',
                             groupnames=['options'],
                             perm='rw',
                             name='Min ride dist.',
                             info='Minimum ride distance. If the distanve between parkings is less, then the person will walk.',
                             ))

        # if hasattr(self,'parking'):
        #    self.delete('parking')

    def _init_constants(self):
        self.get_attrsman().do_not_save_attrs(['_costs', '_fstar', ])

    def prepare_planning(self):
        net = self.get_virtualpop().get_net()
        # ??? do this for all vehicles??
        self._costs = net.edges.get_times(id_mode=net.modes.get_id_mode('passenger'),
                                          is_check_lanes=True)
        self._fstar = net.edges.get_fstar(is_ignor_connections=False)

    def append_stage(self, id_plan, time_start=-1.0, id_veh=-1,
                     time_init=-1,
                     id_parking_from=.1, id_parking_to=-1, **kwargs):
        """
        Appends a ride stage to plan id_plan in case the ride is feasible.
        The ride is feasible if from parking and to parking are connected.

        If feasible, the stage ID and estimated time when stage is finished
        will be returned.

        If not feasible -1 and start time will be returned.
        """
        # print 'Rides.append_stage',id_stage

        # check feasibility
        route, dist, duration = self.get_route_between_parking(id_parking_from, id_parking_to)

        if (len(route) > 0):  # |(dist > self.dist_ride_min.get_value()): # is there a connection
            # create stage
            id_stage, time_end = StageTypeMixin.append_stage(self,
                                                             id_plan,
                                                             time_start,
                                                             durations=duration,
                                                             times_init=time_init,
                                                             ids_iauto=id_veh,
                                                             ids_parking_from=id_parking_from,
                                                             ids_parking_to=id_parking_to,
                                                             ids_edges=route,
                                                             )

            # add this stage to the vehicle database
            # ind_ride gives the index of this ride (within the same plan??)
            #ind_ride = self.parent.get_iautos().append_ride(id_veh, id_stage)
            return id_stage, time_end

        else:
            # not connected or too short of a distance
            return -1, time_start  # no stage creation took place

    def get_route_between_parking(self, id_parking_from, id_parking_to):
        """
        Return route and distance of ride with vehicle type  vtype
        between id_parking_from and id_parking_to

        """
        # print 'get_route_between_parking',id_parking_from, id_parking_to
        scenario = self.get_virtualpop().get_scenario()
        edges = scenario.net.edges
        lanes = scenario.net.lanes

        # print   self.get_demand().getVehicles().cols.maxSpeed
        #v_max = self.get_demand().getVehicles().maxSpeed.get(vtype)
        parking = scenario.landuse.parking

        ids_lanes = parking.ids_lane[[id_parking_from, id_parking_to]]
        id_edge_from, id_edge_to = lanes.ids_edge[ids_lanes]
        pos_from, pos_to = parking.positions[[id_parking_from, id_parking_to]]

        # print '  id_edge_from, id_edge_to=',id_edge_from, id_edge_to
        duration_approx, route = routing.get_mincostroute_edge2edge(
            id_edge_from,
            id_edge_to,
            weights=self._costs,  # mode-specific!!
            fstar=self._fstar  # mode-specific!!
        )

        # here is a big problem: starting with the successive node of edge_from
        # may result that the first edge of the route is not  connected with edge_from
        # And arriving at the preceding node of edge_to may result that from
        # the last edge in route the edge_to is not connected.

        #route = [edge_from]+route+[edge_to]
        dist = np.sum(edges.lengths[route])
        dist = dist - pos_from - (edges.lengths[id_edge_to] - pos_to)
        # if 0:
        #    if len(route)>0:
        #         print '  dist,duration',dist,duration_approx
        #    else:
        #         print '  no route found'

        return route, dist, duration_approx

    # def make_id_veh_ride(self, id_stage, i_ride):
    #    # make a unique vehicle ID for this stage
    #    self.inds_ride[id_stage] = i_ride
    #    return str(self.ids_veh[id_stage])+'.'+str(i_ride)

    def get_writexmlinfo(self, ids_plan, is_route=True):
        print 'AutorideStages.get_writexmlinfo', len(ids_plan)
        iautos = self.get_virtualpop().get_iautos()
        writefunc = iautos.prepare_write_xml()
        ids = self.get_ids_from_ids_plan(ids_plan)
        writefuncs = np.zeros(len(ids), dtype=np.object)
        writefuncs[:] = writefunc
        return self.times_init[ids], writefuncs, ids

    def to_xml(self, id_stage, fd, indent=0):
        #lanes = self.parent.get_scenario().net.lanes
        scenario = self.get_virtualpop().get_scenario()

        edges = scenario.net.edges
        edgeindex = edges.ids_sumo
        parking = scenario.landuse.parking

        ind = self.get_ind(id_stage)
        fd.write(xm.start('ride', indent=indent))

        id_edge_from, pos_from = parking.get_edge_pos_parking(self.ids_parking_from.value[ind])
        id_edge_to, pos_to = parking.get_edge_pos_parking(self.ids_parking_to.value[ind])

        # edgeindex.get_index_from_id(self.ids_edge_to.value[ind])
        fd.write(xm.num('from', edgeindex[id_edge_from]))
        fd.write(xm.num('to',  edgeindex[id_edge_to]))
        fd.write(xm.num('lines', self.ids_iauto.get_linktab().get_id_line_xml(
            self.ids_iauto[id_stage])))  # mode specific

        # if 0:
        #    #pos = pos_from
        #    length = max(edges.lengths[id_edge_from]-4.0,0.0)
        #
        #    if (pos_from>0) & (pos_from < length ):
        #        fd.write(xm.num('departPos', pos))
        #
        #    elif pos_from < 0:
        #        fd.write(xm.num('departPos', 0.0))
        #
        #    else:
        #        fd.write(xm.num('departPos', length))
        #
        #    #pos = self.positions_to.value[ind]
        #    length = max(edges.lengths[id_edge_to]-4.0, 0.0)
        #    if (pos_to>0) & (pos_to < length ):
        #        fd.write(xm.num('arrivalPos', pos_to))
        #
        #    elif pos_to < 0:
        #        fd.write(xm.num('arrivalPos', 0.0))
        #
        #    else:
        #        fd.write(xm.num('arrivalPos', length))

        fd.write(xm.stopit())  # ends stage


class BikerideStages(StageTypeMixin):

    def __init__(self, ident, population,
                 name='Bike rides',
                 info='Rides with privately owned bike.',
                 version=1.0,
                 ):

        self.init_stagetable(ident, population, name=name, info=info)
        # print 'Rides.__init__',self.get_name()
        self._init_attributes()

    def _init_attributes(self):
        # TODO: this structure needs review: the private vehicle is part of a person, not a stage
        # street parking at home and work could be in stage. Private garage is part of person...
        # print '_init_attributes',self.parent.get_iautos(), self.ident,self.parent.get_landuse().parking

        self.add_col(am.IdsArrayConf('ids_ibike', self.get_virtualpop().get_ibikes(),
                                     groupnames=['state'],
                                     name='ID bike',
                                     info='ID of private, individual bike.',
                                     ))

        self.add_col(am.ArrayConf('times_init', -1.0,
                                  name='Init. time',
                                  unit='s',
                                  info='Initialization time, which is the time when the vehicle appears in the scenario. Value -1 means unknown.',
                                  ))

        edges = self.get_virtualpop().get_net().edges

        self.add_col(am.IdsArrayConf('ids_edge_from', edges,
                                     groupnames=['state'],
                                     name='ID Dep. edge',
                                     info='Edge ID at departure of walk.',
                                     ))

        self.add_col(am.IdsArrayConf('ids_edge_to', edges,
                                     groupnames=['state'],
                                     name='ID Arr. edge',
                                     info='Edge ID where walk finishes.',
                                     ))

        self.add_col(am.ArrayConf('positions_from', 0.0,
                                  dtype=np.float32,
                                  #choices = OPTIONMAP_POS_DEPARTURE,
                                  perm='r',
                                  name='Depart pos',
                                  unit='m',
                                  info="Position on edge at the moment of departure.",
                                  #xmltag = 'departPos',
                                  #xmlmap = get_inversemap(OPTIONMAP_POS_ARRIVAL),
                                  ))

        self.add_col(am.ArrayConf('positions_to', 0.0,
                                  dtype=np.float32,
                                  #choices = OPTIONMAP_POS_ARRIVAL,
                                  perm='r',
                                  name='Arrival pos',
                                  unit='m',
                                  info="Position on edge at the moment of arrival.",
                                  #xmltag = 'arrivalPos',
                                  #xmlmap = get_inversemap(OPTIONMAP_POS_ARRIVAL),
                                  ))

        self.add_col(am.IdlistsArrayConf('ids_edges', self.get_virtualpop().get_net().edges,
                                         groupnames=['_private'],
                                         name='Route',
                                         info="The vehicle's route as a sequence of edge IDs.",
                                         ))

    def _init_constants(self):
        self.get_attrsman().do_not_save_attrs(['_costs', '_fstar', ])

    def prepare_planning(self):
        net = self.get_virtualpop().get_net()

        print 'prepare_planning'
        self._costs = net.edges.get_times(id_mode=net.modes.get_id_mode('bicycle'),
                                          is_check_lanes=True)

        ids_edge = net.edges.get_ids()
        for id_edge, cost in zip(ids_edge, self._costs[ids_edge]):
            print '   id_edge', id_edge, 'sumo', net.edges.ids_sumo[id_edge], cost

        self._fstar = net.edges.get_fstar(is_ignor_connections=False)

    def append_stage(self, id_plan, time_start=-1.0, id_veh=-1,
                     time_init=-1,
                     id_edge_from=-1, id_edge_to=-1,
                     position_edge_from=0.0, position_edge_to=0.0,
                     **kwargs):
        """
        Appends a ride stage to plan id_plan in case the ride is feasible.
        The ride is feasible if from parking and to parking are connected.

        If feasible, the stage ID and estimated time when stage is finished
        will be returned.

        If not feasible -1 and start time will be returned.
        """
        # print 'BikeRides.append_stage',id_plan,time_start,time_init
        #edges = self.get_virtualpop().get_net().edges
        # check feasibility
        #route, dist, duration = self.get_route_between_parking(id_parking_from, id_parking_to)

        # print '  id_edge_from, id_edge_to=',id_edge_from, id_edge_to
        duration_approx, route = routing.get_mincostroute_edge2edge(
            id_edge_from,
            id_edge_to,
            weights=self._costs,  # mode-specific!!
            fstar=self._fstar  # mode-specific!!
        )

        #route = [edge_from]+route+[edge_to]

        #dist = np.sum(edges.lengths[route])
        #dist =  dist - pos_from - ( edges.lengths[id_edge_to] - pos_to)

        if (len(route) > 0):  # |(dist > self.dist_ride_min.get_value()): # is there a connection
            # create stage
            id_stage, time_end = StageTypeMixin.append_stage(self,
                                                             id_plan,
                                                             time_start,
                                                             durations=duration_approx,
                                                             times_init=time_init,
                                                             ids_ibike=id_veh,
                                                             ids_edge_from=id_edge_from,
                                                             positions_from=position_edge_from,
                                                             ids_edge_to=id_edge_to,
                                                             positions_to=position_edge_to,
                                                             ids_edges=route,
                                                             )

            # print '  route = ',route
            # print '  ids_edges = ',self.ids_edges[id_stage]
            # add this stage to the vehicle database
            # ind_ride gives the index of this ride (within the same plan??)
            #ind_ride = self.parent.get_iautos().append_ride(id_veh, id_stage)
            return id_stage, time_end

        else:
            # not connected or too short of a distance
            return -1, time_start  # no stage creation took place

    def get_writexmlinfo(self, ids_plan, is_route=True):
        print 'BikerideStages.get_writexmlinfo', len(ids_plan)
        ibikes = self.get_virtualpop().get_ibikes()
        bikewritefunc = ibikes.prepare_write_xml()
        ids = self.get_ids_from_ids_plan(ids_plan)

        bikewritefuncs = np.zeros(len(ids), dtype=np.object)
        bikewritefuncs[:] = bikewritefunc
        return self.times_init[ids], bikewritefuncs, ids

    def to_xml(self, id_stage, fd, indent=0):
        #lanes = self.parent.get_scenario().net.lanes
        scenario = self.get_virtualpop().get_scenario()

        edges = scenario.net.edges
        edgeindex = edges.ids_sumo

        #parking = scenario.landuse.parking

        #ind = self.get_ind(id_stage)
        fd.write(xm.start('ride', indent=indent))

        #id_edge_from, pos_from = parking.get_edge_pos_parking(self.ids_parking_from.value[ind])
        #id_edge_to, pos_to = parking.get_edge_pos_parking(self.ids_parking_to.value[ind])

        # edgeindex.get_index_from_id(self.ids_edge_to.value[ind])
        id_edge_from = self.ids_edge_from[id_stage]
        fd.write(xm.num('from', edgeindex[self.ids_edge_from[id_stage]]))
        fd.write(xm.num('to',  edgeindex[self.ids_edge_to[id_stage]]))
        fd.write(xm.num('lines', self.ids_ibike.get_linktab().get_id_line_xml(
            self.ids_ibike[id_stage])))  # mode specific

        # if 0:
        #    #pos = pos_from
        #    length = max(edges.lengths[id_edge_from]-4.0,0.0)
        #
        #    if (pos_from>0) & (pos_from < length ):
        #        fd.write(xm.num('departPos', pos))
        #
        #    elif pos_from < 0:
        #        fd.write(xm.num('departPos', 0.0))
        #
        #    else:
        #        fd.write(xm.num('departPos', length))
        #
        #    #pos = self.positions_to.value[ind]
        #    length = max(edges.lengths[id_edge_to]-4.0, 0.0)
        #    if (pos_to>0) & (pos_to < length ):
        #        fd.write(xm.num('arrivalPos', pos_to))
        #
        #    elif pos_to < 0:
        #        fd.write(xm.num('arrivalPos', 0.0))
        #
        #    else:
        #        fd.write(xm.num('arrivalPos', length))

        fd.write(xm.stopit())  # ends stage


class WalkStages(StageTypeMixin):
    def __init__(self, ident, stages, name='WalkStages',
                 info='walk from a position on a lane to another position of another lane.'):

        self.init_stagetable(ident, stages, name=name, info=info)

        edges = self.get_virtualpop().get_scenario().net.edges
        self.add_col(am.IdsArrayConf('ids_edge_from', edges,
                                     groupnames=['state'],
                                     name='ID Dep. edge',
                                     info='Edge ID at departure of walk.',
                                     ))

        self.add_col(am.IdsArrayConf('ids_edge_to', edges,
                                     groupnames=['state'],
                                     name='ID Arr. edge',
                                     info='Edge ID where walk finishes.',
                                     ))

        self.add_col(am.ArrayConf('positions_from', 0.0,
                                  dtype=np.float32,
                                  #choices = OPTIONMAP_POS_DEPARTURE,
                                  perm='r',
                                  name='Depart pos',
                                  unit='m',
                                  info="Position on edge at the moment of departure.",
                                  #xmltag = 'departPos',
                                  #xmlmap = get_inversemap(OPTIONMAP_POS_ARRIVAL),
                                  ))
        self.positions_from.set_xmltag(None)

        self.add_col(am.ArrayConf('positions_to', 0.0,
                                  dtype=np.float32,
                                  #choices = OPTIONMAP_POS_ARRIVAL,
                                  perm='r',
                                  name='Arrival pos',
                                  unit='m',
                                  info="Position on edge at the moment of arrival.",
                                  #xmltag = 'arrivalPos',
                                  #xmlmap = get_inversemap(OPTIONMAP_POS_ARRIVAL),
                                  ))

        self.positions_to.set_xmltag(None)

    def append_stage(self, id_plan, time_start=-1.0,
                     id_edge_from=-1, id_edge_to=-1,
                     position_edge_from=0.1, position_edge_to=0.0,
                     **kwargs):
        # print 'WalkStages.append_stage',id_stage
        if kwargs.has_key('duration'):
            duration = kwargs['duration']
        else:
            dist, duration = self.plan_walk(id_edge_from, id_edge_to,
                                            position_edge_from, position_edge_to)

        id_stage, time_end = StageTypeMixin.append_stage(self,
                                                         id_plan,
                                                         time_start,
                                                         durations=duration,
                                                         ids_edge_from=id_edge_from,
                                                         ids_edge_to=id_edge_to,
                                                         positions_from=position_edge_from,
                                                         positions_to=position_edge_to,
                                                         )

        return id_stage, time_end

    def modify_stage(self, id_stage, time_start,
                     id_edge_from=-1, id_edge_to=-1,
                     position_edge_from=0.1, position_edge_to=0.0):

        dist, duration = self.plan_walk(id_edge_from, id_edge_to,
                                        position_edge_from, position_edge_to)

        time_end = StageTypeMixin.modify_stage(self, id_stage, time_start,
                                               durations=duration,
                                               ids_edge_from=id_edge_from,
                                               ids_edge_to=id_edge_to,
                                               positions_from=position_edge_from,
                                               positions_to=position_edge_to,
                                               )

        return time_end

    def plan_walk(self, id_edge_from, id_edge_to, pos_from, pos_to, id_mode=1):
        """
        Routing for pedestrians.
        Currently limited to estimation of line of sight distance
        and walk time.
        """
        # print 'plan_walk',id_edge_from, id_edge_to,id_mode, pos_from, pos_to
        scenario = self.get_virtualpop().get_scenario()
        edges = scenario.net.edges

        coord_from = edges.get_coord_from_pos(id_edge_from, pos_from)
        coord_to = edges.get_coord_from_pos(id_edge_to, pos_to)

        # from lanes, more precis, but less efficient and less robust
        # lanes = scenario.net.lanes
        #coord_from = lanes.get_coord_from_pos(edges.ids_lane[id_edge_from][0], pos_from)
        #coord_to = lanes.get_coord_from_pos(edges.ids_lane[id_edge_to][0], pos_to)

        # print '  coord_from',coord_from,type(coord_from)
        # print '  coord_to',coord_from,type(coord_to)
        # print '  delta',coord_to-coord_from
        # line of sight distance
        dist = np.sqrt(np.sum((coord_to-coord_from)**2))

        duration_approx = dist/scenario.net.modes.speeds_max[id_mode]
        # print '  dist,duration',dist,duration_approx,scenario.net.modes.speeds_max[id_mode]

        return dist, duration_approx

    def to_xml(self, id_stage, fd, indent=0):
        #scenario = self.parent.get_scenario()
        #edges = scenario.net.edges
        edges = self.ids_edge_from.get_linktab()
        edgeindex = edges.ids_sumo

        ind = self.get_ind(id_stage)

        id_edge_from = self.ids_edge_from.value[ind]
        id_edge_to = self.ids_edge_to.value[ind]
        fd.write(xm.start('walk', indent=indent))
        fd.write(xm.num('from', edgeindex[id_edge_from]))
        fd.write(xm.num('to', edgeindex[id_edge_to]))

        pos = self.positions_from.value[ind]
        length = max(edges.lengths[id_edge_from]-4.0, 0.0)

        # depricated
        # if (pos>0) & (pos < length ):
        #    fd.write(xm.num('departPos', pos))
        #
        # elif pos < 0:
        #    fd.write(xm.num('departPos', 0.0))
        #
        # else:
        #    fd.write(xm.num('departPos', length))

        pos = self.positions_to.value[ind]
        length = max(edges.lengths[id_edge_to]-4.0, 0.0)
        if (pos > 0) & (pos < length):
            fd.write(xm.num('arrivalPos', pos))

        elif pos < 0:
            fd.write(xm.num('arrivalPos', 0.0))

        else:
            fd.write(xm.num('arrivalPos', length))

        fd.write(xm.stopit())  # ends walk


class ActivityStages(StageTypeMixin):
    def __init__(self, ident, stages, name='Activities'):
        self.init_stagetable(ident, stages, name=name, info='Do some activity at a position of a lane.')

        self._init_attributes()

    def _init_attributes(self):

        # TODO: this structure needs review: the private vehicle is part of a person, not a stage
        # street parking at home and work could be in stage. Private garage is part of person...

        activities = self.get_virtualpop().get_activities()
        self.add_col(am.IdsArrayConf('ids_activity', activities,
                                     groupnames=['parameters'],
                                     name='Activity ID',
                                     info='Scheduled activity ID. This is the activity which schould be carried out in this stage.',
                                     ))

        # this is redundant information but here for speed when writing xml
        self.add_col(am.ArrayConf('names_activitytype', '',
                                  groupnames=['parameters'],
                                  dtype=np.object,
                                  perm='r',
                                  name='Type name',
                                  info="Name of activity type.",
                                  xmltag='actType',
                                  #xmlmap = get_inversemap( activitytypes.names.get_indexmap()),
                                  ))

        # self.add_col(am.IdsArrayConf( 'ids_facility', self.get_virtualpop().get_landuse().facilities,
        #                                groupnames = ['parameters'],
        #                                name = 'ID facility',
        #                                info = 'Facility where activity takes place.',
        #                                ))

        # lane and position can be computed from facility
        self.add_col(am.IdsArrayConf('ids_lane', self.get_virtualpop().get_net().lanes,
                                     groupnames=['parameters'],
                                     name='Lane ID',
                                     info='Lane ID where activity takes place.',
                                     #xmltag = 'lane',
                                     ))
        # for update..can be removed
        self.ids_lane.set_xmltag(None)

        self.add_col(am.ArrayConf('positions', 0.0,
                                  groupnames=['parameters'],
                                  dtype=np.float32,
                                  perm='r',
                                  name='Lane pos',
                                  unit='m',
                                  info="Position on lane nearby where activity takes place.",
                                  #xmltag = 'startPos',
                                  #xmlmap = get_inversemap(OPTIONMAP_POS_ARRIVAL),
                                  ))

        self.positions.set_xmltag(None)

        self.add_col(am.ArrayConf('durations', 0.0,
                                  groupnames=['parameters'],
                                  dtype=np.int32,
                                  perm='r',
                                  name='Duration',
                                  unit='s',
                                  info="Duration of activity.",
                                  xmltag='duration',
                                  #xmlmap = get_inversemap(OPTIONMAP_POS_ARRIVAL),
                                  ))

        self.durations.set_xmltag('duration')

    def get_edges_positions(self, ids_stage):
        """
        Returns road edge and positions of activity.
        """
        return self.ids_lane.get_linktab().ids_edge[self.ids_lane[ids_stage]],\
            self.positions[ids_stage]

    def _init_constants(self):
            #self._activities  = self.get_virtualpop().get_activities()
            #self._activitytypes = self.get_virtualpop().get_demand().activitytypes
            # self.do_not_save_attrs(['_activities','_activitytypes'])
        pass

    def to_xml(self, id_stage, fd, indent=0):

        # <stop lane="1/4to2/4_0" duration="20" startPos="40" actType="singing"/>

        #ind = self.get_ind(id_stage)
        fd.write(xm.start('stop', indent=indent))

        lanes = self.get_virtualpop().get_net().lanes
        id_lane = self.ids_lane[id_stage]
        # get all xml configs and damp to fd
        for attrconf in self.get_group('parameters'):
            # this will write only if a xmltag is defined
            attrconf.write_xml(fd, id_stage)
        fd.write(xm.num('lane', lanes.get_id_sumo(id_lane)))
        #fd.write(xm.num('duration', self.durations[id_stage]))

        pos = self.positions[id_stage]
        length = max(lanes.get_lengths(id_lane)-4.0, 0.0)

        if (pos > 0) & (pos < length):
            fd.write(xm.num('startPos', pos))

        elif pos < 0:
            fd.write(xm.num('startPos', 0.0))

        else:
            fd.write(xm.num('startPos', length))

        #fd.write(xm.num('lane', self.cols.id_lane[ind]))
        #fd.write(xm.num('startPos', self.cols.pos_lane[ind]))
        #fd.write(xm.num('duration', self.cols.duration[ind]))
        # fd.write(xm.num('actType', self._activitytypes.names[self._activities.)

        fd.write(xm.stopit())  # ends activity


class Plans(am.ArrayObjman):
    def __init__(self, population,  **kwargs):
        """Plans database."""
        self._init_objman(ident='plans',
                          parent=population,
                          name='Plans',
                          info='Mobility plan for virtual population.',
                          #xmltag = ('plans','plan',None),
                          version=0.1,
                          **kwargs)

        self._init_attributes()
        self._init_constants()

    def _init_attributes(self):

        # upgrade
        if self.get_version() < 0.1:
            pass

        self.set_version(0.1)
        persons = self.parent

        #self.add(cm.ObjConf(StageTables('stagetables',self))   )

        self.add_stagetable('walks', WalkStages)
        self.add_stagetable('autorides', AutorideStages)
        self.add_stagetable('bikerides', BikerideStages)
        self.add_stagetable('transits', TransitStages)
        self.add_stagetable('activities', ActivityStages)

        self.add_col(am.IdsArrayConf('ids_person', persons,
                                     groupnames=['links'],
                                     name='Person ID',
                                     info='Person ID to who this plan belongs to.',
                                     ))

        self.add_col(am.IdsArrayConf('ids_strategy', persons.get_strategies(),
                                     groupnames=['links'],
                                     name='Stategy ID',
                                     info='Stategy ID with which this plan has been generated.',
                                     ))

        self.add_col(am.ArrayConf('times_begin', -np.inf,
                                  dtype=np.float32,
                                  name='Begin time',
                                  info='Time when active travelling begins. This is the time in the simulation when the person appears. The first activity is not simulated.',
                                  unit='s',
                                  ))

        self.add_col(am.ArrayConf('times_end', -np.inf,
                                  dtype=np.float32,
                                  name='End time',
                                  info='Time when active travelling ends. This is the time in the simulation when the person disappears. The last activity is not simulated.',
                                  unit='s',
                                  ))

        self.add_col(am.ArrayConf('times_est', 0.0,
                                  dtype=np.float32,
                                  name='Estim. time',
                                  info='Estimated time duration to execute travel plan. Activity times are excluded.',
                                  unit='s',
                                  ))

        self.add_col(am.ArrayConf('times_exec', 0.0,
                                  dtype=np.float32,
                                  name='Exec. time',
                                  info='Last plan execution time from simulation run.',
                                  unit='s',
                                  ))

        self.add_col(am.ArrayConf('utilities', 0.0,
                                  dtype=np.float32,
                                  name='utility',
                                  info='Utility of plan.',
                                  ))

        self.add_col(am.ArrayConf('probabilities', 1.0,
                                  dtype=np.float32,
                                  name='Probability',
                                  info='Probability that the plan is selected out of all plans available for one person.',
                                  ))

        self.add_col(am.TabIdListArrayConf('stagelists',
                                           name='Stages',
                                           info='Sequence of stages of this plan.',
                                           ))

    def _init_constants(self):
        #self._id_mode_bike = self.parent.get_scenario().net.modes.get_id_mode('bicycle')
        # self.do_not_save_attrs([])
        pass

    def clear_plans(self):
        print 'Plans.clear_plans'
        for stagetable in self.get_stagetables():
            # print '  stagetable',stagetable
            stagetable.clear()

        self.clear_rows()
        # for attrconfig in  self.get_attrsman().get_colconfigs():
        #    print '  clear attrconfig',attrconfig.attrname
        #    attrconfig.clear()
        # no: self.clear()

    def add_stagetable(self, ident, StagesClass, **kwargs):
        if not hasattr(self, ident):
            self.add(cm.ObjConf(StagesClass(ident, self, **kwargs),
                                groupnames=['stagetables']))
        return getattr(self, ident).get_value()

    def get_stagetable(self, ident):
        return getattr(self, ident).get_value()

    def get_stagetables(self):
        """Return a list of with all stage objects"""
        stageobjs = []
        # print 'get_stagetables',self.get_group('stagetables')
        for stageobj in self.get_group('stagetables'):
            stageobjs.append(stageobj)
        return stageobjs

    def prepare_stagetables(self, idents_stagetable):
        # print 'prepare_stages',stagenames
        #ids = self.names.get_ids_from_indices_save(stagenames)
        # print '  ids',ids
        # print '  self.stagetables[ids]',self.stagetables[ids]
        for indent in idents_stagetable:
            self.get_stagetable(indent).prepare_planning()

    def get_stages(self, id_plan):
        stages = self.stagelists[id_plan]
        if stages is None:
            return []
        else:
            return stages

    def append_stage(self, id_plan, stage, id_stage):
        # test: stage = cm.TableEntry(stagetable, id_plan)
        # print 'Plans.append_stage',self,id_plan, stage, id_stage

        if self.stagelists[id_plan] is None:
            self.stagelists[id_plan] = [(stage, id_stage)]
        else:
            self.stagelists[id_plan].append((stage, id_stage))
        # print '  after append stagelists[id_plan]',type(self.stagelists[id_plan]),self.stagelists[id_plan]

    # def prepare_stages(self,stagenames):
    #    self.get_stagetables().prepare_stages(stagenames)

    def get_timing_laststage(self, id_plan):
        """
        Return time_start and duration of last stage of plan id_plan
        """
        stages_current = self.stagelists[id_plan]

        if stages_current is not None:
            stage_last, id_stage_last = stages_current[-1]
            return stage_last.get_timing(id_stage_last)
        else:
            return -1, -1


class Virtualpopulation(DemandobjMixin, am.ArrayObjman):
    def __init__(self, ident, demand, **kwargs):
        self._init_objman(ident=ident,
                          parent=demand,
                          name='Virtual population',
                          info='Contains information of each individual of the virtual population.',
                          version=0.2,  # only for new scenarios
                          **kwargs)
        self._init_attributes()
        self._init_constants()

    def _init_attributes(self):

        # update here

        #
        self.set_version(0.2)

        demand = self.parent
        scenario = demand.get_scenario()

        # --------------------------------------------------------------------
        # individual vehicles tables

        self.add(cm.ObjConf(IndividualAutos('iautos', self)))
        self.add(cm.ObjConf(IndividualBikes('ibikes', self)))
        self.add(cm.ObjConf(IndividualMotorcycles('imotos', self)))

        # --------------------------------------------------------------------
        # activity  table
        #self.add(cm.ObjConf(ActivityTypes('activitytypes', self))   )
        self.add(cm.ObjConf(Activities('activities', self)))

        # --------------------------------------------------------------------
        # strategies table (must be before plans)

        self.add(cm.ObjConf(Strategies('strategies', self)))

        # --------------------------------------------------------------------
        # plans table
        self.add(cm.ObjConf(Plans(self)))

        self.get_strategies().add_default()

        # ===================================================================
        # Add person attributes

        # --------------------------------------------------------------------
        # socio economic parameters
        self.add_col(am.ArrayConf('identifications', '',
                                  dtype=np.object,
                                  groupnames=['socioeconomic'],
                                  name='Name',
                                  info='Identification or name of person.',
                                  ))

        self.add_col(am.ArrayConf('ids_gender', default=-1,
                                  dtype=np.int32,
                                  groupnames=['socioeconomic'],
                                  choices=GENDERS,
                                  name='Gender',
                                  info='Gender of person.',
                                  ))

        self.add_col(am.ArrayConf('years_birth', default=-1,
                                  dtype=np.int32,
                                  groupnames=['socioeconomic'],
                                  name='Birth year',
                                  info='Year when person has been born.',
                                  ))

        self.add_col(am.ArrayConf('ids_occupation', default=OCCUPATIONS['unknown'],
                                  dtype=np.int32,
                                  choices=OCCUPATIONS,
                                  groupnames=['socioeconomic'],
                                  name='Occupation',
                                  info='Type of occupation.',
                                  ))

        # --------------------------------------------------------------------
        # household parameters
        self.add_col(am.ArrayConf('numbers_houehold', default=1,
                                  dtype=np.int32,
                                  groupnames=['household'],
                                  name='Number in household',
                                  info='Number of persons in household.',
                                  ))

        self.add_col(am.ArrayConf('numbers_minor', default=0,
                                  dtype=np.int32,
                                  groupnames=['household'],
                                  name='Number minors',
                                  info='Number of minor in household. In the context of traffic simulations minors are persons who need to be accompaigned by adulds when travelling.',
                                  ))

        # --------------------------------------------------------------------
        # activity parameters
        # lists with activity patterns

        self.add_col(am.IdlistsArrayConf('activitypatterns', self.activities.get_value(),
                                         groupnames=['activity'],
                                         name='Activity IDs',
                                         info="Sequence of activity IDs to be accomplished by the person.",
                                         ))

        # --------------------------------------------------------------------
        # mobility parameters
        # --------------------------------------------------------------------

        # give a pedestrian vtype to each person
        vtypes = self.get_demand().vtypes
        self.add_col(am.IdsArrayConf('ids_vtype', vtypes,
                                     id_default=vtypes.select_by_mode(mode='pedestrian')[0],
                                     groupnames=['mobility'],
                                     name='Ped. type',
                                     info='The pedestrian type ID specifies the walking characteristics and visual representation of the person. In SUMO terminology, this is the vehicle type.',
                                     #xmltag = 'type',
                                     ))

        self.add_col(am.ArrayConf('traveltimebudgets', default=55*60,
                                  dtype=np.int32,
                                  groupnames=['mobility'],
                                  name='time budget',
                                  unit='s',
                                  info='Daily time budget used for traveling.',
                                  ))

        self.add_col(am.IdsArrayConf('ids_mode_preferred', scenario.net.modes,
                                     groupnames=['mobility'],
                                     name='ID preferred mode',
                                     info='ID of preferred transport mode of person.',
                                     ))

        self.add_col(am.IdsArrayConf('ids_iauto', self.get_iautos(),
                                     groupnames=['mobility'],
                                     name='ID auto',
                                     info='ID of individual auto. Negative value means no bile available.',
                                     ))

        self.add_col(am.IdsArrayConf('ids_ibike', self.get_ibikes(),
                                     groupnames=['mobility'],
                                     name='ID bike',
                                     info='ID of individual bicycle. Negative value means no bike available.',
                                     ))

        self.add_col(am.IdsArrayConf('ids_imoto', self.get_imotos(),
                                     groupnames=['mobility'],
                                     name='ID motorcycle',
                                     info='ID of individual motorcycle. Negative value means no motorcycle available.',
                                     ))

        self.add_col(am.ArrayConf('dists_walk_max', default=300.0,
                                  dtype=np.float32,
                                  groupnames=['mobility'],
                                  name='Max. walk dist',
                                  info='Maximum acceptable walking distance between origin and destination or for transfers between modes.',
                                  ))

        # --------------------------------------------------------------------
        # plans
        self.add_col(am.IdsArrayConf('ids_plan', self.get_plans(),
                                     groupnames=['plans'],
                                     name='ID Plan',
                                     info='Currently selected mobility plan ID of person. This is the plan which will be simulated.',
                                     ))

        self.add_col(am.IdlistsArrayConf('lists_ids_plan', self.get_plans(),
                                         groupnames=['plans'],
                                         name='Plan IDs',
                                         info='List with alternative, feasible mobility plan IDs for each person.',
                                         ))

    def _init_constants(self):
        modes = self.get_scenario().net.modes
        self.id_mode_bike = modes.get_id_mode('bicycle')
        self.id_mode_auto = modes.get_id_mode('passenger')
        self.id_mode_moto = modes.get_id_mode('motorcycle')
        self._edges = self.get_net().edges
        self.do_not_save_attrs(['id_mode_bike', 'id_mode_auto', 'id_mode_moto',
                                '_edges'])

    def get_demand(self):
        return self.parent

    def clear_population(self):
        # self.clear()
        self.clear_plans()
        self.clear_ivehicles()

        # TODO: this should disappear
        self.get_landuse().parking.clear_booking()

        # for attrconfig in  self.get_attrsman().get_colconfigs():
        #    attrconfig.clear()
        self.clear_rows()

    def clear_plans(self):
        # print 'clear_plans',self.get_stagetables()
        self.ids_plan.reset()
        self.lists_ids_plan.reset()
        self.get_plans().clear_plans()

        # TODO: this should disappear
        self.get_landuse().parking.clear_booking()

    def clear_ivehicles(self):
        """
        Clear all individually owned vehicles.
        """
        print 'clear_ivehicles'
        self.get_iautos().clear_vehicles()
        self.get_ibikes().clear_vehicles()
        self.get_imotos().clear_vehicles()

    def get_activities(self):
        return self.activities.get_value()

    def get_strategies(self):
        return self.strategies.get_value()

    def get_plans(self):
        return self.plans.get_value()

    def get_iautos(self):
        return self.iautos.get_value()

    def get_ibikes(self):
        return self.ibikes.get_value()

    def get_imotos(self):
        return self.imotos.get_value()

    def get_stagetables(self):
        return self.get_plans().get_stagetables()

    def get_landuse(self):
        return self.parent.get_scenario().landuse

    def get_scenario(self):
        return self.parent.get_scenario()

    def get_net(self):
        return self.parent.get_scenario().net

    def get_ptlines(self):
        return self.get_demand().ptlines

    def get_ptstops(self):
        return self.get_net().ptstops

    def get_id_sumo_from_id(self, id_sumo):
        return u'vp.%s' % id_sumo

    def get_id_from_id_sumo(self, id_veh_sumo):
        if len(id_veh_sumo.split('.')) == 2:
            prefix, id_pers = id_veh_sumo.split('.')
            if prefix == 'vp':
                return int(id_pers)
            else:
                return -1
        return -1

    def get_ids_from_ids_sumo(self, ids_sumo):
        ids = np.zeros(len(ids_sumo), dtype=int32)
        for id_sumo in ids_sumo:
            ids[i] = self.get_id_from_id_sumo(id_sumo)
        return ids

    def get_time_depart_first(self):
        # print 'Virtualpop.get_time_depart_first'
        if len(self.get_plans()) > 0:
            plans = self.get_plans()
            ids = self.select_ids(self.ids_plan.get_value() >= 0)
            # print '  ids',ids
            return float(np.min(plans.times_begin[self.ids_plan[ids]]))
        else:
            return 0.0

    def get_time_depart_last(self):
        if len(self.get_plans()) > 0:
            # todo: this can be improved by adding plan execution time
            plans = self.get_plans()
            ids = self.select_ids(self.ids_plan.get_value() >= 0)

            return float(np.max(plans.times_end[self.ids_plan[ids]]))
        else:
            return 0.0

    # def add_stagetable(self,ident,StageClass, **kwargs):
    #    print 'add_stagetable',ident,StageClass#,kwargs
    #    if not hasattr(self,ident):
    #        #print '  StageClass',StageClass(ident, self, **kwargs)
    #        #print '  ObjConf',cm.ObjConf(StageClass(ident, self, **kwargs), goupnames = ['stages'])
    #        self.add(cm.ObjConf(StageClass(ident, self, **kwargs), goupnames = ['stages']) )
    #    return getattr(self, ident).get_value()

    # def get_stagetable(self, ident):
    #    return getattr(self, ident).get_value()

    # def get_stagetables(self):
    #    """Return a list of with all stage objects"""
    #    stageobjs = []
    #    #print 'get_stagetables',self.get_group('stages')
    #    for stageobj in self.get_group('stages'):
    #        stageobjs.append(stageobj)
    #    return stageobjs

    def make_multiple(self, n, **kwargs):
        return self.add_rows(n=n, **kwargs)

    def disaggregate_odflow(self,   time_start, time_end, id_mode,
                            ids_fac, probs_fac_orig, probs_fac_dest,
                            tripnumber_tot,
                            id_activitytype_orig,
                            id_activitytype_dest,
                            hour_begin_earliest_orig,
                            hour_begin_earliest_dest,
                            hour_begin_latest_orig,
                            hour_begin_latest_dest,
                            duration_min_orig,
                            duration_min_dest,
                            duration_max_orig,
                            duration_max_dest,
                            scale=1.0,
                            hour_offset=8.0,  # 08:00
                            hour_tripbudget=25.0/60,  # 25min
                            **kwargs
                            ):
        """
        Disaggregation of demand dem  from taz id_zone_orig to id_zone_dest with id_mode
        during time interval  time_start,time_end, and creation of persons
        which are parameterized accordingly.
        The facility type at origin will be  landusetype_orig
        and at destination landusetype_dest.


        """
        tripnumber = int(scale*tripnumber_tot)
        print 'disaggregate_odflow', time_start, time_end, id_mode, tripnumber

        print ' id_activitytype_orig,id_activitytype_dest', id_activitytype_orig, id_activitytype_dest

        # print '  probs_orig',sum(probs_fac_orig),'\n',probs_fac_orig
        # print '  probs_dest',sum(probs_fac_dest),'\n',probs_fac_dest

        # is there a chance to find facilities to locate persons in
        # origin and destination zone

        #activitytypes= self.get_scenario().demand.activitytypes
        #ctivitynames = self.activitytypes.names
        #get_id_act = activitynames.get_id_from_index

        if (np.sum(probs_fac_orig) > 0) & (np.sum(probs_fac_dest) > 0):
            # if id_mode == self._id_mode_bike:
            #    are_bikeowner =  np.ones(tripnumber, dtype=np.bool)
            # else:
            #    # TODO: assign a default percentage of bike owners
            #    are_bikeowner =  np.zeros(tripnumber, dtype=np.bool)
            #times_start = np.random.randint(time_start,time_end,tripnumber)
            ids_person = self.make_multiple(tripnumber,
                                            ids_mode_preferred=id_mode * np.ones(tripnumber, dtype=np.int32),
                                            #times_start = times_start,
                                            )

            unitvec_int = np.ones(tripnumber, dtype=np.int32)
            unitvec_float = np.ones(tripnumber, dtype=np.int32)

            # activity timing
            #hours_start = hour_offset + np.array(times_start,dtype = np.float32)/3600
            #tol_orig = hour_begin_latest_orig+duration_max_orig-(hour_begin_earliest_orig+duration_min_orig)

            # print '  hours_start[:3]',hours_start[:3]
            # print '  tol_orig',tol_orig

            #hours_end_est = hours_start + hour_tripbudget
            #tol_dest = hour_begin_latest_dest-hour_begin_earliest_dest

            # print '  hours_end_est[:3]',hours_end_est[:3]
            # print '  tol_dest',tol_dest

            #self.map_id_act_to_ids_facattrconf[id_activitytype_orig][ids_person] = ids_fac[random_choice(tripnumber, probs_fac_orig)]
            #self.map_id_act_to_ids_facattrconf[id_activitytype_dest][ids_person] = ids_fac[random_choice(tripnumber, probs_fac_dest)]
            activities = self.get_activities()

            # fix first departure hours for first activity imposed by
            # OD flow data
            hours_end_earliest_orig = (hour_offset+float(time_start)/3600)*unitvec_float
            hours_end_latest_orig = (hour_offset+float(time_end)/3600)*unitvec_float

            duration_mean_orig = 0.5 * duration_min_orig+duration_max_orig
            hours_begin_earliest_orig = hours_end_earliest_orig-duration_mean_orig
            hours_begin_latest_orig = hours_end_latest_orig-duration_mean_orig

            # this estimate could be *optionally* replaced by preliminary routing
            hours_begin_earliest_dest = hours_end_earliest_orig+hour_tripbudget
            hours_begin_latest_dest = hours_end_latest_orig+hour_tripbudget

            #hours_end_earliest_dest = hours_begin_earliest_dest+duration_min_dest
            #hours_end_latest_dest = hours_begin_latest_dest+duration_max_dest

            ids_activity_orig = activities.add_rows(
                n=tripnumber,
                ids_activitytype=id_activitytype_orig * unitvec_int,
                ids_facility=ids_fac[random_choice(tripnumber, probs_fac_orig)],
                hours_begin_earliest=hours_begin_earliest_orig,
                hours_begin_latest=hours_begin_latest_orig,
                durations_min=duration_mean_orig-1.0/60.0 * unitvec_float,  # min mast be less than max to prevent crash
                durations_max=duration_mean_orig * unitvec_float,
            )

            ids_activity_dest = activities.add_rows(
                n=tripnumber,
                ids_activitytype=id_activitytype_dest * unitvec_int,
                ids_facility=ids_fac[random_choice(tripnumber, probs_fac_dest)],
                hours_begin_earliest=hours_begin_earliest_dest,
                hours_begin_latest=hours_begin_latest_dest,
                durations_min=duration_min_dest*unitvec_float,
                durations_max=duration_max_dest*unitvec_float,
            )

            for id_person, id_activity_orig, id_activity_dest in zip(ids_person, ids_activity_orig, ids_activity_dest):
                self.activitypatterns[id_person] = [id_activity_orig, id_activity_dest]

            #activitypatterns = np.zeros((tripnumber,2), dtype = np.int32)
            #activitypatterns[:,0]= ids_activity_orig
            #activitypatterns[:,1]= ids_activity_dest
            # try convert in this way to lists
            # print '  activitypatterns',activitypatterns.tolist()
            #self.activitypatterns[ids_person] = activitypatterns.tolist()
            # for id_person, act_pattern in zip(ids_person,activitypatterns.tolist()):
            #    self.activitypatterns[id_person] =  act_pattern

            return ids_person
        else:
            print 'WARNING in disaggregate_odflow: no probabilities', np.sum(probs_fac_orig), np.sum(probs_fac_dest)

            return []

    def create_pop_from_odflows(self, is_use_landusetypes=False, **kwargs):
        """
        Creates a population and defines home and activity facility
        according to OD matrix defined in odflows.
        The population is distributed within the zones according to 
        the area of the facility.
        if  landusetype_orig and landusetype_dest also landuse types
        of facilities of origin and destination are taken into account.
        """
        print 'create_pop_from_odflows'

        demand = self.parent
        odflowtab = demand.odintervals.generate_odflows()
        landuse = self.get_landuse()
        activitytypes = demand.activitytypes
        log = kwargs.get('logger', self.get_logger())

        if is_use_landusetypes:

            # TODO: not tested and works only for one landusetype per activity
            #activitytypes = self.activitytypes.get_value()
            #id_landusetype_orig = activitytypes.ids_landusetypes[kwargs['id_activitytype_orig']][0]
            #id_landusetype_dest = activitytypes.ids_landusetypes[kwargs['id_activitytype_dest']][0]
            pass
            # TODO: get activitypes from activity
            #probs_fac, ids_fac = self.get_landuse().facilities.get_departure_probabilities_landuse()

        else:
            probs_fac_area, ids_fac = landuse.facilities.get_departure_probabilities()

        # self._make_map_id_act_to_ids_facattrconf()
        ids_flow = odflowtab.get_ids()
        n_flows = len(ids_flow)

        ids_activitytype_orig = odflowtab.ids_activitytype_orig[ids_flow]
        ids_activitytype_dest = odflowtab.ids_activitytype_dest[ids_flow]

        i = 0.0
        for id_flow,\
            id_orig,\
            id_dest,\
            id_mode,\
            time_start,\
            time_end,\
            tripnumber,\
            id_activitytype_orig,\
            id_activitytype_dest,\
            hour_begin_earliest_orig,\
            hour_begin_earliest_dest,\
            hour_begin_latest_orig,\
            hour_begin_latest_dest,\
            duration_min_orig,\
            duration_min_dest,\
            duration_max_orig,\
            duration_max_dest\
            in zip(ids_flow,
                   odflowtab.ids_orig[ids_flow],
                   odflowtab.ids_dest[ids_flow],
                   odflowtab.ids_mode[ids_flow],
                   odflowtab.times_start[ids_flow],
                   odflowtab.times_end[ids_flow],
                   odflowtab.tripnumbers[ids_flow],
                   ids_activitytype_orig,
                   ids_activitytype_dest,
                   activitytypes.hours_begin_earliest[ids_activitytype_orig],
                   activitytypes.hours_begin_earliest[ids_activitytype_dest],
                   activitytypes.hours_begin_latest[ids_activitytype_orig],
                   activitytypes.hours_begin_latest[ids_activitytype_dest],
                   activitytypes.durations_min[ids_activitytype_orig],
                   activitytypes.durations_min[ids_activitytype_dest],
                   activitytypes.durations_max[ids_activitytype_orig],
                   activitytypes.durations_max[ids_activitytype_dest],
                   ):

            log.progress(i/n_flows*100)
            i += 1
            if is_use_landusetypes:
                # TODO: not tested and works only for one landusetype per activity
                # but in activity typrs several landuse types are defined

                # idea: add the probabilities for landuse types of origin and dest
                #probs_fac_orig = probs_fac[id_orig][id_landusetype_orig]
                #probs_fac_dest = probs_fac[id_dest][id_landusetype_dest]
                pass
            else:
                probs_fac_orig = probs_fac_area[id_orig]
                probs_fac_dest = probs_fac_area[id_dest]

            self.disaggregate_odflow(time_start,
                                     time_end,
                                     id_mode,
                                     ids_fac,
                                     probs_fac_orig,
                                     probs_fac_dest,
                                     tripnumber,
                                     id_activitytype_orig,
                                     id_activitytype_dest,
                                     hour_begin_earliest_orig,
                                     hour_begin_earliest_dest,
                                     hour_begin_latest_orig,
                                     hour_begin_latest_dest,
                                     duration_min_orig,
                                     duration_min_dest,
                                     duration_max_orig,
                                     duration_max_dest,
                                     **kwargs
                                     )

        # return odflowtab

    def add_plans(self, ids_person, id_strategy=-1):
        print 'add_plans n, id_strategy', len(ids_person), id_strategy
        n_plans = len(ids_person)
        # print '  get_plans',self.get_plans()
        # print '  stagetables',self.get_plans().get_stagetables().get_ident_abs()
        # print '  stagetables',self.get_plans().get_stagetables().stagetables.get_value()
        ids_plan = self.get_plans().add_rows(n=n_plans,
                                             ids_person=ids_person,
                                             ids_strategy=id_strategy*np.ones(n_plans, dtype=np.int32),
                                             )

        # print '  post stagetables',self.get_plans().get_stagetables().get_ident_abs()
        # print '  post stagetables',self.get_plans().get_stagetables().stagetables.get_value()

        # return ids_plan
        self.ids_plan[ids_person] = 1*ids_plan

        for id_person, id_plan in zip(ids_person, ids_plan):
            if self.lists_ids_plan[id_person] is None:
                self.lists_ids_plan[id_person] = [id_plan]
            else:
                self.lists_ids_plan[id_person].append(id_plan)
        return ids_plan

    def plan_with_strategy(self, id_strategy, evalcrit=0, logger=None):
        strategy = self.get_strategies().strategies[id_strategy]
        ids_person = self.get_ids()
        evals = strategy.preevaluate(ids_person)
        # TODO: check whether at least two activities are in
        # activitypattern...could be done centrally

        ids_person_preeval = ids_person[evals >= evalcrit]
        print 'plan_with_strategy', strategy.ident, 'n_pers', len(ids_person_preeval)
        strategy.plan(ids_person_preeval, logger=logger)

    # def get_times(self, ind, ids_person = None, pdf = 'unit'):
    #    """
    #    Returns person IDs, activity IDs and initial times
    #    for persons with at least one acivity.
    #
    #    ids_person: array of preselected person IDs
    #
    #    pdf: gives the probability density function to be chosen to determin
    #    the departure times within the initial time intervals given by
    #    initial activity attributes.
    #    """
    #    ids_person, ids_activity = self.get_activity_from_pattern(0,ids_person)
    #    times_init = self.get_activities().get_times_init(ids_activity, pdf)
    #
    #    return ids_person, ids_activity, times_init

    def get_activities_from_pattern(self, ind, ids_person=None):
        """
        Returns person IDs and from/to activity IDs for persons who perform an activity
        at the given activity index ind.
        Returns arrays: ids_person, ids_activity_from, ids_activity_to

        ind: index of activity in activity pattern, starting with 0
        ids_person: array of preselected person IDs


        """

        ids_person_activity = []
        ids_activity_from = []
        ids_activity_to = []
        if ids_person is None:
            ids_person = self.get_ids()

        for id_person, activitypattern in zip(ids_person, self.activitypatterns[ids_person]):
            # has person activity at index ind?
            if len(activitypattern) > ind+1:
                ids_person_activity.append(id_person)
                ids_activity_from.append(activitypattern[ind])
                ids_activity_to.append(activitypattern[ind+1])

        return ids_person_activity, ids_activity_from, ids_activity_to

        #        activities.hours_begin_earliest[ids_person_activity],
        #        activities.hours_begin_latest[ids_person_activity],
        #        activities.durations_min[ids_person_activity],
        #        activities.durations_max[ids_person_activity],

    def get_vtypes(self):

        ids_vtypes = set()
        # get individual vehicle types
        ids_vtypes.update(self.get_iautos().ids_vtype.get_value())
        ids_vtypes.update(self.get_imotos().ids_vtype.get_value())
        ids_vtypes.update(self.get_ibikes().ids_vtype.get_value())

        # add public transport
        ids_vtypes.update(self.get_ptlines().ids_vtype.get_value())

        # add pedestrian types
        ids_vtypes.update(self.ids_vtype.get_value())

        return ids_vtypes

    def select_plans_preferred_mode(self, fraction=0.1, **kwargs):
        """
        Selects current plant to satisfy best the preferred mode.
        """

        strategies = self.get_strategies()
        ids_strat = strategies.get_ids()
        n_strat = len(ids_strat)
        ids_pers_all = self.get_ids()
        ids_pers = ids_pers_all[np.random.random(len(ids_pers_all)) > (1.0-fraction)]
        n_pers = len(ids_pers)
        preevals = -1*np.ones((np.max(ids_pers)+1, np.max(ids_strat)+1), dtype=np.int32)
        for ind, id_strategy, strategy in zip(range(n_strat), ids_strat, strategies.strategies[ids_strat]):
            preevals[ids_pers, id_strategy] = strategy.preevaluate(ids_pers)

        preferred = 2
        plans = self.get_plans()
        self.ids_plan.reset()
        for id_pers, ids_plan in zip(ids_pers, self.lists_ids_plan[ids_pers]):
            if len(ids_plan) > 0:
                # print '  id_pers,ids_plan',id_pers,ids_plan
                # print '  ids_strat, preeval',plans.ids_strategy[ids_plan],preevals[id_pers,plans.ids_strategy[ids_plan]]
                inds_sel = preevals[id_pers, plans.ids_strategy[ids_plan]] == preferred
                # print '  inds_sel',inds_sel,np.flatnonzero(inds_sel),inds_sel.dtype
                if len(inds_sel) > 0:
                    #ids_plan_sel = np.array(ids_plan)[inds_sel]
                    # print '    ids_plan_sel',ids_plan_sel
                    # at least one plan contains preferred mode
                    self.ids_plan[id_pers] = np.array(ids_plan)[inds_sel][0]  # whu [1]?
                # else:
                #    assumption: a plan for the preferred mode always exists
                #    # no preferred mode found try to satisfy best possible
                #    #ids_plan[preevals[id_pers,plans.ids_strategy[ids_plan]] == preferred]
                #    self.ids_plan[id_pers] = -1
        return True

    def select_plans_min_time_est(self, fraction=1.0, timedev=-1.0, c_probit=-1.0, **kwargs):
        """
        Select plan with minimum estimated travel time as current plant.
        """
        ids_pers_all = self.get_ids()
        ids_pers = ids_pers_all[np.random.random(len(ids_pers_all)) > (1.0-fraction)]
        times_est = self.get_plans().times_est
        # self.ids_plan.reset()
        for id_pers, ids_plan_all in zip(ids_pers, self.lists_ids_plan[ids_pers]):
            ids_plan = np.array(ids_plan_all, dtype=np.int32)[times_est[ids_plan_all] > 0.1]
            if len(ids_plan) > 0:
                # print '  id_pers,ids_plan',id_pers,ids_plan
                if timedev > 0.1:
                    times_rand = np.random.normal(0.0, timedev, len(ids_plan))
                elif c_probit > 0:
                    times_rand = np.zeros(len(ids_plan), dtype=np.float32)
                    for i, t in zip(xrange(len(ids_plan)), times_est[ids_plan]):
                        times_rand[i] = np.random.normal(0.0, c_probit * t, 1)

                else:
                    times_rand = np.zeros(len(ids_plan), dtype=np.float32)
                self.ids_plan[id_pers] = np.array(ids_plan)[np.argmin(times_est[ids_plan]+times_rand)]
        return True

    def select_plans_random(self, fraction=0.1,  **kwargs):
        """
        A fraction of the population changes a plan. 
        The new plans are chosen randomly. 
        """

        ids_pers_all = self.get_ids()
        print 'select_plans_random', len(ids_pers_all), fraction
        times_est = self.get_plans().times_est
        # self.ids_plan.reset()
        # ids_mode[random_choice(n,shares/np.sum(shares))]

        ids_pers = ids_pers_all[np.random.random(len(ids_pers_all)) > (1.0-fraction)]
        print '  ids_pers', ids_pers
        for id_pers, ids_plan in zip(ids_pers, self.lists_ids_plan[ids_pers]):
            if len(ids_plan) > 0:
                # print '  id_pers,ids_plan',id_pers,ids_plan
                self.ids_plan[id_pers] = ids_plan[np.random.randint(len(ids_plan))]
        return True

    def select_plans_min_time_exec(self, fraction=0.1, timedev=-1, c_probit=-1, **kwargs):
        """
        Select plan with minimum executed travel time as current plant.
        """
        ids_pers_all = self.get_ids()
        # print 'select_plans_random',len(ids_pers_all),fraction
        ids_pers = ids_pers_all[np.random.random(len(ids_pers_all)) > (1.0-fraction)]
        times_exec = self.get_plans().times_exec
        # self.ids_plan.reset()
        for id_pers, ids_plan_all in zip(ids_pers, self.lists_ids_plan[ids_pers]):
            # print '    ids_plan_all',ids_plan_all,type(ids_plan_all)
            ids_plan = np.array(ids_plan_all, dtype=np.int32)[times_exec[ids_plan_all] > 0.1]
            if len(ids_plan) > 0:
                # print '  id_pers,ids_plan',id_pers,ids_plan
                if timedev > 0.1:
                    times_rand = np.random.normal(0.0, timedev, len(ids_plan))
                elif c_probit > 0:
                    times_rand = np.zeros(len(ids_plan), dtype=np.float32)
                    for i, t in zip(xrange(len(ids_plan)), times_exec[ids_plan]):
                        times_rand[i] = np.random.normal(0.0, c_probit * t, 1)

                else:
                    times_rand = np.zeros(len(ids_plan), dtype=np.float32)

                self.ids_plan[id_pers] = np.array(ids_plan)[np.argmin(times_exec[ids_plan]+times_rand)]
        return True

    def select_plans_min_time_exec_est(self, fraction=0.1, timedev=-1, c_probit=-1, **kwargs):
        """
        Select plan with minimum executed or estimated (if executed doesn't exist) travel time as current plant.
        """
        n_analyzed_persons = 0
        ids_pers_all = self.get_ids()
        ids_pers = ids_pers_all[np.random.random(len(ids_pers_all)) > (1.0-fraction)]
        times_exec = self.get_plans().times_exec
        times_est = self.get_plans().times_est
        ids_plans = self.get_plans().get_ids()
        for id_pers, ids_plan_all in zip(ids_pers, self.lists_ids_plan[ids_pers]):
            ids_plan_est = np.array(ids_plan_all, dtype=np.int32)[times_est[ids_plan_all] > 0.1]
            ids_plan_exec = np.array(ids_plan_all, dtype=np.int32)[times_exec[ids_plan_all] > 0.1]
            if len(ids_plan_est) > 0:
                if len(ids_plan_est) == len(ids_plan_exec):
                    ids_plan_est = []
                else:
                    c = np.zeros(len(ids_plan_est))
                    for x in range(len(ids_plan_est)):
                        for y in range(len(ids_plan_exec)):
                            if ids_plan_est[x] == ids_plan_exec[y]:
                                c[x] = 1
                    d = np.delete(ids_plan_est, np.flatnonzero(c))
                    ids_plan_est = d

                n_analyzed_persons += 1

                if timedev > 0.1:
                    if len(ids_plan_est) > 0:
                        times_rand_est = np.random.normal(0.0, timedev, len(ids_plan_est))
                    if len(ids_plan_exec) > 0:
                        times_rand_exec = np.random.normal(0.0, timedev, len(ids_plan_exec))

                elif c_probit > 0:
                    if len(ids_plan_est) > 0:
                        times_rand_est = np.zeros(len(ids_plan_est), dtype=np.float32)
                        for i, t in zip(xrange(len(ids_plan_est)), times_est[ids_plan_est]):
                            times_rand_est[i] = np.random.normal(0.0, c_probit * t, 1)
                    if len(ids_plan_exec) > 0:
                        times_rand_exec = np.zeros(len(ids_plan_exec), dtype=np.float32)
                        for i, t in zip(xrange(len(ids_plan_exec)), times_exec[ids_plan_exec]):
                            times_rand_exec[i] = np.random.normal(0.0, c_probit * t, 1)

                else:
                    if len(ids_plan_exec) > 0:
                        times_rand_exec = np.zeros(len(ids_plan_exec), dtype=np.float32)
                    if len(ids_plan_est) > 0:
                        times_rand_est = np.zeros(len(ids_plan_est), dtype=np.float32)

                if len(ids_plan_exec) > 0 and len(ids_plan_est) > 0:
                    if min(times_exec[ids_plan_exec]+times_rand_exec) < min(times_est[ids_plan_est]+times_rand_est):
                        self.ids_plan[id_pers] = np.array(ids_plan_exec)[np.argmin(
                            times_exec[ids_plan_exec]+times_rand_exec)]
                    else:
                        self.ids_plan[id_pers] = np.array(
                            ids_plan_est)[np.argmin(times_est[ids_plan_est]+times_rand_est)]
                elif len(ids_plan_exec) == 0:
                    self.ids_plan[id_pers] = np.array(ids_plan_est)[np.argmin(times_est[ids_plan_est]+times_rand_est)]

                else:

                    self.ids_plan[id_pers] = np.array(ids_plan_exec)[np.argmin(
                        times_exec[ids_plan_exec]+times_rand_exec)]

        print 'were analyzed  %d persons' % (n_analyzed_persons)
        return True

    def select_plans_next(self, fraction=0.1, **kwargs):
        """
        Select next plan in the plan list as current plant.
        """
        # print 'select_plans_next'
        ids_pers_all = self.get_ids()
        ids_pers = ids_pers_all[np.random.random(len(ids_pers_all)) > (1.0-fraction)]
        for id_pers, id_plan_current, ids_plan in zip(ids_pers, self.ids_plan[ids_pers], self.lists_ids_plan[ids_pers]):
            n_plan = len(ids_plan)
            if n_plan > 0:
                # print '  id_pers,id_plan_current',id_pers,id_plan_current,ids_plan,id_plan_current != -1
                if id_plan_current != -1:
                    ind = ids_plan.index(id_plan_current)
                    # print '    ind',ind,ind +1 < n_plan
                    if ind + 1 < n_plan:
                        ind += 1
                    else:
                        ind = 0
                else:
                    ind = 0

                # print '    ind,n_plan',ind,n_plan,'ids_plan[ind]', ids_plan[ind]
                self.ids_plan[id_pers] = ids_plan[ind]
        # print '  finally: ids_plan=',self.ids_plan.get_value()
        return True

    def prepare_sim(self, process):
        return []  # [(steptime1,func1),(steptime2,func2),...]

    def get_trips(self):
        # returns trip object, method common to all demand objects
        return self.get_iautos()

    def get_writexmlinfo(self, is_route=False):
        """
        Returns three array where the first array is the 
        begin time of the first vehicle and the second array is the
        write function to be called for the respectice vehicle and
        the third array contains the vehicle ids

        Method used to sort trips when exporting to route or trip xml file
        """
        print 'Virtualpop.get_writexmlinfo'
        plans = self.get_plans()

        ids_pers = self.select_ids(self.ids_plan.get_value() >= 0)
        n_pers = len(ids_pers)
        ids_plans = self.ids_plan[ids_pers]

        # get vehicle trip info
        times_depart_bike, writefuncs_bike, ids_rides_bike = plans.get_stagetable(
            'bikerides').get_writexmlinfo(ids_plans, is_route)
        times_depart_auto, writefuncs_auto, ids_rides_auto = plans.get_stagetable(
            'autorides').get_writexmlinfo(ids_plans, is_route)

        #self.add_stagetable('walks', WalkStages)
        #self.add_stagetable('autorides', AutorideStages)
        #self.add_stagetable('bikerides', BikerideStages)
        #self.add_stagetable('transits', TransitStages)
        #self.add_stagetable('activities', ActivityStages)

        #rides = plans.get_stagetable('autorides')

        # do persons

        times_depart_pers = plans.times_begin[ids_plans]
        writefuncs_pers = np.zeros(n_pers, dtype=np.object)
        writefuncs_pers[:] = self.write_person_xml

        # assemble vectors
        print '  times_depart_pers.shape', times_depart_pers.shape
        print '  times_depart_bike.shape', times_depart_bike.shape
        print '  times_depart_auto.shape', times_depart_auto.shape
        times_depart = np.concatenate((times_depart_pers,
                                       times_depart_auto,
                                       times_depart_bike,
                                       ))

        writefuncs = np.concatenate((writefuncs_pers,
                                     writefuncs_auto,
                                     writefuncs_bike,
                                     ))

        ids = np.concatenate((ids_pers,
                              ids_rides_auto,
                              ids_rides_bike,
                              ))

        return times_depart, writefuncs, ids

    def write_person_xml(self, fd, id_pers, time_begin, indent=2):

        stages = self.get_plans().get_stages(self.ids_plan[id_pers])

        fd.write(xm.start('person', indent=indent+2))
        fd.write(xm.num('id', self.get_id_sumo_from_id(id_pers)))
        # fd.write(xm.num('depart',self.times_start[id_pers]))
        fd.write(xm.num('depart', time_begin))
        fd.write(xm.num('type', self.parent.vtypes.ids_sumo[self.ids_vtype[id_pers]]))

        activity_init, id_stage_init = stages[0]
        id_edge_init, pos_init = activity_init.get_edges_positions(id_stage_init)

        # self.ids_edge_depart.write_xml(fd,id_trip)
        # self.positions_depart.write_xml(fd,id_trip)
        fd.write(xm.num('from', self._edges.ids_sumo[id_edge_init]))
        fd.write(xm.num('departPos', pos_init))

        fd.write(xm.stop())

        # write stages of this person.
        # Attention!! first and last stage, which are activities,
        # will NOT be exportes , therefore [1:-1]
        for stage, id_stage in stages[1:-1]:
            stage.to_xml(id_stage, fd, indent+4)

        fd.write(xm.end('person', indent=indent+2))

    def config_results(self, results):

        results.add_resultobj(res.Personresults('virtualpersonresults', results,
                                                self,
                                                self.get_net().edges,
                                                name='Virtual person results',
                                                info='Table with simulation results for person of the virtual population. The results refer to all trips made by the person during the entire simulation period.',
                                                ), groupnames=['Trip results'])

        results.add_resultobj(res.Vehicleresults('iautotripresults', results,
                                                 self.get_iautos(),
                                                 self.get_net().edges,
                                                 name='Auto trip results',
                                                 info='Table with trip results mad with individual autos. The results refer to all trips made by a specific vehicle during the entire simulation period.',
                                                 ), groupnames=['Trip results'])

        results.add_resultobj(res.Vehicleresults('ibiketripresults', results,
                                                 self.get_ibikes(),
                                                 self.get_net().edges,
                                                 name='Bike trip results',
                                                 info='Table with trip results mad with individual bikes. The results refer to all trips made by a specific vehicle during the entire simulation period.',
                                                 ), groupnames=['Trip results'])

    # def process_results(self, results, process = None):
    #    print 'process_results'
    #    ## copy total travel into plan execution time
    #    personresults = results.virtualpersonresults
    #    self.update_plans(personresults)

    def update_results(self, personresults):
        """
        Updates plans with results from previous 
        simulation run, and updates plan choice
        """

        ids_res = personresults.get_ids()
        print 'update_results', len(ids_res)
        ids_person = personresults.ids_person[ids_res]
        ids_plan = self.ids_plan[ids_person]
        self.get_plans().times_exec[ids_plan] = personresults.times_travel_total[ids_res]
        # change mobility plan based on updated travel times
        pass


class PopGenerator(Process):
    def __init__(self, ident='virtualpopgenerator', virtualpop=None,  logger=None, **kwargs):
        print 'PopFromOdfGenerator.__init__ ', ident, virtualpop

        # TODO: let this be independent, link to it or child??
        #
        scenario = virtualpop.get_scenario()
        self._init_common(ident,
                          parent=virtualpop,
                          name='Population generator',
                          logger=logger,
                          info='Create virtual population from basic statistical data.',
                          )

        attrsman = self.set_attrsman(cm.Attrsman(self))

        # make for each possible pattern a field for prob
        activitytypes = self.parent.get_scenario().demand.activitytypes

        self.n_person = attrsman.add(cm.AttrConf('n_person', kwargs.get('n_person', 1000),
                                                 groupnames=['options'],
                                                 perm='rw',
                                                 name='Number of person',
                                                 info='Number of adult persons.',
                                                 ))

        self.ids_acttype_default = activitytypes.get_ids_from_formatted('home,work')
        # self.ids_acttype = attrsman.add(cm.AttrConf( 'ids_acttype',kwargs.get('id_acttype',activitytypes.get_id_from_formatted('home')),
        #                    groupnames = ['options'],
        #                    choices = activitytypes.names.get_indexmap(),
        #                    perm='rw',
        #                    name = 'Activity type',
        #                    info = 'Initial activity type.',
        #                    ))

        self.ttb_mean = attrsman.add(cm.AttrConf('ttb_mean', kwargs.get('ttb_mean', 55*60),
                                                 groupnames=['options'],
                                                 perm='rw',
                                                 name='Avg. of 24h travel time budget',
                                                 unit='s',
                                                 info="""Average travel time budget for one day. 
                            This time excludes time for activities.
                             """,
                                                 ))
        self.ttb_dev = attrsman.add(cm.AttrConf('ttb_dev', kwargs.get('ttb_dev', 10*60),
                                                groupnames=['options'],
                                                perm='rw',
                                                name='Std. of 24h travel time budget',
                                                unit='s',
                                                info="""Standard deviation of travel time budget for one day. 
                            """,
                                                ))

        mode_to_id = self.parent.get_scenario().net.modes.get_id_mode
        self.share_pedestrian = attrsman.add(cm.AttrConf('share_pedestrian', kwargs.get('share_pedestrian', 0.1),
                                                         groupnames=['options', 'modal split'],
                                                         perm='rw',
                                                         id_mode=mode_to_id('pedestrian'),
                                                         name='Pedestrian share',
                                                         info="""Share of pedestrians.""",
                                                         ))

        self.share_autouser = attrsman.add(cm.AttrConf('share_autouser', kwargs.get('share_autouser', 0.5),
                                                       groupnames=['options', 'modal split'],
                                                       perm='rw',
                                                       id_mode=mode_to_id('passenger'),
                                                       name='Auto user share',
                                                       info="""Share of auto users.""",
                                                       ))

        self.share_motorcycleuser = attrsman.add(cm.AttrConf('share_motorcycleuser', kwargs.get('share_motorcycleuser', 0.1),
                                                             groupnames=['options', 'modal split'],
                                                             perm='rw',
                                                             id_mode=mode_to_id('motorcycle'),
                                                             name='Motorcycle user share',
                                                             info="""Share of Motorcycle users.""",
                                                             ))

        self.share_bikeuser = attrsman.add(cm.AttrConf('share_bikeuser', kwargs.get('share_bikeuser', 0.1),
                                                       groupnames=['options', 'modal split'],
                                                       perm='rw',
                                                       id_mode=mode_to_id('bicycle'),
                                                       name='Bike user share',
                                                       info="""Share of bike users.""",
                                                       ))

        self.share_ptuser = attrsman.add(cm.AttrConf('share_ptuser', kwargs.get('share_ptuser', 0.2),
                                                     groupnames=['options', 'modal split'],
                                                     id_mode=mode_to_id('bus'),
                                                     perm='rw',
                                                     name='PT share',
                                                     info="""Share of public transport user.""",
                                                     ))

        #self.modeshares = attrsman.add( cm.ObjConf(ModeShares('modeshares',self,scenario.net.modes),groupnames = ['options']) )

    def do(self):
        print 'PopGenerator.do'
        # links

        virtualpop = self.parent
        virtualpop.clear_population()
        logger = self.get_logger()
        #logger.w('Update Landuse...')
        scenario = virtualpop.get_scenario()
        activitytypes = scenario.demand.activitytypes
        facilities = scenario.landuse.facilities
        edges = scenario.net.edges

        ids_fac = facilities.get_ids()
        map_id_edge_to_ids_fac = {}
        for id_fac, id_edge in zip(ids_fac, facilities.ids_roadedge_closest[ids_fac]):
            if map_id_edge_to_ids_fac.has_key(id_edge):
                map_id_edge_to_ids_fac[id_edge].append(id_fac)
            else:
                map_id_edge_to_ids_fac[id_edge] = [id_fac, ]

        n_pers = self.n_person
        unitvec_int = np.ones(n_pers, dtype=np.int32)

        ids_person = virtualpop.make_multiple(n_pers)
        virtualpop.traveltimebudgets[ids_person] = self.get_ttb(ids_person)

        virtualpop.ids_mode_preferred[ids_person] = self.get_modes_random(n_pers)

        # here we could preselect correct landuse based on
        # percentage of workers, students, employees
        prob_fac_to = facilities.capacities[ids_fac].astype(np.float32)
        prob_fac_to /= np.sum(prob_fac_to)
        # print '  np.sum(prob_fac_to)',np.sum(prob_fac_to)
        ids_fac_to = ids_fac[random_choice(n_pers, prob_fac_to)]

        # determine id_fac_from by backward routing from id_fac_to
        ids_edge_to = facilities.ids_roadedge_closest[ids_fac_to]

        # pre calculate backward star and mode dependent link travel times
        bstar = edges.get_bstar()
        edgetimes = {}
        ids_mode = self.get_ids_mode()
        # idea: do also consider gradient of house prices
        for id_mode, speed_max in zip(ids_mode, scenario.net.modes.speeds_max[ids_mode]):
            edgetimes[id_mode] = edges.get_times(id_mode=id_mode,
                                                 speed_max=speed_max,
                                                 is_check_lanes=True
                                                 )
        # determine home facilities by backwards tracking from work facility
        ids_fac_from = np.ones(n_pers, dtype=np.int32)
        i = 0
        for id_person, id_edge_to, id_mode, ttb\
                in zip(ids_person,
                       ids_edge_to,
                       virtualpop.ids_mode_preferred[ids_person],
                       virtualpop.traveltimebudgets[ids_person],
                       ):

            # print '  Backsearch',id_person,'id_edge_to',id_edge_to,edges.ids_sumo[id_edge_to],'ttb[s]',0.5*ttb
            ids_edge_from, costs, btree = routing.edgedijkstra_backwards(id_edge_to,
                                                                         0.5*ttb,  # to be specified better
                                                                         weights=edgetimes[id_mode],
                                                                         bstar=bstar,
                                                                         )
            if len(ids_edge_from) == 0:
                # routing failed to deliver edges of origins
                # put work and home on same edge
                ids_edge_from = [id_edge_to, ]

            # look at all edges of origin and pick most likely facility
            ids_fac_lim = []
            for id_edge_from in ids_edge_from:

                #id_from_check = id_edge_from
                # print '  check from',id_from_check,'back to',id_edge_to,'time =%.2fs'%costs[id_from_check]
                # while id_from_check != id_edge_to:
                #    id_from_check = btree[id_from_check]
                #    #print '  id_edge = ',id_from_check
                # print '  success = ',id_from_check==id_edge_to
                if map_id_edge_to_ids_fac.has_key(id_edge_from):
                    ids_fac_lim += map_id_edge_to_ids_fac[id_edge_from]

            if len(ids_fac_lim) == 0:
                # no facilities at all destinations found
                # go edges backawards and search there
                # this will reduce travel time
                for id_edge_from in ids_edge_from:
                    # verify if id_edge_from has facilities.
                    while not map_id_edge_to_ids_fac.has_key(id_edge_from):
                        # print '  no facility, go backward'
                        id_edge_from = btree[id_edge_from]

            ids_fac_lim = np.array(ids_fac_lim, dtype=np.int32)

            prob_fac_from = facilities.capacities[ids_fac_lim].astype(np.float32)
            prob_fac_from /= np.sum(prob_fac_from)
            # print '  np.sum(prob_fac_to)',np.sum(prob_fac_to)
            ids_fac_from[i] = ids_fac[random_choice(1, prob_fac_to)]
            i += 1

        # idea: adjust wake-up time with employment type
        activities = virtualpop.get_activities()
        ids_activity_from = activities.add_rows(
            n=n_pers,
            ids_activitytype=self.ids_acttype_default[0] * unitvec_int,
            ids_facility=ids_fac_from,
            # use default
            #hours_begin_earliest = None,
            #hours_begin_latest = None,
            #durations_min = None,
            #durations_max = None,
        )

        ids_activity_to = activities.add_rows(
            n=n_pers,
            ids_activitytype=self.ids_acttype_default[1] * unitvec_int,
            ids_facility=ids_fac_to,
            # use default
            #hours_begin_earliest = None,
            #hours_begin_latest = None,
            #durations_min = None,
            #durations_max = None,
        )

        for id_person, id_activity_from, ids_activity_to in zip(ids_person, ids_activity_from, ids_activity_to):
            virtualpop.activitypatterns[id_person] = [id_activity_from, ids_activity_to, ]

        return True

    def get_ids_mode(self):
        modesplitconfigs = self.get_attrsman().get_group('modal split')
        ids_mode = np.zeros(len(modesplitconfigs), dtype=np.int32)
        i = 0
        for modeconfig in modesplitconfigs:
            ids_mode[i] = modeconfig.id_mode
            i += 1
        return ids_mode

    def get_modes_random(self, n):
        """
        Return a vector with mode IDs of length n. 
        """
        # print 'get_modes_random',n
        modesplitconfigs = self.get_attrsman().get_group('modal split')
        ids_mode = np.zeros(len(modesplitconfigs), dtype=np.int32)
        shares = np.zeros(len(modesplitconfigs), dtype=np.float32)
        i = 0
        for modeconfig in modesplitconfigs:
            ids_mode[i] = modeconfig.id_mode
            shares[i] = modeconfig.get_value()
            i += 1
        # print '  ids_mode',ids_mode
        # print '  shares',shares
        return ids_mode[random_choice(n, shares/np.sum(shares))]

    def get_ttb(self, ids_pers):
        n_pers = len(ids_pers)

        # Truncated Normal dist with scipy
        # load libraries
        #import scipy.stats as stats
        # lower, upper, mu, and sigma are four parameters
        #lower, upper = 0.5, 1
        #mu, sigma = 0.6, 0.1
        # instantiate an object X using the above four parameters,
        #X = stats.truncnorm((lower - mu) / sigma, (upper - mu) / sigma, loc=mu, scale=sigma)
        # generate 1000 sample data
        #samples = X.rvs(1000)

        return np.random.normal(self.ttb_mean, self.ttb_dev, n_pers).clip(0, 2*3600)


class PopFromOdfGenerator(Process):
    def __init__(self, ident, virtualpop,  logger=None, **kwargs):
        print 'PopFromOdfGenerator.__init__'

        # TODO: let this be independent, link to it or child??

        self._init_common(ident,
                          parent=virtualpop,
                          name='Pop from OD-flow generator',
                          logger=logger,
                          info='Create virtual population from origin-to-destination zone flows by disaggregation.',
                          )

        attrsman = self.set_attrsman(cm.Attrsman(self))

        # make for each possible pattern a field for prob
        activitytypes = self.parent.get_scenario().demand.activitytypes

        self.hour_offset = attrsman.add(cm.AttrConf('hour_offset', kwargs.get('hour_offset', 8.0),
                                                    groupnames=['options'],
                                                    perm='rw',
                                                    name='Offset hours',
                                                    unit='h',
                                                    info='Hour when simulation starts. This is the hour (of the day) when simulation time shows zero seconds.',
                                                    ))

        self.hour_tripbudget = attrsman.add(cm.AttrConf('hour_tripbudget', kwargs.get('hour_tripbudget', 0.5),
                                                        groupnames=['options'],
                                                        perm='rw',
                                                        name='Triptime budget',
                                                        unit='h',
                                                        info="""Time budget for this trip. This time is used
                             to initially estimate the time in hours between 
                             the activity at origin and the activity
                             at destination.
                             """,
                                                        ))

        self.scale = attrsman.add(cm.AttrConf('scale', kwargs.get('scale', 1.0),
                                              groupnames=['options'],
                                              perm='rw',
                                              name='Scale',
                                              info='Global scale factor. Scales the number of all OD trips.',
                                              ))

        self.is_use_landusetypes = attrsman.add(cm.AttrConf('is_use_landusetypes', kwargs.get('is_use_landusetypes', False),
                                                            groupnames=['options'],
                                                            perm='rw',
                                                            name='use landuse types',
                                                            info="""If True, use the landuse type of 
                            facilities when assigning the origin and destination facility. 
                            The landuse type is selected according to the activity type.
                            Use this option only if landuse types have been correctly defined for 
                            all facilities.
                            """,
                                                            ))

        self.is_update_landuse = attrsman.add(cm.AttrConf('is_update_landuse', kwargs.get('is_update_landuse', True),
                                                          groupnames=['options'],
                                                          perm='rw',
                                                          name='update Landuse',
                                                          info="""If True, update land use database (zones, facilities, parking) before generating the population. Updating means identifying edges and facilities within zones.
                            """,
                                                          ))

    def do(self):
        print 'PopFromOdfGenerator.do'
        # links

        virtualpop = self.parent
        logger = self.get_logger()
        if self.is_update_landuse:
            logger.w('Update Landuse...')
            scenario = virtualpop.get_scenario()
            scenario.landuse.zones.refresh_zoneedges()
            scenario.landuse.facilities.identify_taz()
            scenario.landuse.facilities.identify_closest_edge()
            scenario.landuse.facilities.update()

        logger.w('Create population...')
        virtualpop.create_pop_from_odflows(logger=logger, **self.get_kwoptions())
        #activitytypes = virtualpop.activitytypes
        return True


class Planner(Process):
    def __init__(self, ident='planner', virtualpop=None, strategy='all', logger=None, **kwargs):
        print 'Planner.__init__'

        # TODO: let this be independent, link to it or child??

        self._init_common(ident,
                          parent=virtualpop,
                          name='Planner',
                          logger=logger,
                          info='Generates mobility plan for population for a specific mobility strategy. Plans are only generated for persons for whome the strategy is applicable.',
                          )

        attrsman = self.set_attrsman(cm.Attrsman(self))

        # make for each possible pattern a field for prob
        strategies = virtualpop.get_strategies()
        strategychoices = {'all': -1}
        strategychoices.update(strategies.names.get_indexmap())
        self.id_strategy = attrsman.add(cm.AttrConf('id_strategy', strategychoices[strategy],
                                                    groupnames=['options'],
                                                    choices=strategychoices,
                                                    perm='rw',
                                                    name='Strategy',
                                                    info='Strategy to be used to create mobility plane. In case of all strategies, the planner generates all applicable plans.',
                                                    ))

        evalcrits = {'apply to all persons if feasible': 0,
                     'apply only if preferred mode is used': 1,
                     'apply only if exclusively preferred mode is used': 2,
                     }
        self.evalcrit = attrsman.add(cm.AttrConf('evalcrit', kwargs.get('evalcrit', evalcrits['apply to all persons if feasible']),
                                                 groupnames=['options'],
                                                 choices=evalcrits,
                                                 perm='rw',
                                                 name='Application criteria',
                                                 info=""" Value that determines for which persons the plans will be generated.
                                        Apply to all persons if feasible:0
                                        Apply only if preferred mode is used:1
                                        Apply only if exclusively preferred mode is used:2
                            """,
                                                 ))

    def do(self):
        print 'Planner.do'
        # links

        virtualpop = self.parent
        logger = self.get_logger()
        #logger.w('Check applicability')
        #strategies = virtualpop.strategies.get_value()

        if self.id_strategy != -1:
            virtualpop.plan_with_strategy(self.id_strategy, evalcrit=self.evalcrit, logger=logger)

        else:  # plan with all strategies
            for id_strategy in virtualpop.get_strategies().get_ids():
                virtualpop.plan_with_strategy(id_strategy, evalcrit=self.evalcrit, logger=logger)

        return True


class PlanSelector(Process):
    def __init__(self, ident='planselector', virtualpop=None, logger=None, **kwargs):
        print 'PlanSelector.__init__'

        # TODO: let this be independent, link to it or child??

        self._init_common(ident,
                          parent=virtualpop,
                          name='Plan Selector',
                          logger=logger,
                          info='Selects the plan for each person which will be executed during the next simulation run according to a defined selection method.',
                          )

        attrsman = self.set_attrsman(cm.Attrsman(self))

        # make for each possible pattern a field for prob
        strategies = virtualpop.get_strategies()
        # strategychoices.update(strategies.names.get_indexmap())

        methods = {'plan with shortest estim. time': virtualpop.select_plans_min_time_est,
                   'plan with shortest exec. time': virtualpop.select_plans_min_time_exec,
                   'plan with preferred mode': virtualpop.select_plans_preferred_mode,
                   'next plan in list': virtualpop.select_plans_next,
                   'random plan': virtualpop.select_plans_random,
                   'plan with shortest exec. time or est. time': virtualpop.select_plans_min_time_exec_est
                   }

        self.method = attrsman.add(cm.AttrConf('method', methods[kwargs.get('methodname', 'plan with shortest estim. time')],
                                               groupnames=['options'],
                                               choices=methods,
                                               perm='rw',
                                               name='Selection method',
                                               info='Selection method used to select current plans.',
                                               ))

        self.fraction = attrsman.add(cm.AttrConf('fraction', kwargs.get('fraction', 1.0),
                                                 groupnames=['options'],
                                                 perm='rw',
                                                 name='Change fraction',
                                                 info="""Fraction of persons that are randomly chosen to change plans according to the defined method.
                            A value of 1.0 mens that the plans of oll persons will be changed.""",
                                                 ))

        self.timedev = attrsman.add(cm.AttrConf('timedev', kwargs.get('timedev', 0.0),
                                                groupnames=['options'],
                                                perm='rw',
                                                name='Time deviation',
                                                info='Time deviation of random time component of estimated or effective time. If zero, no random time is added.',
                                                ))

        self.c_probit = attrsman.add(cm.AttrConf('c_probit', kwargs.get('c_probit', 0.0),
                                                 groupnames=['options'],
                                                 perm='rw',
                                                 name='Probit const',
                                                 info="""Probit constant used to determine the deviation of the normal distributed random time component. 
                                    The deviation is the product of this constant and the travel time. If zero, no random time is added.""",
                                                 ))

    def do(self):
        print 'Planner.do'
        # links

        #virtualpop = self.parent
        logger = self.get_logger()
        #logger.w('Check applicability')
        return self.method(logger=logger, **self.get_kwoptions())


class VehicleProvider(Process):
    def __init__(self, ident='vehicleprovider', virtualpop=None,  logger=None, **kwargs):
        print 'VehicleProvider.__init__'

        # TODO: let this be independent, link to it or child??

        self._init_common(ident,
                          parent=virtualpop,
                          name='Vehicle Provider',
                          logger=logger,
                          info='Provides individual vehicles to persons according to preferred mode and giveb statistical data.',
                          )

        attrsman = self.set_attrsman(cm.Attrsman(self))

        # make for each possible pattern a field for prob

        self.share_autoowner = attrsman.add(cm.AttrConf('share_autoowner', kwargs.get('share_autoowner', 0.8),
                                                        groupnames=['options'],
                                                        perm='rw',
                                                        name='Car auto share',
                                                        info="""Share of auto owners. This specifies the share of auto owners and a car will be created for each car owner. 
                                    Attention if prefeered mode has been already defined: persons who have bicicle as preferred mode get automatically a bike assigned.
                                    """,
                                                        ))

        self.share_motorcycleowner = attrsman.add(cm.AttrConf('share_motorcycleowner', kwargs.get('share_motorcycleowner', 0.3),
                                                              groupnames=['options'],
                                                              perm='rw',
                                                              name='Motorcycle owner share',
                                                              info="""Share of Motorcycle owners. This specifies the share of Motorcycle owners and a bike will be created for each Motorcycle owner. 
                                    Attention if prefeered mode has been already defined: persons who have Motorcycle as preferred mode get automatically a Motorcycle assigned.
                                    """,
                                                              ))

        self.share_bikeowner = attrsman.add(cm.AttrConf('share_bikeowner', kwargs.get('share_bikeowner', 0.5),
                                                        groupnames=['options'],
                                                        perm='rw',
                                                        name='Bike owner share',
                                                        info="""Share of bike owners. This specifies the share of bike owners and a bike will be created for each bike owner. 
                                    Attention if prefeered mode has been already defined: persons who have bicicle as preferred mode get automatically a bike assigned.
                                    """,
                                                        ))

    def do(self):
        print 'VehicleProvider.do'
        # links

        virtualpop = self.parent
        logger = self.get_logger()
        logger.w('Provide vehicles...')

        ids_person = virtualpop.get_ids()
        n_person = len(ids_person)
        modes = virtualpop.get_scenario().net.modes
        id_mode_bike = modes.get_id_mode('bicycle')
        id_mode_auto = modes.get_id_mode('passenger')
        id_mode_moto = modes.get_id_mode('motorcycle')

        iautos = virtualpop.get_iautos()
        ibikes = virtualpop.get_ibikes()
        imotos = virtualpop.get_imotos()

        logger.w('generate individual vehicles for prefered modes')
        ids_prefer_auto = virtualpop.select_ids(
            (virtualpop.ids_mode_preferred.get_value() == id_mode_auto) & (virtualpop.ids_iauto.get_value() == -1))
        ids_iauto = iautos.assign_to_persons(ids_prefer_auto)

        n_current = iautos.get_share(is_abs=True)
        #n_none = int(self.share_autoowner*n_person)-(n_person-n_current)
        n_need = int(self.share_autoowner*n_person)-n_current
        if n_need > 0:
            ids_pers_miss = np.flatnonzero(virtualpop.ids_iauto.get_value() == -1)
            # print '  n_person,n_current,n_target,n_need,len(ids_pers_miss)',n_person,n_current,int(self.share_autoowner*n_person),n_need,len(ids_pers_miss)
            ids_pers_assign = np.random.choice(ids_pers_miss, n_need, replace=False)
            ids_iauto = iautos.assign_to_persons(ids_pers_assign)

        print '  created %d autos, target share=%.2f, share = %.2f' % (
            iautos.get_share(is_abs=True), iautos.get_share(), self.share_autoowner)

        ids_prefer_bike = virtualpop.select_ids(
            (virtualpop.ids_mode_preferred.get_value() == id_mode_bike) & (virtualpop.ids_ibike.get_value() == -1))
        ids_ibikes = ibikes.assign_to_persons(ids_prefer_bike)

        n_current = ibikes.get_share(is_abs=True)
        n_need = int(self.share_bikeowner*n_person)-n_current
        if n_need > 0:
            ids_pers_miss = np.flatnonzero(virtualpop.ids_ibike.get_value() == -1)
            # print '  n_person,n_current,n_target,n_need,len(ids_pers_miss)',n_person,n_current,int(self.share_autoowner*n_person),n_need,len(ids_pers_miss)
            ids_pers_assign = np.random.choice(ids_pers_miss, n_need, replace=False)
            ids_ibike = ibikes.assign_to_persons(ids_pers_assign)

        print '  created %d bikes, target share=%.2f, share = %.2f' % (
            ibikes.get_share(is_abs=True), ibikes.get_share(), self.share_bikeowner)

        ids_prefer_moto = virtualpop.select_ids(
            (virtualpop.ids_mode_preferred.get_value() == id_mode_moto) & (virtualpop.ids_imoto.get_value() == -1))
        ids_imoto = imotos.assign_to_persons(ids_prefer_moto)

        n_current = imotos.get_share(is_abs=True)
        n_need = int(self.share_motorcycleowner*n_person)-n_current
        if n_need > 0:
            ids_pers_miss = np.flatnonzero(virtualpop.ids_imoto.get_value() == -1)
            ids_pers_assign = np.random.choice(ids_pers_miss, n_need, replace=False)
            ids_imoto = imotos.assign_to_persons(ids_pers_assign)

        print '  created %d moto, target share=%.2f, share = %.2f' % (
            imotos.get_share(is_abs=True), imotos.get_share(), self.share_motorcycleowner)
        return True

        # TODO: generate and assign  additional vehicles
        # to satisfy prescribes ownership
