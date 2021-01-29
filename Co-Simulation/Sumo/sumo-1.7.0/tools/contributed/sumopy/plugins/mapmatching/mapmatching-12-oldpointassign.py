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

# @file    mapmatching-12-oldpointassign.py
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
from coremodules.demand.demandbase import DemandobjMixin
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
               'worker': 1,
               'student': 2,
               'employee': 3,
               'public employee': 4,
               'selfemployed': 5,
               'pensioneer': 6,
               'other': 7
               }

GENDERS = {'male': 0, 'female': 1, 'unknown': -1}

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

COLOR_MATCHED_ROUTE = np.array([1.0, 0.4, 0.0, 0.6], np.float32)
COLOR_SHORTEST_ROUTE = np.array([0.23529412, 1.0, 0.0, 0.6], np.float32)
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

def load_results(filepath, parent=None, logger=None):
    # typically parent is the scenario
    results = cm.load_obj(filepath, parent=parent)
    if logger is not None:
        results.set_logger(logger)
    return results


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
        # print '  year_str,month_str,day_str',year_str,month_str,day_str
        (hours_str, minutes_str, seconds_str) = clock.split(sep_clock)
        # print '  hours_str,minutes_str,seconds_str',hours_str,minutes_str,seconds_str

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
    def __init__(self, ident, mapmatching,  logger=None, **kwargs):
        print 'BirgilMatcher.__init__'

        # TODO: let this be independent, link to it or child??

        self._init_common(ident,
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
        logger = self.get_logger()
        timeconst = 0.95

        trips = self.parent.trips
        #routes = trips.get_routes()
        edges = self.get_edges()
        vtypes = self.parent.get_scenario().demand.vtypes

        fstar = edges.get_fstar(is_return_arrays=True, is_ignor_connections=True)
        times = edges.get_times(id_mode=2, is_check_lanes=False, speed_max=5.555)
        ids_trip = trips.get_ids_selected()

        ids_mode = vtypes.ids_mode[trips.ids_vtype[ids_trip]]

        n_trips = len(ids_trip)
        logger.w('Start mapmatching of %d GPS traces with Birgillito method' % n_trips)

        distancesmap = self.parent.get_distancesmap()
        accesslevelsmap = self.parent.get_accesslevelsmap()

        n_trip_matched = 0
        time_match_trace_av = 3.0  # initial gues of match time

        # ,vtypes.speed_max[ids_vtype]
        for id_trip, ids_point, id_mode in zip(ids_trip, trips.ids_points[ids_trip], ids_mode):
            tick_before = time.time()
            route, length_route, length_route_mixed, length_route_exclusive, duration_gps, lengthindex, err_dist, t_match, ids_point_edgeend, is_connected = \
                self.match_trip_birgil(id_trip, ids_point, fstar, distancesmap[id_mode], accesslevelsmap[id_mode])

            trips.set_matched_route(id_trip, route,
                                    length_route,
                                    length_route_mixed,
                                    length_route_exclusive,
                                    duration_gps,
                                    lengthindex,
                                    err_dist,
                                    t_match,
                                    is_connected=is_connected,
                                    ids_point_edgeend=ids_point_edgeend)
            n_trip_matched += 1

            time_match_trace = time.time()-tick_before

            time_match_trace_av = timeconst*time_match_trace_av + (1.0-timeconst)*time_match_trace
            time_end_est = (n_trips-n_trip_matched)*time_match_trace_av

            progress = 100.0*n_trip_matched/float(n_trips)
            logger.w("Matched %d/%d traces, %.2f%% comleted. Avg match time = %.2fs, Terminated in %.1fh" %
                     (n_trip_matched, n_trips, progress, time_match_trace_av, float(time_end_est)/3600.0), key='message')
            logger.w(progress, key='progress')

    def get_edges(self):
        return self.parent.get_scenario().net.edges

    def match_trip_birgil(self, id_trip,  ids_point, fstar, costs, accesslevels):
        # TODO: record time intervals
        # calculate initial and final position on edge to get more precise
        # triplength
        print 79*'='
        print 'match_trip_birgil', id_trip
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
            return [], 0.0, 0.0, -1.0, -1.0, -1.0, -1.0, 0.0, [], False

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
            accesslevel = accesslevels[id_edge]
            cost_tot = cost0+self.weight_cumlength * length_edge - self.weight_access * accesslevel
            routelist.append([cost_tot, cost0, 0.0, length_edge, accesslevel, length_edge, [id_edge], []])

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
                costs_tot, cost, length_partial, length_cum, accesslevel, length_edge, ids_edge, ids_point_edgeend = routeinfo

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
                        ids_point_edgeend_new = []
                    else:

                        id_edge = ids_edge_next[0]
                        accesslevel = accesslevels[id_edge]
                        length_edge = edges.lengths[id_edge]
                        length_cum += length_edge
                        cost_edge = self._get_cost_birgil(point, id_edge, phi_point,
                                                          accesslevel, get_dist_point_to_edge)
                        #self.get_dist_point_edge(coords[ind_point], ind_edge)
                        ids_edge_new = ids_edge + [id_edge]
                        ids_point_edgeend_new = ids_point_edgeend + [ids_point[ind_point]]
                        # print '      add to route id_edge',id_edge,ids_edge_new

                    # save updated data of current route
                    cost0 = cost+cost_edge
                    cost_tot = cost0+self.weight_cumlength * length_cum
                    routeinfo[:] = cost_tot, cost0, length_partial, length_cum, accesslevel, length_edge, ids_edge_new, ids_point_edgeend_new

                    # add new children if forward star greater 1
                    if len(ids_edge_next) > 1:
                        for id_edge in ids_edge_next[1:]:
                            #ind_edge = self._edge_to_id_edge[edge]
                            length_edge = edges.lengths[id_edge]
                            length_cum += length_edge
                            accesslevel = accesslevels[id_edge]
                            cost_edge = self._get_cost_birgil(
                                point, id_edge, phi_point, accesslevel, get_dist_point_to_edge)
                            ids_edge_new = ids_edge + [id_edge]
                            ids_point_edgeend_new = ids_point_edgeend + [ids_point[ind_point]]
                            # print '      new route id_edge',id_edge,ids_edge_new
                            cost0 = cost+cost_edge
                            cost_tot = cost0+self.weight_cumlength * length_cum
                            routelist_new.append([cost_tot, cost0, length_partial, length_cum,
                                                  accesslevel, length_edge, ids_edge_new, ids_point_edgeend_new])

                else:
                    # has not reached end of current edge
                    # save updated data of current route
                    if len(ids_edge) > 0:
                        #dist += self.get_dist_point_edge(coords[ind_point], ids_edge[-1])
                        cost += self._get_cost_birgil(point, id_edge_last, phi_point,
                                                      accesslevel, get_dist_point_to_edge)
                    cost_tot = cost+self.weight_cumlength * length_cum
                    routeinfo[:] = cost_tot, cost, length_partial, length_cum, accesslevel, length_edge, ids_edge, ids_point_edgeend

                ind_route += 1

            routelist += routelist_new
            # print '  routelist',routelist
            routelist.sort()

            # cut list to self.n_routes_follow
            if len(routelist) > self.n_routes_follow+1:
                routelist = routelist[:self.n_routes_follow+1]

            ind_point += 1

        # recover final edge objects of winner route!
        costs_tot, cost, length_partial, length_cum, accesslevel, length_edge, ids_edge, ids_point_edgeend = routelist[
            0]

        # print '  ids_edge[-1]',ids_edge[-1],ids_edge[-1] in ids_edge_final
        # print '  ids_edge_final',ids_edge_final
        t_match = time.time() - tick

        # --------------------------------------------------------------------
        # post matching analisis
        print '\n'+79*'-'
        print '  matched route:', ids_edge
        # print '  ids_point_edgeend',ids_point_edgeend
        route = ids_edge
        if len(route) == 0:
            print 'ABOARD: route contains no edges ids_edge=', ids_edge
            return [], 0.0, 0.0, -1.0, -1.0, -1.0, -1.0, 0.0, [], False

        length_route = np.sum(costs[ids_edge])
        length_route_mixed = 0.0
        length_route_exclusive = 0.0
        for id_edge, length in zip(ids_edge, costs[ids_edge]):
            accesslevel = accesslevels[id_edge]
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
            is_connected = True
        else:
            print 'DISCONNECTED: last matched edge', ids_edge[-1], ' did not reach final edges', ids_edge_final
            is_connected = False

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

        return route, length_route, length_route_mixed, length_route_exclusive, duration_gps, lengthindex, err_dist, t_match, ids_point_edgeend, is_connected

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

    def _init_traceoptions(self, **kwargs):
        attrsman = self.get_attrsman()
        self.dist_trip_min = attrsman.add(cm.AttrConf('dist_trip_min', kwargs.get('dist_trip_min', 100.0),
                                                      groupnames=['options'],
                                                      perm='rw',
                                                      name='Min. trip distance',
                                                      unit='m',
                                                      info='Minimum distance of one trip. Shorter trips will not be selected.',
                                                      ))

        self.dist_trip_max = attrsman.add(cm.AttrConf('dist_trip_max', kwargs.get('dist_trip_max', 25000.0),
                                                      groupnames=['options'],
                                                      perm='rw',
                                                      name='Max. trip distance',
                                                      unit='m',
                                                      info='Maximum distance of one trip. Shorter trips will not be selected.',
                                                      ))

        self.duration_trip_min = attrsman.add(cm.AttrConf('duration_trip_min', kwargs.get('duration_trip_min', 30.0),
                                                          groupnames=['options'],
                                                          perm='rw',
                                                          name='Min. trip duration',
                                                          unit='s',
                                                          info='Minimum duration of one trip. Trips with shorter duration will not be selected.',
                                                          ))

        self.duration_trip_max = attrsman.add(cm.AttrConf('duration_trip_max', kwargs.get('duration_trip_max', 999999.0),
                                                          groupnames=['options'],
                                                          perm='rw',
                                                          name='Max. trip duration',
                                                          unit='s',
                                                          info='Maximum duration of one trip. Trips with longer duration will not be selected.',
                                                          ))

        self.speed_trip_min = attrsman.add(cm.AttrConf('speed_trip_min', kwargs.get('speed_trip_min', 1.0),
                                                       groupnames=['options'],
                                                       perm='rw',
                                                       name='Min. av. trip speed',
                                                       unit='m/s',
                                                       info='Minimum average trip speed. Trips with lower average speed will not be selected.',
                                                       ))

        self.speed_trip_max = attrsman.add(cm.AttrConf('speed_trip_max', kwargs.get('speed_trip_max', 14.0),
                                                       groupnames=['options'],
                                                       perm='rw',
                                                       name='Max. av. trip speed',
                                                       unit='m/s',
                                                       info='Maximum average trip speed. Trips with higher average speed will not be selected.',
                                                       ))

    def _init_filter_time(self, **kwargs):
        attrsman = self.get_attrsman()

        self.hour_from_morning = attrsman.add(cm.AttrConf('hour_from_morning', kwargs.get('hour_from_morning', 0),
                                                          groupnames=['options'],
                                                          perm='rw',
                                                          name='From morning hour',
                                                          unit='h',
                                                          info='Keep only morning trips which start after this hour.',
                                                          ))

        self.hour_to_morning = attrsman.add(cm.AttrConf('hour_to_morning', kwargs.get('hour_to_morning', 12),
                                                        groupnames=['options'],
                                                        perm='rw',
                                                        name='To morning hour',
                                                        unit='h',
                                                        info='Keep only morning trips which start before this hour.',
                                                        ))

        self.hour_from_evening = attrsman.add(cm.AttrConf('hour_from_evening', kwargs.get('hour_from_evening', 12),
                                                          groupnames=['options'],
                                                          perm='rw',
                                                          name='From evening hour',
                                                          unit='h',
                                                          info='Keep only evening trips which start after this hour.',
                                                          ))

        self.hour_to_evening = attrsman.add(cm.AttrConf('hour_to_evening', kwargs.get('hour_to_evening', 24),
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
        # print '  self.hour_from_morning,self.hour_to_morning',self.hour_from_morning,self.hour_to_morning
        localtime = time.localtime
        inds_elim = np.zeros(len(timestamps), dtype=np.bool)
        i = 0
        for timestamp in timestamps:

            dt = localtime(timestamp)
            is_keep = dt.tm_wday in self.weekdays
            # print '   dt.tm_wday,self.weekdays',dt.tm_wday,self.weekdays
            h = dt.tm_hour
            is_keep &= (h > self.hour_from_morning) & (h < self.hour_to_morning)\
                | (h > self.hour_from_evening) & (h < self.hour_to_evening)

            # print '  is_keep,w,h=',is_keep,dt.tm_wday,h
            inds_elim[i] = not is_keep
            i += 1

        return inds_elim

    def is_timestamp_ok(self, timestamp):
        # if is_daytimesaving:
        dt = time.localtime(timestamp)
        # else:
        #    dt = time.gmtime(timestamp)
        is_ok = dt.tm_wday in self.weekdays
        h = dt.tm_hour
        # print 'is_timestamp_ok h,dt.tm_wday',h,dt.tm_wday,is_ok,

        is_ok &= (h >= self.hour_from_morning) & (h < self.hour_to_morning)\
            | (h >= self.hour_from_evening) & (h < self.hour_to_evening)

        # print is_ok
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
                                                        info='Minimum allowed length index.'+info_lengthindex,
                                                        ))
        self.lengthindex_max = attrsman.add(cm.AttrConf('lengthindex_max', kwargs.get('lengthindex_max', 110.0),
                                                        groupnames=['options'],
                                                        name='Min. length index',
                                                        unit='%',
                                                        info='Maximum allowed length index '+info_lengthindex,
                                                        ))

        info_error_dist = 'The distance error is the average distance between the GPS points and the matched route.'
        self.error_dist_max = attrsman.add(cm.AttrConf('error_dist_max', kwargs.get('error_dist_max', 10000.0),
                                                       groupnames=['options'],
                                                       name='Max. distance err.',
                                                       unit='mm',
                                                       info='Maximum allowed distance error. '+info_error_dist,
                                                       ))

        self._init_filter_time(**kwargs)
        # self._init_traceoptions(**kwargs)
        self.speed_trip_min = attrsman.add(cm.AttrConf('speed_trip_min', kwargs.get('speed_trip_min', 1.0),
                                                       groupnames=['options'],
                                                       perm='rw',
                                                       name='Min. av. trip speed',
                                                       unit='m/s',
                                                       info='Minimum average trip speed. Trips with lower average speed will not be selected.',
                                                       ))

        self.speed_trip_max = attrsman.add(cm.AttrConf('speed_trip_max', kwargs.get('speed_trip_max', 14.0),
                                                       groupnames=['options'],
                                                       perm='rw',
                                                       name='Max. av. trip speed',
                                                       unit='m/s',
                                                       info='Maximum average trip speed. Trips with higher average speed will not be selected.',
                                                       ))

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
            trips.lengthindexes[ids_selected] > self.lengthindex_max,
            trips.errors_dist[ids_selected] > self.error_dist_max,
            # (trips.lengths_route_matched[ids_selected]<1.0), # too many args??
        )

        inds_eliminate = np.logical_or(inds_eliminate,
                                       trips.lengths_route_matched[ids_selected] < 1.0
                                       )

        inds_eliminate = np.logical_or(inds_eliminate,
                                       trips.speeds_average[ids_selected] < self.speed_trip_min,
                                       trips.speeds_average[ids_selected] > self.speed_trip_max
                                       )

        # dist > self.dist_trip_min)\
        #           & (dist < self.dist_trip_max)\
        #           & (duration > self.duration_trip_min)\
        #           & (speed_av > self.speed_trip_min)\
        #           & (speed_av < self.speed_trip_max)

        # print '  lengths_route_matched',trips.lengths_route_matched[ids_selected]
        # print '  lengthindexes',self.lengthindex_min,self.lengthindex_max,trips.lengthindexes[ids_selected]
        # print '  errors_dist',self.error_dist_max,trips.errors_dist[ids_selected]
        # print '  lengthindex_min',trips.lengthindexes[ids_selected]<self.lengthindex_min
        # print '  lengthindex_max',trips.lengthindexes[ids_selected]>self.lengthindex_max
        # print '  lengths_route_matched',trips.errors_dist[ids_selected]>self.error_dist_max
        # print '  inds_eliminate',inds_eliminate

        inds_eliminate = np.logical_or(inds_eliminate, self.filter_time(trips.timestamps[ids_selected]))
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

        self.is_eliminate_close_points = attrsman.add(cm.AttrConf('is_eliminate_close_points', kwargs.get('is_eliminate_close_points', False),
                                                                  groupnames=['options'],
                                                                  perm='rw',
                                                                  name='Eliminate close points',
                                                                  info='Eliminate points with a distance less then the minimum distance.',
                                                                  ))

        self.dist_point_min_extr = attrsman.add(cm.AttrConf('dist_point_min_extr', kwargs.get('dist_point_min_extr', 9.0),
                                                            groupnames=['options'],
                                                            perm='rw',
                                                            name='Min. extr. point dist.',
                                                            unit='m',
                                                            info='Keep only the point where the distance to the point at both extremities is more than a minimum distance.',
                                                            ))

        self.dist_point_min_inter = attrsman.add(cm.AttrConf('dist_point_min_inter', kwargs.get('dist_point_min_inter', 2.0),
                                                             groupnames=['options'],
                                                             perm='rw',
                                                             name='Min. int. point dist.',
                                                             unit='m',
                                                             info='Keep only the point where the distance to the previous point is more than a minimum distance.',
                                                             ))

        self.speed_max = attrsman.add(cm.AttrConf('speed_max', kwargs.get('speed_max', 50.0/3.6),
                                                  groupnames=['options'],
                                                  perm='rw',
                                                  name='Max. speed',
                                                  unit='m',
                                                  info='Keep only traces where this maximum speed is not reached. Maximum speed is reached if a consecutive number of Max. overspeed points is reached.',
                                                  ))

        self.n_overspeed_max = attrsman.add(cm.AttrConf('n_overspeed_max', kwargs.get('n_overspeed_max', 3),
                                                        groupnames=['options'],
                                                        perm='rw',
                                                        name='Max. overspeed points',
                                                        info='Trace gets eliminated if this consecutive number of points have speeds over the set maximum speed.',
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
        dist_point_min = self.dist_point_min_extr
        dist_point_min_inter = self.dist_point_min_inter
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
        ids_point_elim_perm = []
        for id_trip, ids_point in zip(ids_trips, ids_points):

            coords = points.coords[ids_point]
            times = points.timestamps[ids_point]
            # print 79*'-'
            # print '  check id_trip ',id_trip
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
                ids_point_elim = []
                coord_last = coords[0]
                coord_start = coords[0]
                coord_end = coords[-1]

                while (not is_eliminate) & (i < (n-1)):
                    i += 1

                    dist_point = np.sqrt((coords[i, 0]-coords[i-1, 0])**2
                                         + (coords[i, 1]-coords[i-1, 1])**2)
                    # print '  ',times[i]-times[i-1],dist_point,coords[i,:2],coords[i-1,:2]
                    if self.is_eliminate_close_points:

                        dist_start = np.sqrt((coords[i, 0]-coord_start[0])**2
                                             + (coords[i, 1]-coord_start[1])**2)

                        if dist_start < dist_point_min:
                            # print '  eliminate',ids_point[i], dist_check
                            ids_point_elim.append(ids_point[i])
                        else:
                            dist_end = np.sqrt((coords[i, 0]-coord_end[0])**2
                                               + (coords[i, 1]-coord_end[1])**2)

                            if dist_end < dist_point_min:
                                # print '  eliminate',ids_point[i], dist_check
                                ids_point_elim.append(ids_point[i])
                            else:

                                dist_check = np.sqrt((coords[i, 0]-coord_last[0])**2
                                                     + (coords[i, 1]-coord_last[1])**2)
                                if dist_check < dist_point_min_inter:
                                    ids_point_elim.append(ids_point[i])
                                else:
                                    coord_last = coords[i]

                    if dist_point > dist_point_max:
                        is_eliminate = True

                    # print '   times[i]-times[i-1]',times[i]-times[i-1]
                    time_delta = times[i]-times[i-1]
                    if time_delta > duration_point_max:
                        is_eliminate = True

                    # test for return distance
                    d = np.sqrt((x-coords[i, 0])**2 + (y-coords[i, 1])**2)
                    if d > dist_max:
                        dist_max = d

                    elif d < c_cutoff*dist_max:
                        is_eliminate = True

                if self.is_eliminate_close_points:
                    if len(ids_point_elim) > 0:

                        ids_point2 = list(ids_point)
                        # print '  before elim ids_point',ids_point2
                        # print '  eliminate',ids_point_elim
                        for id_point in ids_point_elim:
                            ids_point2.remove(id_point)

                        trips.ids_points[id_trip] = ids_point2
                        ids_point_elim_perm += ids_point_elim
                        # print '  after elim ids_point',trips.ids_points[id_trip]

            inds_elim[j] = is_eliminate
            j += 1

        if len(ids_point_elim_perm) > 0:
            print '  permanently eliminate %d GPS points' % (len(ids_point_elim_perm))
            points.del_rows(ids_point_elim_perm)

        return ids_trips[inds_elim]


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
                                             choices={'2013': 2013, '2014': 2014, '2015': 2015, '2016': 2016},
                                             perm='rw',
                                             name='Year of challange',
                                             info='Year of challange is used to identify the correct database format.',
                                             ))

        self._init_traceoptions(**kwargs)

        # self.sep_column_workout = attrsman.add(cm.AttrConf( 'sep_column_workout',kwargs.get('sep_column_workout',','),
        #                    groupnames = ['options'],
        #                    perm='rw',
        #                    name = 'Workoutdata seperator',
        #                    info = 'Workout column seperator of CSV file',
        #                    ))

        # self.sep_column_points = attrsman.add(cm.AttrConf( 'sep_column_points',kwargs.get('sep_column_points',','),
        #                    groupnames = ['options'],
        #                    perm='rw',
        #                    name = 'Point data seperator',
        #                    info = 'Pointdata column seperator of CSV file',
        #                    ))

        self._init_filter_time(**kwargs)

        self._proj = None
        self._offset = None

    def project(self, lons, lats):
        if self._proj is None:
            self._proj, self._offset = self.parent.get_proj_and_offset()
        x, y = self._proj(lons, lats)
        return np.transpose(np.concatenate(([x+self._offset[0]], [y+self._offset[1]]), axis=0))

    def validate_trip(self, dist, duration, speed_av=-1):
        if speed_av < 0:
            if duration > 1.0:
                speed_av = dist/duration
            else:
                speed_av = 0.0

        return (dist > self.dist_trip_min)\
            & (dist < self.dist_trip_max)\
            & (duration > self.duration_trip_min)\
            & (speed_av > self.speed_trip_min)\
            & (speed_av < self.speed_trip_max)

    def do(self):
        print 'TraceImporter.do'
        if self.year == 2014:
            self.import_workouts_2014()
            self.import_points_2014()

        if self.year == 2015:
            self.import_workouts_2015()
            self.import_points_2015()

        if self.year == 2016:
            self.import_workouts_2016()
            self.import_points_2016()

        if self.year == 2013:
            self.import_points_2013()

        self.parent.points.project()

    def import_workouts_2016(self):
        # UserID	                    TripID	                    TimeStamp	Start DT	                Distance	 ECC	 AvgSpeed	 TrackType	 Sex	 Year	 Profession	 Frequent User	 ZIP	 Source	  TypeOfBike	 TipeOfTrip	 Max Spd
        # 57249bcd88c537874f9fa1ae	57515edc88c537576ca3e16f	1464945480	2016-06-03T09:18:00.000Z	4.75	4.75	    10.16	    urban bicycle	F	1999	Studente	    yes		            cy-web-gpx	MyBike	    HomeToSchool	25.45

        j_id_user, j_id_trip, j_timestamp, j_date, j_dist, j_dist_copy, j_speed_av, j_tracktype, j_sex, j_year, j_profession, j_frequent, j_zip, j_device, j_biketype, j_purpose, j_speedmax = range(
            17)
        n_cols = 17
        null = 'NULL'
        trips = self.parent.trips
        persons = self.parent.persons
        #exist_id_person_sumo = persons.ids_sumo.has_index
        ids_person_sumo = {}

        #ids_trips = []
        scenario = self.parent.get_scenario()

        get_vtype_for_mode = scenario.demand.vtypes.get_vtype_for_mode
        id_type = get_vtype_for_mode(self.id_mode)

        f = open(self.workoutsfilepath, 'r')
        if self._logger:
            self._logger.w('import_workouts_2016 %s' % os.path.basename(self.workoutsfilepath))
        i_line = 0
        sep = ';'
        i_line = 0
        #self.get_logger().w(100.0*self.simtime/self.duration, key ='progress')
        for line in f.readlines()[1:]:
            # if True:#i_line>1:
            cols = line.split(sep)
            # print '    len(cols)',len(cols),n_cols
            # print '    cols',cols
            if len(cols) >= n_cols:
                # row is complete
                # print '  id_trip_sumo',cols[j_id_trip]
                # if cols[j_dist] != null:
                dist = get_colvalue(cols[j_dist])*1000.0
                speed_av = get_colvalue(cols[j_speed_av])/3.6
                if speed_av > 0:
                    duration = dist/speed_av
                else:
                    duration = 0.0

                if self.validate_trip(dist, duration, speed_av):
                    timestamp = get_colvalue(cols[j_timestamp])

                    if timestamp is not None:
                        # print '    valid time stamp',timestamp,self.is_timestamp_ok(timestamp)

                        if self.is_timestamp_ok(timestamp):
                            id_trip = trips.make(id_sumo=cols[j_id_trip],
                                                 id_vtype=id_type,
                                                 timestamp=timestamp,
                                                 distance_gps=dist,
                                                 duration_gps=duration,
                                                 speed_average=speed_av,
                                                 speed_max=get_colvalue(cols[j_speedmax])/3.6,
                                                 purpose=cols[j_purpose].strip(),
                                                 device=cols[j_device].strip(),
                                                 )

                            # ids_trips.append(id_trip)
                            #j_id_user, j_id_trip, j_timestamp, j_date, j_dist, j_speed_av, j_tracktype, j_sex, j_year, j_profession, j_frequent, j_zip

                            zip = cols[j_zip].strip()
                            if zip == 'undefined':
                                zip = ''

                            year = cols[j_year]
                            if year.isdigit():
                                year_birth = int(year)
                            else:
                                year_birth = -1

                            id_pers = persons.make(id_sumo=cols[j_id_user].strip(),
                                                   id_trip=id_trip,
                                                   gender=cols[j_sex].strip(),
                                                   year_birth=year_birth,
                                                   occupation=cols[j_profession].strip(),
                                                   is_frequent_user=cols[j_frequent].strip().lower() == 'yes',
                                                   zip=zip,
                                                   )
                            # print '  id_trip,id_trip_sumo,id_pers',id_trip,cols[j_id_trip], id_pers
                            # print
            else:
                print 'WARNING: inconsistent number of columns (%d) in line %d, file %s' % (
                    len(cols), i_line, self.workoutsfilepath)
                print '  cols =', cols

            i_line += 1

    def import_points_2016(self):
        print 'import_points_2016'
        #    0          1    2            3         4          5         6    7
        # TripID, TimeStamp,Latitude, Longitude, Altitude, Distance, Speed, Type
        # 574e98c988c5378163a3e11f,1462347278,44.52606,11.27617,78,0.027255420500625783,5,<start|mid|end>,
        # 5741cdd388c537f10192ee97, 1463926725,44.50842,11.3604,101.0417,0.01615021486623964,3.483146,mid

        ind_id_path = 0
        ind_time = 1
        ind_lat = 2
        ind_lon = 3
        ind_alt = 4
        ind_dist = 5
        ind_speed = 6
        ind_type = 7

        n_cols = 8
        #TripID, TimeStamp,Date, Latitude, Longitude, Altitude, Distance, Speed, Type

        trips = self.parent.trips
        points = self.parent.points

        exist_id_trip_sumo = trips.ids_sumo.has_index
        get_id_trip = trips.ids_sumo.get_id_from_index

        sep = ','  # self.sep_column_points

        f = open(self.pointsfilepath, 'r')
        if self._logger:
            self._logger.w('import_points_2016 %s' % os.path.basename(self.pointsfilepath))
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
                        # print '    timestamps',timestamps
                        if len(timestamps) > 1:
                            trips.durations_gps[id_trip] = timestamps[-1]-timestamps[0]
                            # print '    durations_gps',timestamps[-1]-timestamps[0]
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
                        id_trip = -1

                    id_trip_sumo = id_trip_sumo_current

                if is_valid_trip:
                    # print '    store point timestamp',cols[ind_time]
                    # current point belongs to a valid trip
                    # ids_point_sumo.append(cols[ind_id_point])
                    timestamps.append(get_colvalue(cols[ind_time]))
                    # ids_trip.append(id_trip)
                    longitudes.append(get_colvalue(cols[ind_lon]))
                    latitudes.append(get_colvalue(cols[ind_lat]))
                    altitudes.append(get_colvalue(cols[ind_alt]))

            else:
                print 'WARNING: inconsistent number of columns (%d) in line %d, file %s' % (
                    len(cols), i_line, self.pointsfilepath)
                print '  cols =', cols

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

            if len(timestamps) > 1:
                trips.durations_gps[id_trip] = timestamps[-1]-timestamps[0]

        # self.odtab.print_rows()

        f.close()

    def import_workouts_2015(self):

        # 2015 csv
        # workouts
        # UserID	 TripID	 TimeStamp	 Start DT	  Distance	 AvgSpeed	 TrackType	 Sex	 Year	 Profession	 Frequent User	 ZIP
        # 54eb068de71f393530a9a74d	54eb0737e71f394c2fa9a74d	1424692504	Mon, 23 Feb 2015 11:55:04 GMT	0	0	urban bicycle	M	1987	Developer	no
        # 54eb9374e71f39f02fa9a750	5505cb04e71f39542e25e2d4	1426442994	Sun, 15 Mar 2015 18:09:54 GMT	0	0.7	urban bicycle	M	1974	Worker	yes	40128

        j_id_user, j_id_trip, j_timestamp, j_date, j_dist, j_speed_av, j_tracktype, j_sex, j_year, j_profession, j_frequent, j_zip = range(
            12)
        n_cols = 12
        null = 'NULL'
        trips = self.parent.trips
        persons = self.parent.persons
        #exist_id_person_sumo = persons.ids_sumo.has_index
        ids_person_sumo = {}

        #ids_trips = []
        scenario = self.parent.get_scenario()

        get_vtype_for_mode = scenario.demand.vtypes.get_vtype_for_mode
        id_type = get_vtype_for_mode(self.id_mode)

        f = open(self.workoutsfilepath, 'r')
        if self._logger:
            self._logger.w('import_workouts_2015 %s' % os.path.basename(self.workoutsfilepath))
        i_line = 0
        sep = ';'
        i_line = 1
        #self.get_logger().w(100.0*self.simtime/self.duration, key ='progress')
        for line in f.readlines()[1:]:
            # if True:#i_line>1:
            cols = line.split(sep)
            # print '\n    len(cols)',len(cols),n_cols
            # print '    cols',cols
            if len(cols) >= n_cols:
                # row is complete
                # print '  id_trip_sumo "%s"'%cols[j_id_trip]
                # if cols[j_dist] != null:
                dist = get_colvalue(cols[j_dist])*1000.0
                speed_av = get_colvalue(cols[j_speed_av])/3.6
                #duration = get_colvalue(cols[j_duration])
                # if duration>0:
                #    speed_av = dist/duration
                # else:
                #    speed_av = 0.0
                # print '  dist',dist,'speed_av',speed_av
                if speed_av > 0:
                    duration = dist/speed_av
                else:
                    duration = 0.0

                if self.validate_trip(dist, duration, speed_av):
                    # print  '    parametric conditions verified'
                    timestamp = get_colvalue(cols[j_timestamp])
                    if timestamp is not None:
                        # print '    valid time stamp',timestamp#,self.is_timestamp_ok(timestamp)
                        if self.is_timestamp_ok(timestamp):
                            id_trip = trips.make(id_sumo=cols[j_id_trip],
                                                 id_vtype=id_type,
                                                 timestamp=timestamp,
                                                 distance_gps=dist,
                                                 duration_gps=duration,
                                                 speed_average=speed_av,
                                                 )
                            # ids_trips.append(id_trip)
                            #j_id_user, j_id_trip, j_timestamp, j_date, j_dist, j_speed_av, j_tracktype, j_sex, j_year, j_profession, j_frequent, j_zip

                            zip = cols[j_zip].strip()
                            if zip == 'undefined':
                                zip = ''

                            year = cols[j_year]
                            if year.isdigit():
                                year_birth = int(year)
                            else:
                                year_birth = -1

                            persons.make(id_sumo=cols[j_id_user].strip(),
                                         id_trip=id_trip,
                                         gender=cols[j_sex].strip(),
                                         year_birth=year_birth,
                                         occupation=cols[j_profession].strip(),
                                         is_frequent_user=cols[j_frequent].strip() == 'yes',
                                         zip=zip,
                                         )
            else:
                print 'WARNING: inconsistent number of columns (%d) in line %d, file %s' % (
                    len(cols), i_line, self.workoutsfilepath)
                # print '  cols =',cols

            i_line += 1

    def import_points_2015(self):
        print 'import_points_2015'
        #    0          1    2   3         4          5         6         7       8
        #TripID, TimeStamp,Date, Latitude, Longitude, Altitude, Distance, Speed, Type
        # 54eb0737e71f394c2fa9a74d,1424692509,"Mon, 23 Feb 2015 11:55:09 GMT",44.499096,11.361185,49.419395,0,0.000815,start

        ind_id_path = 0
        ind_time = 1
        ind_day = 2
        ind_date = 3
        ind_lat = 4
        ind_lon = 5
        ind_alt = 6
        ind_dist = 7
        ind_speed = 8
        ind_type = 9

        n_cols = 10
        #TripID, TimeStamp,Date, Latitude, Longitude, Altitude, Distance, Speed, Type

        trips = self.parent.trips
        points = self.parent.points

        exist_id_trip_sumo = trips.ids_sumo.has_index
        get_id_trip = trips.ids_sumo.get_id_from_index

        sep = ','  # self.sep_column_points

        f = open(self.pointsfilepath, 'r')
        if self._logger:
            self._logger.w('import_points_2015 %s' % os.path.basename(self.pointsfilepath))
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
                        # print '    timestamps',timestamps
                        if len(timestamps) > 1:
                            trips.durations_gps[id_trip] = timestamps[-1]-timestamps[0]
                            # print '    durations_gps',timestamps[-1]-timestamps[0]
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
                    timestamps.append(get_colvalue(cols[ind_time]))
                    # ids_trip.append(id_trip)
                    longitudes.append(get_colvalue(cols[ind_lon]))
                    latitudes.append(get_colvalue(cols[ind_lat]))
                    altitudes.append(get_colvalue(cols[ind_alt]))

            else:
                print 'WARNING: inconsistent number of columns (%d) in line %d, file %s' % (
                    len(cols), i_line, self.pointsfilepath)
                # print '  cols =',cols

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

            if len(timestamps) > 1:
                trips.durations_gps[id_trip] = timestamps[-1]-timestamps[0]

        f.close()

    def import_workouts_2014(self):
        print 'import_workouts_2014'
        # 2014 ecomondo workouts
        # id	        pointDBNode	pointPathId	startTime	        distance	duration	sport	calories	maxSpeed	altitudeMin	altitudeMax	metersAscent	metersDescent
        # 329308466	7	        37073516	2014-05-01 19:00:00	    26	    15600	    1	    1182.64	    NULL	    NULL	    NULL	    NULL	        NULL
        # 0         1           2            3                       4          5       6          7        8           9          10          11                   12

        j_id, j_node, j_id_trip, j_time, j_dist, j_duration = range(6)
        j_v_max = 8
        n_cols = 13
        null = 'NULL'
        trips = self.parent.trips
        # persons = self.parent.persons # no person data in 2014 :(
        ids_person_sumo = {}

        ids_trips = []
        scenario = self.parent.get_scenario()

        #get_vtype_for_mode = scenario.demand.vtypes.get_vtype_for_mode
        id_vtype = scenario.demand.vtypes.get_vtype_for_mode(self.id_mode)

        f = open(self.workoutsfilepath, 'r')
        #if self._logger: self._logger.w('import_workouts_2014 %s'%os.path.basename(self.workoutsfilepath))
        i_line = 0
        sep = ','  # self.sep_column_workout

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

                if self.validate_trip(dist, duration, speed_av):
                    # print  'parametric conditions verified'
                    timestamp = calc_seconds(cols[j_time])
                    if timestamp is not None:
                        # print '  valid time stamp',timestamp
                        if self.is_timestamp_ok(timestamp):
                            id_trip = trips.make(id_sumo=cols[j_id_trip],
                                                 id_vtype=id_vtype,
                                                 timestamp=timestamp,
                                                 distance_gps=dist,
                                                 duration_gps=duration,
                                                 speed_average=speed_av,
                                                 speed_max=get_colvalue(cols[j_v_max])/3.6,
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

        sep = ','  # self.sep_column_points

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
                print 'WARNING: inconsistent columns in line %d, file %s' % (
                    i_line, os.path.basename(self.pointsfilepath))

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

    def import_points_2013(self):
        print 'import_points_2013'
        #pointDBNode,   pointPathId,    id,     timestamp,          latitude,   longitude,  altitude,distance,  heartRate,instruction,speed
        # 4,             61565791,   23648171762,2013-05-01 06:33:58,44.501085,  11.372906,  NULL,       0,      NULL,       2,          NULL
        # 0                  1         2          3                      4          5            6       7       8           9           10
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
        id_vtype = self.parent.get_scenario().demand.vtypes.get_vtype_for_mode(self.id_mode)
        sep = ','  # self.sep_column_points

        f = open(self.pointsfilepath, 'r')
        if self._logger:
            self._logger.w('import_points_2013 %s' % os.path.basename(self.pointsfilepath))
        i_line = 0
        id_trip_sumo = None
        id_trip = -1
        is_valid_trip = False
        timestamp_last = -1
        n_points_imported = 0
        timestamps = []

        for line in f.readlines():

            cols = line.split(sep)
            # print '    len(cols)',len(cols),n_cols
            if (len(cols) == n_cols) & (i_line > 0):
                id_trip_sumo_current = cols[ind_id_path]
                if timestamp_last < 0:
                    timestamp_last = calc_seconds(cols[ind_time])

                # print '    id_trip_sumo_current,id_trip_sumo',id_trip_sumo_current,id_trip_sumo,is_valid_trip
                if id_trip_sumo_current != id_trip_sumo:
                    # this point is part of new trip

                    # validate trip data
                    if len(timestamps) == 0:
                        is_valid_trip = False

                    else:
                        coords = self.project(latitudes, longitudes)
                        distance = np.sum(np.sqrt(np.sum((coords[1:, :]-coords[:-1, :])**2, 1)))
                        duration = timestamps[-1]-timestamps[0]
                        if duration > 0:
                            speed_av = distance/duration
                        else:
                            speed_av = 0.0
                        is_valid_trip = self.validate_trip(distance, duration, speed_av)

                    if is_valid_trip:
                        # print '  store past points for valid trip',id_trip
                        id_trip = trips.make(id_sumo=id_trip_sumo_current,
                                             id_vtype=id_vtype,
                                             timestamp=timestamp_last,
                                             distance_gps=distance,
                                             duration_gps=duration,
                                             speed_average=speed_av,
                                             )

                        ids_point = points.add_rows(
                            timestamps=timestamps,
                            ids_trip=id_trip*np.ones(len(timestamps), dtype=np.int32),
                            longitudes=longitudes,
                            latitudes=latitudes,
                            altitudes=altitudes,
                        )

                        trips.set_points(id_trip, ids_point)

                    # check if new trip is valid
                    # if exist_id_trip_sumo(id_trip_sumo_current):

                    is_valid_trip = True  # start recording
                    #id_trip = get_id_trip(id_trip_sumo_current)
                    # print '    found trip',id_trip,id_trip_sumo_current,' exisits-> record'

                    # ids_point_sumo = [] # useless?
                    timestamp_last = calc_seconds(cols[ind_time])
                    timestamps = []
                    ids_trip = []
                    longitudes = []
                    latitudes = []
                    altitudes = []

                    # else:
                    #    #print '    trip',id_trip_sumo_current,'does not exisit'
                    #    is_valid_trip  = False

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
                print 'WARNING: inconsistent columns in line %d, file %s' % (
                    i_line, os.path.basename(self.pointsfilepath))

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


class GpxImporter(FilterMixin):
    def __init__(self,  mapmatching, logger=None, **kwargs):
        print 'GpxImporter.__init__', mapmatching.get_ident()
        self._init_common('gpximporter',
                          parent=mapmatching,
                          name='GPX Importer',
                          logger=logger,
                          info='Import GPS traces from GPX files.',

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

        self.filepaths = attrsman.add(
            cm.AttrConf('filepaths', kwargs.get('filepath', rootfilepath+'.gpx'),
                        groupnames=['options'],
                        perm='rw',
                        name='filenames',
                        wildcards='GPX file (*.gpx)|*.gpx|XML file (*.xml)|*.xml',
                        metatype='filepaths',
                        info="""Paths and file name of GPX file with GPS traces in XML format.""",
                        ))
        self._init_traceoptions(**kwargs)

    def do(self):
        """
        Reads endomondo gpx xml file and stores point data in traces table.
        If there is no traces table, one will be initialized and returned
        """
        print 'GpxImporter.do', self.filepaths
        mapmatching = self.parent
        #scenario = mapmatching.get_scenario()
        logger = self.get_logger()

        get_vtype_for_mode = mapmatching.get_scenario().demand.vtypes.get_vtype_for_mode

        parser = GpxParser(mapmatching.trips, mapmatching.points,
                           logger=logger,
                           id_vtype=get_vtype_for_mode(self.id_mode),
                           dist_trip_min=self.dist_trip_min,
                           dist_trip_max=self.dist_trip_max,
                           duration_trip_min=self.speed_trip_min,
                           duration_trip_max=self.duration_trip_max,
                           speed_trip_min=self.speed_trip_min,
                           speed_trip_max=self.speed_trip_max,
                           )
        for filepath in self.filepaths.split(','):
            print '  parse gpx file',    filepath
            parse(filepath.strip(), parser)

        ids_trips = parser.get_ids_trip()
        if logger:
            logger.w('imported %d traces, project coordinates...' % len(ids_trips))

        # recalculate projection for network in scenario
        # for id_trace in ids_traces:
        #    traces.pointsets.get(id_trace).project( traces._proj, traces.offset)

        mapmatching.points.project()

        if logger:
            logger.w('imported %d trips done.' % len(ids_trips))


class GpxParser(handler.ContentHandler):
    """Reads endomondo gpx xml file and parses lat,lon, ele and time.
    """

    def __init__(self, trips, points, logger=None,
                 id_vtype=0,
                 dist_trip_min=10.0,
                 dist_trip_max=50000.0,
                 duration_trip_min=0.5,
                 duration_trip_max=999999.0,
                 speed_trip_min=0.1,
                 speed_trip_max=100.0,
                 ):

        self._logger = logger
        self._points = points
        self._trips = trips
        self._id_vtype = id_vtype
        self._dist_trip_min = dist_trip_min
        self._dist_trip_max = dist_trip_max
        self._duration_trip_min = speed_trip_min
        self._duration_trip_max = duration_trip_max
        self._speed_trip_min = speed_trip_min
        self._speed_trip_max = speed_trip_max

        self.reset_trip()

    def reset_trip(self):
        self._lons = []
        self._lats = []
        self._eles = []
        self._times = []
        self._is_record_segment = False
        self._is_record_point = False
        self._is_time = False
        self._is_ele = False

        self._ids_trip = []
        self._ids_point = []

    def get_ids_trip(self):
        return self._ids_trip

    def get_ids_point(self):
        return self._ids_point

    def startElement(self, name, attrs):
        # if self._pointset  is None:

        if name == 'trkpt':
            self._is_record_point = True
            self._lons.append(float(attrs['lon']))
            self._lats.append(float(attrs['lat']))

        if name == 'time':
            self._is_time = True

        if name == 'ele':
            self._is_ele = True

        if name == 'trkseg':
            self._is_record_segment = True

        if name == 'trk':
            self._is_record_trip = True

    def characters(self, content):

        if self._is_time:
            # print '  got time',content[:-1]

            if self._is_record_point:
                # 2013-05-30T16:42:33Z 2014-01-26T12:32:21Z
                self._times.append(calc_seconds(content[:-1],
                                                sep_date_clock='T',
                                                sep_date='-',
                                                sep_clock=':',
                                                is_float=False))

        if self._is_ele:
            # print 'characters content',content
            self._eles.append(float(content))

    def endElement(self, name):
        if name == 'ele':
            self._is_ele = False

        if name == 'time':
            self._is_time = False

        if name == 'trkpt':
            # print 'endElement: point',self._lon,self._lat,self._ele,self._time
            #self._is_ele = False
            #self._is_time = False
            self._is_record_point = False

        if name == 'trk':
            # here we could stop recording a segment
            # but currently we join all segments together
            pass

        if name == 'trk':
            # print 'endElement',len(self._lons)

            # print '  self._lons',self._lons
            # print '  self._lats',self._lats
            # print '  self._times',self._times

            # trip recording ends
            n_points = len(self._lons)

            if (n_points > 0):
                timestamp = self._times[0]
                duration = self._times[-1]-timestamp
                # print '    timestamp',timestamp
                # print '    duration',duration,self._duration_trip_min,self._duration_trip_max
                # TODO: make this filter a functin of filtermixin
                if (duration > self._duration_trip_min)\
                        & (duration < self._duration_trip_max):

                    id_trip = self._trips.make(id_vtype=self._id_vtype,
                                               timestamp=timestamp,
                                               #distance_gps = dist,
                                               duration_gps=duration
                                               #speed_average = speed_av,
                                               )
                    self._ids_trip.append(id_trip)

                    if len(self._eles) == n_points:
                        altitudes = self._eles
                    else:
                        altitudes = np.zeros(n_points, dtype=np.float32)

                    ids_point = self._points.add_rows(
                        timestamps=self._times,
                        ids_trip=id_trip*np.ones(n_points, dtype=np.int32),
                        longitudes=self._lons,
                        latitudes=self._lats,
                        altitudes=altitudes,
                    )

                    self._trips.set_points(id_trip, ids_point)
                    self._trips.ids_sumo[id_trip] = str(id_trip)

            self.reset_trip()


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

        self.add_col(am.ArrayConf('lengths_gpsroute_matched', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Matched length',
                                  symbol='L match GPS',
                                  unit='m',
                                  info='Length of the matched part of the GPS trace, measured by linear interpolation of GPS points. Note the only a fraction of the GPS trace ma be within the given network.',
                                  ))

        self.add_col(am.ArrayConf('lengths_route_matched', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Matched length',
                                  symbol='L match',
                                  unit='m',
                                  info='Length of the matched part of the GPS trace, measured by summing the length of edges of the matched route. Note the only a fraction of the GPS trace ma be within the given network.',
                                  ))

        self.add_col(am.ArrayConf('durations_route_matched', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Matched duration',
                                  symbol='T match',
                                  unit='s',
                                  info='Duration of the matched part of the GPS trace. This is the difference in timestamps between last and first GPS point of the matched route. Note the only a fraction of the GPS trace ma be within the given network.',
                                  ))

        self.add_col(am.ArrayConf('lengths_route_shortest', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Shortest length',
                                  symbol='L short',
                                  unit='m',
                                  info='Length of the shortest route.  Shortest route is connecting the first matched edge and the final matched edge.',
                                  ))

        self.add_col(am.ArrayConf('lengths_route_matched_mixed', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Matched length mixed access',
                                  symbol='L match mix',
                                  unit='m',
                                  info='Length of the matched part of the GPS trace. Note the only a fraction of the GPS trace ma be within the given network.',
                                  ))

        self.add_col(am.ArrayConf('lengths_route_matched_exclusive', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Matched length exclusive access',
                                  symbol='L match excl',
                                  unit='m',
                                  info='Length of the matched part of the GPS trace. Note the only a fraction of the GPS trace ma be within the given network.',
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

        self.add_col(am.ArrayConf('are_match_connected', default=False,
                                  dtype=np.bool,
                                  groupnames=['results', ],
                                  name='Match connected',
                                  #symbol = 'Match conn.',
                                  info='The matched route connects first and last network edge where GPS points have been detected.',
                                  ))

        self.add_col(am.IdlistsArrayConf('ids_points', self.parent.points,
                                         #groupnames = ['_private'],
                                         name='Point IDs',
                                         info="GPS point IDs.",
                                         ))

        self.add_col(am.IdlistsArrayConf('ids_points_edgeend', self.parent.points,
                                         groupnames=['results', '_private'],
                                         name='Edge endpoint IDs',
                                         info="This is a list of GPS point IDs which represent the last point associated with each matched edge.",
                                         ))
        # update
        self.ids_points_edgeend.add_groupnames(['_private'])

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

    def invert_selection(self):
        self.are_selected.get_value()[:] = np.logical_not(self.are_selected.get_value()[:])

    def set_matched_route(self, id_trip, route,
                          length_matched=0.0,
                          length_route_mixed=0.0,
                          length_route_exclusive=0.0,
                          duration_matched=0.0,
                          lengthindex=-1.0,
                          error_dist=-1.0,
                          comptime=0.0,
                          is_connected=False,
                          ids_point_edgeend=[],
                          ):

        if len(route) > 0:
            id_route = self.ids_route_matched[id_trip]
            if id_route >= 0:
                # already a matched route existant
                self.get_routes().ids_edges[id_route] = route
                self.get_routes().colors[id_route] = COLOR_MATCHED_ROUTE
                # self.get_routes().set_row(  id_route,
                #                            ids_edges = route,
                #                            colors = COLOR_MATCHED_ROUTE,
                #                            )

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
                                                     colors=COLOR_MATCHED_ROUTE,
                                                     )
        else:
            id_route = -1

        # print 'set_matched_route id_trip', id_trip,'id_route', id_route
        # print '  ids_point_edgeend',ids_point_edgeend
        #self.ids_route_matched[id_trip] = id_route

        self.set_row(id_trip,
                     ids_route_matched=id_route,
                     lengths_gpsroute_matched=length_matched/lengthindex,
                     durations_route_matched=duration_matched,
                     lengths_route_matched=length_matched,
                     lengths_route_matched_mixed=length_route_mixed,
                     lengths_route_matched_exclusive=length_route_exclusive,
                     lengthindexes=100*lengthindex,
                     errors_dist=1000 * error_dist,
                     times_computation=1000*comptime,
                     are_match_connected=is_connected
                     )
        # TODO:  do this extra! this is a bug!!
        # if included in set_row, only first value in list is taken!!!
        self.ids_points_edgeend[id_trip] = ids_point_edgeend

        # print '  ids_points_edgeend[id_trip]',self.ids_points_edgeend[id_trip]

        return id_route

    def make(self,  **kwargs):
        # if self.ids_sumo.has_index(id_sumo):
        #    id_trip = self.ids_sumo.get_id_from_index(id_sumo)
        #    #self.set_row(id_sumo, **kwargs)
        #    return id_trip
        # else:

        #purpose = cols[j_speed_av].strip(),
        #device = cols[j_device].strip(),
        device = kwargs.get('device', None)
        if device in DEVICES:
            id_device = DEVICES[device]
        else:
            id_device = DEVICES['unknown']

        purpose = kwargs.get('purpose', None)
        if purpose in TRIPPUROPSES:
            id_purpose = TRIPPUROPSES[purpose]
        else:
            id_purpose = TRIPPUROPSES['unknown']

        id_trip = self.add_row(ids_sumo=kwargs.get('id_sumo', None),
                               ids_vtype=kwargs.get('id_vtype', None),
                               timestamps=kwargs.get('timestamp', None),
                               distances_gps=kwargs.get('distance_gps', None),
                               durations_gps=kwargs.get('duration_gps', None),
                               speeds_average=kwargs.get('speed_average', None),
                               speeds_max=kwargs.get('speed_max', None),
                               ids_purpose=id_purpose,
                               ids_device=id_device,
                               ids_points=kwargs.get('ids_point', None),
                               )
        return id_trip

    def set_points(self, id_trip, ids_point):
        self.ids_points[id_trip] = ids_point

    def get_ids_selected(self):
        return self.select_ids(self.are_selected.get_value())

    def route_shortest(self, dist_modespecific=5.0, c_modespecific=0.9,
                       is_ignor_connections=False):
        """
        Shortest path routint.
        """
        print 'route_shortest', dist_modespecific, c_modespecific
        # TODO: if too mant vtypes, better go through id_modes
        exectime_start = time.clock()
        scenario = self.parent.get_scenario()
        net = scenario.net
        edges = net.edges
        vtypes = scenario.demand.vtypes
        routes = self.get_routes()
        #ids_edges = []
        #ids_trip = []
        #costs = []

        distancesmap = self.parent.get_distancesmap()
        accesslevelsmap = self.parent.get_accesslevelsmap()

        # delete current
        #ids_with_shortes = self.select_ids(np.logical_and(self.are_selected.get_value(), self.ids_route_shortest.get_value()>=0))
        # routes.del_rows(self.ids_route_shortest[ids_with_shortes])
        #self.ids_route_shortest[ids_with_shortes] = -1

        fstar = edges.get_fstar(is_ignor_connections=is_ignor_connections)
        for id_vtype in self.get_vtypes():
            id_mode = vtypes.ids_mode[id_vtype]

            # no routing for pedestrians
            if id_mode != net.modes.get_id_mode('pedestrian'):
                dists_orig = distancesmap[id_mode].copy()
                weights = dists_orig.copy()

                # this will subtract some meters dependent on
                # access-level of the edge
                accesslevels = accesslevelsmap[id_mode]
                ids_edge = edges.get_ids()
                are_valid = weights > 3*dist_modespecific
                weights[ids_edge] -= dist_modespecific * are_valid[ids_edge] * accesslevels[ids_edge]
                weights[ids_edge] *= 1 - (1-c_modespecific) * are_valid[ids_edge] * (accesslevels[ids_edge] == 2)

                ids_trip_vtype = self.select_ids(np.logical_and(self.ids_vtype.get_value(
                ) == id_vtype, self.are_selected.get_value(), self.ids_route_matched.get_value() >= 0))
                #ids_trip_vtype = self.get_trips_for_vtype(id_vtype)
                # print '  id_vtype,id_mode',id_vtype,id_mode#,ids_trip_vtype
                # print '  weights',weights

                #ids_edge_depart = self.ids_edge_depart[ids_trip_vtype]
                #ids_edge_arrival = self.ids_edge_arrival[ids_trip_vtype]

                for id_trip, id_route in zip(ids_trip_vtype, self.ids_route_matched[ids_trip_vtype]):
                    route_matched = routes.ids_edges[id_route]

                    cost, route = routing.get_mincostroute_edge2edge(route_matched[0],
                                                                     route_matched[-1],
                                                                     weights=weights,
                                                                     fstar=fstar)
                    if len(route) > 0:
                        # ids_edges.append(route)
                        # ids_trip.append(id_trip)
                        # costs.append(cost)

                        id_route = self.ids_route_shortest[id_trip]
                        if id_route >= 0:
                            # there is already a previous shortest route
                            routes.set_row(id_route,
                                           #ids_edges = list(route),
                                           costs=cost,
                                           colors=COLOR_SHORTEST_ROUTE,
                                           )
                            # TODO!!! this assigment does not work in set_row!!!!
                            # tales only first edge !! why??
                            routes.ids_edges[id_route] = route
                            # print '  old route',id_route,type(route)
                        else:
                            # create new route
                            id_route = routes.add_row(ids_trip=id_trip,
                                                      ids_edges=route,
                                                      costs=cost,
                                                      colors=COLOR_SHORTEST_ROUTE,
                                                      )
                            self.ids_route_shortest[id_trip] = id_route
                            # print '  new route',type(route)

                        self.lengths_route_shortest[id_trip] = np.sum(dists_orig[route])
                        # print '  route', route
                        # print '  routes.ids_edges' ,routes.ids_edges[id_route]

        print '  exectime', time.clock()-exectime_start

    def get_speedprofile(self, id_trip):

        points = self.parent.points
        ids_point = self.ids_points[id_trip]
        ids_point_edgeend = self.ids_points_edgeend[id_trip]
        ids_edge = self.get_routes().ids_edges[self.ids_route_matched[id_trip]]
        id_edge_current = -1
        n_edges = len(ids_edge)

        positions_gps = []
        times_gps = []
        ids_edges_profile = []
        ids_nodes_profile = []

        for id_point, coord, timestamp in zip(ids_point, points.coords[ids_point], points.timestamps[ids_point]):
            #get_pos_from_coord(id_edge_current, coord)

            if id_point in ids_point_edgeend:
                ind = ids_point_edgeend.index(id_point)
                if ind == n_edges-1:
                    id_edge_current = -1
                else:
                    id_edge_current = ids_edge[ind+1]

    def get_flows(self, is_shortest_path=False):
        """
        Determine the total number of vehicles for each edge.
        returns ids_edge and flows
        """
        ids_edges = self.get_routes.ids_edges
        counts = np.zeros(np.max(self.get_net().edges.get_ids())+1, int)

        ids_trip = self.get_ids_selected()
        if not is_shortest_path:
            ids_route = self.ids_route_matched[ids_trip]
        else:
            ids_route = self.ids_route_shortest[ids_trip]
        inds_valid = np.flatnonzero(ids_route > 0)
        for id_trip, id_route in zip(ids_trip[inds_valid], ids_route[inds_valid]):
            counts[ids_edges[id_route][:]] += 1

        ids_edge = np.flatnonzero(counts)

        return ids_edge, counts[ids_edge].copy()

    def get_ids_route_selected(self):
        # TODO: here we could append direct routes
        # print 'get_ids_route_selected'
        ids_route_matched = self.ids_route_matched[self.get_ids_selected()]
        ids_route_shortest = self.ids_route_shortest[self.get_ids_selected()]
        ids_route = np.concatenate([ids_route_matched[ids_route_matched >= 0],
                                    ids_route_shortest[ids_route_shortest >= 0]])
        # print '  ids_route_matched.dtype',ids_route_matched.dtype
        # print '  ids_route_shortest.dtype',ids_route_shortest.dtype
        # print '  ids_route_matched[ids_route_matched >= 0] ',ids_route_matched[ids_route_matched >= 0]
        return np.concatenate([ids_route_matched[ids_route_matched >= 0], ids_route_shortest[ids_route_shortest >= 0]])
        # return  ids_route_matched[ids_route_matched >= 0]


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
                          version=0.2,
                          **kwargs)

        self._init_attributes()

    def _init_attributes(self):
        trips = self.parent.trips

        # TODO: add/update vtypes here
        self.add_col(SumoIdsConf('User', xmltag='id'))

        self.add_col(am.ArrayConf('ids_gender', default=-1,
                                  dtype=np.int32,
                                  groupnames=['parameters'],
                                  choices=GENDERS,
                                  name='Gender',
                                  info='Gender of person.',
                                  ))

        self.add_col(am.ArrayConf('years_birth', default=-1,
                                  dtype=np.int32,
                                  groupnames=['parameters'],
                                  name='Birth year',
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
                                  dtype=np.bool,
                                  groupnames=['parameters'],
                                  name='frequent user',
                                  info='If true, this person is a frequent user of the recorded transport mode.',
                                  ))

        # change from int ti string
        print 'GpsPersons.versio,', self.get_version(), self.get_version() < 0.2

        if hasattr(self, 'zips'):
                # print '  zips',self.zips.get_value().dtype,self.zips.get_value()
            if self.zips.get_value().dtype in [np.dtype(np.int32), np.dtype(np.int64)]:
                print 'WARNING: delete old person.zips'
                self.delete('zips')

        # if self.get_version()<0.2:
        #    #if hasattr(self,'zips'):
        #   self.delete('zips')

        self.add_col(am.ArrayConf('zips', '',
                                  dtype=np.object,
                                  groupnames=['parameters'],
                                  name='ZIP',
                                  info="ZIP code of person's home.",
                                  ))

        self.add_col(am.IdlistsArrayConf('ids_trips', trips,
                                         #groupnames = ['_private'],
                                         name='Trip IDs',
                                         info="IDs of trips made by this vehicle. This is a collection of recorded trips associated with this person.",
                                         ))

        self.add_col(am.ArrayConf('numbers_tot_trip_gps', 0,
                                  dtype=np.int32,
                                  groupnames=['results'],
                                  name='tot. trips',
                                  symbol='N tot GPS',
                                  info='Total number of recorded GPS traces.',
                                  ))

        self.add_col(am.ArrayConf('lengths_tot_route_gps', 0.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='tot. GPS length',
                                  symbol='L tot GPS',
                                  unit='m',
                                  info='Total distances of recorded GPS traces.',
                                  ))
        self.add_col(am.ArrayConf('times_tot_route_gps', 0.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Tot. GPS time',
                                  symbol='T tot GPS',
                                  unit='s',
                                  info='Total trip times of recorded GPS traces.',
                                  ))

        self.add_col(am.ArrayConf('speeds_av_gps', 0.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Tot. GPS time',
                                  symbol='V avg GPS',
                                  unit='m/s',
                                  info='Average speed of recorded GPS traces.',
                                  ))

        self.add_col(am.ArrayConf('numbers_tot_trip_mached', 0,
                                  dtype=np.int32,
                                  groupnames=['results'],
                                  name='tot. trips',
                                  symbol='N tot match',
                                  info='Total number of recorded GPS traces.',
                                  ))

        self.add_col(am.ArrayConf('lengths_tot_route_matched', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Tot. matched length',
                                  symbol='L tot match',
                                  unit='m',
                                  info='Total length of the matched parts of the GPS traces, measured by summing the length of edges of the matched route. Note the only a fraction of the GPS trace ma be within the given network.',
                                  ))

        # self.add_col(am.ArrayConf('times_tot_route_matched', default = -1.0,
        #                            dtype = np.float32,
        #                            groupnames = ['results'],
        #                            name = 'Tot. matched duration',
        #                            symbol = 'T tot match',
        #                            unit = 's',
        #                            info = 'Total duration of all matched part of the GPS trace. This is the difference in timestamps between last and first GPS point of the matched route. Note the only a fraction of the GPS trace ma be within the given network.',
        #                            ))

        self.add_col(am.ArrayConf('lengths_tot_route_shortest', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Tot. shortest length',
                                  symbol='L tot short',
                                  unit='m',
                                  info='Total length of the shortest routes.  Shortest route is connecting the first matched edge and the final matched edge.',
                                  ))

        self.add_col(am.ArrayConf('lengths_tot_route_matched_mixed', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Tot. matched length mixed access',
                                  symbol='L tot match mix',
                                  unit='m',
                                  info='Total length of the matched part of the GPS traces. Note the only a fraction of the GPS trace ma be within the given network.',
                                  ))

        self.add_col(am.ArrayConf('lengths_tot_route_matched_exclusive', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Matched length exclusive access',
                                  symbol='L tot match excl',
                                  unit='m',
                                  info='Length of the matched part of the GPS trace. Note the only a fraction of the GPS trace ma be within the given network.',
                                  ))

        # upgrade
        if hasattr(self, 'ids_genders'):
            self.delete('ids_genders')

    def analyze(self):
        print 'Persons.analyze'
        # ids_person = self.select_ids(self.)
        trips = self.parent.trips
        points = self.parent.points
        npsum = np.sum

        ids_person = self.get_ids()
        n_pers = len(ids_person)

        numbers_tot_trip_gps = np.zeros(n_pers, dtype=np.int32)
        lengths_tot_route_gps = np.zeros(n_pers, dtype=np.float32)
        times_tot_route_gps = np.zeros(n_pers, dtype=np.float32)
        speeds_av_gps = np.zeros(n_pers, dtype=np.float32)
        numbers_tot_trip_mached = np.zeros(n_pers, dtype=np.int32)
        lengths_tot_route_matched = np.zeros(n_pers, dtype=np.float32)
        times_tot_route_matched = np.zeros(n_pers, dtype=np.float32)
        lengths_tot_route_shortest = np.zeros(n_pers, dtype=np.float32)
        lengths_tot_route_matched_mixed = np.zeros(n_pers, dtype=np.float32)
        lengths_tot_route_matched_exclusive = np.zeros(n_pers, dtype=np.float32)

        timebudgets = -1*np.ones(n_pers, dtype=np.float32)

        i = 0
        for id_person, ids_trip_all in zip(ids_person, self.ids_trips[ids_person]):
            # get selected trips only
            ids_trip = np.array(ids_trip_all)[trips.are_selected[ids_trip_all]]

            numbers_tot_trip_gps[i] = len(ids_trip)
            # print '  id_person, ids_trip',id_person, ids_trip
            if len(ids_trip) > 0:

                length_tot = npsum(trips.distances_gps[ids_trip])
                time_tot = npsum(trips.durations_gps[ids_trip])

                lengths_tot_route_gps[i] = length_tot
                times_tot_route_gps[i] = time_tot
                if time_tot > 0:
                    speeds_av_gps[i] = length_tot/time_tot

                # do statistics on matched trips
                # print '    ids_route_shortest',trips.ids_route_shortest[ids_trip]
                # print '    inds',trips.ids_route_shortest[ids_trip]>=0
                ids_trip_matched = ids_trip[trips.ids_route_shortest[ids_trip] >= 0]
                if len(ids_trip_matched) > 0:
                    numbers_tot_trip_mached[i] = len(ids_trip_matched)
                    lengths_tot_route_matched[i] = npsum(trips.lengths_route_matched[ids_trip_matched])
                    #times_tot_route_matched[i] = npsum(trips.lengths_route_matched[ids_trip])
                    lengths_tot_route_shortest[i] = npsum(trips.lengths_route_shortest[ids_trip_matched])
                    lengths_tot_route_matched_mixed[i] = npsum(trips.lengths_route_matched_mixed[ids_trip_matched])
                    lengths_tot_route_matched_exclusive[i] = npsum(
                        trips.lengths_route_matched_exclusive[ids_trip_matched])

                # do statistics time budget
                # coord0 = None#points.coords[ids_points[ids_trip[0]]]
                # date = None# points.timestamps[ids_points[ids_trip[0]]]
                # for ids_point in trips.ids_points[ids_trip]:
                #    coord0 = None#points.coords[ids_points[ids_trip[0]]]
                #    date = None# points.timestamps[ids_points[ids_trip[0]]]

            i += 1

        self.set_rows(ids_person,
                      numbers_tot_trip_gps=numbers_tot_trip_gps,
                      lengths_tot_route_gps=lengths_tot_route_gps,
                      times_tot_route_gps=times_tot_route_gps,
                      speeds_av_gps=speeds_av_gps,
                      numbers_tot_trip_mached=numbers_tot_trip_mached,
                      lengths_tot_route_matched=lengths_tot_route_matched,
                      times_tot_route_matched=times_tot_route_matched,
                      lengths_tot_route_shortest=lengths_tot_route_shortest,
                      lengths_tot_route_matched_mixed=lengths_tot_route_matched_mixed,
                      lengths_tot_route_matched_exclusive=lengths_tot_route_matched_exclusive,
                      )

    def make(self, id_sumo, **kwargs):
        print 'make id_pers_sumo', id_sumo

        id_trip = kwargs.get('id_trip', -1)
        if self.ids_sumo.has_index(id_sumo):
            # person exisis
            id_pers = self.ids_sumo.get_id_from_index(id_sumo)

            if id_trip >= 0:
                self.ids_trips[id_pers].append(id_trip)

            # debug
            # print '  exists id_pers',id_pers,self.ids_sumo[id_pers]
            #trips = self.parent.trips
            # for id_trip in self.ids_trips[id_pers]:
            #    print '    id_trip',id_trip,trips.ids_sumo[id_trip]

            #self.set_row(id_pers, **kwargs)
            return id_pers

        else:
            # print 'make new person',kwargs
            # add new person
            # if id_trip >= 0:
            #    ids_trip = [id_trip]
            # else:
            #    ids_trip = []

            gender = kwargs.get('gender', '').lower()
            if gender == 'm':
                # print '  m gender=*%s*'%gender
                id_gender = GENDERS['male']
            elif gender == 'f':
                # print '  f gender=*%s*'%gender
                id_gender = GENDERS['female']
            else:
                # print '  u gender=*%s*'%gender
                id_gender = GENDERS['unknown']

            occupation = kwargs.get('occupation', '')
            occupation = occupation.lower()
            if occupation in ['None', '', 'unknown']:
                id_occupation = OCCUPATIONS['unknown']
            elif occupation in OCCUPATIONS:
                id_occupation = OCCUPATIONS[occupation]
            else:
                id_occupation = OCCUPATIONS['other']

            id_pers = self.add_row(ids_sumo=id_sumo,
                                   ids_gender=id_gender,
                                   years_birth=kwargs.get('year_birth', None),
                                   ids_occupation=id_occupation,
                                   are_frequent_user=kwargs.get('is_frequent_user', None),
                                   zips=kwargs.get('zip', None),
                                   )
            if id_trip >= 0:
                self.ids_trips[id_pers] = [id_trip]
            else:
                self.ids_trips[id_pers] = []

            # debug
            # print '  made id_pers',id_pers, self.ids_sumo[id_pers]
            #trips = self.parent.trips
            # for id_trip in self.ids_trips[id_pers]:
            #    print '    id_trip',id_trip,trips.ids_sumo[id_trip]

            return id_pers


class Mapmatching(DemandobjMixin, cm.BaseObjman):
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
        self._init_constants()

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
        self._distancesmap = None
        self._accesslevelsmap = None

        attrsman = self.get_attrsman()
        attrsman.do_not_save_attrs(['_segvertices_xy', '_proj', '_distancesmap', '_accesslevelsmap'])

    def clear_routes(self):
        self.trips.clear_routes()

    def delete_unselected_trips(self):
        trips = self.trips
        points = self.points
        persons = self.persons

        if len(persons) == 0:
                # no persons, just trips
            ids_del = trips.select_ids(np.logical_not(trips.are_selected.get_value()))

            for ids_point in trips.ids_points[ids_del]:
                if ids_point is not None:
                    points.del_rows(ids_point)
            trips.del_rows(ids_del)
        else:
            for ids_trip in persons.ids_trips[persons.get_ids()]:
                if ids_trip is not None:
                    ids_del = np.array(ids_trip, dtype=np.int32)[np.logical_not(trips.are_selected[ids_trip])]
                    for id_del, ids_point in zip(ids_del, trips.ids_points[ids_del]):
                        ids_trip.remove(id_del)
                        if ids_point is not None:
                            points.del_rows(ids_point)

                    trips.del_rows(ids_del)

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

    def get_distancesmap(self, is_check_lanes=False):
        """
        Returns a dictionary where key is id_mode and
        value is a distance-lookup table, mapping id_edge to edge distance
        """
        # print 'get_distancesmap',self._distancesmap is None

        if self._distancesmap is None:

            vtypes = self.get_scenario().demand.vtypes
            edges = self.get_scenario().net.edges
            ids_vtype = self.trips.ids_vtype[self.trips.get_ids_selected()]
            ids_mode = vtypes.ids_mode[ids_vtype]
            # print '    ids_mode',ids_mode

            self._distancesmap = {}
            for id_mode in set(ids_mode):
                #ids_vtype_mode = vtypes.select_by_mode(id_mode)

                self._distancesmap[id_mode] = edges.get_distances(id_mode=id_mode,
                                                                  is_check_lanes=is_check_lanes,
                                                                  )

        # print '  len(self._distancesmap)',len(self._distancesmap)
        return self._distancesmap

    def get_accesslevelsmap(self):
        """
        Returns a dictionary where key is id_mode and
        value is a distance-lookup table, mapping id_edge to edge distance
        """

        if self._accesslevelsmap is None:
            vtypes = self.get_scenario().demand.vtypes
            edges = self.get_scenario().net.edges
            ids_vtype = self.trips.ids_vtype[self.trips.get_ids_selected()]
            ids_mode = vtypes.ids_mode[ids_vtype]

            self._accesslevelsmap = {}
            for id_mode in set(ids_mode):
                #ids_vtype_mode = vtypes.select_by_mode(id_mode)
                self._accesslevelsmap[id_mode] = edges.get_accesslevels(id_mode)
        return self._accesslevelsmap


class Matchresults(cm.BaseObjman):
    def __init__(self, ident, mapmatching,
                 name='Mapmatching results',
                 info='Results of mapmatching analysis.',
                 **kwargs):

        # make results a child of process or of wxgui
        # use these objects to access matched trips

        self._init_objman(ident, parent=mapmatching, name=name,
                          info=info, **kwargs)
        attrsman = self.set_attrsman(cm.Attrsman(self))

        self._init_attributes()

    def _init_attributes(self):
        attrsman = self.get_attrsman()
        mapmatching = self.parent
        # self.routesresults = attrsman.add(cm.ObjConf( Routesresults('routesresults',
        #                                                        self, mapmatching.trips.routes),
        #                                            groupnames = ['Route results'],
        #                                            ))

        # add trip results from all demand objects
        print 'Matchresults._init_attributes'

    def config(self, resultobj, **kwargs):
        # attention: need to check whether already set
        # because setattr is set explicitely after add
        if not hasattr(self, resultobj.get_ident()):
            if kwargs.has_key('groupnames'):
                kwargs['groupnames'].append('Results')
            else:
                kwargs['groupnames'] = ['Results']
            attrsman = self.get_attrsman()
            attrsman.add(cm.ObjConf(resultobj, **kwargs))
            setattr(self, resultobj.get_ident(), resultobj)

    def get_scenario(self):
        return self.parent.get_scenario()

    def clear_all(self):
        self.clear()

    def save(self, filepath=None, is_not_save_parent=True):
        if filepath is None:
            self.get_scenario().get_rootfilepath()+'.mmatch.obj'
        # parent will not be saved because in no_save set
        cm.save_obj(self, filepath, is_not_save_parent=is_not_save_parent)


class Nodeinfolists(am.ListArrayConf):
    def format_value(self, _id, show_unit=False, show_parentesis=False):
        if show_unit:
            unit = ' '+self.format_unit(show_parentesis)
        else:
            unit = ''
        # return repr(self[_id])+unit

        #self.min = minval
        #self.max = maxval
        #self.digits_integer = digits_integer
        #self.digits_fraction = digits_fraction
        val = self[_id]
        tt = type(val)

        if tt in (np.int, np.int32, np.float64):
            return str(val)+unit

        elif tt in (np.float, np.float32, np.float64):
            if hasattr(self, 'digits_fraction'):
                digits_fraction = self.digits_fraction
            else:
                digits_fraction = 3
            s = "%."+str(digits_fraction)+"f"
            return s % (val)+unit

        else:
            return str(val)+unit


class Accesslevellists(am.ListArrayConf):
    """
    NOT IN USE
    """

    def format_value(self, _id, show_unit=False, show_parentesis=False):
        if show_unit:
            unit = ' '+self.format_unit(show_parentesis)
        else:
            unit = ''
        # return repr(self[_id])+unit

        #self.min = minval
        #self.max = maxval
        #self.digits_integer = digits_integer
        #self.digits_fraction = digits_fraction
        val = self[_id]
        tt = type(val)

        if tt in (np.int, np.int32, np.float64):
            return str(val)+unit

        elif tt in (np.float, np.float32, np.float64):
            if hasattr(self, 'digits_fraction'):
                digits_fraction = self.digits_fraction
            else:
                digits_fraction = 3
            s = "%."+str(digits_fraction)+"f"
            return s % (val)+unit

        else:
            return str(val)+unit

    def format_value(self, _id, show_unit=False, show_parentesis=False):
        if show_unit:
            unit = ' '+self.format_unit(show_parentesis)
        else:
            unit = ''
        # return repr(self[_id])+unit

        #self.min = minval
        #self.max = maxval
        #self.digits_integer = digits_integer
        #self.digits_fraction = digits_fraction
        val = self[_id]
        tt = type(val)

        if tt in (np.int, np.int32, np.float64):
            return str(val)+unit

        elif tt in (np.float, np.float32, np.float64):
            if hasattr(self, 'digits_fraction'):
                digits_fraction = self.digits_fraction
            else:
                digits_fraction = 3
            s = "%."+str(digits_fraction)+"f"
            return s % (val)+unit

        else:
            return str(val)+unit


class Edgesresults(am.ArrayObjman):
    def __init__(self, ident, parent,
                 name='Edges results',
                 info='Table with results from edges that are part of matched routes or alternative routes.',
                 **kwargs):

        self._init_objman(ident=ident,
                          parent=parent,  # main results object
                          info=info,
                          name=name,
                          **kwargs)

        # self.add(cm.AttrConf(  'datapathkey',datapathkey,
        #                        groupnames = ['_private'],
        #                        name = 'data pathkey',
        #                        info = "key of data path",
        #                        ))

        self.add_col(am.IdsArrayConf('ids_edge', parent.get_scenario().net.edges,
                                     groupnames=['state'],
                                     is_index=True,
                                     name='Edge ID',
                                     info='ID of network edge.',
                                     ))
        self._init_attributes()

    def _init_attributes(self):

        self.add_col(am.ArrayConf('speeds_average', default=0.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Avg. speed',
                                  unit='m/s',
                                  info='Average speed on this edge.',
                                  ))

        self.add_col(am.ArrayConf('durations_tot_matched', default=0.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Tot. duration',
                                  unit='s',
                                  info='Total time of matched routes spent on this edge.',
                                  ))

        self.add_col(am.ArrayConf('numbers_tot_matched', default=0,
                                  dtype=np.int32,
                                  groupnames=['results'],
                                  name='number matched',
                                  info='Total number of matched routes crossing this edge.',
                                  ))

        self.add_col(am.ArrayConf('numbers_tot_shortest', default=0,
                                  dtype=np.int32,
                                  groupnames=['results'],
                                  name='number shortest',
                                  info='Total number of shortest routes crossing this edge.',
                                  ))

        self.add_col(am.ArrayConf('differences_dist_tot_shortest', default=0.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Tot. length difference',
                                  unit='m',
                                  info='Sum of length differences between matched route length and the corrisponding shortest route using this edge.',
                                  ))

        self.add_col(am.ArrayConf('probabilities_tot_matched', default=0.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='trip probab.',
                                  info='The probability that the edge has been used by the totality of all trips.',
                                  ))

    def init_for_routes(self, routes):
        """
        Initializes a row for each edge ID in routes
        """
        ids_edge_init = set()
        for ids_edge in routes.ids_edges.get_value():
            if ids_edge is not None:
                ids_edge_init.update(ids_edge)
        ids_edge_init = list(ids_edge_init)
        # return result ids and respective edge ids
        ids_edgeresmap = np.zeros(np.max(ids_edge_init)+1, dtype=np.int32)
        ids_edgeresmap[ids_edge_init] = self.add_rows(n=len(ids_edge_init), ids_edge=ids_edge_init)

        return ids_edgeresmap


class Routesresults(am.ArrayObjman):
    def __init__(self, ident, parent,
                 name='Route results',
                 info='Table with results from analysis of each matched route.',
                 **kwargs):

        self._init_objman(ident=ident,
                          parent=parent,  # main results object
                          info=info,
                          name=name,
                          **kwargs)

        # self.add(cm.AttrConf(  'datapathkey',datapathkey,
        #                        groupnames = ['_private'],
        #                        name = 'data pathkey',
        #                        info = "key of data path",
        #                        ))

        self.add_col(am.IdsArrayConf('ids_route', parent.parent.trips.get_routes(),
                                     groupnames=['state'],
                                     is_index=True,
                                     name='ID route',
                                     info='ID of route.',
                                     ))
        self._init_attributes()

    def _init_attributes(self):

        self.add_col(am.ArrayConf('distances', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='distance',
                                  unit='m',
                                  info='Length of the route.',
                                  ))

        self.add_col(am.ArrayConf('durations', default=0.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='duration',
                                  unit='s',
                                  info='Time duration of the route.',
                                  ))

        self.add_col(am.ArrayConf('lengths_mixed', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Length mixed access',
                                  symbol='L mix',
                                  unit='m',
                                  info='Length of roads with mixed access.',
                                  ))

        self.add_col(am.ArrayConf('lengths_exclusive', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Length exclusive access',
                                  symbol='L excl',
                                  unit='m',
                                  info='Length of roads with exclusive access.',
                                  ))

        self.add_col(am.ArrayConf('lengths_low_priority', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Length low priority',
                                  symbol='L lowprio',
                                  unit='m',
                                  info='Length of low priority roads. These are roads with either speed limit to 30 or below, or classified as residential, or exclusive bike or pedestrian ways. This correspond to SUMO priorities 1-6.',
                                  ))

        self.add_col(am.ArrayConf('lengths_overlap_matched', default=-1.0,
                                  dtype=np.float32,
                                  groupnames=['results'],
                                  name='Length overlap',
                                  symbol='L overl',
                                  unit='m',
                                  info='Length of overlap with matched route.',
                                  ))

        self.add_col(am.ArrayConf('numbers_nodes', default=-1,
                                  dtype=np.int32,
                                  groupnames=['results'],
                                  name='number of nodes',
                                  symbol='N nodes',
                                  info='Total number of nodes.',
                                  ))

        self.add_col(am.ArrayConf('numbers_nodes_tls', default=-1,
                                  dtype=np.int32,
                                  groupnames=['results'],
                                  name='number of TL nodes',
                                  symbol='N TL',
                                  info='Total number of traffic light controlled nodes.',
                                  ))

        self.add_col(am.ArrayConf('numbers_prioritychange', default=-1,
                                  dtype=np.int32,
                                  groupnames=['results'],
                                  name='number of prio. change',
                                  symbol='N prio',
                                  info='Total number of change in road priority.',
                                  ))

        # self.add_col(am.ListArrayConf( 'intervallists',
        #                                    groupnames = ['_private'],
        #                                    perm='rw',
        #                                    name = 'Intervals',
        #                                    unit = 's',
        #                                    info = 'Time interval as tuple for each edge of the route. These time intervals are obtained from timestamps of GPS points which can be perpendicularly projected to the respective route edge.',
        #                                    ))

        # self.add_col(am.ListArrayConf( 'edgedurationlists',
        #                                    groupnames = ['_private'],
        #                                    perm='rw',
        #                                    name = 'Edge durations',
        #                                    unit = 's',
        #                                    info = 'Duration on each edge of the route. These times are obtained from timestamps of GPS points which can be perpendicularly projected to the respective route edge.',
        #                                    ))

        # self.add_col(am.ListArrayConf( 'nodedurationlists',
        #                                    groupnames = ['_private'],
        #                                    perm='rw',
        #                                    name = 'Node durations',
        #                                    unit = 's',
        #                                    info = 'Duration on each intersection of the route. These times are obtained from timestamps of GPS points on the edges before and after the respective node.',
        #                                    ))

        # self.add_col(Nodeinfolists( 'nodeinfolists',
        #                                    groupnames = ['_private'],
        #                                    perm='rw',
        #                                    name = 'Node info',
        #                                    info = 'Node info for each node on the route. Node info is defined as (node type, number of legs, accesslevel) and  for traffic light nodes (node type, total cycle time, green time).',
        #                                   ))

        # self.nodetypemap = {\
        #                                "priority":0,
        #                                "traffic_light":1,
        #                                "right_before_left":2,
        #                                "unregulated":3,
        #                                "priority_stop":4,
        #                                "traffic_light_unregulated":5,
        #                                "allway_stop":6,
        #                                "rail_signal":7,
        #                                "zipper":8,
        #                                "traffic_light_right_on_red":9,
        #                                "rail_crossing":10,
        #                                "dead_end":11,
        #                                }

        # self.add_col(Accesslevellists( 'accesslevellists',
        #                                    groupnames = ['_private'],
        #                                    perm='rw',
        #                                    name = 'Accesslevels',
        #                                    info = 'Accesslevels for each edge of the route. -1 = no access, 0 = all have access, 1 = reserved access together with other modes, 2 = exclusive access.',
        #                                    ))


class Shortestrouter(Process):
    def __init__(self, ident, mapmatching,  logger=None, **kwargs):
        print 'Shortestrouter.__init__'

        # TODO: let this be independent, link to it or child??

        self._init_common(ident,
                          parent=mapmatching,
                          name='Shortest Router',
                          logger=logger,
                          info='Shortest path router.',
                          )

        attrsman = self.set_attrsman(cm.Attrsman(self))

        self.c_modespecific = attrsman.add(cm.AttrConf('c_modespecific', kwargs.get('c_modespecific', 0.9),
                                                       groupnames=['options'],
                                                       perm='rw',
                                                       name='Mode constant',
                                                       info='The Mode constant is multiplied with the the distance of each edge if vehicle has exclusive access.',
                                                       ))

        self.dist_modespecific = attrsman.add(cm.AttrConf('dist_modespecific', kwargs.get('dist_modespecific', 0.0),
                                                          groupnames=['options'],
                                                          perm='rw',
                                                          name='Mode dist',
                                                          unit='m',
                                                          info='The mode distance is multiplied with the accesslevel and added to the distance of each edge.',
                                                          ))

        self.is_ignor_connections = attrsman.add(cm.AttrConf('is_ignor_connections', kwargs.get('is_ignor_connections', True),
                                                             groupnames=['options'],
                                                             perm='rw',
                                                             name='Ignore connections',
                                                             info='Ignore connections means that the vehicle can always make turns into all possible edges, whether they are allowed or not.',
                                                             ))

    def do(self):
        print 'Shortestrouter.do'
        # links
        mapmatching = self.parent
        trips = mapmatching.trips
        trips.route_shortest(dist_modespecific=self.dist_modespecific,
                             c_modespecific=self.c_modespecific,
                             is_ignor_connections=self.is_ignor_connections,
                             )


class Routesanalyzer(Process):
    def __init__(self, ident, mapmatching, results=None,  logger=None, **kwargs):
        print 'Routesanalyzer.__init__'

        # TODO: let this be independent, link to it or child??

        if results is None:
            self._results = Matchresults('matchresults', mapmatching)
        else:
            self._results = results

        self._init_common(ident,
                          parent=mapmatching,
                          name='Routes Analyzer',
                          logger=logger,
                          info='Determines attributes of matched route and alternative routes.',
                          )

        attrsman = self.set_attrsman(cm.Attrsman(self))

        self.priority_max_low = attrsman.add(cm.AttrConf('priority_max_low', kwargs.get('priority_max_low', 6),
                                                         groupnames=['options'],
                                                         perm='rw',
                                                         name='Max. low priority',
                                                         info='Maximum priority of an edge, such that it is still considered low priority.',
                                                         ))

        self.timeloss_intersection = attrsman.add(cm.AttrConf('timeloss_intersection', kwargs.get('timeloss_intersection', 5.0),
                                                              groupnames=['options'],
                                                              perm='rw',
                                                              name='Timeloss intersection',
                                                              info='Estimated timeloss at intersections due to waiting or reduction of speed. This value is used to estimate travel time of route alternatives.',
                                                              ))

        self.timeloss_tl = attrsman.add(cm.AttrConf('timeloss_tl', kwargs.get('timeloss_tl', 15.0),
                                                    groupnames=['options'],
                                                    perm='rw',
                                                    name='Timeloss TL',
                                                    info='Additional estimated timeloss at traffic light intersections due to waiting at the red light. This value is used to estimate travel time of route alternatives.',
                                                    ))

        self.edgedurations_tot_min = attrsman.add(cm.AttrConf('edgedurations_tot_min', kwargs.get('edgedurations_tot_min', 10.0),
                                                              groupnames=['options'],
                                                              perm='rw',
                                                              name='min tot. edge duration',
                                                              unit='s',
                                                              info='Minimum total duration of all users in an edge in order to calculate average speed. A too small value distors the results.',
                                                              ))

        self.edgespeed_max = attrsman.add(cm.AttrConf('edgespeed_max', kwargs.get('edgespeed_max', 14.0),
                                                      groupnames=['options'],
                                                      perm='rw',
                                                      name='Max edge speed',
                                                      unit='m/s',
                                                      info='Edge speed estimates will be clipped at this speed.',
                                                      ))

        results = self.get_results()
        results.config(Routesresults('routesresults_matched', results,
                                     name='Results of matched routes'), name='Results of matched routes')
        results.config(Routesresults('routesresults_shortest', results,
                                     name='Results of shortest routes'), name='Results of shortest routes')
        results.config(Routesresults('routesresults_matched_nonoverlap', results, name='Results of matched, non-overlapping routes'),
                       name='Results of matched routes using only edges which do not overlap with the shortest route.')
        results.config(Routesresults('routesresults_shortest_nonoverlap', results, name='Results of shortest, non-overlapping routes'),
                       name='Results of shortest routes using only edges which do not overlap with the matched route.')

        results.config(Edgesresults('edgesresults', results))

    def get_results(self):
        return self._results

    def do(self):
        print 'Routesanalyzer.do'
        # results
        results = self.get_results()
        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched

        routesresults_shortest_nonoverlap = results.routesresults_shortest_nonoverlap
        routesresults_matched_nonoverlap = results.routesresults_matched_nonoverlap

        edgesresults = results.edgesresults

        # links
        mapmatching = self.parent
        trips = mapmatching.trips

        routes = trips.get_routes()
        scenario = mapmatching.get_scenario()
        edges = scenario.net.edges
        vtypes = scenario.demand.vtypes
        points = mapmatching.points
        timestamps = points.timestamps

        priority_max_low = self.priority_max_low
        timeloss_intersection = self.timeloss_intersection
        timeloss_tl = self.timeloss_tl

        distancesmap = mapmatching.get_distancesmap()
        accesslevelsmap = mapmatching.get_accesslevelsmap()
        priorities = edges.priorities
        ids_tonode = edges.ids_tonode
        nodetypes = scenario.net.nodes.types
        tlstype = nodetypes.choices['traffic_light']
        ids_tls = scenario.net.nodes.ids_tls

        ids_trip_sel = trips.get_ids_selected()

        ids_route_sel = trips.ids_route_matched[ids_trip_sel]
        inds_valid = np.flatnonzero(ids_route_sel > 0)

        ids_route = ids_route_sel[inds_valid]
        ids_trip = ids_trip_sel[inds_valid]

        ids_route_shortest = trips.ids_route_shortest[ids_trip]

        ids_vtype = trips.ids_vtype[ids_trip]
        ids_mode = vtypes.ids_mode[ids_vtype]

        #ind = 0

        print '  analyzing %d trips' % len(ids_trip)

        routesresults_shortest.clear()
        routesresults_matched.clear()
        routesresults_shortest_nonoverlap.clear()
        routesresults_matched_nonoverlap.clear()

        edgesresults.clear()
        ids_res = routesresults_matched.add_rows(n=len(ids_trip))
        routesresults_shortest.add_rows(ids=ids_res)
        routesresults_matched_nonoverlap.add_rows(ids=ids_res)
        routesresults_shortest_nonoverlap.add_rows(ids=ids_res)

        ids_edgeresmap = edgesresults.init_for_routes(routes)

        for id_trip, id_route_matched, id_route_shortest, id_mode, id_res, dist_gps, duration_gps\
                in zip(ids_trip,
                       ids_route,
                       ids_route_shortest,
                       ids_mode,
                       ids_res,
                       trips.lengths_gpsroute_matched[ids_trip],
                       trips.durations_route_matched[ids_trip]):

            #ids_point = trips.ids_points[id_trip]
            distances = distancesmap[id_mode]

            ids_edge_matched = np.array(routes.ids_edges[id_route_matched][1:], dtype=np.int32)
            ids_points_edgeend = np.array(trips.ids_points_edgeend[id_trip], dtype=np.int32)  # [1:]
            accesslevels_matched = accesslevelsmap[id_mode][ids_edge_matched]
            priorities_matched = priorities[ids_edge_matched]
            nodetypes_matched = nodetypes[ids_tonode[ids_edge_matched]]

            # print '  analyzing id_trip',id_trip
            # print '    len(ids_edge_matched)',len(ids_edge_matched)
            # print '    len(ids_points_edgeend)',len(ids_points_edgeend)
            # print '    ids_edge_matched',ids_edge_matched
            # print '    accesslevels_matched',accesslevels_matched
            # print '    tlstype',tlstype
            # print '    accesslevels_mixed',accesslevels_matched==1
            # print '    accesslevels_mixed',np.flatnonzero(accesslevels_matched==1)
            # print '    distances',
            # print '    nodetypes_matched',nodetypes_matched
            if len(ids_points_edgeend) > 1:
                dist_matched = np.sum(distances[ids_edge_matched])
                duration_matched = timestamps[ids_points_edgeend[-1]] - timestamps[ids_points_edgeend[0]]
                n_nodes_matched = len(nodetypes_matched)
                n_tls_matched = np.sum(nodetypes_matched == tlstype)
                lineduration_matched = duration_matched-n_nodes_matched*timeloss_intersection-n_tls_matched*timeloss_tl
                linespeed_matched = dist_matched/lineduration_matched

            else:
                dist_matched = 0.0
                duration_matched = 0.0
                n_nodes_matched = 0
                n_tls_matched = 0
                lineduration_matched = 0.0
                linespeed_matched = 0.0

            # do edge by edge analyses if at least more than 1 edge
            if len(ids_edge_matched) > 1:

                # cut off last edge

                # print '  ids_edge_matched',ids_edge_matched.shape,ids_edge_matched
                # print '  ids_points_edgeend',ids_points_edgeend.shape,ids_points_edgeend
                ids_edge = ids_edge_matched[:-1]
                ids_points_ee = ids_points_edgeend  # [:-1]

                # print '  ids_edge',ids_edge.shape
                # print '  ids_points_ee',ids_points_ee[1:].shape,ids_points_ee[1:]
                durations_edge = timestamps[ids_points_ee[1:]] - timestamps[ids_points_ee[:-1]]
                # print '  durations_edge',durations_edge.shape,durations_edge
                edgesresults.numbers_tot_matched[ids_edgeresmap[ids_edge]] += 1
                edgesresults.durations_tot_matched[ids_edgeresmap[ids_edge]] += durations_edge
                #dists_edge = distances[ids_edge]
                # for id_edge, duration_edge, dists_edge, id_point_from, id_point_to  in zip(ids_edge,durations_edge,ids_points_ee[:1],ids_points_ee[1:]):
                #    edgesresults.numbers_tot_matched[id_edge] += 1
                #    edgesresults.durations_tot_matched[id_edge] += duration_edge
                # identify gps points with  id_point_from, id_point_to
                # and compute line speed, waiting time....

                routesresults_matched.set_row(id_res,
                                              ids_route=id_route_matched,
                                              distances=dist_matched,
                                              durations=duration_matched,
                                              lengths_mixed=np.sum(
                                                  distances[ids_edge_matched[accesslevels_matched == 1]]),
                                              lengths_exclusive=np.sum(
                                                  distances[ids_edge_matched[accesslevels_matched == 2]]),
                                              lengths_low_priority=np.sum(
                                                  distances[ids_edge_matched[priorities_matched <= priority_max_low]]),
                                              lengths_overlap_matched=np.sum(distances[ids_edge_matched]),
                                              numbers_nodes=n_nodes_matched,
                                              numbers_nodes_tls=n_tls_matched,
                                              numbers_prioritychange=np.sum(np.logical_not(
                                                  priorities_matched[:-1], priorities_matched[1:])),
                                              )

            if id_route_shortest >= 0:
                ids_edge_shortest = np.array(routes.ids_edges[id_route_shortest], dtype=np.int32)[1:]
                accesslevels_shortest = accesslevelsmap[id_mode][ids_edge_shortest]
                priorities_shortest = priorities[ids_edge_shortest]
                nodetypes_shortest = nodetypes[ids_tonode[ids_edge_shortest]]

                ids_edge_overlap = list(set(ids_edge_matched).intersection(ids_edge_shortest))
                ids_edge_shortest_nonoverlap = np.array(
                    list(set(ids_edge_shortest).difference(ids_edge_matched)), dtype=np.int32)
                ids_edge_match_nonoverlap = np.array(
                    list(set(ids_edge_matched).difference(ids_edge_shortest)), dtype=np.int32)
                dist_shortest = np.sum(distances[ids_edge_shortest])
                n_nodes_shortest = len(nodetypes_shortest)
                n_tls_shortest = np.sum(nodetypes_shortest == tlstype)

                edgesresults.numbers_tot_shortest[ids_edgeresmap[ids_edge_shortest]] += 1
                edgesresults.differences_dist_tot_shortest[ids_edgeresmap[ids_edge_shortest_nonoverlap]
                                                           ] += dist_matched-dist_shortest

                routesresults_shortest.set_row(id_res,
                                               ids_route=id_route_shortest,
                                               distances=dist_shortest,
                                               durations=dist_shortest/linespeed_matched+timeloss_intersection * n_nodes_shortest + timeloss_tl * n_tls_shortest,
                                               lengths_mixed=np.sum(
                                                   distances[ids_edge_shortest[accesslevels_shortest == 1]]),
                                               lengths_exclusive=np.sum(
                                                   distances[ids_edge_shortest[accesslevels_shortest == 2]]),
                                               lengths_low_priority=np.sum(
                                                   distances[ids_edge_shortest[priorities_shortest <= priority_max_low]]),
                                               numbers_nodes=n_nodes_shortest,
                                               numbers_nodes_tls=n_tls_shortest,
                                               numbers_prioritychange=np.sum(np.logical_not(
                                                   priorities_shortest[:-1], priorities_shortest[1:])),
                                               lengths_overlap_matched=np.sum(distances[ids_edge_overlap]),
                                               )

                accesslevels_nonoverlap = accesslevelsmap[id_mode][ids_edge_match_nonoverlap]
                # print '  ids_edge_match_nonoverlap',ids_edge_match_nonoverlap
                # print '  accesslevels_nonoverlap==1',np.flatnonzero(accesslevels_nonoverlap==1),np.flatnonzero(accesslevels_nonoverlap==1).dtype
                # print '  ids_edge_match_nonoverlap[accesslevels_nonoverlap==1]',ids_edge_match_nonoverlap[np.flatnonzero(accesslevels_nonoverlap==1)]
                priorities_nonoverlap = priorities[ids_edge_match_nonoverlap]
                nodetypes_nonoverlap = nodetypes[ids_tonode[ids_edge_match_nonoverlap]]

                dist_nonoverlap = np.sum(distances[ids_edge_match_nonoverlap])
                n_nodes_nonoverlap = len(nodetypes_nonoverlap)
                n_tls_nonoverlap = np.sum(nodetypes_nonoverlap == tlstype)

                routesresults_matched_nonoverlap.set_row(id_res,
                                                         ids_route=id_route_matched,
                                                         distances=dist_nonoverlap,
                                                         durations=dist_nonoverlap/linespeed_matched+timeloss_intersection * n_nodes_nonoverlap + timeloss_tl * n_tls_nonoverlap,
                                                         lengths_mixed=np.sum(
                                                             distances[ids_edge_match_nonoverlap[accesslevels_nonoverlap == 1]]),
                                                         lengths_exclusive=np.sum(
                                                             distances[ids_edge_match_nonoverlap[accesslevels_nonoverlap == 2]]),
                                                         lengths_low_priority=np.sum(
                                                             distances[ids_edge_match_nonoverlap[priorities_nonoverlap <= priority_max_low]]),
                                                         lengths_overlap_matched=np.sum(
                                                             distances[ids_edge_match_nonoverlap]),
                                                         numbers_nodes=n_nodes_nonoverlap,
                                                         numbers_prioritychange=np.sum(np.logical_not(
                                                             priorities_nonoverlap[:-1], priorities_nonoverlap[1:])),
                                                         numbers_nodes_tls=n_tls_nonoverlap,
                                                         )

                accesslevels_nonoverlap = accesslevelsmap[id_mode][ids_edge_shortest_nonoverlap]
                priorities_nonoverlap = priorities[ids_edge_shortest_nonoverlap]
                nodetypes_nonoverlap = nodetypes[ids_tonode[ids_edge_shortest_nonoverlap]]

                dist_nonoverlap = np.sum(distances[ids_edge_shortest_nonoverlap])
                n_nodes_nonoverlap = len(nodetypes_nonoverlap)
                n_tls_nonoverlap = np.sum(nodetypes_nonoverlap == tlstype)

                routesresults_shortest_nonoverlap.set_row(id_res,
                                                          ids_route=id_route_matched,
                                                          distances=dist_nonoverlap,
                                                          durations=dist_nonoverlap/linespeed_matched+timeloss_intersection * n_nodes_nonoverlap + timeloss_tl * n_tls_nonoverlap,
                                                          lengths_mixed=np.sum(
                                                              distances[ids_edge_shortest_nonoverlap[accesslevels_nonoverlap == 1]]),
                                                          lengths_exclusive=np.sum(
                                                              distances[ids_edge_shortest_nonoverlap[accesslevels_nonoverlap == 2]]),
                                                          lengths_low_priority=np.sum(
                                                              distances[ids_edge_shortest_nonoverlap[priorities_nonoverlap <= priority_max_low]]),
                                                          lengths_overlap_matched=np.sum(
                                                              distances[ids_edge_shortest_nonoverlap]),
                                                          numbers_nodes=n_nodes_nonoverlap,
                                                          numbers_nodes_tls=n_tls_nonoverlap,
                                                          numbers_prioritychange=np.sum(np.logical_not(
                                                              priorities_nonoverlap[:-1], priorities_nonoverlap[1:])),
                                                          )
            # print '  analyzing id_trip',id_trip,'Done.'

        ids_valid = edgesresults.select_ids(edgesresults.durations_tot_matched.get_value() > self.edgedurations_tot_min)

        # do mass edge result operations
        edgesresults.speeds_average[ids_valid] = np.clip(edges.lengths[edgesresults.ids_edge[ids_valid]]/(
            edgesresults.durations_tot_matched[ids_valid]/edgesresults.numbers_tot_matched[ids_valid]), 0.0, self.edgespeed_max)

        edgesresults.probabilities_tot_matched[ids_valid] = edgesresults.numbers_tot_matched[ids_valid] / \
            float(len(ids_valid))
        print '  Route analyses done.'
