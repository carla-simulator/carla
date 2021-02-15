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

# @file    demandbase.py
# @author  Joerg Schweizer
# @date


import numpy as np
import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
import agilepy.lib_base.xmlman as xm
from agilepy.lib_base.misc import random_choice, get_inversemap

OPTIONMAP_POS_DEPARTURE = {"random": -1, "free": -2,
                           "random_free": -3, "base": -4, "last": -5, "first": -6}
OPTIONMAP_POS_ARRIVAL = {"random": -1, "max": -2}
OPTIONMAP_SPEED_DEPARTURE = {"random": -1, "max": -2}
OPTIONMAP_SPEED_ARRIVAL = {"current": -1}
OPTIONMAP_LANE_DEPART = {"random": -1, "free": 2,
                         "allowed": -3, "best": -4, "first": -5}
OPTIONMAP_LANE_ARRIVAL = {"current": -1}


class ModeShares(am.ArrayObjman):
    """
    Utility table with some default mode shares.
    """

    def __init__(self, ident, parent, modes, **kwargs):

        self._init_objman(ident, parent=parent, name='Mode shares',
                          version=0.0,
                          **kwargs)

        self.add_col(am.IdsArrayConf('ids_mode', modes,
                                     groupnames=['parameters'],
                                     name='Mode ID',
                                     info='Transport Mode ID.',
                                     ))

        self.add_col(am.ArrayConf('shares', '',
                                  dtype=np.float32,
                                  is_index=True,
                                  groupnames=['parameters'],
                                  perm='rw',
                                  name='Share',
                                  info='Mode share.',
                                  ))

        # self.add_col(am.ArrayConf( 'speeds_max', 50.0/3.6,
        #                            dtype = np.float32,
        #                            groupnames = ['parameters'],
        #                            perm='rw',
        #                            name = 'Max. Speed',
        #                            unit = 'm/s',
        #                            info = 'Maximum possible speed for this mode. Speed is used to estimate free flow link travel times, mainly for routig purposes. Note that speeds are usully limited by the lane speed attribute',
        #                            ))
        self._init_attributes()
        self.add_default()

    def _init_attributes(self, landuse=None):
        # self.add_col(SumoIdsConf('Activitytypes'))
        pass

    def add_share(self, mode, share):
        modes = self.ids_mode.get_linktab()
        return self.add_row(ids_mode=modes.get_id_from_formatted(mode),
                            shares=share)

    def add_default(self):
        """
        Sets the default maximum possible speed for certain modes.
        """
        self.add_share("pedestrian", 0.1)
        self.add_share("bicycle", 0.1)
        self.add_share("motorcycle", 0.1)
        self.add_share("passenger", 0.5)
        self.add_share("bus", 0.2)

    def get_modes_random(self, n):
        """
        Return a vector with mode IDs of length n.
        """
        ids = self.get_ids()
        ids_modes_all = self.ids_mode[ids]
        return ids_modes_all[random_choice(n, self.shares[ids])]


class ActivityTypes(am.ArrayObjman):
    # http://www.sumo.dlr.de/userdoc/Networks/Building_Networks_from_own_XML-descriptions.html#Edge_Descriptions
    def __init__(self, ident, demand, **kwargs):

        self._init_objman(ident, parent=demand, name='Activity Types',
                          version=0.0,
                          xmltag=('actTypes', 'actType', 'names'),
                          **kwargs)

        self._init_attributes()
        self.add_default()

    def _init_attributes(self, landuse=None):
        # self.add_col(SumoIdsConf('Activitytypes'))

        self.add_col(am.ArrayConf('names', '',
                                  dtype=np.object,
                                  is_index=True,
                                  groupnames=['parameters'],
                                  perm='rw',
                                  name='Type name',
                                  info='Human readable name of activity type.',
                                  ))

        self.add_col(am.ArrayConf('symbols', '',
                                  dtype=np.object,
                                  perm='rw',
                                  is_index=True,
                                  name='Type symbol',
                                  info='Symbol of activity type name. Used to represent activity sequences.',
                                  ))

        self.add_col(am.ArrayConf('descriptions', '',
                                  dtype=np.object,
                                  perm='rw',
                                  name='Description',
                                  info='Description of activity.',
                                  ))

        # this works only for first init
        # if landuse is not None:
        self.add_col(am.IdlistsArrayConf('ids_landusetypes', self.parent.get_scenario().landuse.landusetypes,
                                         name='Landuse types',
                                         info="Landuse type IDs, eher this activity type can take place.",
                                         ))

        self.add_col(am.ArrayConf('hours_begin_earliest', 0.0,
                                  dtype=np.float32,
                                  groupnames=['parameters'],
                                  perm='rw',
                                  name='Earliest hour begin',
                                  unit='h',
                                  info='Default value for earliest hour when this activity can begin.',
                                  ))

        self.add_col(am.ArrayConf('hours_begin_latest', 1.0,
                                  dtype=np.float32,
                                  groupnames=['parameters'],
                                  perm='rw',
                                  name='Latest begin hour',
                                  unit='h',
                                  info='Default value for latest hour when this activity can begin.',
                                  ))

        self.add_col(am.ArrayConf('durations_min', 6.0,
                                  dtype=np.float32,
                                  groupnames=['parameters'],
                                  perm='rw',
                                  name='Min. Duration',
                                  unit='h',
                                  info='Default value for minimum activity duration for a person within a day.',
                                  ))

        self.add_col(am.ArrayConf('durations_max', 8.0,
                                  dtype=np.float32,
                                  groupnames=['parameters'],
                                  perm='rw',
                                  name='Max. Duration',
                                  unit='h',
                                  info='Default value for maximum activity duration for a person within a day.',
                                  ))

    def format_ids(self, ids):
        return ', '.join(self.names[ids])

    def get_id_from_formatted(self, idstr):
        return self.names.get_id_from_index(idstr)

    def get_ids_from_formatted(self, idstrs):
        return self.names.get_ids_from_indices_save(idstrs.split(','))

    def get_id_from_name(self, activitytypename):
        return self.names.get_id_from_index(activitytypename)

    def get_id_from_symbol(self, activitytypesymbol):
        return self.symbols.get_id_from_index(activitytypesymbol)

    def add_default(self):
        """
        Sets the default maximum possible speed for certain modes.
        """
        landusetypekeys = self.parent.get_scenario().landuse.landusetypes.typekeys
        self.add_row(names='none',
                     descriptions='None activity type. Will be skipped when planning.',
                     ids_landusetypes=landusetypekeys.get_ids_from_indices([]),
                     symbols='n',
                     hours_begin_earliest=0.0,
                     hours_begin_latest=0.0,
                     durations_min=0.0,
                     durations_max=0.0,
                     )

        self.add_row(names='home',
                     descriptions='General home activity, like sleeping, eating, watching TV, etc.',
                     ids_landusetypes=landusetypekeys.get_ids_from_indices(
                         ['residential', 'mixed']),
                     symbols='h',
                     hours_begin_earliest=-1.0,
                     hours_begin_latest=-1.0,
                     durations_min=7.0,
                     durations_max=8.0,
                     )

        self.add_row(names='work',
                     descriptions="""Work activity, for example work in
                     industry, offices or as employee at
                     educational facilities.""",
                     ids_landusetypes=landusetypekeys.get_ids_from_indices(
                         ['industrial', 'commercial', 'education', 'mixed']),
                     symbols='w',
                     hours_begin_earliest=8.5,
                     hours_begin_latest=9.0,
                     durations_min=6.0,
                     durations_max=9.0,
                     )

        self.add_row(names='education',
                     descriptions='Education activity, for example visiting courses at schools or at universities.',
                     ids_landusetypes=landusetypekeys.get_ids_from_indices(
                         ['education', ]),
                     symbols='e',
                     hours_begin_earliest=8.0,
                     hours_begin_latest=10.0,
                     durations_min=4.0,
                     durations_max=6.0,
                     )

        self.add_row(names='shopping',
                     descriptions='Shopping activity',
                     ids_landusetypes=landusetypekeys.get_ids_from_indices(
                         ['commercial', 'mixed']),
                     symbols='s',
                     hours_begin_earliest=16.0,
                     hours_begin_latest=19.0,
                     durations_min=0.2,
                     durations_max=2.0,
                     )

        self.add_row(names='leisure',
                     descriptions='Leisure activity',
                     ids_landusetypes=landusetypekeys.get_ids_from_indices(
                         ['leisure', 'mixed']),
                     symbols='l',
                     hours_begin_earliest=12.0,
                     hours_begin_latest=15.0,
                     durations_min=1.0,
                     durations_max=3.0,
                     )


class DemandobjMixin:
    def export_trips_xml(self, filepath=None, encoding='UTF-8',
                         ids_vtype_exclude=[]):
        """
        Export trips to SUMO xml file.
        Method takes care of sorting trips by departure time.
        """
        return False

    def get_writexmlinfo(self, is_route=False):
        """
        Returns three array where the first array is the
        begin time of the first vehicle and the second array is the
        write function to be called for the respectice vehicle and
        the third array contains the vehicle ids

        Method used to sort trips when exporting to route or trip xml file
        """
        return [], [], []

    def config_results(self, results):
        # tripresults = res.Tripresults(          'tripresults', results,
        #                                        self,
        #                                        self.get_net().edges
        #                                        )
        #
        #
        #results.add_resultobj(tripresults, groupnames = ['Trip results'])
        pass

    def process_results(self, results, process=None):
        pass

    def get_time_depart_first(self):
        return np.inf

    def get_time_depart_last(self):
        return 0.0
