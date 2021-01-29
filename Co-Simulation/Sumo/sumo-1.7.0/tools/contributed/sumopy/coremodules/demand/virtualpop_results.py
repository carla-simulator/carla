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

# @file    virtualpop_results.py
# @author  Joerg Schweizer
# @date

import os
import sys
import time
from xml.sax import saxutils, parse, handler
from collections import OrderedDict
import numpy as np
import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
import agilepy.lib_base.xmlman as xm
from coremodules.simulation import results as res


class Vehicleresults(res.Tripresults):
    pass


class Personresults(am.ArrayObjman):
    def __init__(self, ident, parent, persons, edges, datapathkey='tripdatapath',
                 is_add_default=True,
                 name='Person results',
                 info='Table with simulation results for each person.',
                 **kwargs):

        self._init_objman(ident=ident,
                          parent=parent,  # main results object
                          info=info,
                          name=name,
                          **kwargs)

        self.add(cm.AttrConf('datapathkey', datapathkey,
                             groupnames=['_private'],
                             name='data pathkey',
                             info="key of data path",
                             ))

        self.add_col(am.IdsArrayConf('ids_person', persons,
                                     groupnames=['state'],
                                     is_index=True,
                                     name='person ID',
                                     info='ID of person.',
                                     ))
        attrinfos = OrderedDict([
            ('times_travel_total', {'name': 'Total travel time',     'unit': 's',     'default': 0.0,
                                    'info': 'Total travel time, including all trips and waiting times, but excluding activities.', 'groupnames': ['tripdata']}),
            ('times_walk', {'name': 'Walking time',     'unit': 's',     'default': 0.0,
                            'info': 'Time walking, excluding waiting time.', 'groupnames': ['tripdata']}),
            ('times_ride', {'name': 'Riding time',     'unit': 's',     'default': 0.0,
                            'info': 'Time riding on a vehicle, excluding waiting time.', 'groupnames': ['tripdata']}),
            ('times_wait', {'name': 'Waiting time',     'unit': 's',     'default': 0.0,
                            'info': 'Time waiting for a vehicle.', 'groupnames': ['tripdata']}),
            ('times_activity', {'name': 'Activity time',     'unit': 's',     'default': 0.0,
                                'info': 'Time having some activities.', 'groupnames': ['tripdata']}),
            ('times_depart',    {'name': 'Dep. time', 'xmltag': 'depart',   'unit': 's',
                                 'default': 0.0, 'info': 'Time beginning firts trip or activity.', 'groupnames': ['tripdata']}),
            ('times_arrival',   {'name': 'Arr. time', 'xmltag': 'arrival',   'unit': 's',
                                 'default': 0.0, 'info': 'Time ending last trip or activity.', 'groupnames': ['tripdata']}),
        ])

        for attrname, kwargs in attrinfos.iteritems():
            self.add_resultattr(attrname, **kwargs)

        # this is a special for route info
        self.add_col(am.IdlistsArrayConf('ids_edges', edges,
                                         name='Edge IDs',
                                         groupnames=['routeinfo'],
                                         info='List of edge IDs constituting the actually taken route.',
                                         xmltag='edges',
                                         ))

    def get_persons(self):
        return self.ids_person.get_linktab()

    def add_resultattr(self, attrname, **kwargs):

        # default cannot be kwarg
        default = kwargs['default']
        del kwargs['default']
        if 'groupnames' in kwargs:
            kwargs['groupnames'].append('results')
        else:
            kwargs['groupnames'] = ['results']

        self.add_col(am.ArrayConf(attrname, default, **kwargs))

    def import_xml(self, sumo, datapaths):
        datapathkey = self.datapathkey.get_value()
        if datapathkey in datapaths:
            self.import_sumoxml(datapaths[datapathkey], sumo)
            self.get_persons().update_results(self)

    def import_sumoxml(self, filepath, sumo):
        print 'Personresults.import_sumoxml', self.get_persons().ident, filepath

        reader = PersonReader(self.get_persons(), sumo)
        parse(filepath, reader)

        times_walk = np.array(reader.times_walk, dtype=np.int32)
        times_ride = np.array(reader.times_ride, dtype=np.int32)
        times_wait = np.array(reader.times_wait, dtype=np.int32)

        ids = self.add_rows(ids_person=reader.ids_person,
                            times_travel_total=times_walk + times_ride + times_wait,
                            times_walk=times_walk,
                            times_ride=times_ride,
                            times_wait=times_wait,
                            times_depart=reader.times_depart,
                            times_arrival=reader.times_arrival,
                            )
        return ids


class PersonReader(handler.ContentHandler):
    """
    Reads person info from trip output xml file.

    """

    def __init__(self, persons, sumo):
        # print 'PersonReader'
        self._persons = persons
        self._element = 'personinfo'
        self._sumo = sumo
        self.is_person_valid = False
        self.ids_person = []
        self.times_depart = []
        self.times_wait = []
        self.times_ride = []
        self.times_walk = []
        self.times_arrival = []

    def startElement(self, name, attrs):
        # if attrs.has_key('id'):
        #    print '  parse',name,self._element,name == self._element, attrs['id']

        if name == self._element:

            id_elem = self._persons.get_id_from_id_sumo(attrs['id'])
            # print '  ---id_elem',id_elem
            if id_elem == -1:
                return  # id did not fit requested trip type
            time_depart = float(attrs['depart'])
            if time_depart > self._sumo.time_warmup:
                self.is_person_valid = True
                self.ids_person.append(id_elem)
                self._time_laststage = time_depart
                self.times_depart.append(self._time_laststage)
                self._time_wait = 0
                self._time_ride = 0
                self._time_walk = 0
            else:
                self.is_person_valid = False

        elif name == 'walk':
            if self.is_person_valid:
                time_arrival = float(attrs['arrival'])
                self._time_walk += time_arrival - self._time_laststage
                self._time_laststage = time_arrival

        elif name == 'ride':
            if self.is_person_valid:
                time_depart = float(attrs['depart'])
                time_arrival = float(attrs['arrival'])
                self._time_wait += time_depart-self._time_laststage
                self._time_ride += time_arrival - time_depart
                self._time_laststage = time_arrival

    def endElement(self, name):
        if name == self._element:
            if self.is_person_valid:
                self.times_wait.append(self._time_wait)
                self.times_ride.append(self._time_ride)
                self.times_walk.append(self._time_walk)
                self.times_arrival.append(self._time_laststage)
