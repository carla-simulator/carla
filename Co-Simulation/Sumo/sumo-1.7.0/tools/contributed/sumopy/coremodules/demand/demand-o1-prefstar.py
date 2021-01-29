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

# @file    demand-o1-prefstar.py
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

OPTIONMAP_POS_DEPARTURE = {"random": -1, "free": -2, "random_free": -3, "base": -4, "last": -5, "first": -6}
OPTIONMAP_POS_ARRIVAL = {"random": -1, "max": -2}
OPTIONMAP_SPEED_DEPARTURE = {"random": -1, "max": -2}
OPTIONMAP_SPEED_ARRIVAL = {"current": -1}
OPTIONMAP_LANE_DEPART = {"random": -1, "free": -2, "allowed": -3, "best": -4, "first": -5}
OPTIONMAP_LANE_ARRIVAL = {"current": -1}

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
import vehicles
import origin_to_destination
import virtualpop
import turnflows


class Demand(cm.BaseObjman):
    def __init__(self, scenario=None, net=None, zones=None, name='Demand', info='Transport demand', **kwargs):
        # print 'Network.__init__',name,kwargs

        # we need a network from somewhere
        if net is None:
            net = scenario.net
            zones = scenario.landuse.zones

        self._init_objman(ident='demand', parent=scenario, name=name, info=info, **kwargs)
        attrsman = self.set_attrsman(cm.Attrsman(self))

        self.vtypes = attrsman.add(cm.ObjConf(vehicles.VehicleTypes(self, net)))
        self.trips = attrsman.add(cm.ObjConf(Trips(self, net)))

        self.odintervals = attrsman.add(cm.ObjConf(origin_to_destination.OdIntervals('odintervals', self, net, zones)))
        self.turnflows = attrsman.add(cm.ObjConf(turnflows.Turnflows('turnflows', self, net)))
        if scenario is not None:
            self.virtualpop = attrsman.add(cm.ObjConf(virtualpop.Virtualpopulation(self)))
        # print 'Demand',self.odintervals#,self.odintervals.times_start
        # print ' ',dir(self.odintervals)

    def get_scenario(self):
        return self.parent

    def get_net(self):
        return self.parent.net

    def update_netoffset(self, deltaoffset):
        """
        Called when network offset has changed.
        Children may need to adjust theur coordinates.
        """
        # self.odintervals.update_netoffset(deltaoffset)
        pass

    def import_xml(self, rootname, dirname=''):
        """
        Import whatever is available.
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


class Trips(am.ArrayObjman):
    def __init__(self, demand, net, **kwargs):
        # print 'Trips.__init__'
        self._init_objman(ident='trips',
                          parent=demand,
                          name='Trips',
                          info='Table with trip and route info.',
                          xmltag=('trips', 'trip', 'ids_sumo'),
                          **kwargs)

        self.add_col(SumoIdsConf('Trip', xmltag='id'))

        self.add_col(am.IdsArrayConf('ids_vtype', demand.vtypes,
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
                                  xmltag='departLane',
                                  xmlmap=get_inversemap(OPTIONMAP_LANE_DEPART),
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
                                  xmltag='arrivalLane',
                                  xmlmap=get_inversemap(OPTIONMAP_LANE_ARRIVAL),
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

        # print '  self.routes.get_value()',self.routes.get_value()
        self.add_col(am.IdlistsArrayConf('ids_routes', self.routes.get_value(),
                                         name='Route IDs',
                                         info='List of edge IDs constituting the route.',
                                         ))

    def clear_trips(self):
        self.routes.get_value().clear_routes()
        self.clear()

    def clear_routes(self):
        self.routes.get_value().clear_routes()
        self.ids_routes.reset()

    def get_net(self):
        return self.parent.get_scenario().net

    def get_scenario(self):
        return self.parent.get_scenario()

    def get_time_depart_first(self):
        if len(self) > 0:
            return float(np.min(self.times_depart.get_value()))
        else:
            return 0.0

    def get_time_depart_last(self):
        if len(self) > 0:
            return float(np.max(self.times_depart.get_value()))
        else:
            return 0.0

    def get_tripfilepath(self):
        return self.get_scenario().get_rootfilepath()+'.trip.xml'

    def get_routefilepath(self):
        return self.get_scenario().get_rootfilepath()+'.rou.xml'

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

    def route(self, is_check_lanes=True):
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
        for id_vtype in self.get_vtypes():
            id_mode = vtypes.ids_mode[id_vtype]

            # no routing for pedestrians
            if id_mode != net.modes.get_id_mode('pedestrian'):
                weights = edges.get_times(id_mode=id_mode,
                                          speed_max=vtypes.speeds_max[id_vtype],
                                          is_check_lanes=is_check_lanes)

                ids_trip_vtype = self.get_trips_for_vtype(id_vtype)
                # print '  id_vtype,id_mode',id_vtype,id_mode#,ids_trip_vtype
                # print '  weights',weights
                ids_edge_depart = self.ids_edge_depart[ids_trip_vtype]
                ids_edge_arrival = self.ids_edge_arrival[ids_trip_vtype]

                for id_trip, id_edge_depart, id_edge_arrival in zip(ids_trip_vtype, ids_edge_depart,  ids_edge_arrival):
                    cost, route = routing.get_mincostroute_edge2edge(id_edge_depart,
                                                                     id_edge_arrival,
                                                                     edges=edges,
                                                                     weights=weights)
                    if len(route) > 0:
                        ids_edges.append(route)
                        ids_trip.append(id_trip)
                        costs.append(cost)

        ids_route = self.routes.get_value().add_rows(ids_trip=ids_trip,
                                                     ids_edges=ids_edges,
                                                     costs=costs,
                                                     )
        self.add_routes(ids_trip, ids_route)
        print '  exectime', time.clock()-exectime_start
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
            id_route = self.routes.get_value().add_row(ids_trip=id_trip,
                                                       ids_edges=kwargs['route']
                                                       )
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
                       attrconfigs_excluded=[self.routes, self.ids_routes],
                       is_print_begin_end=False)

        fd.write(xm.end(xmltag))
        fd.close()
        return filepath

    def export_routes_xml(self, filepath=None, method_routechoice=None, encoding='UTF-8'):
        """
        Export routes to SUMO xml file.
        Method takes care of sorting trips by departure time.
        """
        if method_routechoice is None:
            method_routechoice = self.get_route_first

        if filepath is None:
            filepath = self.get_routefilepath()
        print 'export_routes_xml', filepath
        try:
            fd = open(filepath, 'w')
        except:
            print 'WARNING in write_obj_to_xml: could not open', filepath
            return False

        xmltag_routes, xmltag_veh, attrname_id = ("routes", "vehicle", "ids_sumo")
        xmltag_trip = "trip"
        xmltag_rou = "route"

        fd.write('<?xml version="1.0" encoding="%s"?>\n' % encoding)
        fd.write(xm.begin(xmltag_routes))
        indent = 2

        #ids_modes_used = set(self.parent.vtypes.ids_mode[self.ids_vtype.get_value()])
        self.parent.vtypes.write_xml(fd, indent=indent,
                                     ids=set(self.ids_vtype.get_value()),
                                     is_print_begin_end=False
                                     )

        ids_mode = self.parent.vtypes.ids_mode
        id_pedestrian = MODES['pedestrian']
        routes = self.routes.get_value()

        # here we could write the route info
        # but we do write it inside each trip so that it can be parsed
        # in the same way as duarouter output
        # routes.write_xml(   fd, indent=indent,
        #                    attrconfigs_excluded = [routes.costs, routes.probabilities],
        #                    is_print_begin_end = False)

        # let's write trip info manually
        tripconfigs = [self.ids_vtype,
                       self.times_depart,
                       self.ids_edge_depart,
                       self.ids_edge_arrival,
                       self.inds_lane_depart,
                       self.positions_depart,
                       self.speeds_depart,
                       self.inds_lane_arrival,
                       self.positions_arrival,
                       self.speeds_arrival,
                       ]

        routeconfigs = [routes.ids_edges,
                        routes.colors,
                        ]

        attrconfig_id = getattr(self.get_attrsman(), attrname_id)
        xmltag_id = attrconfig_id.xmltag

        for id_trip in self.times_depart.get_ids_sorted():

            if ids_mode[self.ids_vtype[id_trip]] == id_pedestrian:
                self.write_persontrip_xml(fd, id_trip,
                                          method_routechoice=method_routechoice,
                                          indent=indent+2)

            else:
                id_route = method_routechoice(id_trip)
                if id_route >= 0:  # a valid route has been found
                    # init vehicle route only if valid route exists
                    fd.write(xm.start(xmltag_veh, indent+2))
                else:
                    # init trip instead of route
                    fd.write(xm.start(xmltag_trip, indent+2))

                # print '   make tag and id',_id
                fd.write(xm.num(xmltag_id, attrconfig_id[id_trip]))

                # print ' write columns',len(scalarcolconfigs)>0,len(idcolconfig_include_tab)>0,len(objcolconfigs)>0
                for attrconfig in tripconfigs:
                    # print '    attrconfig',attrconfig.attrname
                    attrconfig.write_xml(fd, id_trip)

                if id_route >= 0:  # a valid route has been found
                    # write route id
                    #fd.write(xm.num('route', id_route ))

                    # instead of route id we write entire route here
                    fd.write(xm.stop())
                    fd.write(xm.start(xmltag_rou, indent+4))
                    for attrconfig in routeconfigs:
                        # print '    attrconfig',attrconfig.attrname
                        attrconfig.write_xml(fd, id_route)

                    # end route and vehicle
                    fd.write(xm.stopit())
                    fd.write(xm.end(xmltag_veh, indent+2))

                else:
                    # end trip without route
                    fd.write(xm.stopit())

        fd.write(xm.end(xmltag_routes))
        fd.close()
        return filepath

    def write_persontrip_xml(self, fd, id_trip, indent=2, method_routechoice=None):
        # currently no routes are exported, only origin and destination edges
        # if method_routechoice is None:
        #    method_routechoice = self.get_route_first
        xmltag_person = 'person'
        id_route = method_routechoice(id_trip)

        fd.write(xm.start(xmltag_person, indent))

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
        fd.write(xm.end(xmltag_person, indent=indent))

    def get_route_first(self, id_trip):
        ids_route = self.ids_routes[id_trip]
        if ids_route is None:
            return -1
        elif len(ids_route) > 0:
            return ids_route[0]
        else:
            return -1  # no route found

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

    def __init__(self, trips,  n_trip):
        # print 'RouteReader.__init__',demand.ident
        self._trips = trips
        demand = trips.parent

        net = demand.get_scenario().net

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
        if name == 'trip':
            # print '  startElement',attrs['id'],attrs['depart']
            self._ind_trip += 1

            self._id_sumoveh_current = attrs['id']
            self.ids_sumo[self._ind_trip] = self._id_sumoveh_current
            print 'startElement ids_vtype', attrs['type'], self._ids_vtype_sumo.get_id_from_index(str(attrs['type']))
            self.ids_vtype[self._ind_trip] = self._ids_vtype_sumo.get_id_from_index(str(attrs['type']))
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
            self.ids_vtype[self._ind_trip] = self._ids_vtype_sumo.get_id_from_index(str(attrs['type']))
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
