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

# @file    netgenerate.py
# @author  Joerg Schweizer
# @date

import subprocess
import os
import sys
import numpy as np
from numpy import random
import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
import agilepy.lib_base.xmlman as xm
#from coremodules.modules_common import *
#from coremodules.network.network import SumoIdsConf, MODES
from agilepy.lib_base.processes import Process, CmlMixin
import netconvert
from agilepy.lib_base.geometry import get_length_polypoints, get_dist_point_to_segs, get_diff_angle_clockwise


class NetGenerateMixin(netconvert.NetConvertMixin):
    def init_common_netgen(self, ident, nettype, net, name, info,
                           netfilepath,
                           logger, **kwargs):

        # nettype
        self.init_common_netconvert(ident, net, netfilepath=None,
                                    name=name,
                                    info=info,
                                    logger=logger,
                                    cml='netgenerate')

        print 'init_common_netgen', nettype

        self.add_option('nettype', '--'+nettype,
                        groupnames=['_private'],
                        cml='',  # value itself acts as switch
                        perm='rw',
                        name='Net type',
                        info='Either "--grid", "--spider" or "--rand" must be supplied.',
                        #is_enabled = lambda self: self.projparams  is not None,
                        )
        self.init_options_edge(**kwargs)
        self.init_options_topology(**kwargs)
        self.init_options_pedestrians(**kwargs)
        self.init_options_nodes(**kwargs)
        self.init_options_tls(**kwargs)


class GridGenerate(NetGenerateMixin):
    def __init__(self, net, netfilepath=None,
                 logger=None, **kwargs):

        self.init_common_netgen('gridgenerate', 'grid',
                                net,
                                'Grid Network generator',
                                'Generates grid-style road network.',
                                netfilepath,
                                logger, **kwargs)

        self.add_option('n_grid_x', kwargs.get('n_grid_x', 4),
                        name='Grid X-number',
                        cml='--grid.x-number',
                        nettypename='Grid',
                        groupnames=['options'],
                        perm='rw',
                        info='For grid networks: number of junctions in X-direction.',
                        )

        self.add_option('n_grid_y', kwargs.get('n_grid_y', 4),
                        name='Grid Y-number',
                        cml='--grid.y-number',
                        nettypename='Grid',
                        groupnames=['options'],
                        perm='rw',
                        info='For  grid networks: number of junctions in Y-direction.',
                        )

        self.add_option('length_grid_x', kwargs.get('length_grid_x', 100.0),
                        name='Grid X-length',
                        cml='--grid.x-length',
                        nettypename='Grid',
                        unit='m',
                        groupnames=['options'],
                        perm='rw',
                        info='For grid networks: lenth of cells in X-direction.',
                        )

        self.add_option('length_grid_y', kwargs.get('length_grid_y', 100.0),
                        name='Grid Y-lenth',
                        cml='--grid.y-length',
                        nettypename='Grid',
                        unit='m',
                        groupnames=['options'],
                        perm='rw',
                        info='For  grid networks: lenth of cells in Y-direction.',
                        )

        self.add_option('length_grid_attach', kwargs.get('length_grid_attach', 0.0),
                        name='Attached length',
                        cml='--grid.attach-length',
                        nettypename='Grid',
                        unit='m',
                        groupnames=['options'],
                        perm='rw',
                        info='For  grid networks: The length of streets attached at the boundary;0 means no streets are attached.',
                        )


class SpiderGenerate(NetGenerateMixin):
    def __init__(self, net, netfilepath=None,
                 logger=None, **kwargs):

        self.init_common_netgen('spidergenerate', 'spider',  # {'Grid':'--grid','Spider':'--spider','Random':'--rand'}
                                net,
                                'Spider Network generator',
                                'Generates spider-style road network.',
                                netfilepath,
                                logger, **kwargs)

        self.add_option('n_arms', kwargs.get('n_arms', 4),
                        name='Spider arms',
                        cml='--spider.arm-number',
                        groupnames=['options'],
                        perm='rw',
                        info='For spider networks: number of arms.',
                        )

        self.add_option('n_circles', kwargs.get('n_circles', 4),
                        name='Spider circles',
                        cml='--spider.circle-number',
                        groupnames=['options'],
                        perm='rw',
                        info='For spider networks: number of circles.',
                        )

        self.add_option('rad_spacing', kwargs.get('rad_spacing', 100.0),
                        name='Spider rad space',
                        cml='--spider.space-radius',
                        groupnames=['options'],
                        perm='rw',
                        info='For spider networks: the distance between the circles.',
                        )

        self.add_option('is_not_center', kwargs.get('is_not_center', False),
                        name='No center',
                        cml='--spider.omit-center',
                        groupnames=['options'],
                        perm='rw',
                        info='For spider networks: omit central node.',
                        )


class RandomGenerate(NetGenerateMixin):
    def __init__(self, net, netfilepath=None,
                 logger=None, **kwargs):

        self.init_common_netgen('randomgenerate', 'rand',  # {'Grid':'--grid','Spider':'--spider','Random':'--rand'}
                                net,
                                'Random Network generator',
                                'Generates road network with random-topology.',
                                netfilepath,
                                logger, **kwargs)

        self.add_option('n_iter_rand', kwargs.get('n_iter_rand', 200),
                        name='Random iterations',
                        cml='--rand.iterations',
                        groupnames=['options'],
                        perm='rw',
                        info='For random networks: how many times an egde is added to the net.',
                        )

        self.add_option('prob_bidir', kwargs.get('prob_bidir', 0.5),
                        name='Bidir. prob.',
                        cml='--rand.bidi-probability',
                        groupnames=['options'],
                        perm='rw',
                        info='For random networks: probability to find a bidirectional edge, values between 0 and 1.',
                        )

        self.add_option('dist_min', kwargs.get('dist_min', 70.0),
                        name='Min. dist',
                        cml='--rand.min-distance',
                        groupnames=['options'],
                        perm='rw',
                        unit='m',
                        info='For random networks: minimum distance between junctions.',
                        )

        self.add_option('dist_max', kwargs.get('dist_max', 200.0),
                        name='Max. dist.',
                        cml='--rand.max-distance',
                        groupnames=['options'],
                        perm='rw',
                        unit='m',
                        info='For random networks: maximum distance between junctions.',
                        )
