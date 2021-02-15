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

# @file    demand.py
# @author  Joerg Schweizer
# @date

import os
import sys
import time
from xml.sax import saxutils, parse, handler

if __name__ == '__main__':
    try:
        APPDIR = os.path.dirname(os.path.abspath(__file__))
    except:
        APPDIR = os.path.dirname(os.path.abspath(sys.argv[0]))
    SUMOPYDIR = os.path.join(APPDIR, '..', '..')
    sys.path.append(SUMOPYDIR)


# Trip depart and arrival options, see
# http://www.sumo.dlr.de/userdoc/Definition_of_Vehicles,_Vehicle_Types,_and_Routes.html#A_Vehicle.27s_depart_and_arrival_parameter


from coremodules.modules_common import *
import numpy as np
import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
import agilepy.lib_base.xmlman as xm
from agilepy.lib_base.misc import get_inversemap

#from agilepy.lib_base.geometry import find_area
#from agilepy.lib_base.processes import Process,CmlMixin,ff,call
from coremodules.network.network import SumoIdsConf, MODES
from coremodules.network import routing
from coremodules.simulation import results as res

import vehicles
import origin_to_destination
import virtualpop
import turnflows
import publictransportservices as pt

from demandbase import *

try:
    try:
        import pyproj
        is_pyproj = True
    except:
        from mpl_toolkits.basemap import pyproj
        is_pyproj = True

except:
    is_pyproj = False


class Demand(cm.BaseObjman):
    def __init__(self, scenario=None, net=None, zones=None, name='Demand', info='Transport demand', **kwargs):
        print 'Demand.__init__', name, kwargs

        # we need a network from somewhere
        if net is None:
            net = scenario.net
            zones = scenario.landuse.zones

        self._init_objman(ident='demand', parent=scenario, name=name, info=info, **kwargs)
        attrsman = self.set_attrsman(cm.Attrsman(self))

        self.vtypes = attrsman.add(cm.ObjConf(vehicles.VehicleTypes(self, net)))

        self.activitytypes = attrsman.add(cm.ObjConf(ActivityTypes('activitytypes', self)))

        self.trips = attrsman.add(cm.ObjConf(Trips(self, net), groupnames=['demand objects']))

        self.odintervals = attrsman.add(cm.ObjConf(
                                        origin_to_destination.OdIntervals('odintervals', self, net, zones),
                                        ))

        self.turnflows = attrsman.add(cm.ObjConf(turnflows.Turnflows('turnflows', self, net),
                                                 ))
        self._init_attributes()
        self._init_constants()

    def _init_attributes(self):
        attrsman = self.get_attrsman()

        scenario = self.parent
        print 'Demand._init_attributes', scenario

        if scenario is not None:
            self.ptlines = attrsman.add(cm.ObjConf(pt.PtLines('ptlines', self),
                                                   groupnames=['demand objects'])
                                        )
            self.virtualpop = attrsman.add(cm.ObjConf(virtualpop.Virtualpopulation('virtualpop', self),
                                                      groupnames=['demand objects'])
                                           )
            # if hasattr(self,'virtualpolulation'):
            #    #Virtualpolulation
            #    self.delete('virtualpolulation')

        # update

        # if hasattr(self,'trips'):
        #attrsman.trips.add_groupnames(['demand objects'])

        # if hasattr(self,'ptlines'):
        #attrsman.ptlines.add_groupnames(['demand objects'])

        # if hasattr(self,'virtualpop'):
        #attrsman.virtualpop.add_groupnames(['demand objects'])
        # print attrsman.get_

    def _init_constants(self):
        self._xmltag_routes = "routes"
        self._xmltag_trips = "trips"
        #self._demandobjects = set([self.trips, self.ptlines, self.virtualpop])

    def get_vtypes(self):
        return self.vtypes

    def get_scenario(self):
        return self.parent

    def get_net(self):
        return self.parent.net

    def get_tripfilepath(self):
        return self.get_scenario().get_rootfilepath()+'.trip.xml'

    def get_routefilepath(self):
        return self.get_scenario().get_rootfilepath()+'.rou.xml'

    def update_netoffset(self, deltaoffset):
        """
        Called when network offset has changed.
        Children may need to adjust their coordinates.
        """
        # self.odintervals.update_netoffset(deltaoffset)
        pass

    def add_demandobject(self, obj=None, ident=None, DemandClass=None, **kwargs):
        if obj is not None:
            ident = obj.get_ident()

        if not hasattr(self, ident):
            if obj is None:
                # make demandobject a child of demand

                # if ident is None:
                #    ident = obj.get_ident()
                obj = DemandClass(ident, self, **kwargs)
                is_child = True
            else:
                # link to simobject, which must be a child of another object
                is_child = False

            attrsman = self.get_attrsman()

            attrsman.add(cm.ObjConf(obj,
                                    groupnames=['demand objects'],
                                    is_child=is_child,
                                    ))

            setattr(self, ident, obj)

        return getattr(self, ident)

    def get_demandobjects(self):
        #demandobjects = set([])
        # for ident, conf in self.get_group_attrs('').iteritems():
        #    demandobjects.add(conf.get_value())
        return self.get_attrsman().get_group_attrs('demand objects').values()

    def get_time_depart_first(self):
        # print 'get_time_depart_first'
        time = 10**10
        for obj in self.get_demandobjects():
            # print '  obj',obj.ident,obj.get_time_depart_first()
            time = min(time, obj.get_time_depart_first())
        return time

    def get_time_depart_last(self):
        time = 0
        for obj in self.get_demandobjects():
            time = max(time, obj.get_time_depart_last())
        return time

    def remove_demandobject(self, demandobject):
        # self._demandobjects.discard(demandobject)
        self.get_attrsman().delete(demandobject.ident)

    def export_routes_xml(self, filepath=None, encoding='UTF-8',
                          demandobjects=None, is_route=True):
        """
        Export routes available from the demand  to SUMO xml file.
        Method takes care of sorting trips by departure time.
        """

        if demandobjects is None:
            demandobjects = self.get_demandobjects()

        # is_route = True # add edge ids, if available

        if filepath is None:
            filepath = self.get_routefilepath()
        print 'export_routes_xml', filepath, demandobjects
        try:
            fd = open(filepath, 'w')
        except:
            print 'WARNING in write_obj_to_xml: could not open', filepath
            return False

        fd.write('<?xml version="1.0" encoding="%s"?>\n' % encoding)
        fd.write(xm.begin(self._xmltag_routes))
        indent = 2

        times_begin = np.zeros((0), dtype=np.int32)
        writefuncs = np.zeros((0), dtype=np.object)
        ids_trip = np.zeros((0), dtype=np.int32)

        ids_vtype = set()
        for exportobj in demandobjects:
            times, funcs, ids = exportobj.get_writexmlinfo(is_route=is_route)
            print '  exportobj', exportobj, len(times)
            if len(times) > 0:
                ids_vtype.update(exportobj.get_vtypes())
                times_begin = np.concatenate((times_begin, times), 0)
                writefuncs = np.concatenate((writefuncs, funcs), 0)
                ids_trip = np.concatenate((ids_trip, ids), 0)

        self.vtypes.write_xml(fd, indent=indent,
                              ids=ids_vtype,
                              is_print_begin_end=False
                              )

        # sort trips
        inds_trip = np.argsort(times_begin)

        #time0 = times_begin[inds_trip[0]]
        # write trips
        for writefunc, id_trip, time_begin in zip(
                writefuncs[inds_trip],
                ids_trip[inds_trip],
                times_begin[inds_trip]):

            writefunc(fd, id_trip, time_begin, indent)

        fd.write(xm.end(self._xmltag_routes))
        fd.close()
        return filepath

    def import_xml(self, rootname, dirname=''):
        """
        Import trips and/or routes, if available.
        """

        filepath = os.path.join(dirname, rootname+'.trip.xml')
        if os.path.isfile(filepath):
            # import trips
            self.trips.import_trips_xml(filepath, is_generate_ids=False)

            # now try to add routes to existing trips
            filepath = os.path.join(dirname, rootname+'.rou.xml')
            if os.path.isfile(filepath):
                self.trips.import_routes_xml(filepath, is_generate_ids=False, is_add=True)

            else:
                self.get_logger().w('import_xml: files not found:'+filepath, key='message')

        else:
            self.get_logger().w('import_xml: files not found:'+filepath, key='message')

            # no trip file exists, but maybe just a route file with trips
            filepath = os.path.join(dirname, rootname+'.rou.xml')
            if os.path.isfile(filepath):
                self.trips.import_routes_xml(filepath, is_generate_ids=False, is_add=False)

            else:
                self.get_logger().w('import_xml: files not found:'+filepath, key='message')


class Routes(am.ArrayObjman):
    def __init__(self, ident, trips, net, **kwargs):

        self._init_objman(ident=ident,
                          parent=trips,
                          name='Routes',
                          info='Table with route info.',
                          xmltag=('routes', 'route', None),
                          **kwargs)

        #self.add_col(SumoIdsConf('Route', xmltag = 'id'))

        self.add_col(am.IdsArrayConf('ids_trip', trips,
                                     groupnames=['state'],
                                     name='Trip ID',
                                     info='Route for this trip ID.',
                                     ))

        self.add_col(am.IdlistsArrayConf('ids_edges', net.edges,
                                         name='Edge IDs',
                                         info='List of edge IDs constituting the route.',
                                         xmltag='edges',
                                         ))

        self.add_col(am.ArrayConf('costs', 0.0,
                                  dtype=np.float32,
                                  perm='r',
                                  name='Costs',
                                  info="Route costs.",
                                  xmltag='cost',
                                  ))

        self.add_col(am.ArrayConf('probabilities', 1.0,
                                  dtype=np.float32,
                                  perm='r',
                                  name='Probab.',
                                  info="Route route choice probability.",
                                  xmltag='probability',
                                  ))

        self.add_col(am.ArrayConf('colors', np.ones(4, np.float32),
                                  dtype=np.float32,
                                  metatype='color',
                                  perm='rw',
                                  name='Color',
                                  info="Route color. Color as RGBA tuple with values from 0.0 to 1.0",
                                  xmltag='color',
                                  ))

    def clear_routes(self):
        self.clear()

    def get_shapes(self, ids=None):
        if ids is None:
            ids = self.get_ids()
        n = len(ids)
        edges = self.ids_edges.get_linktab()
        shapes = np.zeros(n, dtype=np.object)
        i = 0
        # TODO: if edge shapes were a list, the following would be possible:
        # np.sum(shapes)
        for ids_edge in self.ids_edges[ids]:
            routeshape = []
            # print '  ids_edge',ids_edge
            for shape in edges.shapes[ids_edge]:
                # print '  routeshape',routeshape
                # print '  shape',shape,type(shape)
                routeshape += list(shape)
            shapes[i] = routeshape
            i += 1

        return shapes


class Trips(DemandobjMixin, am.ArrayObjman):
    def __init__(self, demand, net=None, **kwargs):
        # print 'Trips.__init__'
        self._init_objman(ident='trips',
                          parent=demand,
                          name='Trips',
                          info='Table with trip and route info.',
                          xmltag=('trips', 'trip', 'ids_sumo'),
                          version=0.1,
                          **kwargs)

        self._init_attributes()
        self._init_constants()

    def _init_attributes(self):
        scenario = self.get_scenario()
        net = self.get_net()
        self.add_col(SumoIdsConf('Trip', xmltag='id'))

        self.add_col(am.IdsArrayConf('ids_vtype', self.get_obj_vtypes(),
                                     groupnames=['state'],
                                     name='Type',
                                     info='Vehicle type.',
                                     xmltag='type',
                                     ))

        self.add_col(am.ArrayConf('times_depart', 0,
                                  dtype=np.int32,
                                  perm='rw',
                                  name='Depart time',
                                  info="Departure time of vehicle in seconds. Must be an integer!",
                                  xmltag='depart',
                                  ))

        self.add_col(am.IdsArrayConf('ids_edge_depart', net.edges,
                                     groupnames=['state'],
                                     name='ID from-edge',
                                     info='ID of network edge where trip starts.',
                                     xmltag='from',
                                     ))

        self.add_col(am.IdsArrayConf('ids_edge_arrival', net.edges,
                                     groupnames=['state'],
                                     name='ID to-edge',
                                     info='ID of network edge where trip ends.',
                                     xmltag='to',
                                     ))

        self.add_col(am.ArrayConf('inds_lane_depart', OPTIONMAP_LANE_DEPART["free"],
                                  dtype=np.int32,
                                  #choices = OPTIONMAP_LANE_DEPART,
                                  perm='r',
                                  name='Depart lane',
                                  info="Departure lane index. 0 is rightmost lane or sidewalk, if existant.",
                                  #xmltag = 'departLane',
                                  #xmlmap = get_inversemap(OPTIONMAP_LANE_DEPART),
                                  ))

        self.add_col(am.ArrayConf('positions_depart', OPTIONMAP_POS_DEPARTURE["random_free"],
                                  dtype=np.float32,
                                  #choices = OPTIONMAP_POS_DEPARTURE,
                                  perm='r',
                                  name='Depart pos',
                                  unit='m',
                                  info="Position on edge at the moment of departure.",
                                  xmltag='departPos',
                                  xmlmap=get_inversemap(OPTIONMAP_POS_DEPARTURE),
                                  ))

        self.add_col(am.ArrayConf('speeds_depart', 0.0,
                                  dtype=np.float32,
                                  #choices = OPTIONMAP_SPEED_DEPARTURE,
                                  perm='r',
                                  name='Depart speed',
                                  unit='m/s',
                                  info="Speed at the moment of departure.",
                                  xmltag='departSpeed',
                                  xmlmap=get_inversemap(OPTIONMAP_SPEED_DEPARTURE),
                                  ))
        self.add_col(am.ArrayConf('inds_lane_arrival', OPTIONMAP_LANE_ARRIVAL["current"],
                                  dtype=np.int32,
                                  #choices = OPTIONMAP_LANE_ARRIVAL,
                                  perm='r',
                                  name='Arrival lane',
                                  info="Arrival lane index. 0 is rightmost lane or sidewalk, if existant.",
                                  #xmltag = 'arrivalLane',
                                  #xmlmap = get_inversemap(OPTIONMAP_LANE_ARRIVAL),
                                  ))

        self.add_col(am.ArrayConf('positions_arrival', OPTIONMAP_POS_ARRIVAL["random"],
                                  dtype=np.float32,
                                  #choices = OPTIONMAP_POS_ARRIVAL,
                                  perm='r',
                                  name='Arrival pos',
                                  unit='m',
                                  info="Position on edge at the moment of arrival.",
                                  xmltag='arrivalPos',
                                  xmlmap=get_inversemap(OPTIONMAP_POS_ARRIVAL),
                                  ))

        self.add_col(am.ArrayConf('speeds_arrival', 0.0,
                                  dtype=np.float32,
                                  #choices = OPTIONMAP_SPEED_ARRIVAL,
                                  perm='r',
                                  name='Arrival speed',
                                  unit='m/s',
                                  info="Arrival at the moment of departure.",
                                  xmltag='arrivalSpeed',
                                  xmlmap=get_inversemap(OPTIONMAP_SPEED_ARRIVAL),
                                  ))

        self.add(cm.ObjConf(Routes('routes', self, net)))

        # this could be extended to a list with more plans
        self.add_col(am.IdsArrayConf('ids_route_current', self.routes.get_value(),
                                     name='Route ID ',
                                     info='Currently chosen route ID.',
                                     ))

        # print '  self.routes.get_value()',self.routes.get_value()
        self.add_col(am.IdlistsArrayConf('ids_routes', self.routes.get_value(),
                                         name='IDs route alt.',
                                         info='IDs of route alternatives for this trip.',
                                         ))

        if self.get_version() < 0.1:
            self.inds_lane_depart.set_xmltag(None)
            self.inds_lane_arrival.set_xmltag(None)
        self.set_version(0.1)

    def _init_constants(self):
        #self._method_routechoice = self.get_route_first

        self._xmltag_routes = "routes"
        self._xmltag_veh = "vehicle"
        self._xmltag_id = "id"
        self._xmltag_trip = "trip"
        self._xmltag_rou = "route"
        self._xmltag_person = 'person'

    def clear_trips(self):
        self.routes.get_value().clear_routes()
        self.clear()

    def clear_routes(self):
        self.routes.get_value().clear_routes()
        self.ids_route_current.reset()
        self.ids_routes.reset()

    def get_id_from_id_sumo(self, id_veh_sumo):
        if len(id_veh_sumo.split('.')) == 1:
            return int(id_veh_sumo)
        return -1

    def get_routes(self):
        return self.routes.get_value()

    def get_obj_vtypes(self):
        return self.parent.vtypes

    def get_net(self):
        return self.get_scenario().net

    def get_scenario(self):
        return self.parent.get_scenario()

    def get_time_depart_first(self):
        if len(self) > 0:
            return float(np.min(self.times_depart.get_value()))
        else:
            return np.inf

    def get_time_depart_last(self):
        if len(self) > 0:
            return float(np.max(self.times_depart.get_value()))+600.0
        else:
            return 0.0

    def get_tripfilepath(self):
        return self.parent.get_tripfilepath()

    def get_routefilepath(self):
        return self.parent.get_routefilepath()

    def duaroute(self, is_export_net=False, is_export_trips=True,
                 routefilepath=None,  **kwargs):
        """
        Simple fastest path routing using duarouter.
        """
        print 'duaroute'
        exectime_start = time.clock()

        #routesattrname = self.get_routesattrname(routesindex)
        vtypes = self.parent.vtypes
        if (not os.path.isfile(self.get_tripfilepath())) | is_export_trips:
            ids_vtype_pedestrian = vtypes.select_by_mode(mode='pedestrian', is_sumoid=False)
            self.export_trips_xml(ids_vtype_exclude=ids_vtype_pedestrian)

        if (not os.path.isfile(self.get_net().get_filepath())) | is_export_net:
            self.get_net().export_netxml()

        if routefilepath is None:
            routefilepath = self.get_routefilepath()

        if routing.duaroute(self.get_tripfilepath(), self.get_net().get_filepath(),
                            routefilepath, **kwargs):

            self.import_routes_xml(routefilepath,   is_clear_trips=False,
                                   is_generate_ids=False,
                                   is_add=True)

            print '  exectime', time.clock()-exectime_start
            return routefilepath

        else:
            return None

    def get_trips_for_vtype(self, id_vtype):
        return self.select_ids(self.ids_vtype.get_value() == id_vtype)

    def get_vtypes(self):
        return set(self.ids_vtype.get_value())

    def route(self, is_check_lanes=True, is_del_disconnected=False):
        """
        Fastest path python router.
        """
        print 'route'
        # TODO: if too mant vtypes, better go through id_modes
        exectime_start = time.clock()

        net = self.get_scenario().net
        edges = net.edges
        vtypes = self.parent.vtypes

        ids_edges = []
        ids_trip = []
        costs = []

        ids_trip_disconnected = []

        fstar = edges.get_fstar()
        for id_vtype in self.get_vtypes():
            id_mode = vtypes.ids_mode[id_vtype]

            # no routing for pedestrians
            if id_mode != net.modes.get_id_mode('pedestrian'):
                weights = edges.get_times(id_mode=id_mode,
                                          speed_max=vtypes.speeds_max[id_vtype],
                                          is_check_lanes=is_check_lanes,
                                          modeconst_excl=-10.0, modeconst_mix=-5.0,
                                          )

                ids_trip_vtype = self.get_trips_for_vtype(id_vtype)
                # print '  id_vtype,id_mode',id_vtype,id_mode#,ids_trip_vtype
                # print '  weights',weights
                ids_edge_depart = self.ids_edge_depart[ids_trip_vtype]
                ids_edge_arrival = self.ids_edge_arrival[ids_trip_vtype]

                for id_trip, id_edge_depart, id_edge_arrival in zip(ids_trip_vtype, ids_edge_depart,  ids_edge_arrival):
                    cost, route = routing.get_mincostroute_edge2edge(id_edge_depart,
                                                                     id_edge_arrival,
                                                                     weights=weights,
                                                                     fstar=fstar)
                    if len(route) > 0:
                        ids_edges.append(route)
                        ids_trip.append(id_trip)
                        costs.append(cost)

                    else:
                        ids_trip_disconnected.append(id_trip)

        ids_route = self.routes.get_value().add_rows(ids_trip=ids_trip,
                                                     ids_edges=ids_edges,
                                                     costs=costs,
                                                     )
        self.add_routes(ids_trip, ids_route)
        print '  exectime', time.clock()-exectime_start

        if is_del_disconnected:
            self.del_rows(ids_trip_disconnected)

        return ids_trip, ids_route

    def estimate_entered(self, method_routechoice=None):
        """
        Estimates the entered number of vehicles for each edge.
        returns ids_edge and entered_vec
        """
        # TODO: we could specify a mode
        if method_routechoice is None:
            method_routechoice = self.get_route_first

        ids_edges = self.routes.get_value().ids_edges
        counts = np.zeros(np.max(self.get_net().edges.get_ids())+1, int)

        for id_trip in self.get_ids():
            id_route = method_routechoice(id_trip)
            if id_route >= 0:
                # here the [1:] eliminates first edge as it is not entered
                counts[ids_edges[id_route][1:]] += 1

        ids_edge = np.flatnonzero(counts)
        entered_vec = counts[ids_edge].copy()
        return ids_edge, entered_vec

    def import_trips_from_scenario(self, scenario2):
        """
        Import trips from another scenario.
        """
        print 'import_trips_from_scenario', scenario2.ident
        if not is_pyproj:
            print("WARNING in import_trips_from_scenario: pyproj module not installed")
            return None

        scenario = self.get_scenario()
        demand = scenario.demand
        net = scenario.net
        edges = net.edges
        ids_edge_depart = demand.trips.ids_edge_depart
        ids_edge_arrival = demand.trips.ids_edge_arrival

        demand2 = scenario2.demand
        net2 = scenario2.net

        # copy all vtypes from scenario2

        # get vtypes of demand2 that are not in demand
        ids_veh2 = demand2.vtypes.get_ids()
        ids_vtype_sumo2 = demand2.vtypes.ids_sumo[ids_veh2]
        # print '  ids_vtype_sumo2',ids_vtype_sumo2
        # print '  ids_vtype_sumo',demand.vtypes.ids_sumo.get_value()
        ids_vtype_sumo_diff = list(set(ids_vtype_sumo2).difference(demand.vtypes.ids_sumo.get_value()))
        # print '  ids_vtype_sumo_diff',ids_vtype_sumo_diff
        # for id_sumo2 in demand2.vtypes.ids_sumo.get_value():
        #    if demand.vtypes.has_index(id_sumo2):
        #       for attrconf in demand2.vtypes.get_group('parameters'):

        # copy all attributes from   ids_vtype_sumo_diff
        ids_vtype_diff = demand.vtypes.copy_cols(
            demand2.vtypes, ids=demand2.vtypes.ids_sumo.get_ids_from_indices(ids_vtype_sumo_diff))
        #ids_vtype_sumo = demand.vtypes.ids_sumo.get_value()
        #ids_vtype = demand.vtypes.get_ids_from_indices(ids_vtype_sumo)
        #ids_vtype2 = demand2.vtypes.ids_sumoget_ids_from_indices(ids_vtype_sumo)

        # map id_vtypes from scenario2 to present scenario
        vtypemap = np.zeros(np.max(ids_veh2)+1)
        vtypemap[ids_veh2] = demand.vtypes.ids_sumo.get_ids_from_indices(ids_vtype_sumo2)

        # print '  vtypemap',vtypemap
        # copy trip parameters, by mapping trip types
        ids_trip2 = demand2.trips.get_ids()

        ids_trips = self.copy_cols(demand2.trips)  # ids_trip in present scenrio

        # delete routes, that cannot be transferred (at the moment)
        self.ids_route_current[ids_trips] = -1
        self.ids_routes[ids_trips] = len(ids_trips) * [None]

        self.ids_vtype[ids_trips] = vtypemap[demand2.trips.ids_vtype[ids_trip2]]

        # print '  ids_trip2,ids_trips',ids_trip2,ids_trips

        ids_mode = demand.vtypes.ids_mode[self.ids_vtype[ids_trips]]
        # for each used mode, we need to select the network edges that are accessible by this mode
        ids_modeset = set(ids_mode)

        #maps_edge_laneind = {}
        # for id_mode in ids_modeset:
        #    ids_edge, inds_lane = edges.select_accessible(id_mode)
        #    maps_edge_laneind[id_mode] = dict(zip(ids_edge,inds_lane))

        # project depart points and arrival points
        proj_params = str(net.get_projparams())
        proj_params2 = str(net2.get_projparams())

        if (proj_params == '!') | (proj_params2 == '!'):
            print 'WARNING in import_trips_from_scenario: unknown projections, use only offsets.', proj_params, proj_params2
            is_proj = False

        elif proj_params == proj_params2:
            #  with identical projections, projecting is useless
            is_proj = False

        else:
            is_proj = True
            proj = pyproj.Proj(proj_params)
            proj2 = pyproj.Proj(proj_params2)

        offset = net.get_offset()
        offset2 = net2.get_offset()

        # if self._proj is None:
        #    self._proj, self._offset = self.parent.get_proj_and_offset()
        #x,y = self._proj(lons, lats)
        # return np.transpose(np.concatenate(([x+self._offset[0]],[y+self._offset[1]]),axis=0))

        # adjust edge ids check lane access
        n_failed = 0
        ids_trip_failed = set()

        for id_mode in ids_modeset:
            # make_segment_edge_map for all edges of this mode
            print '  make segment_edge_map for mode', id_mode
            ids_edge_access, inds_lane_access = edges.select_accessible_mode(id_mode)
            print '    found accessible edges', len(ids_edge_access), len(edges.get_ids())
            # dict(zip(ids_edge,inds_lane))
            edges.make_segment_edge_map(ids_edge_access)

            # select trips with id_mode
            ind_trips = np.flatnonzero(ids_mode == id_mode)

            print '  number of trips for this mode:', len(ind_trips)
            for id_trip, id_edge_depart2, id_edge_arrival2 in zip(ids_trips[ind_trips],
                                                                  demand2.trips.ids_edge_depart[ids_trip2[ind_trips]],
                                                                  demand2.trips.ids_edge_arrival[ids_trip2[ind_trips]]
                                                                  ):

                # match departure edge

                # treat special numbers of position
                pos2 = 0.0

                # get coordinate in scenario2
                x2, y2, z2 = net2.edges.get_coord_from_pos(id_edge_depart2, pos2)

                # project coord from scenario2 in present scenario
                if is_proj:
                    xp, yp = pyproj.transform(proj2, proj, x2-offset2[0], y2-offset2[1])
                else:
                    xp, yp = x2-offset2[0], y2-offset2[1]

                coord = (xp+offset[0], yp+offset[1], z2)
                # print '  coord2 = ',(x2,y2,z2)
                # print '  coord  = ',coord
                # get edge id in present scenario
                id_edge_depart = edges.get_closest_edge(coord)

                # check eucledian distance
                #d = edges.get_dist_point_to_edge(coord, id_edge_depart)
                # print '    id_edge_depart,d,id_mode',id_edge_depart,d,id_mode
                print '    id_edge_depart', id_edge_depart, id_edge_depart in ids_edge_access

                ids_edge_depart[id_trip] = id_edge_depart

                # match arrival edge

                # treat special numbers of position
                pos2 = 0.0

                # get coordinate in scenario2
                x2, y2, z2 = net2.edges.get_coord_from_pos(id_edge_arrival2, pos2)

                # project coord from scenario2 in present scenario
                if is_proj:
                    xp, yp = pyproj.transform(proj2, proj, x2-offset2[0], y2-offset2[1])
                else:
                    xp, yp = x2-offset2[0], y2-offset2[1]

                coord = (xp+offset[0], yp+offset[1], z2)
                # print '  coord2 = ',(x2,y2,z2)
                # print '  coord  = ',coord
                # get edge id in present scenario
                id_edge_arrival = edges.get_closest_edge(coord)

                # check eucledian distance
                #d = edges.get_dist_point_to_edge(coord, id_edge_arrival)
                print '    id_edge_arrival', id_edge_arrival, id_edge_arrival in ids_edge_access

                ids_edge_arrival[id_trip] = id_edge_arrival

        # redo segment map
        edges.make_segment_edge_map()

    def make_trip(self, is_generate_ids=True,  **kwargs):
        id_trip = self.add_row(ids_vtype=kwargs.get('id_vtype', None),
                               times_depart=kwargs.get('time_depart', None),
                               ids_edge_depart=kwargs.get('id_edge_depart', None),
                               ids_edge_arrival=kwargs.get('id_edge_arrival', None),
                               inds_lane_depart=kwargs.get('ind_lane_depart', None),
                               positions_depart=kwargs.get('position_depart', None),
                               speeds_depart=kwargs.get('speed_depart', None),
                               inds_lane_arrival=kwargs.get('ind_lane_arrival', None),
                               positions_arrival=kwargs.get('position_arrival', None),
                               speeds_arrival=kwargs.get('speed_arrival', None),
                               ids_routes=[],
                               )

        if is_generate_ids:
            self.ids_sumo[id_trip] = str(id_trip)
        else:
            self.ids_sumo[id_trip] = kwargs.get('id_sumo', str(id_trip))  # id

        if kwargs.has_key('route'):
            route = kwargs['route']
            if len(route) > 0:
                id_route = self.routes.get_value().add_row(ids_trip=id_trip,
                                                           ids_edges=kwargs['route']
                                                           )
                self.ids_route_current[id_trip] = id_route
                self.ids_routes[id_trip] = [id_route]

        return id_trip

    def make_trips(self, ids_vtype, is_generate_ids=True, **kwargs):
        # print 'make_trips ids_vtype =',ids_vtype
        # print '  kwargs=',kwargs
        ids_trip = self.add_rows(
            ids_vtype=ids_vtype,
            times_depart=kwargs.get('times_depart', None),
            ids_edge_depart=kwargs.get('ids_edge_depart', None),
            ids_edge_arrival=kwargs.get('ids_edge_arrival', None),
            inds_lane_depart=kwargs.get('inds_lane_depart', None),
            positions_depart=kwargs.get('positions_depart', None),
            speeds_depart=kwargs.get('speeds_depart', None),
            inds_lane_arrival=kwargs.get('inds_lane_arrival', None),
            positions_arrival=kwargs.get('positions_arrival', None),
            speeds_arrival=kwargs.get('speeds_arrival', None),
            #ids_routes = len(ids_vtype)*[[]],
        )

        if is_generate_ids:
            self.ids_sumo[ids_trip] = np.array(ids_trip, np.str)
        else:
            self.ids_sumo[ids_trip] = kwargs.get('ids_sumo', np.array(ids_trip, np.str))
        return ids_trip

    def make_routes(self, ids_vtype, is_generate_ids=True, routes=None, ids_trip=None, **kwargs):
        # print 'make_routes',is_generate_ids

        if ids_trip is None:  # is_generate_ids = is_generate_ids,
            # print '  generate new trips'
            ids_trip = self.make_trips(ids_vtype, is_generate_ids=is_generate_ids,  **kwargs)

        # print '  ids_trip =',ids_trip
        ids_routes = self.routes.get_value().add_rows(ids_trip=ids_trip,
                                                      ids_edges=routes,
                                                      )
        self.add_routes(ids_trip, ids_routes)

        # no!:self.ids_routes[ids_trip] = ids_routes.reshape((-1,1)).tolist()# this makes an array of lists
        # print '  self.ids_routes.get_value()',self.ids_routes[ids_trip]
        # print '  ids_routes.reshape((-1,1)).tolist()',ids_routes.reshape((-1,1)).tolist()
        # print '  make_routes DONE'
        return ids_routes, ids_trip

    def add_routes(self, ids_trip, ids_routes):
        for id_trip, id_route in zip(ids_trip, ids_routes):
            # no!: self.ids_routes[id_trip].append(id_route)
            # print '  self.ids_routes[id_trip]',self.ids_routes[id_trip],id_route
            if self.ids_routes[id_trip] is None:
                self.ids_routes[id_trip] = [id_route]  # this works!

            else:
                self.ids_routes[id_trip].append(id_route)

            if self.ids_route_current[id_trip] == -1:
                self.ids_route_current[id_trip] = id_route

    def prepare_sim(self, process):
        return []  # [(steptime1,func1),(steptime2,func2),...]

    def export_trips_xml(self, filepath=None, encoding='UTF-8',
                         ids_vtype_exclude=[]):
        """
        Export trips to SUMO xml file.
        Method takes care of sorting trips by departure time.
        """
        if filepath is None:
            filepath = self.get_tripfilepath()
        print 'export_trips_xml', filepath
        try:
            fd = open(filepath, 'w')
        except:
            print 'WARNING in write_obj_to_xml: could not open', filepath
            return False

        xmltag, xmltag_item, attrname_id = self.xmltag
        fd.write('<?xml version="1.0" encoding="%s"?>\n' % encoding)
        fd.write(xm.begin(xmltag))
        indent = 2

        ids_trip = self.times_depart.get_ids_sorted()
        ids_vtype = self.ids_vtype[ids_trip]
        #ids_vtypes_exclude = self.ids_vtype.get_ids_from_indices(vtypes_exclude)

        inds_selected = np.ones(len(ids_vtype), np.bool)
        for id_vtype in ids_vtype_exclude:
            inds_selected[ids_vtype == id_vtype] = False
        ids_trip_selected = ids_trip[inds_selected]
        ids_vtype_selected = set(ids_vtype[inds_selected])
        #ids_vtypes_selected = set(ids_vtypes).difference(ids_vtypes_exclude)

        self.parent.vtypes.write_xml(fd, indent=indent,
                                     ids=ids_vtype_selected,
                                     is_print_begin_end=False)

        self.write_xml(fd,     indent=indent,
                       ids=ids_trip_selected,
                       attrconfigs_excluded=[self.routes,
                                             self.ids_routes,
                                             self.ids_route_current,
                                             self.inds_lane_depart,
                                             self.inds_lane_arrival],
                       is_print_begin_end=False)

        fd.write(xm.end(xmltag))
        fd.close()
        return filepath

    def get_vtypes(self):
        return set(self.ids_vtype.get_value())

    def get_trips(self):
        # returns trip object, method common to all demand objects
        return self

    def get_writexmlinfo(self, is_route=False):
        """
        Returns three array where the first array is the
        begin time of the first vehicle and the second array is the
        write function to be called for the respectice vehicle and
        the third array contains the vehicle ids

        Method used to sort trips when exporting to route or trip xml file
        """
        n = len(self)
        ids = self.get_ids()

        # define different route write functions for pedestriand and vehicles
        writefuncs = np.zeros(n, dtype=np.object)
        inds_ped = self.parent.vtypes.ids_mode[self.ids_vtype[ids]] == MODES['pedestrian']
        writefuncs[inds_ped] = self.write_persontrip_xml
        if is_route:
            writefuncs[np.logical_not(inds_ped) & (self.ids_route_current[ids] > -1)] = self.write_vehroute_xml
            writefuncs[np.logical_not(inds_ped) & (self.ids_route_current[ids] == -1)] = self.write_noroute_xml
        else:
            # TODO: here we should write just origin and destination edge
            writefuncs[np.logical_not(inds_ped)] = self.write_vehroute_xml

        return self.times_depart[ids], writefuncs, ids

    def write_noroute_xml(self, fd, id_trip, time_begin, indent=2):
        """
        Function called when respective vehicle has an invalid route
        """
        pass

    def write_vehroute_xml(self, fd, id_trip, time_begin, indent=2):
        # print 'write_vehroute_xml',id_trip,time_begin
        id_route = self.ids_route_current[id_trip]  # self.get_route_first(id_trip)#self._method_routechoice(id_trip)#

        if id_route >= 0:  # a valid route has been found
            # init vehicle route only if valid route exists
            fd.write(xm.start(self._xmltag_veh, indent))
        else:
            # init trip instead of route
            fd.write(xm.start(self._xmltag_trip, indent))

        # print '   make tag and id',_id
        fd.write(xm.num(self._xmltag_id, self.ids_sumo[id_trip]))

        # print ' write columns',len(scalarcolconfigs)>0,len(idcolconfig_include_tab)>0,len(objcolconfigs)>0
        for attrconfig in [self.ids_vtype,
                           self.times_depart,
                           self.ids_edge_depart,
                           self.ids_edge_arrival,
                           self.inds_lane_depart,
                           self.positions_depart,
                           self.speeds_depart,
                           self.inds_lane_arrival,
                           self.positions_arrival,
                           self.speeds_arrival, ]:
            # print '    attrconfig',attrconfig.attrname
            attrconfig.write_xml(fd, id_trip)

        if (id_route >= 0):  # a valid route has been found
            # write route id
            #fd.write(xm.num('route', id_route ))

            # instead of route id we write entire route here
            fd.write(xm.stop())
            fd.write(xm.start(self._xmltag_rou, indent+2))

            routes = self.routes.get_value()
            for attrconfig in [routes.ids_edges, routes.colors]:
                # print '    attrconfig',attrconfig.attrname
                attrconfig.write_xml(fd, id_route)

            # end route and vehicle
            fd.write(xm.stopit())
            fd.write(xm.end(self._xmltag_veh, indent+2))

        else:
            # end trip without route
            fd.write(xm.stopit())

    def write_vehtrip_xml(self, fd, id_trip, time_begin, indent=2):
        # vehicle trip write function
        # no route is written, even if it exisis

        # init trip instead of route
        fd.write(xm.start(self._xmltag_trip, indent))

        # print '   make tag and id',_id
        fd.write(xm.num(self._xmltag_id, self.ids_sumo[id_trip]))

        # print ' write columns',len(scalarcolconfigs)>0,len(idcolconfig_include_tab)>0,len(objcolconfigs)>0
        for attrconfig in [self.ids_vtype,
                           self.times_depart,
                           self.ids_edge_depart,
                           self.ids_edge_arrival,
                           self.inds_lane_depart,
                           self.positions_depart,
                           self.speeds_depart,
                           self.inds_lane_arrival,
                           self.positions_arrival,
                           self.speeds_arrival, ]:
            # print '    attrconfig',attrconfig.attrname
            attrconfig.write_xml(fd, id_trip)

        # end trip without route
        fd.write(xm.stopit())

    def write_persontrip_xml(self, fd, id_trip, time_begin, indent=2):
        # currently no routes are exported, only origin and destination edges

        fd.write(xm.start(self._xmltag_person, indent))

        self.ids_sumo.write_xml(fd, id_trip)
        self.times_depart.write_xml(fd, id_trip)
        self.ids_vtype.write_xml(fd, id_trip)
        fd.write(xm.stop())

        fd.write(xm.start('walk', indent=indent+2))
        # print 'write walk',id_trip,self.positions_depart[id_trip],self.positions_arrival[id_trip]
        self.ids_edge_depart.write_xml(fd, id_trip)
        if self.positions_depart[id_trip] > 0:
            self.positions_depart.write_xml(fd, id_trip)

        self.ids_edge_arrival.write_xml(fd, id_trip)
        if self.positions_arrival[id_trip] > 0:
            self.positions_arrival.write_xml(fd, id_trip)

        fd.write(xm.stopit())  # ends walk
        fd.write(xm.end(self._xmltag_person, indent=indent))

    # def get_route_first(self, id_trip):
    #    ids_route = self.ids_routes[id_trip]
    #    if ids_route is None:
    #        return -1
    #    elif len(ids_route)>0:
    #        return ids_route[0]
    #    else:
    #        return -1 # no route found

    def import_routes_xml(self, filepath,  is_clear_trips=False,
                          is_generate_ids=True, is_add=False):
        print 'import_routes_xml from %s generate own trip ' % (filepath)
        if is_clear_trips:
            self.clear_trips()

        counter = RouteCounter()
        parse(filepath, counter)
        reader = RouteReader(self, counter)
        try:
            parse(filepath, reader)
            # print '  call make_routes',is_generate_ids,is_add
            reader.insert_routes(is_generate_ids=is_generate_ids,
                                 is_add=is_add)
        except KeyError:
            print >> sys.stderr, "Error: Problems with reading routes!"
            raise

    def import_trips_xml(self, filepath,  is_clear_trips=False, is_generate_ids=True):
        print 'import_trips_xml from %s generate own trip ' % (filepath)
        if is_clear_trips:
            self.clear_trips()

        counter = TripCounter()
        parse(filepath, counter)
        reader = TripReader(self, counter.n_trip)
        print '  n_trip=', counter.n_trip

        try:
            parse(filepath, reader)
            reader.insert_trips(is_generate_ids=is_generate_ids)
        except KeyError:
            print >> sys.stderr, "Error: Problems with reading trips!"
            raise

    def config_results(self, results):
        # print 'DEMAND.config_results'
        tripresults = res.Tripresults('tripresults', results,
                                      self,
                                      self.get_net().edges
                                      )

        results.add_resultobj(tripresults, groupnames=['Trip results'])

    def process_results(self, results, process=None):
        pass


class TripCounter(handler.ContentHandler):
    """Parses a SUMO route XML file and counts trips."""

    def __init__(self):
        self.n_trip = 0

    def startElement(self, name, attrs):
        # print 'startElement',name,self.n_trip
        if name == 'trip':
            self.n_trip += 1


class TripReader(handler.ContentHandler):
    """Reads trips from trip or route file into trip table"""

    def __init__(self, trips,  n_trip, vtype_default=None):
        # print 'RouteReader.__init__',demand.ident
        self._trips = trips
        demand = trips.parent

        net = demand.get_scenario().net

        self._ids_vtype_sumo = demand.vtypes.ids_sumo
        self._modemap = net.modes.names.get_indexmap()
        self._get_vtype_for_mode = demand.vtypes.get_vtype_for_mode

        if id_vtype_default is None:
            self._id_vtype_default = self.get_ids()[0]
        else:
            self._id_vtype_default = id_vtype_default

        self._ids_edge_sumo = net.edges.ids_sumo

        self.ids_sumo = np.zeros(n_trip, np.object)
        self.ids_vtype = np.zeros(n_trip, np.int32)
        self.times_depart = np.zeros(n_trip, np.int32)
        self.ids_edge_depart = np.zeros(n_trip, np.int32)
        self.ids_edge_arrival = np.zeros(n_trip, np.int32)
        self.inds_lane_depart = np.zeros(n_trip, np.int32)
        self.positions_depart = np.zeros(n_trip, np.float32)
        self.speeds_depart = np.zeros(n_trip, np.float32)
        self.inds_lane_arrival = np.zeros(n_trip, np.int32)
        self.positions_arrival = np.zeros(n_trip, np.float32)
        self.speeds_arrival = np.zeros(n_trip, np.float32)
        self.routes = np.zeros(n_trip, np.object)

        self._ind_trip = -1

        self._has_routes = False
        self._ids_sumoedge_current = []
        self._id_sumoveh_current = None
        #self._time_depart = 0
        #self._attrs = {}
        #self._is_generate_ids = is_generate_ids
        self._intervals_current = ''

    # def _init_reader(self):

    def _get_id_vtype(self, attrs):
        vtype = str(attrs['type'])
        if self._ids_vtype_sumo.has_index(vtype):
            # vtype is known
            return self._ids_vtype_sumo.get_id_from_index(vtype)

        # vtype is not known, so check out whether vtype
        # is actually a mode
        elif vtype in self._modemap:
            # pick id_vtype according to its probability with
            # all vtypes of this mode
            id_vtype = self._get_vtype_for_mode(self._modemap[vtype])

            if id_vtype >= 0:
                return id_vtype
            else:
                return self._id_vtype_default

        else:
            # no valid vtype, get a default
            return self._id_vtype_default

    def startElement(self, name, attrs):
        # <vehicle id="3_21" type="bus" depart="2520.00">
        # <route edges="bottom1to1/0 1/0to0/0 0/0tobottom0"/>
        # </vehicle>
        # print 'startElement',name
        if name == 'trip':
            # print '  startElement',attrs['id'],attrs['depart']
            self._ind_trip += 1

            self._id_sumoveh_current = attrs['id']
            self.ids_sumo[self._ind_trip] = self._id_sumoveh_current
            print 'startElement ids_vtype', attrs['type'], self._ids_vtype_sumo.get_id_from_index(str(attrs['type']))

            self.ids_vtype[self._ind_trip] = self._get_id_vtype(attrs)
            self.times_depart[self._ind_trip] = int(float(attrs['depart']))

            if attrs.has_key('from'):
                self.ids_edge_depart[self._ind_trip] = self._ids_edge_sumo.get_id_from_index(str(attrs['from']))
            if attrs.has_key('to'):
                self.ids_edge_arrival[self._ind_trip] = self._ids_edge_sumo.get_id_from_index(str(attrs['to']))

            ind_lane_depart_raw = attrs.get('departLane', 'free')
            if OPTIONMAP_LANE_DEPART.has_key(ind_lane_depart_raw):
                self.inds_lane_depart[self._ind_trip] = OPTIONMAP_LANE_DEPART[ind_lane_depart_raw]
            else:
                self.inds_lane_depart[self._ind_trip] = int(ind_lane_depart_raw)

            positions_depart_raw = attrs.get('departPos', 'base')
            if OPTIONMAP_POS_DEPARTURE.has_key(positions_depart_raw):
                self.positions_depart[self._ind_trip] = OPTIONMAP_POS_DEPARTURE[positions_depart_raw]
            else:
                self.positions_depart[self._ind_trip] = float(positions_depart_raw)

            self.speeds_depart[self._ind_trip] = attrs.get('departSpeed', 0.0)

            ind_lane_arrival_raw = attrs.get('arrivalLane', 'current')
            if OPTIONMAP_LANE_ARRIVAL.has_key(ind_lane_arrival_raw):
                self.inds_lane_arrival[self._ind_trip] = OPTIONMAP_LANE_ARRIVAL[ind_lane_arrival_raw]
            else:
                self.inds_lane_arrival[self._ind_trip] = int(ind_lane_arrival_raw)

            positions_arrival_raw = attrs.get('arrivalPos', 'max')
            if OPTIONMAP_POS_ARRIVAL.has_key(positions_arrival_raw):
                self.positions_arrival[self._ind_trip] = OPTIONMAP_POS_ARRIVAL[positions_arrival_raw]
            else:
                self.positions_arrival[self._ind_trip] = float(positions_arrival_raw)

            self.speeds_arrival[self._ind_trip] = attrs.get('arrivalSpeed', 0.0)

    def _get_kwargs(self):
        return {'ids_sumo': self.ids_sumo,
                'times_depart': self.times_depart,
                'ids_edge_depart': self.ids_edge_depart,
                'ids_edge_arrival': self.ids_edge_arrival,
                'inds_lane_depart': self.inds_lane_depart,
                'positions_depart': self.positions_depart,
                'speeds_depart': self.speeds_depart,
                'inds_lane_arrival': self.inds_lane_arrival,
                'positions_arrival': self.positions_arrival,
                'speeds_arrival': self.speeds_arrival,
                }

    def insert_trips(self, is_generate_ids=True):

        # print 'TripReader.insert_trips self.ids_vtype',self.ids_vtype
        kwargs = self._get_kwargs()
        ids_trips = self._trips.make_trips(self.ids_vtype,
                                           is_generate_ids=is_generate_ids,
                                           **kwargs)

        return ids_trips


class RouteCounter(handler.ContentHandler):
    """Parses a SUMO route XML file and counts trips."""

    def __init__(self):
        self.n_veh = 0
        self.n_pers = 0
        #self.n_rou = 0

    def startElement(self, name, attrs):
        # print 'startElement',name,self.n_trip
        if name == 'vehicle':
            self.n_veh += 1
        elif name == 'person':
            self.n_pers += 1
        # elif name == 'route':
        #    if attrs.has_key('id'):
        #        self.n_rou += 1


class RouteReader(TripReader):
    """Reads trips from trip or route file into trip table"""

    def __init__(self, trips,  counter):
        # print 'RouteReader.__init__',demand.ident
        self._trips = trips
        n_veh = counter.n_veh
        n_per = counter.n_pers
        #n_rou = counter.n_rou
        n_trip = n_veh+n_per
        demand = trips.parent

        net = demand.get_scenario().net
        self._modemap = net.modes.names.get_indexmap()
        self._get_vtype_for_mode = demand.vtypes.get_vtype_for_mode

        self._ids_vtype_sumo = demand.vtypes.ids_sumo
        self._ids_edge_sumo = net.edges.ids_sumo

        self.ids_sumo = np.zeros(n_trip, np.object)
        self.ids_vtype = np.zeros(n_trip, np.int32)
        self.times_depart = np.zeros(n_trip, np.int32)
        self.ids_edge_depart = np.zeros(n_trip, np.int32)
        self.ids_edge_arrival = np.zeros(n_trip, np.int32)
        self.inds_lane_depart = np.zeros(n_trip, np.int32)
        self.positions_depart = np.zeros(n_trip, np.float32)
        self.speeds_depart = np.zeros(n_trip, np.float32)
        self.inds_lane_arrival = np.zeros(n_trip, np.int32)
        self.positions_arrival = np.zeros(n_trip, np.float32)
        self.speeds_arrival = np.zeros(n_trip, np.float32)
        self.routes = np.zeros(n_trip, np.object)

        self._ind_trip = -1

        self._has_routes = False
        self._ids_sumoedge_current = []
        self._id_sumoveh_current = None
        #self._time_depart = 0
        #self._attrs = {}
        #self._is_generate_ids = is_generate_ids
        self._intervals_current = ''

    def startElement(self, name, attrs):
        # <vehicle id="3_21" type="bus" depart="2520.00">
        # <route edges="bottom1to1/0 1/0to0/0 0/0tobottom0"/>
        # </vehicle>
        # print 'startElement',name
        if name == 'vehicle':
            # print '  startElement',attrs['id'],attrs['depart']
            self._ind_trip += 1

            self._id_sumoveh_current = attrs['id']
            self.ids_sumo[self._ind_trip] = self._id_sumoveh_current
            # print 'startElement ids_vtype',attrs['type'], self._ids_vtype_sumo.get_id_from_index(str(attrs['type']))

            self.ids_vtype[self._ind_trip] = self.ids_vtype[self._ind_trip] = self._get_id_vtype(attrs)

            self.times_depart[self._ind_trip] = int(float(attrs['depart']))

            if attrs.has_key('from'):
                self.ids_edge_depart[self._ind_trip] = self._ids_edge_sumo.get_id_from_index(str(attrs['from']))
            if attrs.has_key('to'):
                self.ids_edge_arrival[self._ind_trip] = self._ids_edge_sumo.get_id_from_index(str(attrs['to']))

            ind_lane_depart_raw = attrs.get('departLane', 'free')
            if OPTIONMAP_LANE_DEPART.has_key(ind_lane_depart_raw):
                self.inds_lane_depart[self._ind_trip] = OPTIONMAP_LANE_DEPART[ind_lane_depart_raw]
            else:
                self.inds_lane_depart[self._ind_trip] = int(ind_lane_depart_raw)

            positions_depart_raw = attrs.get('departPos', 'base')
            if OPTIONMAP_POS_DEPARTURE.has_key(positions_depart_raw):
                self.positions_depart[self._ind_trip] = OPTIONMAP_POS_DEPARTURE[positions_depart_raw]
            else:
                self.positions_depart[self._ind_trip] = float(positions_depart_raw)

            self.speeds_depart[self._ind_trip] = attrs.get('departSpeed', 0.0)

            ind_lane_arrival_raw = attrs.get('arrivalLane', 'current')
            if OPTIONMAP_LANE_ARRIVAL.has_key(ind_lane_arrival_raw):
                self.inds_lane_arrival[self._ind_trip] = OPTIONMAP_LANE_ARRIVAL[ind_lane_arrival_raw]
            else:
                self.inds_lane_arrival[self._ind_trip] = int(ind_lane_arrival_raw)

            positions_arrival_raw = attrs.get('arrivalPos', 'max')
            if OPTIONMAP_POS_ARRIVAL.has_key(positions_arrival_raw):
                self.positions_arrival[self._ind_trip] = OPTIONMAP_POS_ARRIVAL[positions_arrival_raw]
            else:
                self.positions_arrival[self._ind_trip] = float(positions_arrival_raw)

            self.speeds_arrival[self._ind_trip] = attrs.get('arrivalSpeed', 0.0)

        if name == 'route':
            self._has_routes = True
            # print ' ',attrs.get('edges', '')
            self._ids_sumoedge_current = attrs.get('edges', '')
            self._intervals_current = attrs.get('intervals', '')

    # def characters(self, content):
    #    if (len(self._route_current)>0)&(self._intervals_current!=''):
    #        self._intervals_current = self._intervals_current + content

    def endElement(self, name):

        if name == 'vehicle':
            # print 'endElement',name,self._id_current,len(self._intervals_current)
            if (self._id_sumoveh_current is not None):
                ids_edge = []
                for id_sumoedge in self._ids_sumoedge_current.split(' '):
                    if not id_sumoedge in ('', ' ', ','):
                        if self._ids_edge_sumo.has_index(id_sumoedge):
                            ids_edge.append(self._ids_edge_sumo.get_id_from_index(id_sumoedge.strip()))
                self.routes[self._ind_trip] = ids_edge

                if len(ids_edge) >= 1:
                    self.ids_edge_depart[self._ind_trip] = ids_edge[0]
                    self.ids_edge_arrival[self._ind_trip] = ids_edge[-1]

                self._id_sumoveh_current = None
                #self._attrs = {}
                self._ids_sumoedge_current = []

        # elif name in ['routes','trips']:
        #    self.make_trips()

    def process_intervals(self):
        interval = []
        es = self._intervals_current.rstrip().split(" ")
        for e in es:
            p = e.split(",")
            interval.append((float(p[0]), float(p[1])))
        self._intervals_current = ''
        return interval

    def _get_kwargs(self):
        return {'ids_sumo': self.ids_sumo,
                'times_depart': self.times_depart,
                'ids_edge_depart': self.ids_edge_depart,
                'ids_edge_arrival': self.ids_edge_arrival,
                'inds_lane_depart': self.inds_lane_depart,
                'positions_depart': self.positions_depart,
                'speeds_depart': self.speeds_depart,
                'inds_lane_arrival': self.inds_lane_arrival,
                'positions_arrival': self.positions_arrival,
                'speeds_arrival': self.speeds_arrival,
                }

    def insert_routes(self, is_generate_ids=True, is_add=False):
        # print 'TripReader.make_routes',is_generate_ids, is_add
        ids_trip = None
        if is_add:
            is_generate_ids = False
            # get trip ids from xml file
            ids_trip = self._trips.ids_sumo.get_ids_from_indices(self.ids_sumo)

        # print '  ids_trip',ids_trip
        ids_routes, ids_trips = self._trips.make_routes(self.ids_vtype,
                                                        is_generate_ids=is_generate_ids,
                                                        routes=self.routes,
                                                        ids_trip=ids_trip,
                                                        **self._get_kwargs())

        return ids_routes, ids_trips


if __name__ == '__main__':
    ###############################################################################
    # print 'sys.path',sys.path
    from agilepy.lib_wx.objpanel import objbrowser
    from agilepy.lib_base.logger import Logger
    #from coremodules.scenario import scenario
    from coremodules.network import network
    logger = Logger()
    NETPATH = os.path.join(SUMOPYDIR, 'coremodules', 'network', 'testnet')
    net = network.Network(logger=logger)
    rootname = 'facsp2'
    net.import_xml(rootname, NETPATH)
    # net.read_sumonodes(os.path.join(NETPATH,'facsp2.nod.xml'))
    # net.read_sumoedges(os.path.join(NETPATH,'facsp2.edg.xml'))
    demand = Demand(net=net, logger=logger)
    # demand.set_net(net)
    # landuse.facilities.import_poly(os.path.join(NETPATH,'facsp2.poly.xml'))
    #landuse.import_xml(rootname, NETPATH)
    objbrowser(demand)
