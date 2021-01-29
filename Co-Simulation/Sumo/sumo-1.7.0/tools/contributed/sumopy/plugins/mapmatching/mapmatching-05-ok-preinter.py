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

# @file    mapmatching-05-ok-preinter.py
# @author  Joerg Schweizer
# @date

"""
This plugin provides methods to match GPS traces to a SUMO road network and to generate routes.

The general structure is as follows:
GPSPoints
routes

Traces
    
"""
import os
import sys
from xml.sax import saxutils, parse, handler

import time
import string
import numpy as np
from xml.sax import saxutils, parse, handler

if __name__ == '__main__':
    try:
        APPDIR = os.path.dirname(os.path.abspath(__file__))
    except:
        APPDIR = os.path.dirname(os.path.abspath(sys.argv[0]))
    SUMOPYDIR = os.path.join(APPDIR, '..', '..')
    sys.path.append(SUMOPYDIR)


from coremodules.modules_common import *
import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
import agilepy.lib_base.xmlman as xm
#from agilepy.lib_base.misc import get_inversemap
from agilepy.lib_base.geometry import get_dist_point_to_segs
from agilepy.lib_base.processes import Process  # ,CmlMixin,ff,call
from coremodules.network.network import SumoIdsConf
from coremodules.network import routing
from coremodules.demand.demand import Trips, Routes

try:
    try:
        import pyproj
    except:
        from mpl_toolkits.basemap import pyproj
    try:
        from shapely.geometry import Polygon, MultiPolygon, MultiLineString, Point, LineString, MultiPoint, asLineString, asMultiPoint
        from shapely.ops import cascaded_union
    except:
        print 'Import error: No shapely module available.'
except:
    print 'Import error: in order to run the traces plugin please install the following modules:'
    print '   mpl_toolkits.basemap and shapely'
    print 'Please install these modules if you want to use it.'
    print __doc__
    raise

TRIPPUROPSES = {'unknown': -1,
                'HomeToWork': 1,
                'WorkToHome': 2,
                'HomeToSchool': 3,
                'SchoolToHome': 4,
                'SchoolToHome': 5,
                'Leisure': 6,
                'Other': 7,
                }

OCCUPATIONS = {'unknown': -1,
               'Worker': 1,
               'Student': 2,
               'Employee': 3,
               'Public employee': 4,
               'Selfemployed': 5,
               'Pensioneer': 6,
               'Other': 7
               }

DEVICES = {'unknown': -1,
           'cy-android': 1,
           'cy-windows': 2,
           'cy-ios': 3,
           'cy-web-gpx': 4,
           'cy-web-manual': 5,
           'Other': 7
           }

WEEKDAYCHOICES = {'Monday-Friday': [0, 1, 2, 3, 4],
                  'Monday-Saturday': [0, 1, 2, 3, 4, 5],
                  'Saturday, Sunday': [5, 6],
                  'all': [0, 1, 2, 3, 4, 5, 6],
                  }

# Structure GPS traces


# 2016

# UserID	                    TripID	                    TimeStamp	Start DT	                Distance	 ECC	 AvgSpeed	 TrackType	 Sex	 Year	 Profession	 Frequent User	 ZIP	 Source	  TypeOfBike	 TipeOfTrip	 Max Spd
# 57249bcd88c537874f9fa1ae	57515edc88c537576ca3e16f	1464945480	2016-06-03T09:18:00.000Z	4.75	4.75	    10.16	    urban bicycle	F	1999	Studente	    yes		            cy-web-gpx	MyBike	    HomeToSchool	25.45
# 5550800888c53765217661aa	574ec13788c537476aa3e122	1464797040	2016-06-01T16:04:00.000Z	2.93	2.93	    5.87	    urban bicycle	F	1972	Worker	        yes	        40136	cy-web-gpx	--	--	                    0
# 5535586e88c53786637b23c6	574f3c3688c537c877a3e139	1464796080	2016-06-01T15:48:00.000Z	8.95	8.95	    7.84	    urban bicycle	M	1973	Lavoratore	    yes	        40133	cy-web-gpx	MyBike	    HomeToWork	    34.08
# 5550800888c53765217661aa	574ec12b88c537812fa3e118	1464781800	2016-06-01T11:50:00.000Z	3.75	3.75	    14.99	    urban bicycle	F	1972	Worker	        yes	        40136	cy-web-gpx	--	--	    0
# 5550800888c53765217661aa	574ec11988c5370944a3e107	1464778980	2016-06-01T11:03:00.000Z	2.77	2.77	    11.09	    urban bicycle	F	1972	Worker	        yes	        40136	cy-web-gpx	--	--	    0
# 55e5edfc88c5374b4974fdbc	574e98c988c5378163a3e11f	1464765060	2016-06-01T07:11:00.000Z	11.09	11.14	    15.63	    urban bicycle		1982		            SI		            cy-web-gpx	MyBike	    HomeToWork	    27.9


# csv 2016
# TripID, TimeStamp,Latitude, Longitude, Altitude, Distance, Speed, Type
# 574e98c988c5378163a3e11f,1462347278,44.52606,11.27617,78,0.027255420500625783,5,<start|mid|end>,
# 5741cdd388c537f10192ee97, 1463926725,44.50842,11.3604,101.0417,0.01615021486623964,3.483146,mid
# timestamp: 1464160924 after 1970


# 2015 csv

# workouts
# UserID	 TripID	 TimeStamp	 Start DT	  Distance	 AvgSpeed	 TrackType	 Sex	 Year	 Profession	 Frequent User	 ZIP
# 54eb068de71f393530a9a74d	54eb0737e71f394c2fa9a74d	1424692504	Mon, 23 Feb 2015 11:55:04 GMT	0	0	urban bicycle	M	1987	Developer	no
# 54eb9374e71f39f02fa9a750	5505cb04e71f39542e25e2d4	1426442994	Sun, 15 Mar 2015 18:09:54 GMT	0	0.7	urban bicycle	M	1974	Worker	yes	40128


#TripID, TimeStamp,Date, Latitude, Longitude, Altitude, Distance, Speed, Type
# 54eb0737e71f394c2fa9a74d,1424692509,"Mon, 23 Feb 2015 11:55:09 GMT",44.499096,11.361185,49.419395,0,0.000815,start


# https://docs.python.org/2/library/time.html
# >>> t = time.mktime((2017,03,07,13,46,0,0,0,0))
# >>> lt = time.localtime(t)
# >>> time.strftime("%a, %d %b %Y %H:%M:%S +0000", lt)
#'Tue, 07 Mar 2017 13:46:00 +0000'
# >>> time.strftime("%a, %d %b %Y %H:%M:%S", lt)
#'Tue, 07 Mar 2017 13:46:00'


def calc_seconds(t_data,
                 sep_date_clock=' ', sep_date='-', sep_clock=':',
                 is_float=False):
    """
    Returns time in seconds after 1/1/1970.
    Time format for time data string used:
        2012-05-02 12:57:08.0
    """

    if len(t_data.split(sep_date_clock)) != 2:
        return -1
    (date, clock) = t_data.split(sep_date_clock)

    if (len(clock.split(sep_clock)) == 3) & (len(date.split(sep_date)) == 3):
        (year_str, month_str, day_str) = date.split(sep_date)
        (hours_str, minutes_str, seconds_str) = clock.split(sep_clock)
        t = time.mktime((int(year_str), int(month_str), int(day_str),
                         int(hours_str), int(minutes_str), int(float(seconds_str)), -1, -1, -1))

        # print 'calc_seconds',t
        # print '  t_data'
        # print '  tupel',int(year_str),int(month_str),int(day_str), int(hours_str),int(minutes_str),int(float(seconds_str)),0,0,0
        if is_float:
            return t
        else:
            return int(t)
    else:
        return -1


def get_routelinestring(route, edges):
        # print 'get_routelinestring'
        # TODO: we could make the np.sum of shapes if shapes where lists
    shape = []
    for id_edge in route:
            # print '    ',edges.shapes[id_edge ],type(edges.shapes[id_edge ])
        shape += list(edges.shapes[id_edge])
    # print '  ',shape

    return LineString(shape)


def get_boundary(coords):
    # print 'get_boundary',len(coords),coords.shape
    # print '  coords',coords
    if len(coords) == 0:
        return [0, 0, 0, 0]
    else:
        x_min, y_min = coords[:, :2].min(0)
        x_max, y_max = coords[:, :2].max(0)

        return [x_min, y_min, x_max, y_max]


class BirgilMatcher(Process):
    def __init__(self,  mapmatching, logger=None, **kwargs):
        print 'BirgilMatcher.__init__'
        self._init_common('birgilmatcher',
                          parent=mapmatching,
                          name='Birgillito Map matching',
                          logger=logger,
                          info='Birgillito Map matching.',
                          )

        attrsman = self.set_attrsman(cm.Attrsman(self))

        self.width_buffer_max = attrsman.add(cm.AttrConf('width_buffer_max', kwargs.get('width_buffer_max', 40.0),
                                                         groupnames=['options'],
                                                         perm='rw',
                                                         name='Max. buffer width',
                                                         unit='m',
                                                         info='GPS points are only valid if they are within the given maximum buffer width distant from a network edge.',
                                                         ))

        self.n_points_min = attrsman.add(cm.AttrConf('n_points_min', kwargs.get('n_points_min', 5),
                                                     groupnames=['options'],
                                                     perm='rw',
                                                     name='Min. point number',
                                                     info='Minimum number of valid GPS points. Only if this minimum number is reached, a map-matching attempt is performed.',
                                                     ))

        # self.dist_min = attrsman.add(cm.AttrConf( 'dist_min',kwargs.get('dist_min',3.0),
        #                    groupnames = ['options'],
        #                    name = 'Min. dist',
        #                    unit = 'm',
        #                    info = 'Minimum distance used in the calculation of the edge cost function.',
        #                    ))

        self.weight_cumlength = attrsman.add(cm.AttrConf('weight_cumlength', kwargs.get('weight_cumlength', 0.01),
                                                         groupnames=['options'],
                                                         name='Cumlength weight',
                                                         info='Cost function weight of cumulative length.',
                                                         ))

        self.weight_angle = attrsman.add(cm.AttrConf('weight_angle', kwargs.get('weight_angle', 20.0),
                                                     groupnames=['options'],
                                                     name='Angle weight',
                                                     info='Cost function weight of angular coincidence.',
                                                     ))

        self.weight_access = attrsman.add(cm.AttrConf('weight_access', kwargs.get('weight_access', 5.0),
                                                      groupnames=['options'],
                                                      name='Access weight',
                                                      info='Cost function weight of access level for matched mode.',
                                                      ))

        self.n_routes_follow = attrsman.add(cm.AttrConf('n_routes_follow', kwargs.get('n_routes_follow', 20),
                                                        groupnames=['options'],
                                                        name='Followed routes',
                                                        info='Number of routes which are followed in parallel.',
                                                        ))

    def do(self):
        trips = self.parent.trips
        #routes = trips.get_routes()
        edges = self.get_edges()
        fstar = edges.get_fstar(is_return_arrays=True, is_ignor_connections=True)
        times = edges.get_times(id_mode=2, is_check_lanes=False, speed_max=5.555)
        ids_trip = trips.get_ids_selected()

        vtypes = self.parent.get_scenario().demand.vtypes
        ids_vtype = trips.ids_vtype[ids_trip]
        ids_mode = vtypes.ids_mode[ids_vtype]

        costs = {}
        for id_mode in set(ids_mode):
            ids_vtype_mode = vtypes.select_by_mode(id_mode)
            costs[id_mode] = edges.get_times(id_mode=id_mode,
                                             speed_max=np.max(vtypes.speeds_max[ids_vtype_mode]),
                                             #is_check_lanes = True
                                             )

        # ,vtypes.speed_max[ids_vtype]
        for id_trip, ids_point, id_mode in zip(ids_trip, trips.ids_points[ids_trip], ids_mode):

            route, length_route, length_route_mixed, length_route_exclusive, duration_gps, lengthindex, err_dist, t_match = \
                self.match_trip_birgil(id_trip, ids_point, id_mode, fstar, costs)

            trips.set_matched_route(id_trip, route,
                                    length_route,
                                    length_route_mixed,
                                    length_route_exclusive,
                                    duration_gps,
                                    lengthindex,
                                    err_dist,
                                    t_match)

    def get_edges(self):
        return self.parent.get_scenario().net.edges

    def match_trip_birgil(self, id_trip,  ids_point, id_mode,  fstar, costsdir):
        # TODO: record time intervals
        # calculate initial and final position on edge to get more precise
        # triplength
        print 79*'='
        print 'match_trip_birgil', id_trip
        costs = costsdir[id_mode]
        tick = time.time()
        routes = []
        route = None
        route_mindist = None
        intervals_route = []
        length_route = -1.0
        length_bikeway = -1.0
        length_mindist = -1.0
        matchindex = -1.0
        lengthindex = -1.0
        err_dist = -1.0
        err_dist_alg = -1.0
        t_match = -1.0
        duration_gps = -1.0
        duration_est = -1.0

        points = self.parent.points
        #trips = self.parent.trips
        #ids_point = trips.ids_points[id_trip]
        #pointset = self.pointsets.get(id_trip)

        # create a multi point object with points of traces
        # should no longer be necessary
        #tracepoints = MultiPoint(pointset.get_coords().tolist())

        # make a array with time stamps of all points
        pointtimes = points.timestamps[ids_point]
        # segind_to_id_edge = self._segind_to_id_edge #<<<<<<<<<<<
        coords = points.coords[ids_point][:, :2]

        x1, y1, x2, y2 = self.parent.get_segvertices_xy()
        edges = self.get_edges()
        get_ids_edge_from_inds_seg = edges.get_ids_edge_from_inds_seg
        get_dist_point_to_edge = edges.get_dist_point_to_edge
        #edgehit_counts = np.zeros(len(self._net.getEdges()),int)
        # segind_to_id_edge = self._segind_to_id_edge
        # for p in pointset.get_coords():
        #    print '  ',p
        #    hitinds = get_dist_point_to_segs(p,self._x1,self._y1,self._x2,self._y2, is_ending=True)< self.width_buffer**2
        #    edgehit_counts[segind_to_id_edge[hitinds]]+= 1

        # ---------------------------------------------------------------------
        # 1. initialization

        # Set of initial edges

        # create a list with lists  for each point
        # Each list contans the edges, where the point is in the edge buffer
        #edges_containing_point = np.zeros(len(ids_points), object)
        #edgedists_to_point = np.zeros(len(ids_points), object)

        ids_edge_initial = []
        ids_edge_final = []
        dists_initial = []
        n_points = len(ids_point)
        ind_point_initial = -1
        ind_point_final = -1
        t_point_initial = -1
        t_point_final = -1

        # print '  search initial edges'
        ind_point = 0
        for p in coords:
            dists2 = get_dist_point_to_segs(p, x1, y1, x2, y2)
            inds_hit = np.flatnonzero(dists2 < self.width_buffer_max**2)
            # print '    id_point,inds_hit',ids_point[ind_point],inds_hit
            #ids_edge = get_ids_edge_from_inds_seg(inds_hit)

            if len(inds_hit) > 0:
                if ind_point_initial < 0:
                    # print '   inds_hit',ind_point,inds_hit
                    ids_edge_initial = set(get_ids_edge_from_inds_seg(inds_hit))
                    # print '    ids_edge_initial',ids_edge_initial
                    #dists_initial = np.sqrt(dists2[inds_hit])
                    ind_point_initial = ind_point
                    t_point_initial = pointtimes[ind_point]
                    # print '   ind_point, inds_hit, ind_point_initial',ind_point,inds_hit,ind_point_initial,ids_edge_initial
                else:
                    ids_edge_final = set(get_ids_edge_from_inds_seg(inds_hit))
                    # print '    ids_edge_final',ids_edge_final
                    ind_point_final = ind_point
                    t_point_final = pointtimes[ind_point]
                    # print '   ind_point, inds_hit, ind_point_final',ind_point,inds_hit,ind_point_initial,ids_edge_final

            ind_point += 1

        n_points_eff = ind_point_final - ind_point_initial

        duration_gps = pointtimes[ind_point_final] - pointtimes[ind_point_initial]

        # if self._logger:
        #    self._logger.w( '>>match_trip_birgil : n_points_eff=%d, len(ids_edge_initial)=%d,len(ids_edge_final)=%d'%(n_points_eff, len(ids_edge_initial),len(ids_edge_final)) )
        # print '  completed init:'
        print '  ids_edge_initial', ids_edge_initial
        print '  ids_edge_final', ids_edge_final

        # print '  ind_point_initial,ind_point_final,n_points_eff',ind_point_initial,ind_point_final,n_points_eff
        print '  id_point_initial=%d,id_point_final=%d,n_points_eff=%d' % (
            ids_point[ind_point_initial], ids_point[ind_point_final], n_points_eff)
        if (ind_point_initial < 0) | (ind_point_final < 0) | (n_points_eff < self.n_points_min):
            print 'ABOARD: insufficient valid points'
            return [], 0.0, 0.0, -1.0, -1.0, -1.0, -1.0, 0.0

        # print '  coords',coords
        # create initial routeset
        routelist = []

        #id_edge_unique = []
        for id_edge in ids_edge_initial:

            cost0 = get_dist_point_to_edge(coords[ind_point_initial], id_edge)

            # print '  coords of initial point:',coords[ind_point_initial]
            # print '  cost0, id_edge',cost0, id_edge

            #get_ind_seg_from_id_edge(self, id_edge)

            length_edge = edges.lengths[id_edge]
            accesslevel = edges.get_accesslevel(id_edge, id_mode)
            cost_tot = cost0+self.weight_cumlength * length_edge - self.weight_access * accesslevel
            routelist.append([cost_tot, cost0, 0.0, length_edge, accesslevel, length_edge, [id_edge]])

        # print '  initial routelist',routelist

        # ---------------------------------------------------------------------
        # 2. main loop through rest of the points

        # for i in xrange(ind_point+1, n_points):#coords[ind_point+1:]:
        #is_connected = True
        ind_point = ind_point_initial+1
        length_gps = 0.0
        while (ind_point < ind_point_final):  # & is_connected:
            point = coords[ind_point]
            delta_point = point-coords[ind_point-1]
            dist_interpoint = np.sqrt(np.sum(delta_point**2))
            phi_point = np.arctan2(delta_point[1], delta_point[0])
            length_gps += dist_interpoint

            # print 79*'_'
            # print '    check ID point %d,  dist_interpoint=%.2fm, length_gps=%.2fm, phi_point %d deg'%(ids_point[ind_point],dist_interpoint,length_gps,phi_point/np.pi*180)

            routelist_new = []
            ind_route = 0
            for routeinfo in routelist:
                costs_tot, cost, length_partial, length_cum, accesslevel, length_edge, ids_edge = routeinfo

                if len(ids_edge) == 0:
                    break

                length_partial += dist_interpoint
                id_edge_last = ids_edge[-1]
                # minimum distance point-edge
                dist_point_edge = get_dist_point_to_edge(point, id_edge_last,
                                                         is_detect_initial=False,
                                                         is_detect_final=True,)
                #dist_point_edge = self.get_dist_point_edge(coords[ind_point], id_edge_last, is_detect_final = True)
                # if dist_point_edge is not a number (nan) the point is outside projection of edge

                # print 79*'-'
                # print '      route ',ind_route,'costs_tot',costs_tot,'dist_point_edge',dist_point_edge, len(ids_edge),id_edge_last
                # print '        cost= %.3f, length_cum=%.3f'%(cost,length_cum,)
                # print '        Xost= %.3f, Xength_cum=%.3f'%(cost,self.weight_cumlength *length_cum,)
                # print '        ids_edge',ids_edge
                # if length_partial > self.alpha * length_edge:
                if (np.isnan(dist_point_edge)) | (dist_point_edge > self.width_buffer_max):
                    # point is beyond edge

                    # has reached end of edge
                    length_partial = length_partial-length_edge

                    # get FSTAR
                    # print '        fstar',id_edge_last,fstar[id_edge_last]
                    ids_edge_next_all = fstar[id_edge_last]

                    # filter allowed edges, by verifying positivness of
                    # travel costs
                    ids_edge_next = ids_edge_next_all[costs[ids_edge_next_all] >= 0]
                    # print '        parse ids_edge_next',ids_edge_next

                    if len(ids_edge_next) == 0:
                        # route not connected
                        cost_edge = np.Inf  # punish route with infinite costs
                        length_edge = np.Inf
                        length_cum = np.Inf
                        accesslevel = -1
                        ids_edge_new = []
                    else:

                        id_edge = ids_edge_next[0]
                        accesslevel = edges.get_accesslevel(id_edge, id_mode)
                        length_edge = edges.lengths[id_edge]
                        length_cum += length_edge
                        cost_edge = self._get_cost_birgil(point, id_edge, phi_point,
                                                          accesslevel, get_dist_point_to_edge)
                        #self.get_dist_point_edge(coords[ind_point], ind_edge)
                        ids_edge_new = ids_edge + [id_edge]
                        # print '      add to route id_edge',id_edge,ids_edge_new

                    # save updated data of current route
                    cost0 = cost+cost_edge
                    cost_tot = cost0+self.weight_cumlength * length_cum
                    routeinfo[:] = cost_tot, cost0, length_partial, length_cum, accesslevel, length_edge, ids_edge_new

                    # add new children if forward star greater 1
                    if len(ids_edge_next) > 1:
                        for id_edge in ids_edge_next[1:]:
                            #ind_edge = self._edge_to_id_edge[edge]
                            length_edge = edges.lengths[id_edge]
                            length_cum += length_edge
                            accesslevel = edges.get_accesslevel(id_edge, id_mode)
                            cost_edge = self._get_cost_birgil(
                                point, id_edge, phi_point, accesslevel, get_dist_point_to_edge)
                            ids_edge_new = ids_edge + [id_edge]
                            # print '      new route id_edge',id_edge,ids_edge_new
                            cost0 = cost+cost_edge
                            cost_tot = cost0+self.weight_cumlength * length_cum
                            routelist_new.append([cost_tot, cost0, length_partial, length_cum,
                                                  accesslevel, length_edge, ids_edge_new])

                else:
                    # has not reached end of current edge
                    # save updated data of current route
                    if len(ids_edge) > 0:
                        #dist += self.get_dist_point_edge(coords[ind_point], ids_edge[-1])
                        cost += self._get_cost_birgil(point, id_edge_last, phi_point,
                                                      accesslevel, get_dist_point_to_edge)
                    cost_tot = cost+self.weight_cumlength * length_cum
                    routeinfo[:] = cost_tot, cost, length_partial, length_cum, accesslevel, length_edge, ids_edge

                ind_route += 1

            routelist += routelist_new
            # print '  routelist',routelist
            routelist.sort()

            # cut list to self.n_routes_follow
            if len(routelist) > self.n_routes_follow+1:
                routelist = routelist[:self.n_routes_follow+1]

            ind_point += 1

        # recover final edge objects of winner route!
        costs_tot, cost, length_partial, length_cum, accesslevel, length_edge, ids_edge = routelist[0]

        # print '  ids_edge[-1]',ids_edge[-1],ids_edge[-1] in ids_edge_final
        # print '  ids_edge_final',ids_edge_final
        t_match = time.time() - tick

        # --------------------------------------------------------------------
        # post matching analisis
        print '\n'+79*'-'
        print '  matched route:', ids_edge
        route = ids_edge
        if len(route) == 0:
            print 'ABOARD: route contains no edges ids_edge=', ids_edge
            return [], 0.0, 0.0, -1.0, -1.0, -1.0, -1.0, 0.0

        length_route = np.sum(edges.lengths[ids_edge])
        length_route_mixed = 0.0
        length_route_exclusive = 0.0
        for id_edge, length in zip(ids_edge, edges.lengths[ids_edge]):
            accesslevel = edges.get_accesslevel(id_edge, id_mode)
            if accesslevel == 1:
                length_route_mixed += length
            elif accesslevel == 2:
                length_route_exclusive += length

        if length_gps > 0:
            lengthindex = length_route/length_gps
        else:
            lengthindex = -1.0

        if ids_edge[-1] in ids_edge_final:  # dist!=np.Inf: DID WE ARRIVE?
            print 'SUCCESS: target', ids_edge[-1], 'reached'

        else:
            print 'ABOARD: last matched edge', ids_edge[-1], ' did not reach final edges', ids_edge_final
            return [], 0.0, 0.0, -1.0, -1.0, -1.0, -1.0, 0.0

        # check dist error
        inds_point = xrange(ind_point_initial, ind_point_final)
        n_points = len(inds_point)

        if (n_points >= 2) & (len(route) > 0):
            # print '  initialize distance error measurement',len(route)
            dist_points_tot = 0.0
            routestring = get_routelinestring(route, edges)
            # print '  routestring =',routestring

            # print '  coords',coords[inds_point].tolist()
            tracepoints = MultiPoint(coords[inds_point].tolist())

            # measure precision
            #routeset = set(route)
            #n_points_boundary = 0
            ind_tracepoint = 0
            for ind_point in inds_point:  # xrange(n_points):
                d = routestring.distance(tracepoints[ind_tracepoint])
                dist_points_tot += d
                # print '   v distance measurement',d,tracepoints[ind_tracepoint],coords[ind_point]#,n_points#,n_points_eff

                ind_tracepoint += 1

            err_dist = dist_points_tot/float(n_points)

        # print '  check',(lengthindex<self.lengthindex_max),(lengthindex >= self.lengthindex_min),(duration_gps>self.duration_min),(length_route>self.dist_min),(len(route)>0)
        # print '   twice',(lengthindex<self.lengthindex_max)&(lengthindex >= self.lengthindex_min)&(duration_gps>self.duration_min)&(length_route>self.dist_min)&(len(route)>0)

        return route, length_route, length_route_mixed, length_route_exclusive, duration_gps, lengthindex, err_dist, t_match

    def _get_cost_birgil(self, p, id_edge, phi_point, accesslevel, get_dist_point_to_edge):
        # print '_get_cost_birgil p, id_edge',p, id_edge

        dist, segment = get_dist_point_to_edge(p, id_edge,
                                               is_return_segment=True)
        x1, y1, x2, y2 = segment

        # print '  x1,y1',x1,y1
        # print '  p',p[0],p[1]
        # print '  x2,y2',x2,y2
        # print '  dist',dist
        #seginds = self._id_edge_to_seginds[ind_edge]
        #dists2 = get_dist_point_to_segs( p, self._x1[seginds], self._y1[seginds], self._x2[seginds], self._y2[seginds])

        #ind_segind = np.argmin(dists2)
        #segind = seginds[ind_segind]
        #x1,y1,x2,y2 = (self._x1[segind], self._y1[segind], self._x2[segind], self._y2[segind])
        phi_seg = np.arctan2(y2-y1, x2-x1)
        #dist_ps = max(dist,  self.dist_min)
        # print '  x1,y1,x2,y2',x1,y1,x2,y2
        # print '     dist =%.1f phi_point=%.2f,phi_seg=%.2f,phi_point-phi_seg=%.2f, delta_phi=%.2f'%(dist,phi_point/np.pi*180,phi_seg/np.pi*180,(phi_point-phi_seg)/np.pi*180,np.clip(phi_point-phi_seg,-np.pi/2,np.pi/2)/np.pi*180)
        #cost = dist_ps*np.abs(np.sin(np.abs(phi_point-phi_seg)))
        cost_angle = np.abs(np.sin(np.clip(phi_point-phi_seg, -np.pi/2, np.pi/2)))
        cost = dist + self.weight_angle * cost_angle - self.weight_access * accesslevel

        # print '     cost_birgil=%.3f, dist=%.3f,  cost_angle=%.3f, accesslevel=%d'%(cost,dist,cost_angle,accesslevel)
        # print '     cost_birgil=%.3f, Xist=%.3f,  Xost_angle=%.3f, Xccesslevel=%.3f'%(cost,dist,self.weight_angle *cost_angle,self.weight_access*accesslevel)
        # print '  cost_birgil=',cost
        return cost

    def get_dist_point_edge(self, p, ind_edge, is_detect_final=False):
        seginds = self._id_edge_to_seginds[ind_edge]
        dists2 = get_dist_point_to_segs(p, self._x1[seginds], self._y1[seginds], self._x2[seginds],
                                        self._y2[seginds], is_ending=True, is_detect_final=is_detect_final)
        if is_detect_final:
            is_final = np.isnan(dists2)
            if np.all(is_final):  # point outside final segction of edge
                return np.nan
            else:
                return np.sqrt(np.min(dists2[~is_final]))

        return np.sqrt(np.min(dists2))

    def get_edge_info(self, id_trip, log=None):
        # print 'get_edge_info for trace %s with c_length=%.6f'%(id_trip,self.c_length)
        pointset = self.pointsets.get(id_trip)

        # create a multi point object with points of traces
        # should no longer be necessary
        #tracepoints = MultiPoint(pointset.get_coords().tolist())

        # make a array with time stamps of all points
        pointtimes = pointset.get_times()
        ids_points = pointset.get_ids()
        segind_to_id_edge = self._segind_to_id_edge
        #edgehit_counts = np.zeros(len(self._net.getEdges()),int)
        # segind_to_id_edge = self._segind_to_id_edge
        # for p in pointset.get_coords():
        #    print '  ',p
        #    hitinds = get_dist_point_to_segs(p,self._x1,self._y1,self._x2,self._y2, is_ending=True)< self.width_buffer**2
        #    edgehit_counts[segind_to_id_edge[hitinds]]+= 1

        # create a list with lists  for each point
        # Each list contans the edges, where the point is in the edge buffer
        edges_containing_point = np.zeros(len(ids_points), object)
        edgedists_to_point = np.zeros(len(ids_points), object)
        ind_point = 0
        for p in pointset.get_coords():
            dists2 = get_dist_point_to_segs(p, self._x1, self._y1, self._x2, self._y2, is_ending=True)
            inds_hit = np.nonzero(dists2 < self.width_buffer**2)
            edges_containing_point[ind_point] = self._edges[segind_to_id_edge[inds_hit]]
            edgedists_to_point[ind_point] = np.sqrt(dists2[inds_hit])

            # print '  ind_point,p,inds_hit',ind_point,p,len(np.nonzero(inds_hit)[0])#,edges_containing_point[ind_point]
            ind_point += 1

        # this is a dictionary with edge instance as key and
        # the weight as value.
        occurrences = {}
        for edge in self._edges:
            occurrences[edge] = 0.0

        id_mode = self.id_mode

        # 2. determine part of weight inversely proportional to the number
        # of GPS points contained in the buffer
        ind = 0
        time_start = +10**10
        time_end = -10**10

        ind_point_start = -1
        ind_point_end = -1
        intervals = {}
        for pointedges in edges_containing_point:
            n_edges = len(pointedges)
            if n_edges > 0:
                t_point = pointtimes[ind]
                if t_point < time_start:
                    time_start = t_point
                    ind_point_start = ind

                elif t_point > time_end:
                    time_end = t_point
                    ind_point_end = ind

                # compute partial weight for containing GPS points in edge buffer
                wp = 1.0/n_edges

                for edge in pointedges:
                    # assign weight component to edges
                    occurrences[edge] += wp

                    # determine intervals per edge
                    if intervals.has_key(edge):
                        t_edge_start, t_edge_end = intervals[edge]
                        if t_point < t_edge_start:
                            t_edge_start = t_point
                        elif t_point > t_edge_end:
                            t_edge_end = t_point
                        intervals[edge] = (t_edge_start, t_edge_end)

                    else:
                        intervals[edge] = (t_point, t_point)

            ind += 1

        edges_start = set(edges_containing_point[ind_point_start])
        edges_end = set(edges_containing_point[ind_point_end])

        # 1. calculate length proportional edge weight
        c_length = float(self.c_length)
        c_bike = float(self.c_bike)
        weights = {}

        for edge in self._edges:
            len_edge = edge.getLength()
            p_empty = c_length*len_edge

            l = len_edge
            allowed = edge.getLanes()[0].getAllowed()
            if len(allowed) > 0:
                if id_mode in allowed:
                    l *= c_bike  # weight for dedicated lanes

            if occurrences[edge] > 0.0:
                # compute edge length proportional weight
                weights[edge] = l/occurrences[edge]

            else:
                weights[edge] = l/p_empty

        # if self._logger:
        #    self._logger.w( '    New: duration in network %d s from %ds to %ds'%(time_end-time_start,time_start,time_end))

            #log.w( '    Check dimensions len(weights)=%d,len(intervals)=%d'%(len(weights), len(intervals)))
            # print '  intervals',intervals
            #log.w( '    Possible start edges')
            # for edge in edges_start:
            #    log.w( '      %s'%edge.getID())
            #log.w(  '    Possible end edges')
            # for edge in edges_end:
            #    log.w( '      %s'%edge.getID())
            #
            #log.w('get_edge_info done.')

        #self.intervals.set(id_trip, [ time_start,time_end])
        return weights, edges_start, edges_end, intervals, edges_containing_point, ind_point_start, ind_point_end


def get_colvalue(val, default=0.0):

    if (len(val) > 0) & (val != 'NULL'):
        return float(val)
    else:
        return default


class FilterMixin(Process):
    def _init_filter_preview(self):
        attrsman = self.get_attrsman()
        trips = self.parent.trips
        self.filterresults = attrsman.add(cm.FuncConf('filterresults',
                                                      'filterpreview',  # function attribute of object
                                                      '%d/%d' % (len(trips.get_ids_selected()), len(trips)),
                                                      name='Preview selected trips',
                                                      groupnames=['options', 'results'],
                                                      ))

    def filterpreview(self):
        """
        Previews selected trips after filtering.
        """
        trips = self.parent.trips
        n_trips = len(trips)
        n_sel_current = len(trips.get_ids_selected())
        n_sel_eliminate = len(self.filter_ids())
        n_sel_after = n_sel_current-n_sel_eliminate
        return '%d/%d (currently %d/%d)' % (n_sel_after, n_trips, n_sel_current, n_trips)

    def _init_filter_time(self, **kwargs):
        attrsman = self.get_attrsman()

        self.hour_from_morning = attrsman.add(cm.AttrConf('hour_from_morning', kwargs.get('hour_from_morning', 5),
                                                          groupnames=['options'],
                                                          perm='rw',
                                                          name='From morning hour',
                                                          unit='h',
                                                          info='Keep only morning trips which start after this hour.',
                                                          ))

        self.hour_to_morning = attrsman.add(cm.AttrConf('hour_to_morning', kwargs.get('hour_to_morning', 9),
                                                        groupnames=['options'],
                                                        perm='rw',
                                                        name='To morning hour',
                                                        unit='h',
                                                        info='Keep only morning trips which start before this hour.',
                                                        ))

        self.hour_from_evening = attrsman.add(cm.AttrConf('hour_from_evening', kwargs.get('hour_from_evening', 15),
                                                          groupnames=['options'],
                                                          perm='rw',
                                                          name='From evening hour',
                                                          unit='h',
                                                          info='Keep only evening trips which start after this hour.',
                                                          ))

        self.hour_to_evening = attrsman.add(cm.AttrConf('hour_to_evening', kwargs.get('hour_to_evening', 19),
                                                        groupnames=['options'],
                                                        perm='rw',
                                                        name='To evening hour',
                                                        unit='h',
                                                        info='Keep only evening trips which start before this hour.',
                                                        ))

        self.weekdays = attrsman.add(cm.AttrConf('weekdays', kwargs.get('weekdays', WEEKDAYCHOICES['all']),
                                                 groupnames=['options'],
                                                 name='Weekdays',
                                                 choices=WEEKDAYCHOICES,
                                                 info='Keep only trips at the given weekdays.',
                                                 ))

    def filter_time(self, timestamps):
        """
        timestamps is an array with timestamps.
        Function returns a binary array, with a True value for each
        time stamp that does NOT SATISFY the specified time constrants.  
        """
        # print 'filter_time'
        localtime = time.localtime
        inds_elim = np.zeros(len(timestamps), dtype=np.bool)
        i = 0
        for timestamp in timestamps:

            dt = localtime(timestamp)
            is_keep = dt.tm_wday in self.weekdays
            h = dt.tm_hour
            is_keep &= (h > self.hour_from_morning) & (h < self.hour_to_morning)\
                | (h > self.hour_from_evening) & (h < self.hour_to_evening)

            # print '  is_keep,w,h=',is_keep,dt.tm_wday,h
            inds_elim[i] = not is_keep
            i += 1

        return inds_elim

    def is_timestamp_ok(self, timestamp):
        dt = time.localtime(timestamp)
        is_ok = dt.tm_wday in self.weekdays
        h = dt.tm_hour
        is_ok &= (h > self.hour_from_morning) & (h < self.hour_to_morning)\
            | (h > self.hour_from_evening) & (h < self.hour_to_evening)

        return is_ok

    def filter_ids(self):
        """
        Returns an array of ids to be eliminated or deselected.
        To be overridden
        """
        return []


class PostMatchfilter(FilterMixin):
    def __init__(self,  mapmatching, logger=None, **kwargs):
        print 'PostMatchfilter.__init__'
        self._init_common('postmatchfilter',
                          parent=mapmatching,
                          name='Post matchfilter',
                          logger=logger,
                          info='Removes matched tripe with defined characteristics from current selection.',
                          )

        attrsman = self.set_attrsman(cm.Attrsman(self))

        info_lengthindex = "Length index is length of matched route divided by length of line interpolated GPS points in percent."
        self.lengthindex_min = attrsman.add(cm.AttrConf('lengthindex_min', kwargs.get('lengthindex_min', 80.0),
                                                        groupnames=['options'],
                                                        name='Min. length index',
                                                        unit='%',
                                                        info='Minimum allowed length index. '+info_lengthindex,
                                                        ))
        self.lengthindex_max = attrsman.add(cm.AttrConf('lengthindex_max', kwargs.get('lengthindex_max', 110.0),
                                                        groupnames=['options'],
                                                        name='Min. length index',
                                                        unit='%',
                                                        info='Maximum allowed length index '+info_lengthindex,
                                                        ))

        self._init_filter_time()
        self._init_filter_preview()

    def filter_ids(self):
        """
        Returns an array of ids to be eliminated or deselected.
        """
        # print 'filter_ids'
        trips = self.parent.trips
        ids_selected = trips.get_ids_selected()
        inds_eliminate = np.logical_or(
            trips.lengthindexes[ids_selected] < self.lengthindex_min,
            trips.lengthindexes[ids_selected] > self.lengthindex_max
        )
        # print '  inds_eliminate',inds_eliminate
        inds_eliminate = np.logical_and(inds_eliminate, self.filter_time(trips.timestamps[ids_selected]))
        # print '  inds_eliminate',inds_eliminate
        return ids_selected[inds_eliminate]

    def do(self):

        # execute filtering
        self.parent.trips.are_selected[self.filter_ids()] = False


class TripGeomfilter(FilterMixin):
    def __init__(self,  mapmatching, logger=None, **kwargs):
        print 'TripGeomfilter.__init__'
        self._init_common('tripGeomfilter',
                          parent=mapmatching,
                          name='Geometry trip filter',
                          logger=logger,
                          info='Removes trips with defined geometric characteristics from current selection.',
                          )

        attrsman = self.set_attrsman(cm.Attrsman(self))

        self.dist_point_max = attrsman.add(cm.AttrConf('dist_point_max', kwargs.get('dist_point_max', 1000.0),
                                                       groupnames=['options'],
                                                       perm='rw',
                                                       name='Max. point dist.',
                                                       unit='m',
                                                       info='Keep only traces where the distance between all successive points is below this maximal distance.',
                                                       ))

        self.duration_point_max = attrsman.add(cm.AttrConf('duration_point_max', kwargs.get('duration_point_max', 300.0),
                                                           groupnames=['options'],
                                                           perm='rw',
                                                           name='Max. point duration.',
                                                           unit='s',
                                                           info='Keep only traces where the duration between all successive points is below this maximal duration.',
                                                           ))

        self.const_return_max = attrsman.add(cm.AttrConf('const_return_max', kwargs.get('const_return_max', 0.3),
                                                         groupnames=['options'],
                                                         perm='rw',
                                                         name='Max. return const',
                                                         info='Keep only traces where the user is not returning more than a share of this constant from the maximum (line of site) distance from the origin.',
                                                         ))

        self._init_filter_preview()

    def do(self):

        # execute filtering
        self.parent.trips.are_selected[self.filter_ids()] = False

    def filter_ids(self):
        """
        Returns an array of ids to be eliminated or deselected.
        """
        c_cutoff = 1.0 - self.const_return_max
        print 'TripGeomfilter.do c_cutoff', c_cutoff
        dist_point_max = self.dist_point_max
        duration_point_max = self.duration_point_max
        trips = self.parent.trips
        points = self.parent.points

        ids_trips = trips.get_ids_selected()

        ids_points = trips.ids_points[ids_trips]
        # print '  ids_trips',ids_trips
        # print '  ids_points',ids_points

        intersects_boundaries = self.parent.get_scenario().net.intersects_boundaries

        inds_elim = np.zeros(len(ids_trips), dtype=np.bool)
        j = 0
        for id_trip, ids_point in zip(ids_trips, ids_points):

            coords = points.coords[ids_point]
            times = points.timestamps[ids_point]
            # print 79*'-'
            # print '  check split ',id_trip
            # print '  ids_point', ids_point
            # print '  times',times
            # print '  coords', coords
            # print '  duration_point_max',duration_point_max
            #dist = pointset.get_distance()

            if ids_point is None:
                # this happens if the points of a trip in the workout file
                # have not been imported for some reason
                is_eliminate = True

            elif not intersects_boundaries(get_boundary(coords)):
                is_eliminate = True

            elif np.any(times < 0):
                is_eliminate = True
            else:
                dist_max = 0.0
                is_eliminate = False
                i = 0
                n = len(times)
                x, y, z = coords[0]
                while (not is_eliminate) & (i < (n-1)):
                    i += 1
                    dist_point = np.sqrt((coords[i, 0]-coords[i-1, 0])**2
                                         + (coords[i, 1]-coords[i-1, 1])**2)

                    if dist_point > dist_point_max:
                        is_eliminate = True

                    # print '   times[i]-times[i-1]',times[i]-times[i-1]

                    if (times[i]-times[i-1]) > duration_point_max:
                        is_eliminate = True

                    d = np.sqrt((x-coords[i, 0])**2 + (y-coords[i, 1])**2)
                    if d > dist_max:
                        dist_max = d

                    elif d < c_cutoff*dist_max:
                        is_eliminate = True

            inds_elim[j] = is_eliminate
            j += 1

        return ids_trips[inds_elim]


class TripTimefilter(Process):
    def __init__(self,  mapmatching, logger=None, **kwargs):
        print 'Timefilter.__init__'
        self._init_common('triptimefilter',
                          parent=mapmatching,
                          name='Trip time  filter',
                          logger=logger,
                          info='Keeps only trips with specified time charactaristict in selection.',
                          )

    def do(self):
        c_cutoff = 1.0 - self.const_return_max
        print 'TripGeomfilter.do c_cutoff', c_cutoff
        dist_point_max = self.dist_point_max
        duration_point_max = self.duration_point_max
        trips = self.parent.trips
        #points = self.parent.points
        ids_trips = trips.get_ids_selected()

        #ids_points = trips.ids_points[ids_trips]
        # print '  ids_trips',ids_trips
        # print '  ids_points',ids_points

        for id_trip, ids_point in zip(ids_trips, ids_points):

            coords = points.coords[ids_point]
            times = points.timestamps[ids_point]
            print 79*'-'
            print '  check split ', id_trip
            print '  ids_point', ids_point
            print '  times', times
            print '  duration_point_max', duration_point_max
            #dist = pointset.get_distance()

            if np.any(times < 0):
                is_eliminate = True
            else:
                dist_max = 0.0
                is_eliminate = False
                i = 0
                n = len(times)
                x, y, z = coords[0]
                while (not is_eliminate) & (i < (n-1)):
                    i += 1
                    dist_point = np.sqrt((coords[i, 0]-coords[i-1, 0])**2
                                         + (coords[i, 1]-coords[i-1, 1])**2)

                    if dist_point > dist_point_max:
                        is_eliminate = True

                    print '   times[i]-times[i-1]', times[i]-times[i-1]

                    if (times[i]-times[i-1]) > duration_point_max:
                        is_eliminate = True

                    d = np.sqrt((x-coords[i, 0])**2 + (y-coords[i, 1])**2)
                    if d > dist_max:
                        dist_max = d

                    elif d < c_cutoff*dist_max:
                        is_eliminate = True

            trips.are_selected[id_trip] = not is_eliminate


class EccTracesImporter(FilterMixin):
    def __init__(self,  mapmatching, logger=None, **kwargs):
        print 'EccTracesImporter.__init__', mapmatching.get_ident()
        self._init_common('traceimporter',
                          parent=mapmatching,
                          name='ECC Trace Importer',
                          logger=logger,
                          info='Import workouts and GPS points of a European cycling challange.',
                          )

        attrsman = self.set_attrsman(cm.Attrsman(self))

        scenario = mapmatching.get_scenario()
        rootfilepath = scenario.get_rootfilepath()

        # here we ged classes not vehicle type
        # specific vehicle type within a class will be generated later
        modechoices = scenario.net.modes.names.get_indexmap()

        # print '  modechoices',modechoices
        self.id_mode = attrsman.add(am.AttrConf('id_mode',  modechoices['bicycle'],
                                                groupnames=['options'],
                                                choices=modechoices,
                                                name='Mode',
                                                info='Transport mode to be matched.',
                                                ))

        self.workoutsfilepath = attrsman.add(
            cm.AttrConf('workoutsfilepath', kwargs.get('workoutsfilepath', rootfilepath+'.workouts.csv'),
                        groupnames=['options'],
                        perm='rw',
                        name='Workout file',
                        wildcards='CSV file (*.csv)|*.csv',
                        metatype='filepath',
                        info="""CSV text file with workout database.""",
                        ))

        self.pointsfilepath = attrsman.add(cm.AttrConf('pointsfilepath', kwargs.get('pointsfilepath', rootfilepath+'.points.csv'),
                                                       groupnames=['options'],
                                                       perm='rw',
                                                       name='Points file',
                                                       wildcards='CSV file (*.csv)|*.csv',
                                                       metatype='filepath',
                                                       info="CSV text file with GPS point database.",
                                                       ))

        self.year = attrsman.add(cm.AttrConf('year', kwargs.get('year', 2014),
                                             groupnames=['options'],
                                             choices={'2014': 2014, 'from 2015': 2015},
                                             perm='rw',
                                             name='Year of challange',
                                             info='Year of challange is used to identify the correct database format.',
                                             ))

        self.dist_trip_min = attrsman.add(cm.AttrConf('dist_trip_min', kwargs.get('dist_trip_min', 100.0),
                                                      groupnames=['options'],
                                                      perm='rw',
                                                      name='Min. trip distance',
                                                      unit='m',
                                                      info='Minimum distance of one trip. Shorter trips will not be imported.',
                                                      ))

        self.dist_trip_max = attrsman.add(cm.AttrConf('dist_trip_max', kwargs.get('dist_trip_max', 50000.0),
                                                      groupnames=['options'],
                                                      perm='rw',
                                                      name='Max. trip distance',
                                                      unit='m',
                                                      info='Maximum distance of one trip. Shorter trips will not be imported.',
                                                      ))

        self.duration_trip_min = attrsman.add(cm.AttrConf('duration_trip_min', kwargs.get('duration_trip_min', 30.0),
                                                          groupnames=['options'],
                                                          perm='rw',
                                                          name='Min. trip duration',
                                                          unit='s',
                                                          info='Minimum duration of one trip. Trips with shorter duration will not be imported.',
                                                          ))

        self.speed_trip_min = attrsman.add(cm.AttrConf('speed_trip_min', kwargs.get('speed_trip_min', 3.0),
                                                       groupnames=['options'],
                                                       perm='rw',
                                                       name='Min. av. trip speed',
                                                       unit='m/s',
                                                       info='Minimum average trip speed. Trips with lower average speed will not be imported.',
                                                       ))

        self.speed_trip_max = attrsman.add(cm.AttrConf('speed_trip_max', kwargs.get('speed_trip_max', 50.0),
                                                       groupnames=['options'],
                                                       perm='rw',
                                                       name='Max. av. trip speed',
                                                       unit='m/s',
                                                       info='Maximum average trip speed. Trips with higher average speed will not be imported.',
                                                       ))

        self.sep_column_workout = attrsman.add(cm.AttrConf('sep_column_workout', kwargs.get('sep_column_workout', ','),
                                                           groupnames=['options'],
                                                           perm='rw',
                                                           name='Workoutdata seperator',
                                                           info='Workout column seperator of CSV file',
                                                           ))

        self.sep_column_points = attrsman.add(cm.AttrConf('sep_column_points', kwargs.get('sep_column_points', ','),
                                                          groupnames=['options'],
                                                          perm='rw',
                                                          name='Point data seperator',
                                                          info='Pointdata column seperator of CSV file',
                                                          ))

        self._init_filter_time(**kwargs)

    def do(self):
        print 'TraceImporter.do'
        if self.year == 2014:
            self.import_workouts_2014()
            self.import_points_2014()

        self.parent.points.project()

    def import_workouts_2014(self):
        print 'import_workouts_2014'
        # 2014 ecomondo workouts
        # id	        pointDBNode	pointPathId	startTime	        distance	duration	sport	calories	maxSpeed	altitudeMin	altitudeMax	metersAscent	metersDescent
        # 329308466	7	        37073516	2014-05-01 19:00:00	    26	    15600	    1	    1182.64	    NULL	    NULL	    NULL	    NULL	        NULL
        # 0         1           2            3                       4          5       6          7        8           9          10          11                   12

        j_id, j_node, j_id_trip, j_time, j_dist, j_duration = range(6)
        n_cols = 13
        null = 'NULL'
        trips = self.parent.trips
        # persons = self.parent.persons # no person data in 2014 :(
        ids_person_sumo = {}

        ids_trips = []
        scenario = self.parent.get_scenario()

        get_vtype_for_mode = scenario.demand.vtypes.get_vtype_for_mode

        f = open(self.workoutsfilepath, 'r')
        #if self._logger: self._logger.w('import_workouts_2014 %s'%os.path.basename(self.workoutsfilepath))
        i_line = 0
        sep = self.sep_column_workout

        #self.get_logger().w(100.0*self.simtime/self.duration, key ='progress')
        for line in f.readlines()[1:]:
            # if True:#i_line>1:
            cols = line.split(sep)
            # print '    len(cols)',len(cols),n_cols
            if len(cols) == n_cols:
                # row is complete

                # if cols[j_dist] != null:
                dist = get_colvalue(cols[j_dist])*1000.0
                duration = get_colvalue(cols[j_duration])
                if duration > 0:
                    speed_av = dist/duration
                else:
                    speed_av = 0.0

                if (dist > self.dist_trip_min)\
                   & (dist < self.dist_trip_max)\
                   & (duration > self.duration_trip_min)\
                   & (speed_av > self.speed_trip_min)\
                   & (speed_av < self.speed_trip_max):
                    # print  'parametric conditions verified'
                    timestamp = calc_seconds(cols[j_time])
                    if timestamp is not None:
                        # print '  valid time stamp',timestamp
                        if self.is_timestamp_ok(timestamp):
                            id_trip = trips.make(id_sumo=cols[j_id_trip],
                                                 id_vtype=get_vtype_for_mode(self.id_mode),
                                                 timestamp=timestamp,
                                                 distance_gps=dist,
                                                 duration_gps=duration,
                                                 speed_average=speed_av,
                                                 )
                            ids_trips.append(id_trip)

    def import_points_2014(self):
        print 'import_points_2014'
        # csv2014
        # pointDBNode,pointPathId,id,timestamp,latitude,longitude,altitude,distance,heartRate,instruction,speed
        # 4,61565791,23648171762,2013-05-01 06:33:58,44.501085,11.372906,NULL,0,NULL,2,NULL

        # pointDBNode,pointPathId,id,timestamp,latitude,longitude,altitude,distance,heartRate,instruction,speed
        # 7,37073516,15138524460,NULL,44.51579,11.36257,NULL,NULL,NULL,NULL,NULL

        # pointDBNode,pointPathId,id,timestamp,latitude,longitude,altitude,distance,heartRate,instruction,speed
        # 7,37073516,15138524460,NULL,44.51579,11.36257,NULL,NULL,NULL,NULL,NULL

        # Endomondo export format
        # pointDBNode,pointPathId,id,timestamp,latitude,longitude,altitude,distance,heartRate,instruction,speed
        #    0          1         2   3         4          5         6       7       8          9          10
        ind_id_path = 1
        ind_id_point = 2
        ind_time = 3
        ind_lat = 4
        ind_lon = 5
        ind_alt = 6
        ind_dist = 7
        ind_speed = 10

        n_cols = 11
        #TripID, TimeStamp,Date, Latitude, Longitude, Altitude, Distance, Speed, Type

        trips = self.parent.trips
        points = self.parent.points

        exist_id_trip_sumo = trips.ids_sumo.has_index
        get_id_trip = trips.ids_sumo.get_id_from_index

        sep = self.sep_column_points

        f = open(self.pointsfilepath, 'r')
        if self._logger:
            self._logger.w('import_points_2014 %s' % os.path.basename(self.pointsfilepath))
        i_line = 0
        id_trip_sumo = None
        id_trip = -1
        is_valid_trip = False

        n_points_imported = 0
        for line in f.readlines():

            cols = line.split(sep)
            # print '    len(cols)',len(cols),n_cols
            if len(cols) == n_cols:
                id_trip_sumo_current = cols[ind_id_path]
                # print '    id_trip_sumo_current,id_trip_sumo',id_trip_sumo_current,id_trip_sumo,is_valid_trip
                if id_trip_sumo_current != id_trip_sumo:
                    # this point is part of new trip

                    if is_valid_trip:
                        # print '  store past points for valid trip',id_trip
                        ids_point = points.add_rows(
                            timestamps=timestamps,
                            ids_trip=id_trip*np.ones(len(timestamps), dtype=np.int32),
                            longitudes=longitudes,
                            latitudes=latitudes,
                            altitudes=altitudes,
                        )

                        trips.set_points(id_trip, ids_point)

                    # check if new trip is valid
                    if exist_id_trip_sumo(id_trip_sumo_current):

                        is_valid_trip = True  # start recording
                        id_trip = get_id_trip(id_trip_sumo_current)
                        # print '    found trip',id_trip,id_trip_sumo_current,' exisits-> record'

                        # ids_point_sumo = [] # useless?
                        timestamps = []
                        ids_trip = []
                        longitudes = []
                        latitudes = []
                        altitudes = []

                    else:
                        # print '    trip',id_trip_sumo_current,'does not exisit'
                        is_valid_trip = False

                    id_trip_sumo = id_trip_sumo_current

                if is_valid_trip:
                    # print '    store point timestamp',cols[ind_time]
                    # current point belongs to a valid trip
                    # ids_point_sumo.append(cols[ind_id_point])
                    timestamps.append(calc_seconds(cols[ind_time]))
                    # ids_trip.append(id_trip)
                    longitudes.append(get_colvalue(cols[ind_lon]))
                    latitudes.append(get_colvalue(cols[ind_lat]))
                    altitudes.append(get_colvalue(cols[ind_alt]))

            else:
                print 'WARNING: inconsistent columns in line %d, file %s' % (i_line, filepath)

            i_line += 1

        # register points of last trip
        if is_valid_trip:
            # store past points for valid trip
            ids_point = points.add_rows(
                timestamps=timestamps,
                ids_trip=id_trip*np.ones(len(timestamps), dtype=np.int32),
                longitudes=longitudes,
                latitudes=latitudes,
                altitudes=altitudes,
            )

            trips.set_points(id_trip, ids_point)

        # self.odtab.print_rows()

        f.close()


class GpsTrips(Trips):
    def __init__(self, ident, mapmatching,  **kwargs):
        # print 'Trips.__init__'
        self._init_objman(ident=ident,
                          parent=mapmatching,
                          name='Trips',
                          info='Table with GPS trips, matched routes and alternative routes.',
                          xmltag=('trips', 'trip', 'ids_sumo'),
                          version=0.0,
                          **kwargs)

        self._init_attributes()
        self._init_constants()

    def _init_attributes(self):

        self.add_col(SumoIdsConf('GPS Trip', xmltag='id', info='GPS trip data.'))

        self.add_col(am.ArrayConf('are_selected', default=True,
                                  dtype=np.bool,
                                  groupnames=['parameters', ],
                                  name='selected',
                                  symbol='Sel.',
                                  info='Selected for being processed (example mapmatching, export, etc).',
                                  ))

        self.add_col(am.ArrayConf('timestamps', default=0,
                                  dtype=np.int,
                                  perm='r',
                                  groupnames=['parameters', 'gps'],
                                  name='timestamp',
                                  unit='s',
                                  metatype='datetime',
                                  info='Timestamp when trip started in seconds after 01 January 1970.',
                                  ))

        self.timestamps.metatype = 'datetime'
        self.timestamps.set_perm('r')

        self.add_col(am.ArrayConf('durations_gps', default=0.0,
                                  dtype=np.float32,
                                  groupnames=['parameters', 'gps'],
                                  name='GPS duration',
                                  unit='s',
                                  info='Time duration measure with GPS points.',
                                  ))

        self.add_col(am.ArrayConf('distances_gps', default=0.0,
                                  dtype=np.float32,
                                  groupnames=['parameters', 'gps'],
                                  name='GPS distance',
                                  unit='m',
                                  info='Distance measure with GPS points.',
                                  ))

        self.add_col(am.ArrayConf('speeds_average', default=0.0,
                                  dtype=np.float32,
                                  groupnames=['parameters', 'gps'],
                                  name='Av. speed',
                                  unit='m/s',
                                  info='Average speed based on GPS info.',
                                  ))
        self.add_col(am.ArrayConf('speeds_max', default=0.0,
                                  dtype=np.float32,
                                  groupnames=['parameters', 'gps'],
                                  name='Max. speed',
                                  unit='m/s',
                                  info='Maximum speed based on GPS info.',
                                  ))

        # Trips._init_attributes(self)
        self.add_col(am.IdsArrayConf('ids_vtype', self.get_obj_vtypes(),
                                     groupnames=['state'],
                                     name='Type',
                                     info='Vehicle type.',
                                     xmltag='type',
                                     ))

        self.add_col(am.ArrayConf('ids_purpose', default=TRIPPUROPSES['unknown'],
                                  dtype=np.int32,
                                  groupnames=['parameters', 'gps'],
                                  choices=TRIPPUROPSES,
                                  name='Purpose',
                                  info='Trip purpose ID',
                                  ))

        self.add_col(am.ArrayConf('ids_device', default=DEVICES['unknown'],
                                  dtype=np.int32,
                                  groupnames=['parameters', 'gps'],
                                  choices=DEVICES,
                                  name='Devices',
                                  info='Device ID',
                                  ))
        self.add(cm.ObjConf(Routes('routes', self, self.parent.get_scenario().net)))

        self.add_col(am.IdsArrayConf('ids_route_matched', self.get_routes(),
                                     groupnames=['results'],
                                     name='ID matched route',
                                     info='Route ID of mached route.',
                                     ))

        self.add_col(am.IdsArrayConf('ids_route_shortest', self.get_routes(),
                                     groupnames=['results'],
                                     name='ID shortest route',
                                     info='Route ID of shortest route.',
                                     ))

        self.add_col(am.ArrayConf('lengths_route_matched', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Matched length',
                                  symbol='L_{M}',
                                  unit='m',
                                  info='Length of the matched part of the GPS trace. Note the only a fraction of the GPS trace ma be within the given network.',
                                  ))

        self.add_col(am.ArrayConf('lengths_route_matched_mixed', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Matched length mixed access',
                                  symbol='L_{MM}',
                                  unit='m',
                                  info='Length of the matched part of the GPS trace. Note the only a fraction of the GPS trace ma be within the given network.',
                                  ))

        self.add_col(am.ArrayConf('lengths_route_matched_exclusive', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Matched length exclusive access',
                                  symbol='L_{ME}',
                                  unit='m',
                                  info='Length of the matched part of the GPS trace. Note the only a fraction of the GPS trace ma be within the given network.',
                                  ))

        self.add_col(am.ArrayConf('durations_route_matched', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Matched duration',
                                  unit='s',
                                  info='Duration of the matched part of the GPS trace. Note the only a fraction of the GPS trace ma be within the given network.',
                                  ))

        self.add_col(am.ArrayConf('lengthindexes', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Length index',
                                  unit='%',
                                  info='Length index is the length of the matched route divided by length of line-interpolated GPS points.',
                                  ))

        self.add_col(am.ArrayConf('errors_dist', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Distance error',
                                  unit='mm',
                                  info='The distance error is the average distance between the GPS points and the matched route.',
                                  ))

        self.add_col(am.ArrayConf('times_computation', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Computation time',
                                  unit='ms',
                                  info='Computation time of the match algorithm.',
                                  ))

        self.add_col(am.IdlistsArrayConf('ids_points', self.parent.points,
                                         #groupnames = ['_private'],
                                         name='Point IDs',
                                         info="GPS point IDs.",
                                         ))

    def get_obj_vtypes(self):
        return self.parent.get_scenario().demand.vtypes

    def get_routes(self):
        return self.routes.get_value()

    def clear_routes(self):
        self.get_routes().clear()
        for attrconf in self.get_group('results'):
            attrconf.reset()

    def select_all(self):
        self.are_selected.get_value()[:] = True

    def unselect_all(self):
        self.are_selected.get_value()[:] = False

    def set_matched_route(self, id_trip, route,
                          length_matched=0.0,
                          length_route_mixed=0.0,
                          length_route_exclusive=0.0,
                          duration_matched=0.0,
                          lengthindex=-1.0,
                          error_dist=-1.0,
                          comptime=0.0,
                          ):

        if len(route) > 0:
            id_route = self.ids_route_matched[id_trip]
            if id_route >= 0:
                # already a matched route existant
                self.get_routes().ids_edges[id_route] = route
                # self.get_routes().set_row(  id_route,
                #                            ids_trip = id_trip,
                #                            ids_edges = route,
                #                            #costs = duration_matched,
                #                            #probabilities = 1.0,
                #                        )
            else:
                id_route = self.get_routes().add_row(ids_trip=id_trip,
                                                     ids_edges=route,
                                                     #costs = duration_matched,
                                                     #probabilities = 1.0,
                                                     )
        else:
            id_route = -1

        # print 'set_matched_route id_trip', id_trip,'id_route', id_route
        #self.ids_route_matched[id_trip] = id_route

        self.set_row(id_trip,
                     ids_route_matched=id_route,
                     lengths_route_matched=length_matched,
                     lengths_route_matched_mixed=length_route_mixed,
                     lengths_route_matched_exclusive=length_route_exclusive,
                     durations_route_matched=duration_matched,
                     lengthindexes=100*lengthindex,
                     errors_dist=1000 * error_dist,
                     times_computation=1000*comptime,
                     )

        return id_route

    def make(self,  **kwargs):
        # if self.ids_sumo.has_index(id_sumo):
        #    id_trip = self.ids_sumo.get_id_from_index(id_sumo)
        #    #self.set_row(id_sumo, **kwargs)
        #    return id_trip
        # else:
        id_trip = self.add_row(ids_sumo=kwargs.get('id_sumo', None),
                               ids_vtype=kwargs.get('id_vtype', None),
                               timestamps=kwargs.get('timestamp', None),
                               distances_gps=kwargs.get('distance_gps', None),
                               durations_gps=kwargs.get('duration_gps', None),
                               speeds_average=kwargs.get('speed_average', None),
                               speeds_max=kwargs.get('speed_max', None),
                               ids_purpose=kwargs.get('id_purpose', None),
                               ids_device=kwargs.get('id_device', None),
                               ids_points=kwargs.get('ids_point', None),
                               )
        return id_trip

    def set_points(self, id_trip, ids_point):
        self.ids_points[id_trip] = ids_point

    def get_ids_selected(self):
        return self.select_ids(self.are_selected.get_value())

    def get_ids_route_selected(self):
        # TODO: here we could append direct routes
        print 'get_ids_route_selected'
        ids_route_selected = self.ids_route_matched[self.get_ids_selected()]

        # print '  ids_route_selected',ids_route_selected
        #ids_edges = self.get_routes().ids_edges[ids_route_selected[ids_route_selected >= 0]]
        # print '  ids_edges',type(ids_edges),ids_edges
        #inds_nonempty =  ids_edges == []
        # print '  inds_nonempty',inds_nonempty
        #ids_route = ids_route_selected[inds_nonempty]
        #
        # print '  ids_route',ids_route
        # if hasattr(ids_route, '__iter__'):
        #    return ids_route
        # else:
        #    return np.array([ids_route], dtype = np.int32)

        return ids_route_selected[ids_route_selected >= 0]


class GpsPoints(am.ArrayObjman):
    """
    Contains data of points of a single trace.
    """

    def __init__(self, ident, mapmatching, **kwargs):
        # print 'PrtVehicles vtype id_default',vtypes.ids_sumo.get_id_from_index('passenger1')
        self._init_objman(ident=ident,
                          parent=mapmatching,
                          name='GPS Points',
                          info='GPS points database.',
                          version=0.0,
                          **kwargs)

        self._init_attributes()
        self._init_constants()

    def _init_attributes(self):
        scenario = self.get_scenario()

        # ident is the path id of the trace

        # the actual numpy arrays are stored in .cols
        self.add_col(am.ArrayConf('longitudes',     default=0.0,
                                  dtype=np.float32,
                                  groupnames=['parameters', ],
                                  perm='rw',
                                  name='Longitude',
                                  symbol='Lon',
                                  unit='deg',
                                  info='Longitude  of point',
                                  ))

        self.add_col(am.ArrayConf('latitudes',   default=0.0,
                                  groupnames=['parameters', ],
                                  dtype=np.float32,
                                  perm='rw',
                                  name='Latitude',
                                  symbol='Lat',
                                  unit='deg',
                                  info='Latitude of point',
                                  ))

        self.add_col(am.ArrayConf('altitudes',   default=-100000.0,
                                  groupnames=['parameters', ],
                                  dtype=np.float32,
                                  perm='rw',
                                  name='Altitude',
                                  symbol='Alt',
                                  unit='m',
                                  info='Altitude of point',
                                  ))

        self.add_col(am.ArrayConf('radii',  10.0,
                                  dtype=np.float32,
                                  groupnames=['parameters', ],
                                  perm='rw',
                                  name='Radius',
                                  unit='m',
                                  info='Point radius, representing the imprecision of the point, which depends on the recording device ane the environment.',
                                  ))

        self.add_col(am.ArrayConf('coords',    default=[0.0, 0.0, 0.0],
                                  groupnames=['parameters', ],
                                  dtype=np.float32,
                                  perm='rw',
                                  name='Coordinate',
                                  symbol='x,y,z',
                                  unit='m',
                                  info='Local 3D coordinate  of point',
                                  ))

        self.add_col(am.ArrayConf('timestamps',  default=0.0,
                                  dtype=np.float,
                                  groupnames=['parameters', ],
                                  perm='r',
                                  name='timestamp',
                                  symbol='t',
                                  metatype='datetime',
                                  unit='s',
                                  digits_fraction=2,
                                  info='Time stamp of point in seconds after 01 January 1970.',
                                  ))
        # test:
        self.timestamps.metatype = 'datetime'
        self.timestamps.set_perm('r')

    def set_trips(self, trips):
        self.add_col(am.IdsArrayConf('ids_trip', trips,
                                     groupnames=['state'],
                                     name='Trip ID',
                                     info='ID of trips to which this point belongs to.',
                                     ))
        # put in geometry filter
        # self.add_col(am.ArrayConf( 'are_inside_boundary',  default =False,
        #                                groupnames = ['parameters',],
        #                                perm='r',
        #                                name = 'in boundary',
        #                                info = 'True if this the data point is within the boundaries of the road network.',
        #                                ))

    def get_ids_selected(self):
        """
        Returns point ids of selected traces
        """
        # print 'GpsPoints.get_ids_selected'
        # print '  ??ids_points = ',self.select_ids(self.parent.trips.are_selected[self.ids_trip.get_value()] )
        # TODO: why is this working??? do we need trips.ids_points????
        return self.select_ids(self.parent.trips.are_selected[self.ids_trip.get_value()])
        # return self.get_ids()#self.select_ids(self.get_ids()
        # return self.select_ids(

    def get_scenario(self):
        return self.parent.get_scenario()

    def get_coords(self):
        """
        Returns an array of x,y coordinates of all points.
        """
        return self.coords.get_value()

    def get_times(self):
        """
        Returns an array of time stamps of all points.
        """
        return self.timestamp.get_value()

    def get_interval(self):
        if len(self) == 0:
            return [0.0, 0.0]
        else:
            timestamps = self.timestamps.get_value()
            return [timestamps.min(), timestamps.max()]

    def get_duration(self):
        ts, te = self.get_interval()
        return te-ts

    def get_distance(self):
        v = self.get_coords()
        # return np.linalg.norm( self.cols.coords[0:-1] - self.cols.coords[1:] )
        return np.sum(np.sqrt((v[1:, 0]-v[0:-1, 0])**2 + (v[1:, 1]-v[0:-1, 1])**2))

    def get_boundary(self):
        if len(self) == 0:
            return [0, 0, 0, 0]
        else:
            x_min, y_min = self.get_coords().min(0)
            x_max, y_max = self.get_coords().max(0)

            return [x_min, y_min, x_max, y_max]

    def project(self, proj=None, offset=None):
        if proj is None:
            proj, offset = self.parent.get_proj_and_offset()
        x, y = proj(self.longitudes.get_value(), self.latitudes.get_value())
        self.get_coords()[:] = np.transpose(np.concatenate(
            ([x+offset[0]], [y+offset[1]], [self.altitudes.get_value()]), axis=0))


class GpsPersons(am.ArrayObjman):

    def __init__(self, ident, mapmatching, **kwargs):
        # print 'PrtVehicles vtype id_default',vtypes.ids_sumo.get_id_from_index('passenger1')
        self._init_objman(ident=ident,
                          parent=mapmatching,
                          name='GPS Persons',
                          info='GPS person database.',
                          **kwargs)

        self._init_attributes()

    def _init_attributes(self):
        trips = self.parent.trips

        # TODO: add/update vtypes here
        self.add_col(SumoIdsConf('User', xmltag='id'))

        self.add_col(am.IdlistsArrayConf('ids_trips', trips,
                                         #groupnames = ['_private'],
                                         name='Trip IDs',
                                         info="IDs of trips made by this vehicle. This is a collection of recorded trips associated with this person.",
                                         ))

        self.add_col(am.ArrayConf('ids_genders', default=0,
                                  dtype=np.int,
                                  groupnames=['parameters'],
                                  name='gender',
                                  info='Gender of person.',
                                  ))

        self.add_col(am.ArrayConf('years_birth', default=-1,
                                  dtype=np.int,
                                  groupnames=['parameters'],
                                  name='birth year',
                                  info='Year when person has been born.',
                                  ))

        self.add_col(am.ArrayConf('ids_occupation', default=OCCUPATIONS['unknown'],
                                  dtype=np.int32,
                                  choices=OCCUPATIONS,
                                  groupnames=['parameters'],
                                  name='occupation',
                                  info='Tupe of occupation',
                                  ))

        self.add_col(am.ArrayConf('are_frequent_user', False,
                                  dtype=np.int32,
                                  groupnames=['parameters'],
                                  name='frequent user',
                                  info='If true, this person is a frequent user of the recorded transport mode.',
                                  ))

        self.add_col(am.ArrayConf('zips', -1,
                                  dtype=np.int32,
                                  groupnames=['parameters'],
                                  name='ZIP',
                                  info='ZIP code of persons home.',
                                  ))

    def make(self, id_sumo, **kwargs):
        if self.ids_sumo.has_index(id_sumo):
            id_pers = self.ids_sumo.get_id_from_index(id_sumo)
            #self.set_row(id_pers, **kwargs)
            return id_pers
        else:
            id_pers = self.add_row(ids_sumo=id_sumo,
                                   ids_trips=kwargs.get('ids_trip', None),
                                   ids_genders=kwargs.get('id_gender', None),
                                   years_birth=kwargs.get('year_birth', None),
                                   ids_occupation=kwargs.get('id_occupation', None),
                                   are_frequent_user=kwargs.get('is_frequent_user', None),
                                   zips=kwargs.get('zip', None),
                                   )

            return id_pers


class Mapmatching(cm.BaseObjman):
    def __init__(self, ident, demand=None,
                 name='Mapmatching', info='Mapmatching functionality.',
                 **kwargs):

        self._init_objman(ident=ident, parent=demand,
                          name=name, info=info, **kwargs)

        attrsman = self.set_attrsman(cm.Attrsman(self))

        self._init_attributes()
        self._init_constants()

    def get_scenario(self):
        return self.parent.parent

    def clear_all(self):
        self.trips.clear()
        self.points.clear()
        self.persons.clear()

    def clear_routes(self):
        self.trips.clear_routes()

    def _init_attributes(self):
        print 'Mapmatching._init_attributes'
        attrsman = self.get_attrsman()

        self.points = attrsman.add(cm.ObjConf(GpsPoints('points', self)))
        self.trips = attrsman.add(cm.ObjConf(GpsTrips('trips', self)))
        self.points.set_trips(self.trips)
        self.persons = attrsman.add(cm.ObjConf(GpsPersons('persons', self)))

    def _init_constants(self):
        self._proj = None
        self._segvertices_xy = None
        attrsman = self.get_attrsman()
        attrsman.do_not_save_attrs(['_segvertices_xy', '_proj'])

    def get_proj_and_offset(self):
        if self._proj is None:
            net = self.get_scenario().net
            proj_params = str(net.get_projparams())
            # try:
            self._proj = pyproj.Proj(proj_params)
            # except:
            #    proj_params ="+proj=utm +zone=32 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"
            #    self._proj = pyproj.Proj(self.proj_params)

            self._offset = net.get_offset()

        return self._proj, self._offset

    def get_segvertices_xy(self):
        if self._segvertices_xy is None:
            self._segvertices_xy = self.get_scenario().net.edges.get_segvertices_xy()

        return self._segvertices_xy


class GpxParser(handler.ContentHandler):
    """Reads endomondo gpx xml file and parses lat,lon, ele and time.
    """

    def __init__(self, scenario, traces, log=None):

        self._traces = traces

        self._pointset = None
        self._id_point = None

        self._lon = None
        self._lat = None
        self._ele = None
        self._time = None
        self._time_flag = False
        self._ele_flag = False
        self._ids_traces = []

    def get_ids_traces(self):
        return self._ids_traces

    def get_traces(self):
        return self._traces

    def startElement(self, name, attrs):
        if self._pointset is None:
            if name == 'trkseg':
                # start new trace with new pointse
                if len(self._traces) == 0:
                    id_trip = '0'
                else:
                    id_trip = str(np.max(self._traces.get_ids())+1)
                print 'startElement id_trip', id_trip, self._traces.get_ids(), self._traces
                self._pointset = PointSet(id_trip, self._traces)
                self._traces.set_row(id_trip, pointsets=self._pointset)
                self._ids_traces.append(id_trip)

        if self._id_point is None:
            if name == 'trkpt':

                self._id_point = self._pointset.suggest_id()
                # print 'startElement trkpt',self._id_point
                self._lon = float(attrs['lon'])
                self._lat = float(attrs['lat'])

        if name == 'time':
            self._time_flag = True

        if name == 'ele':
            self._ele_flag = True

    def characters(self, content):
        if self._time_flag:
            # print '  got time',content[:-1]

            # 2013-05-30T16:42:33Z
            self._time = calc_seconds(content[:-1],
                                      sep_date_clock='T', sep_date='-', sep_clock=':',
                                      is_float=False)

        if self._ele_flag:
            self._ele = float(content)

    def endElement(self, name):
        if name == 'ele':
            self._ele_flag = False

        if name == 'time':
            self._time_flag = False

        if name == 'trkpt':
            # print 'endElement:add point to pointset',self._id_point,self._lon,self._lat,self._ele,self._time

            if (self._time is not None)\
                & (self._id_point is not None)\
                & (self._lon is not None) & (self._lat is not None)\
                    & (self._pointset is not None):
                if (self._ele is not None):
                    self._pointset.set_row(self._id_point,
                                           lon=self._lon,
                                           lat=self._lat,
                                           altitude=self._ele,
                                           timestamp=self._time,
                                           ids_point=self._id_point
                                           )
                else:
                    # no altutude provided, may be corrected later
                    self._pointset.set_row(self._id_point,
                                           lon=self._lon,
                                           lat=self._lat,
                                           timestamp=self._time,
                                           ids_point=self._id_point
                                           )

            self._id_point = None
            self._lon = None
            self._lat = None
            self._lat = None
            self._ele = None

            self._ele_flag = False
            self._time_flag = False
