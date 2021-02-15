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

# @file    results_mpl.py
# @author  Joerg Schweizer
# @date

import os
import numpy as np
from collections import OrderedDict
#import  matplotlib as mpl
#from matplotlib.patches import Arrow,Circle, Wedge, Polygon,FancyArrow
#from matplotlib.collections import PatchCollection
#import matplotlib.colors as colors
#import matplotlib.cm as cmx
#import matplotlib.pyplot as plt
#import matplotlib.image as image

from coremodules.misc.matplottools import *

import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
from agilepy.lib_base.geometry import *
from agilepy.lib_base.processes import Process


class Resultplotter(PlotoptionsMixin, Process):
    def __init__(self, results, name='Plot results with Matplotlib',
                 info="Creates plots of different results using matplotlib",
                 logger=None, **kwargs):

        self._init_common('resultplotter', parent=results, name=name,
                          info=info, logger=logger)

        # print 'Resultplotter.__init__',results,self.parent
        attrsman = self.get_attrsman()

        # edgeresultes....
        attrnames_edgeresults = OrderedDict()
        edgeresultattrconfigs = self.parent.edgeresults.get_group_attrs('results')
        edgeresultattrnames = edgeresultattrconfigs.keys()
        # edgeresultattrnames.sort()
        for attrname in edgeresultattrnames:
            attrconfig = edgeresultattrconfigs[attrname]

            attrnames_edgeresults[attrconfig.format_symbol()] = attrconfig.attrname

        #attrnames_edgeresults = {'Entered':'entered'}
        self.edgeattrname = attrsman.add(cm.AttrConf('edgeattrname', 'entered',
                                                     choices=attrnames_edgeresults,
                                                     groupnames=['options'],
                                                     name='Edge Quantity',
                                                     info='The edge related quantity to be plotted.',
                                                     ))

        self.add_plotoptions(**kwargs)

    def show(self):
        # print 'show',self.edgeattrname
        # if self.axis  is None:
        axis = init_plot()
        if (self.edgeattrname is not ""):
            resultattrconf = getattr(self.parent.edgeresults, self.edgeattrname)
            ids = self.parent.edgeresults.get_ids()
            title = resultattrconf.get_info()  # +resultattrconf.format_unit(show_parentesis=True)#format_symbol()
            self.plot_results_on_map(axis, ids, resultattrconf[ids], title, valuelabel=resultattrconf.format_symbol())

        show_plot()

    def do(self):
        # print 'do',self.edgeattrname
        self.show()

    def get_scenario(self):
        return self._scenario
