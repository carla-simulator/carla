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

# @file    mapmatching-01-brokesave.py
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
#from agilepy.lib_base.geometry import find_area
from agilepy.lib_base.processes import Process  # ,CmlMixin,ff,call
from coremodules.network.network import SumoIdsConf
from coremodules.network import routing
from coremodules.demand.demand import Trips

try:
    try:
        import pyproj
    except:
        from mpl_toolkits.basemap import pyproj
    #from shapely.geometry import Polygon, MultiPolygon, MultiLineString, Point, LineString, MultiPoint, asLineString, asMultiPoint
    #from shapely.ops import cascaded_union
except:
    print 'Import error: in order to run the traces plugin please install the following modules:'
    print '   mpl_toolkits.basemap'
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


def load_mapmatching(filepath, demand, logger=None):
    # typically parent is the scenario
    mapmatching = cm.load_obj(filepath, parent=demand)
    if logger is not None:
        mapmatching.set_logger(logger)
    return mapmatching

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
                 is_float=True):
    """
    Returns time in seconds after 1/1/1970.
    Time format for time data string used:
        2012-05-02 12:57:08.0
    """

    if len(t_data.split(sep_date_clock)) != 2:
        return None
    (date, clock) = t_data.split(sep_date_clock)

    if (len(clock.split(sep_clock)) == 3) & (len(date.split(sep_date)) == 3):
        (day_str, month_str, year_str) = date.split(sep_date)
        (hours_str, minutes_str, seconds_str) = clock.split(sep_clock)
        t = time.mktime((int(year_str), int(month_str), int(day_str),
                         int(hours_str), int(minutes_str), int(float(seconds_str)), 0, 0, 0))

        if is_float:
            return float(t)
        else:
            return int(t)
    else:
        return None


def get_colvalue(val, default=0.0):

    if (len(val) > 0) & (val != 'NULL'):
        return float(val)
    else:
        return default


class EccTracesImporter(Process):
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
                   & (duration > self.duration_trip_min)\
                   & (speed_av > self.speed_trip_min)\
                   & (speed_av < self.speed_trip_max):
                    # print  'parametric conditions verified'
                    timestamp = calc_seconds(cols[j_time])
                    if timestamp is not None:
                        # print '  valid time stamp',timestamp
                        id_trip = trips.make(id_sumo=cols[j_id_trip],
                                             timestamp=timestamp,
                                             distance_gps=dist,
                                             duration_gps=duration,
                                             speed_average=speed_av,
                                             )
            #i_line +=1

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

        is_valid_trip = False

        for line in f.readlines():

            cols = line.split(sep)
            # print '    len(cols)',len(cols),n_cols
            if len(cols) == n_cols:
                id_trip_sumo_current = cols[ind_id_path]
                # print '    id_trip_sumo_current,id_trip_sumo',id_trip_sumo_current,id_trip_sumo,is_valid_trip
                if id_trip_sumo_current != id_trip_sumo:
                    # this point is part of new trip

                    if is_valid_trip:
                        # store past points for valid trip
                        ids_point = points.add_rows(
                            timestamps=timestamps,
                            #ids_trip = ids_trip,
                            longitudes=longitudes,
                            latitudes=latitudes,
                            altitudes=altitudes,
                        )

                        trips.set_points(id_trip, ids_point)

                    # check if new trip is valid
                    if exist_id_trip_sumo(id_trip_sumo_current):
                        # print '    trip',id_trip_sumo_current,' exisits'
                        is_valid_trip = True  # start recording
                        id_trip = get_id_trip(id_trip_sumo_current)

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
                #ids_trip = ids_trip,
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

        self.add_col(am.ArrayConf('timestamps', default=0,
                                  dtype=np.int,
                                  groupnames=['parameters', 'gps'],
                                  name='timestamp',
                                  unit='s',
                                  info='Timestamp when trip started in seconds after 01 January 1970.',
                                  ))

        self.add_col(am.ArrayConf('durations_gps', default=0.0,
                                  dtype=np.int,
                                  groupnames=['parameters', 'gps'],
                                  name='GPS duration',
                                  unit='s',
                                  info='Time duration measure with GPS points.',
                                  ))

        self.add_col(am.ArrayConf('distances_gps', default=0.0,
                                  dtype=np.int,
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

        Trips._init_attributes(self)
        self.add_col(am.IdsArrayConf('ids_route_mapped', self.routes.get_value(),
                                     name='ID mapped route',
                                     info='Route ID of mapped route.',
                                     ))

        self.add_col(am.IdsArrayConf('ids_route_shortest', self.routes.get_value(),
                                     name='ID shortest route',
                                     info='Route ID of shortest route.',
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

        self.add_col(am.IdlistsArrayConf('ids_points', self.parent.points,
                                         #groupnames = ['_private'],
                                         name='Point IDs',
                                         info="GPS point IDs.",
                                         ))

    def get_obj_vtypes(self):
        return self.parent.parent.vtypes

    def make(self,  **kwargs):
        # if self.ids_sumo.has_index(id_sumo):
        #    id_trip = self.ids_sumo.get_id_from_index(id_sumo)
        #    #self.set_row(id_sumo, **kwargs)
        #    return id_trip
        # else:
        id_trip = self.add_row(ids_sumo=kwargs.get('id_sumo', None),
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

    def get_scenario(self):
        return self.parent.get_scenario()

    def _init_attributes(self):
        scenario = self.get_scenario()

        # ident is the path id of the trace

        # the actual numpy arrays are stored in .cols
        self.add_col(am.ArrayConf('longitudes',     default=0.0,
                                  dtype=np.float,
                                  groupnames=['parameters', ],
                                  perm='rw',
                                  name='Longitude',
                                  symbol='Lon',
                                  unit='deg',
                                  info='Longitude  of point',
                                  ))

        self.add_col(am.ArrayConf('latitudes',   default=0.0,
                                  groupnames=['parameters', ],
                                  dtype=np.float,
                                  perm='rw',
                                  name='Latitude',
                                  symbol='Lat',
                                  unit='deg',
                                  info='Latitude of point',
                                  ))

        self.add_col(am.ArrayConf('altitudes',   default=-100000.0,
                                  groupnames=['parameters', ],
                                  dtype=np.float,
                                  perm='rw',
                                  name='Altitude',
                                  symbol='Alt',
                                  unit='m',
                                  info='Altitude of point',
                                  ))

        self.add_col(am.ArrayConf('coords',    default=[0.0, 0.0, 0.0],
                                  groupnames=['parameters', ],
                                  dtype=np.float,
                                  perm='rw',
                                  name='Coordinate',
                                  symbol='x,y,z',
                                  unit='m',
                                  info='Local 3D coordinate  of point',
                                  ))

        self.add_col(am.ArrayConf('timestamps',  default=0,
                                  groupnames=['parameters', ],
                                  perm='rw',
                                  name='timestamp',
                                  symbol='t',
                                  unit='s',
                                  digits_fraction=2,
                                  info='Time stamp of point in seconds after 01 January 1970.',
                                  ))

        self.add_col(am.ArrayConf('are_inside_boundary',  default=False,
                                  groupnames=['parameters', ],
                                  perm='r',
                                  name='in boundary',
                                  info='True if this the data point is within the boundaries of the road network.',
                                  ))

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

        self.add_col(am.ArrayConf('years_birth', default='',
                                  dtype=np.object,
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

    def _init_attributes(self):
        print 'Mapmatching._init_attributes'
        attrsman = self.get_attrsman()

        self.points = attrsman.add(cm.ObjConf(GpsPoints('points', self)))
        self.trips = attrsman.add(cm.ObjConf(GpsTrips('trips', self)))
        self.persons = attrsman.add(cm.ObjConf(GpsPersons('persons', self)))

    def _init_constants(self):
        self._proj = None

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

    def save(self, filepath=None, is_not_save_parent=True):
        if filepath is None:
            self.get_scenario().get_rootfilepath()+'.mmatch.obj'
        cm.save_obj(self, filepath, is_not_save_parent=is_not_save_parent)

    def get_offset(self):
        return self.get_scenario().net.get_offset()


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
                    id_trace = '0'
                else:
                    id_trace = str(np.max(self._traces.get_ids())+1)
                print 'startElement id_trace', id_trace, self._traces.get_ids(), self._traces
                self._pointset = PointSet(id_trace, self._traces)
                self._traces.set_row(id_trace, pointsets=self._pointset)
                self._ids_traces.append(id_trace)

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
